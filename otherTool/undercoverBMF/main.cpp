#include <iostream>
#include <cassert>
#include <csignal>
#include <zlib.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <set>
#include <map>
#include <tuple>
#include <cassert>
#include <memory>
#include <deque>

#include "EvalMaxSAT.h"
#include "lib/CLI11.hpp"
#include "coutUtil.h"
#include "Chrono.h"
#include "matrixloarder.h"
#include "Moy.h"
#include "rand.h"


using namespace MaLib;
using namespace std;


//// GLOBAL /////
std::string GLOBAL_externalSolver = "";
bool GLOBAL_unactivateCardGen = 0;
bool GLOBAL_noBreakSym = 0;
unsigned int verbosity = 1;
///


std::vector<std::vector<bool>> mult(const std::vector<std::vector<bool>> &M1, const std::vector<std::vector<bool>> &M2) {
    if(M2.size()==0) {
        return {};
    }
    assert(M2.size() > 0);
    assert(M1.size() > 0);
    assert(M1[0].size() == M2.size());
    unsigned int kM = M1[0].size();
    std::vector<std::vector<bool>> M(M1.size(), std::vector<bool>(M2[0].size()));

    for(unsigned int i=0; i<M.size(); i++) {
        for(unsigned int j=0; j<M[i].size(); j++) {

            bool res = false;
            for(unsigned int k=0; k<kM; k++) {
                assert(M1.size() > i);
                assert(M2.size() > k);
                assert(M1[i].size() > k);
                assert(M2[k].size() > j);

                if(M1[i][k] && M2[k][j]) {
                    res=true;
                    break;
                }
            }

            M[i][j] = res;
        }
    }

    return M;
}


MaMatriceADJ multAdj(const std::vector<std::vector<bool>> &M1, const std::vector<std::vector<bool>> &M2) {
    if(M2.size()==0) {
        return {};
    }
    assert(M2.size() > 0);
    assert(M1.size() > 0);
    assert(M1[0].size() == M2.size());
    unsigned int kM = M1[0].size();
    MaMatriceADJ M;
    M.resize(M1.size(), M2[0].size());

    for(unsigned int i=0; i<M.m(); i++) {
        for(unsigned int j=0; j<M.n(); j++) {
            bool res = false;
            for(unsigned int k=0; k<kM; k++) {
                assert(M1.size() > i);
                assert(M2.size() > k);
                assert(M1[i].size() > k);
                assert(M2[k].size() > j);

                if(M1[i][k] && M2[k][j]) {
                    res=true;
                    break;
                }
            }

            M.add(i, j, res);
        }
    }

    return M;
}

std::string exec(std::string cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

class ExternalSolver : public VirtualMAXSAT {
    std::string cmd;

    std::vector< std::tuple< unsigned int, std::vector<int> > > clauses;
    std::vector< int > isSoftVar; // vars[i] = 1  => soft var    var[i] = 0 => hard var
    unsigned int nbSoft=0;

public:
    ExternalSolver(std::string cmd) : cmd(cmd) {
        this->cmd.append(" /tmp/formula3.wcnf");
        isSoftVar.push_back(0); // Fake var to not use vars[0]
    }

    virtual bool isSoft(unsigned int var) {
        assert(!"No need");
        return true;
    }

    virtual void setVarSoft(unsigned int var, bool value, t_weight weight=1) {
        assert(value == true);
        assert(isSoftVar[var] == 0);
        if(value)
            isSoftVar[var] = weight;
        else
            isSoftVar[var] = -weight;
        nbSoft++;
    }

    virtual void setNInputVars(unsigned int nb) {
        assert(!"No need");
    }

    int newVar(bool decisionVar=true) {
        isSoftVar.push_back(0);
        return isSoftVar.size()-1;
    }

    virtual unsigned int nVars() {
        return isSoftVar.size()-1;
    }

    virtual int newSoftVar(bool value, bool decisionVar, t_weight weight=1) {
        if(value)
            isSoftVar.push_back(weight);
        else
            isSoftVar.push_back(-weight);
        nbSoft++;

        return isSoftVar.size()-1;
    }

    void atMostKOnSoftVar(const std::vector<int> &clause, unsigned int k) {
        unsigned int w = std::numeric_limits<unsigned int>::max();

        for(auto lit: clause) {
            unsigned int var = abs(lit);
            assert(lit != 0);
            if(lit > 0) {
                assert(isSoftVar[var] >= 1);
            } else {
                assert(isSoftVar[var] <= -1);
            }

            if(abs(isSoftVar[var]) < w) {
                w = abs(isSoftVar[var]);
            }
        }

        assert(w < std::numeric_limits<unsigned int>::max());

        for(auto lit: clause) {
            unsigned int var = abs(lit);

            assert(lit != 0);
            if(lit > 0) {
                assert(isSoftVar[var] >= 1);
                isSoftVar[var] = isSoftVar[var] - w;
                if(isSoftVar[var]==0) {
                    nbSoft--;
                }
            } else {
                assert(isSoftVar[var] <= -1);
                isSoftVar[var] = isSoftVar[var] + w;
                if(isSoftVar[var]==0) {
                    nbSoft--;
                }
            }
        }

        cost += w * (clause.size()-k);
        std::vector<int> L;
        for(auto lit: clause)
            L.push_back(-lit);
        auto card = newCard(L);

        assert(k<clause.size());
        unsigned int kp = clause.size()-k;
        for(unsigned int i = kp; i<L.size(); i++) {
            addClause({(*card) <= i}, w);
        }

    }

    void exportFormula(std::string vFile) {
        unsigned int tmp=0;
        std::ofstream file( vFile );
        file << "p wcnf "<<isSoftVar.size()<<" "<<(clauses.size()+nbSoft)<<" " << (unsigned int)(-1) << std::endl;
        for(unsigned int var=1; var<isSoftVar.size(); var++) {
            if(isSoftVar[var] != 0) {
                if(isSoftVar[var] > 0) {
                    file << isSoftVar[var] << " " << var << " 0" << std::endl;
                } else {
                    file << -(isSoftVar[var]) << " " << var << " 0" << std::endl;
                }
                tmp++;
            }
        }
        for(auto &line: clauses) {
            file << std::get<0>(line) << " ";
            for(auto &l: std::get<1>(line)) {
                file << l << " ";
            }
            file << 0 << std::endl;
        }
        assert(tmp == nbSoft);
    }

    t_weight cost=0;
    std::vector<bool> values;

    virtual bool solve(int valMin=std::numeric_limits<int>::max()) {

        unsigned int costSolver = std::numeric_limits<unsigned int>::max();
        values.clear();

        exportFormula("/tmp/formula3.wcnf");

        std::string result = exec(cmd);
        std::istringstream issResult(result);

        std::string line;

        std::string res;
        while (std::getline(issResult, line))
        {
            std::istringstream iss(line);
            char action;
            if(!(iss >> action)) {
                continue;
            }
            switch (action) {

            case 'c':
            {
                break;
            }

            case 'o':
            {
                if(!(iss >> costSolver)) {
                    assert(false);
                }
                break;
            }

            case 's':
            {
                if(!(iss >> res)) {
                    assert(false);
                }
                if( (res.compare("OPTIMUM") != 0) && (res.compare("SATISFIABLE") != 0) ) {
                    return false;
                }
                break;
            }

            case 'v':
            {
                if(!(iss >> res)) {
                    assert(false);
                }
                values.push_back(0); // fake lit

                int res2;
                if(!(iss >> res2)) {
                    if(res.compare("-1") ==  0) {
                        res = "0"; // special case of a single variable
                    }

                    // New format
                    for(unsigned int i=0; i<res.size(); i++) {
                        values.push_back(res[i] == '1');
                    }
                } else {
                    // Old format
                    int lit = std::atoi(res.c_str());
                    values.push_back(lit > 0);
                    assert((values.size()-1) == abs(lit));

                    values.push_back(res2 > 0);

                    while(iss>>lit) {
                        values.push_back(lit > 0);
                        assert((values.size()-1) == abs(lit));
                    }
                }

                break;
            }

            default:
                assert(false);
            }

        }

        assert(costSolver != std::numeric_limits<unsigned int>::max());
        assert(values.size());

        cost += costSolver;

        return true;
    }

    bool getValue(unsigned int var) {
        assert(values.size() > var);
        return values[var];
    }

    void addClause(const std::vector<int> &clause, unsigned int weight=-1) {
        clauses.push_back( {weight, clause} );
    }

    virtual void addClause(const std::vector<int> &vclause)  {
        clauses.push_back( {-1, vclause} );
    }

    t_weight getCost() {
        return cost;
    }

};


class Infering {
    VirtualMAXSAT *solver;

    class VarSAT {
        static VirtualMAXSAT* solver;
     public:
        int id;
        VarSAT() {
            id = solver->newVar();
        }

        static void init(VirtualMAXSAT* solver) {
            VarSAT::solver = solver;
        }

        static int newVar() {
            return solver->newVar();
        }

        friend class Infering;
    };

    class VarAux {
        static VirtualMAXSAT* solver;
     public:
        int id;
        VarAux() {
            id = solver->newVar(false);
        }

        static void init(VirtualMAXSAT* solver) {
            VarAux::solver = solver;
        }

        static int newVar() {
            return solver->newVar(false);
        }

        friend class Infering;
    };


    std::vector<std::vector<VarSAT>> M1;
    int getVarM1(unsigned int i, unsigned int j) {
        assert(i < M1.size());
        assert(j < M1[i].size());
        if(i >= M1.size())
            M1.resize(i+1);

        if(j >= M1[i].size())
            M1[i].resize(j+1);

        return M1[i][j].id;
    }

    std::vector<std::vector<VarSAT>> M2;
    int getVarM2(unsigned int i, unsigned int j) {

        assert(i < M2.size());
        assert(j < M2[i].size());

        if(i >= M2.size())
            M2.resize(i+1);

        if(j >= M2[i].size())
            M2[i].resize(j+1);

        return M2[i][j].id;
    }

    std::vector<std::vector<VarSAT>> Z;
    int getVarZ(unsigned int i, unsigned int j) {
        if(i >= Z.size())
            Z.resize(i+1);

        if(j >= Z[i].size())
            Z[i].resize(j+1);

        return Z[i][j].id;
    }

    bool toUpdate=true;
    bool lastResult=false;

    unsigned int k;


    std::vector<std::vector<std::optional<int>>> Mlit;
    MaMatriceADJ Mone;

public:

    Infering(unsigned int nbLine, unsigned int nbCol, unsigned int k)
        : k(k) {

        if(GLOBAL_externalSolver.size()) {
            solver = new ExternalSolver(GLOBAL_externalSolver);
        } else {
            solver = new EvalMaxSAT(1);
        }

        VarSAT::solver = solver;
        VarAux::solver = solver;

        Mlit.resize(nbLine, std::vector<std::optional<int>>(nbCol));

        M1.resize(nbLine);
        for(unsigned int i=0; i<nbLine; i++) {
            M1[i].resize(k);
        }
        M2.resize(k);
        for(unsigned int x=0; x<k; x++) {
            M2[x].resize(nbCol);
        }

        if(!GLOBAL_noBreakSym) {
            for(unsigned int i=0; i<M2.size(); i++) {
                // Initialization of Z
                // Z[i][0]
                solver->addClause({getVarZ(i, 0)});

            }

            for(unsigned int i=0; i<M2.size()-1; i++) {
                for(unsigned int j=0; j<M2[i].size()-1; j++) {
                    // Propagate Z true

                    // Z[i][j] ^ M2[i][j] ^ M2[i+1][j] => Z[i][j+1]

                    solver->addClause({
                                        -getVarZ(i,j),
                                        -getVarM2(i,j),
                                        -getVarM2(i+1,j),
                                        getVarZ(i,j+1)
                                    });


                    // Z[i][j] ^ !M2[i][j] ^ !M2[i+1][j] => Z[i][j+1]

                    solver->addClause({
                                        -getVarZ(i,j),
                                        getVarM2(i,j),
                                        getVarM2(i+1,j),
                                        getVarZ(i,j+1)
                                    });
                }
            }

            for(unsigned int i=0; i<M2.size()-1; i++) {
                for(unsigned int j=0; j<M2[i].size(); j++) {
                    // If equal so far then M2[i][j] must be greater than or equal to M2[i+1][j]
                    // Z[i][j] ^ !M2[i][j] => !M2[i+1][j]

                    solver->addClause({
                                        -getVarZ(i,j),
                                        getVarM2(i,j),
                                        -getVarM2(i+1,j)
                                    });
                }
            }
        }
    }

    void setToUpdate() {
        toUpdate = true;
    }

    ~Infering() {
        delete solver;
    }

    void genCard() {
        int nbAtMostK=0;
        auto Mbis = Mone;

        unsigned int nbGroupe=0;
        for(;;nbGroupe++) {
            std::vector< std::vector< std::tuple<unsigned int, unsigned int> > > sortedMap; // It's faster to rebuild every time than to update...
            for(auto [i,j]: Mbis.Ones()) {
                unsigned int size = (Mbis.OneOnLine(i).size()-1)*(Mbis.OneOnCol(j).size()-1);

                if(sortedMap.size() <= size) {
                    sortedMap.resize( size + 1 );
                }
                sortedMap[size].push_back({i, j});
            }

            std::vector< std::tuple<unsigned int, unsigned int> > selected;
            {
                for(int size = sortedMap.size()-1; size >= 0; size--)
                for(auto [i, j]: sortedMap[size]) {
                    bool toAdd = true;
                    for(auto [i2, j2]: selected) {
                        if( (Mlit[i][j2].value_or(1) >= 1) && (Mlit[i2][j].value_or(1) >= 1) ) {
                            toAdd = false;
                            break;
                        }
                    }
                    if(toAdd) {
                        selected.push_back({i, j});
                    }
                }
            }

            if(selected.size() == 0/*1*//*0*/) { // TODO: 0 or 1 ? 1 is faster, but we can miss some card
                break;
            }

            if(selected.size() <= k) {
                Mbis.remove(std::get<0>(selected[0]), std::get<1>(selected[0]));
                continue;
            }

            std::vector<int> clause;
            {
                for(unsigned int id=0; id<selected.size(); id++) {
                    auto [i, j] = selected[id];

                    assert( Mbis.get(i, j).value_or(0) != 0 );
                    assert( Mlit[i][j].value_or(0) >= 1 );
                    clause.push_back(Mlit[i][j].value());
                    Mbis.remove(i, j);
                }
            }

            if(clause.size() > k) {
                solver->atMostKOnSoftVar(clause, k);
                nbAtMostK++;
            }
        }
    }

    bool inferModel() {
        if(!toUpdate)
            return lastResult;
        toUpdate = false;

        if(!GLOBAL_unactivateCardGen) {
            genCard();
        }

        lastResult = solver->solve();

        return lastResult;
    }

    unsigned int getCost() {
        return solver->getCost();
    }

    std::tuple<std::vector<std::vector<bool>>, std::vector<std::vector<bool>>> getModel() {
        assert(!toUpdate);
        assert(lastResult);
        std::tuple<std::vector<std::vector<bool>>, std::vector<std::vector<bool>>> result;
        std::vector<std::vector<bool>> res1 (M1.size(), std::vector<bool>(M1[0].size()));
        std::vector<std::vector<bool>> res2 (M2.size(), std::vector<bool>(M2[0].size()));

        for(unsigned int i=0; i<M1.size(); i++) {
            for(unsigned int k=0; k<M1[i].size(); k++) {
                if(solver->getValue(getVarM1(i,k)))
                    res1[i][k] = true;
            }
        }

        for(unsigned int k=0; k<M2.size(); k++) {
            for(unsigned int j=0; j<M2[k].size(); j++) {
                if(solver->getValue(getVarM2(k,j)))
                    res2[k][j] = true;
            }
        }

        return {res1, res2};
    }

    bool getValueLine(unsigned int i) {
        assert(k==1);
        return solver->getValue( getVarM1(i, 0) );
    }

    bool getValueCol(unsigned int j) {
        assert(k==1);
        return solver->getValue( getVarM2(0, j) );
    }

    void forceOne(unsigned int i, unsigned int j) {
        if(k > 1) {
            std::vector<VarSAT> TMP(k);
            std::vector<int> clause;
            for(auto &V: TMP) {
                clause.push_back( V.id );
            }
            solver->addClause(clause);

            for(unsigned int x=0; x<k; x++) {
                solver->addClause( {getVarM1(i, x), -TMP[x].id} );
                solver->addClause( {getVarM2(x, j), -TMP[x].id} );
            }
        } else {
            solver->addClause( {getVarM1(i, 0)} );
            solver->addClause( {getVarM2(0, j)} );
        }

    }

    void exportFormula(std::string vFile) {

        if(!GLOBAL_unactivateCardGen) {
            genCard();
        }
        solver->exportFormula(vFile);
    }

    void forceSolution(const std::vector< std::vector< bool > > &A, const std::vector< std::vector< bool > > &B) {
        assert(A.size() == M1.size());
        for(unsigned int i=0; i<A.size(); i++) {
            assert(M1[i].size() == A[i].size());
            for(unsigned int k=0; k<A[i].size(); k++) {
                if(A[i][k]) {
                    solver->addClause( {getVarM1(i, k)} );
                } else {
                    solver->addClause( {-getVarM1(i, k)} );
                }
            }
        }

        assert(B.size() == M2.size());
        for(unsigned int k=0; k<M2.size(); k++) {
            assert(M2[k].size() == B[k].size());
            for(unsigned int j=0; j<B[k].size(); j++) {
                if(B[k][j]) {
                    solver->addClause( {getVarM2(k, j)} );
                } else {
                    solver->addClause( {-getVarM2(k, j)} );
                }
            }
        }

    }

    int nbOne=0;
    void add(unsigned int i, unsigned int j, bool value) {
        assert(i < Mlit.size());
        assert(j < Mlit[i].size());
        assert(Mlit[i][j].has_value() == false);

        if(value) {
            nbOne++;

            int tmp = solver->newSoftVar(true, true, 1);
            Mlit[i][j] = tmp;
            Mone.add(i, j, 1);

            if(k > 1) {
                std::vector<VarSAT> TMP(k);
                std::vector<int> clause;
                for(auto &V: TMP) {
                    clause.push_back( V.id );
                }
                clause.push_back(-tmp);
                solver->addClause(clause);

                for(unsigned int x=0; x<k; x++) {
                    solver->addClause( {getVarM1(i, x), -TMP[x].id} );
                    solver->addClause( {getVarM2(x, j), -TMP[x].id} );
                }
            } else {
                solver->addClause( {getVarM1(i, 0), -tmp} );
                solver->addClause( {getVarM2(0, j), -tmp} );
            }

        } else {

            Mlit[i][j] = 0;

            for(unsigned int x=0; x<k; x++) {
                std::vector<int> clause;
                clause.push_back(-getVarM1(i, x));
                clause.push_back(-getVarM2(x, j));
                solver->addClause(clause);
            }
        }

        toUpdate = true;
    }


    unsigned int getNombreVar() {
        return static_cast<unsigned int>(solver->nVars());
    }

    unsigned int getNombreClause() {

        return static_cast<unsigned int>(solver->nClauses());
    }
};

VirtualMAXSAT* Infering::VarSAT::solver = nullptr;
VirtualMAXSAT* Infering::VarAux::solver = nullptr;



unsigned int sum(const std::vector<bool> &v) {
    unsigned int result = 0;
    for(unsigned int i=0; i<v.size(); i++) {
        result += v[i];
    }
    return result;
}

void transpose(std::vector<std::vector<bool>> &M) {
    std::vector<std::vector<bool>> M2(M[0].size());

    for(unsigned int i=0; i<M.size(); i++) {
        for(unsigned int j=0; j<M[i].size(); j++) {
            M2[j].push_back(M[i][j]);
        }
    }

    M = M2;
}

std::tuple<std::vector<bool>, std::vector<bool>> FRUI(const MaMatriceADJ &M)
{
    ////// Select a, b such that M[a][b] = 1 and |M[a][:]| * |M[:][b]| is maximal
    unsigned int a = std::numeric_limits<unsigned int>::max();
    unsigned int b = std::numeric_limits<unsigned int>::max();
    {
        unsigned int size = 0;
        for(auto [i, j]: M.Ones()) {
            unsigned int nbOnLine = M.OneOnLine(i).size();
            unsigned int nbOnCol = M.OneOnCol(j).size();
            if(size < nbOnLine * nbOnCol) {
                size = nbOnLine * nbOnCol;
                a=i;
                b=j;
            }
        }
    }

    if(a == std::numeric_limits<unsigned int>::max()) {
        return { {}, {} };
    }

    unsigned int nbLineActive = M.OneOnCol(b).size();
    unsigned int nbColActive = M.OneOnLine(a).size();

    Infering infer(nbLineActive, nbColActive, 1);
    std::map<unsigned int, unsigned int> itoI;
    std::map<unsigned int, unsigned int> jtoJ;
    std::vector<unsigned int> Itoi;
    std::vector<unsigned int> Jtoj;

    for(auto i: M.OneOnCol(b)) {
        for(auto j: M.OneOnLine(a)) {
            if(M.get(i, j).has_value()) {
                if( (M.get(a, j).value_or(1) == 1) && (M.get(i, b).value_or(1) == 1) ) {
                    if(itoI.count(i)==0) {
                        unsigned int tmp = itoI.size();
                        itoI[i] = tmp;
                        assert(Itoi.size() == tmp);
                        Itoi.push_back(i);
                    }

                    if(jtoJ.count(j)==0) {
                        unsigned int tmp = jtoJ.size();
                        jtoJ[j] = tmp;
                        assert(Jtoj.size() == tmp);
                        Jtoj.push_back(j);
                    }

                    infer.add(itoI[i], jtoJ[j], M.get(i, j).value());
                }
            }
        }
    }

    infer.forceOne(itoI[a], jtoJ[b]);

    auto res = infer.inferModel();
    assert(res);


    std::vector<bool> A(M.m(), false);
    std::vector<bool> B(M.n(), false);


    // Retrieve results
    for(unsigned int I = 0; I < nbLineActive; I++) {
        A[ Itoi[I] ] = infer.getValueLine(I);
    }
    for(unsigned int J = 0; J < nbColActive; J++) {
        B[ Jtoj[J] ] = infer.getValueCol(J);
    }

    return {A, B};
}


MaMatriceADJ generateRdmMatrix(unsigned int U, unsigned int V, double dencity, unsigned int rank=0) {


    if(rank > 0) {
        vector< vector<bool> > A(U, vector<bool>(rank));
        vector< vector<bool> > B(rank, vector<bool>(V));

        double p = ( sqrt(1 - std::pow(1-dencity, 1/(double)rank) ) );
        for(unsigned int i=0; i<U; i++) {
            for(unsigned int k=0; k<rank; k++) {
                A[i][k] = MonRand::getBool(p);
            }
        }

        for(unsigned int k=0; k<rank; k++) {
            for(unsigned int j=0; j<V; j++) {
                B[k][j] = MonRand::getBool(p);
            }
        }
        return multAdj(A, B);
    }

    MaMatriceADJ M;
    M.resize(U, V);

    for(unsigned int i=0; i<U; i++) {
        for(unsigned int j=0; j<V; j++) {
            M.add(i, j, MonRand::getBool(dencity));
        }
    }

    return M;
}

unsigned int verif(const vector< vector<bool> > &A, const vector< vector<bool> > &B, const MaMatriceADJ& M) {
    if(verbosity >= 1)
        std::cout << "k = " << B.size() << std::endl;

    unsigned int okOnOne = 0;
    unsigned int okOnZero = 0;
    unsigned int errOnOne=0;
    unsigned int errOnZero=0;
    assert(B.size());

    for(unsigned int i=0; i<A.size(); i++) {
        for(unsigned int j=0; j<B[0].size(); j++) {

            if(M.get(i,j).has_value()) {

                bool value = false;
                for(unsigned int k=0; k<B.size(); k++) {
                    if( A[i][k] && B[k][j]) {
                        value = true;
                    }
                }

                if(M.get(i,j).value() == value) {
                    if(M.get(i,j).value())
                        okOnOne++;
                    else
                        okOnZero++;
                } else {
                    if(M.get(i,j).value())
                        errOnOne++;
                    else
                        errOnZero++;
                }
            }

        }
    }

    if(verbosity >= 1) {
        std::cout << "okOnOne: " << okOnOne << std::endl;
        std::cout << "errOnZero: " << errOnZero << std::endl;
        std::cout << "errOnOne: " << errOnOne << std::endl;
    }
    assert(errOnZero == 0);

    return errOnOne;
}

std::tuple<unsigned int, unsigned int, unsigned int> countScore(const vector< vector<bool> > &A, const vector< vector<bool> > &B, const MaMatriceADJ& M) {
    unsigned int errorOnZero = 0;
    auto _M = mult(A, B);

    assert(_M.size());
    assert(_M.size() <= M.m());
    //assert(_M[0].size() <= M.n());

    if(_M.size() < M.m()) {
        _M.resize( M.m(), std::vector<bool>(M.n(), 0) );
    }
    if(_M[0].size() < M.n()) {
        for(unsigned int i=0; i<_M.size(); i++) {
            _M[i].resize(M.n(), 0);
        }
    }

    unsigned int oneBienClasse = 0;
    unsigned int error = 0;

    for( auto [i, j]: M.Ones() ) {
        assert(_M.size() > i);
        assert(_M[i].size() > j);

        oneBienClasse += (1 == _M[i][j]);
        error += (1 != _M[i][j]);
    }
    for( auto [i, j]: M.Zeros() ) {
        assert(_M.size() > i);
        assert(_M[i].size() > j);

        error += (0 != _M[i][j]);

        errorOnZero += (0 != _M[i][j]);
    }


    return {oneBienClasse, error, errorOnZero};
}

std::tuple<MaMatriceADJ, MaMatriceADJ> addNoise(const MaMatriceADJ &M, double noise, double observateData) {
    MaMatriceADJ result;
    MaMatriceADJ removed;

    for(unsigned int i=0; i<M.m(); i++) {
        for(unsigned int j=0; j<M.n(); j++) {
            if(M.get(i, j).has_value()) {
                if(MonRand::getBool( observateData )) {
                    if(MonRand::getBool( noise )) {
                        result.add(i, j, !(M.get(i, j).value()));
                    } else {
                        result.add(i, j, M.get(i, j).value());
                    }
                } else {
                    removed.add(i, j, M.get(i, j).value());
                }
            }
        }
    }
    return {result, removed};
}

std::tuple<std::vector<std::vector<bool>>, std::vector<std::vector<bool>>> RUI(MaMatriceADJ &M, unsigned int k) {

    Infering infer(M.m(), M.n(), k);

    bool pasDeOne=true;
    for(auto [i, j]: M.Ones()) {
        pasDeOne=false;
        infer.add(i, j, 1);
        assert(M.get(i, j).value() == 1);
    }

    if(pasDeOne) {
        return {{},{}};
    }
    for(auto [i, j]: M.Zeros()) {
        infer.add(i, j, 0);
    }

    auto res = infer.inferModel();
    if(!res) {
        return {{},{}};
    }

    std::vector<std::vector<bool>> M1;
    std::vector<std::vector<bool>> M2;
    std::tie(M1, M2) = infer.getModel();

    return {M1, M2};
}

void etendre(const MaMatriceADJ &Minit, std::vector<std::vector<bool>> &M1, std::vector<std::vector<bool>> &M2, const std::set< std::tuple<unsigned int, unsigned int> > &OneAlreadyCovered){

    std::vector< std::tuple<unsigned int, unsigned int, bool> > tmp;

    if(M2.size() == 0)
        return;


    if(M1.size() == 0)
        return;;


    assert(M1[0].size() == M2.size());


    for(unsigned int i=0; i<M1.size(); i++) {
        for(unsigned int g=0; g<M1[i].size(); g++) {
            if(M1[i][g] == 0) {
                tmp.push_back({i,g,0});
            }
        }
    }

    for(unsigned int g=0; g<M2.size(); g++) {
        for(unsigned int j=0; j<M2[g].size(); j++) {
            if(M2[g][j] == 0) {
                tmp.push_back({g,j,1});
            }
        }
    }

    std::random_shuffle(tmp.begin(), tmp.end());

    for(auto [a, b, c]: tmp) {
        if(!c) {
            unsigned int i = a;
            unsigned int g = b;

            bool ok=true;
            bool utile=false;

            for(unsigned int j=0; j<M2[g].size(); j++) {
                if(M2[g][j]) {
                    if( !Minit.get(i, j).value_or(true) ) {
                        ok = false;
                        break;
                    } else {
                        //if( Minit.get(i, j).has_value() == false ) {
                            if( OneAlreadyCovered.count({i, j}) ) {
                                utile = true;
                            }
                        //}
                    }

                }
            }

            if(ok && utile) {
                M1[i][g] = 1;
            }
        } else {
            unsigned int g = a;
            unsigned int j = b;

            bool ok=true;
            bool utile=false;

            for(unsigned int i=0; i<M1.size(); i++) {
                if(M1[i][g]) {
                    if( !Minit.get(i, j).value_or(true) ) {
                        ok = false;
                        break;
                    } else {
                        //if( Minit.get(i, j).has_value() == false ) {
                            if( OneAlreadyCovered.count({i, j}) ) {
                                utile = true;
                            }
                        //}
                    }
                }
            }

            if(ok & utile) {
                M2[g][j] = 1;
            }
        }
    }
}

std::tuple< std::vector<std::vector<bool>>, std::vector<std::vector<bool>> > inferer(
            const MaMatriceADJ &Minit,
            unsigned int k,
            unsigned int nbIt,
            bool fastUndercover,
            bool optiblock) {

    auto M = Minit;
    std::vector<std::vector<bool>> A(M.m());
    std::vector<std::vector<bool>> B;

    std::vector< std::vector<std::vector<bool>> > G_M1;
    std::vector< std::vector<std::vector<bool>> > G_M2;

    std::set< std::tuple<unsigned int, unsigned int> > OneAlreadyCovered;

    for(unsigned int it=0 ; it<nbIt ; it++) {
        if(M.numberOfOne() == 0) {
            if(optiblock) {
                std::vector<std::vector<bool>> saveA;
                std::vector<std::vector<bool>> saveB;
                
    		//auto [nbOneCovered, numberError, errorOnZero] = countScore(A, B, Minit);
    		unsigned int numberError=-1;
                do {
                	numberError=-1;
                        saveA = A;
                        saveB = B;

                        // Remove the block k
                        G_M1.pop_back();
                        G_M2.pop_back();
                        
                        std::cout << "try with k=" <<   G_M1.size() << std::endl;
                                              
			for(unsigned int ccc=2;;ccc++) {
			    for(unsigned int it=0 ; it<G_M1.size() ; it++) {
				auto M = Minit;

				for(unsigned int it2=0; it2<G_M1.size(); it2++) {
				    if(it2 == it)
				        continue;

				    auto _M = mult(G_M1[it2], G_M2[it2]);

				    for(unsigned int i=0; i<_M.size(); i++) {
				        for(unsigned int j=0; j<_M[0].size(); j++) {
				            if(_M[i][j]) {
				                M.remove(i, j);
				            }
				        }
				    }
				}

				std::vector<std::vector<bool>> M1;
				std::vector<std::vector<bool>> M2;

				tie(M1, M2) = RUI(M, 1);

				etendre(Minit, M1, M2, OneAlreadyCovered);

				G_M1[it] = M1;
				G_M2[it] = M2;
			    }

			    A.clear();
			    A.resize(M.m());
			    B.clear();
			    for(unsigned int it=0 ; it<G_M1.size() ; it++) {
				for(unsigned int i=0; i<G_M1[it].size(); i++) {
				    for(unsigned int j=0; j<G_M1[it][i].size(); j++) {
				        A[i].push_back(G_M1[it][i][j]);
				    }
				}
				for(unsigned int i=0; i<G_M2[it].size(); i++) {
				    B.push_back(G_M2[it][i]);
				}
			    }

			    auto [nbOneCovered2, numberError2, errorOnZero2] = countScore(A, B, Minit);
			    std::cout << "numberError = " << numberError2 << std::endl;

			    if( numberError2 < numberError ) {
				numberError = numberError2;
				if(verbosity >= 1)
				    std::cout << "Number of reconstruction errors after "<< ccc <<" itterations: " << numberError << std::endl;
			    } else {
			    	assert(numberError2 == numberError);
				break;
			    }
			    if(numberError == 0)
			        break;
			}
			
			
                } while(numberError == 0);
                
                return {saveA, saveB};
            }
            return {A, B};
        }

        std::vector<std::vector<bool>> M1;
        std::vector<std::vector<bool>> M2;

        if( fastUndercover ) {


            auto [line, col] = FRUI(M);

            M1.push_back(line);
            M2.push_back(col);
            transpose(M1);
        } else {
            tie(M1, M2) = RUI(M, k);
        }

        if(M1.size() == 0) {
            break;
        }

        etendre(Minit, M1, M2, OneAlreadyCovered);

        assert(M2.size());
        auto _M = mult(M1, M2);
        G_M1.push_back(M1);
        G_M2.push_back(M2);
        assert(_M.size() == M.m());
        assert(_M[0].size() == M.n());
        for(unsigned int i=0; i<_M.size(); i++) {
            for(unsigned int j=0; j<_M[0].size(); j++) {
                if(_M[i][j]) {
                    if(M.get(i, j).value_or(false)) {
                        assert( M.get(i, j).value() == true );
                        OneAlreadyCovered.insert({i, j});
                    }
                    M.remove(i, j);
                }
            }
        }
        for(unsigned int i=0; i<M1.size(); i++) {
            for(unsigned int j=0; j<M1[i].size(); j++) {
                A[i].push_back(M1[i][j]);
            }
        }
        for(unsigned int i=0; i<M2.size(); i++) {
            B.push_back(M2[i]);
        }
    }

    auto [nbOneCovered, numberError, errorOnZero] = countScore(A, B, Minit);

    if(optiblock)
    if(nbIt > 1)
    {
        if(verbosity >= 1)
            std::cout << "Number of reconstruction errors after the first itteration: " << numberError << std::endl;
        if(numberError == 0) {
            return {A, B};
        }
        for(unsigned int ccc=2;;ccc++) {
            for(unsigned int it=0 ; it<nbIt ; it++) {
                auto M = Minit;

                for(unsigned int it2=0; it2<nbIt; it2++) {
                    if(it2 == it)
                        continue;

                    auto _M = mult(G_M1[it2], G_M2[it2]);

                    for(unsigned int i=0; i<_M.size(); i++) {
                        for(unsigned int j=0; j<_M[0].size(); j++) {
                            if(_M[i][j]) {
                                M.remove(i, j);
                            }
                        }
                    }
                }

                std::vector<std::vector<bool>> M1;
                std::vector<std::vector<bool>> M2;

                tie(M1, M2) = RUI(M, 1);

                etendre(Minit, M1, M2, OneAlreadyCovered);

                G_M1[it] = M1;
                G_M2[it] = M2;
            }

            A.clear();
            A.resize(M.m());
            B.clear();
            for(unsigned int it=0 ; it<nbIt ; it++) {
                for(unsigned int i=0; i<G_M1[it].size(); i++) {
                    for(unsigned int j=0; j<G_M1[it][i].size(); j++) {
                        A[i].push_back(G_M1[it][i][j]);
                    }
                }
                for(unsigned int i=0; i<G_M2[it].size(); i++) {
                    B.push_back(G_M2[it][i]);
                }
            }

            auto [nbOneCovered2, numberError2, errorOnZero2] = countScore(A, B, Minit);

            if( numberError2 < numberError ) {
                numberError = numberError2;
                if(verbosity >= 1)
                    std::cout << "Number of reconstruction errors after "<< ccc <<" itterations: " << numberError << std::endl;
            } else {
                break;
            }
        }
    }

    return {A, B};
}

void exportToCSV(const MaMatriceADJ &M, const std::string &inputFile) {
    if(inputFile.size()) {
        fstream out(inputFile, ios::out);
        for(unsigned int i=0; i<M.m(); i++) {
            for(unsigned int j=0; j<M.n(); j++) {
                if(j!=0)
                    out << ",";
                if(M.get(i, j).has_value()) {
                    out << M.get(i, j).value();
                } else {
                    out << "nan";
                }
            }
            out << "\n";
        }
    }
}

void exportToCSV(const std::vector<std::vector<bool>> &M, const std::string &file ) {
    fstream out(file, ios::out);

    for(auto &line: M) {
        bool first=true;
        for(auto v: line) {
            if(!first)
                out << ",";
            first=false;
            out << v;
        }
        out << "\n";
    }
    out.close();
}

void exportToDAT(const MaMatriceADJ &M, const std::string &inputFile) {
    if(inputFile.size()) {
        fstream out(inputFile, ios::out);
        for(unsigned int i=0; i<M.m(); i++) {
            bool first = true;
            for(unsigned int j=0; j<M.n(); j++) {
                if(M.get(i, j).value_or(false)) {

                    if(!first)
                        out << " ";
                    first = false;
                    out << j;
                }
            }
            out << "\n";
        }
    }
}


MaLib::Chrono C_TOTAL;
int main(int argc, char *argv[]) {

    CLI::App app("Undercover Boolean Matrix Factorization");

    GLOBAL_externalSolver = "";
    app.add_option("--solver", GLOBAL_externalSolver, "External solver cmd (internal solver by default)");

    GLOBAL_unactivateCardGen = false;
    app.add_flag("--noCardGen", GLOBAL_unactivateCardGen, "Unactivate Card Generation");

    GLOBAL_noBreakSym = false;
    app.add_flag("--noBS", GLOBAL_noBreakSym, "Activate Break Symmetry");

    unsigned int seed = 42;
    app.add_option("--seed", seed, "seed (0 means random seed)");

    bool optiblock=false;
    app.add_flag("--OptiBlock", optiblock, "Activate the OptiBlock strategy");

    bool noFastUndercover=false;
    app.add_flag("--noFastUndercover", noFastUndercover, "Unactivate the FastUndercover strategy");

    unsigned int k=1;
    app.add_option("-k", k, "k (default = 1)");

    bool optimalStrategy=false;
    app.add_flag("--optimal", optimalStrategy, "Search for an optimal k undercover");

    double observateData = 1.0;
    app.add_option("--obs", observateData, "observate data (default: 1.0)");

    std::string inputFile = "";
    app.add_option("--IG", inputFile, "save input in a graph file");

    std::string inputFileDAT = "";
    app.add_option("--IDAT", inputFileDAT, "save input in a dat file");

    app.add_option("-v", verbosity, "vebosity (default: 1)");

    MaMatriceADJ M;
    MaMatriceADJ Moracle;
    MaMatriceADJ Mremoved;

    ////////////////
    /// fromFile ///
    ////////////////
    auto fromFile = app.add_subcommand("fromFile", "Input from file");

    bool adjList=false;
    fromFile->add_flag("--adj", adjList, "Read the input file as an adjacency list");

    string output="";
    fromFile->add_option("-o", output, "output file for A and B");

    string productOutput="";
    fromFile->add_option("-O", productOutput, "output file for A o B");

    string file;
    fromFile->add_option("CSV_file", file, "CSV file")->check(CLI::ExistingFile)->required();

    fromFile->callback( [&]() {
        MatrixLoarder MLoader(file);
        if(!adjList) {
            auto Mtab = MLoader.getMatrix();

            for(unsigned int i=0; i<Mtab.size(); i++) {
                for(unsigned int j=0; j<Mtab[i].size(); j++) {
                    if(Mtab[i][j].has_value()) {
                        Moracle.add(i, j, Mtab[i][j].value());
                    }
                }
            }
        } else {
            auto adj = MLoader.getADJ();
            for(auto & [i, j, v]: adj) {
                if((v != 0) && (v != 1)) {
                    std::cerr << "The input matrix must be binary. The input " << v << " is not accepted." << std::endl;
                    exit(-1);
                }
                assert(v == 0 || v == 1);
                Moracle.add(i, j, v);
            }
        }
    });
    ///////////////////



    ///////////////
    /// fromRdm ///
    ///////////////
    auto fromRdm = app.add_subcommand("fromRdm", "Generate an random Matrix");

    unsigned int rank = 0;
    fromRdm->add_option("-k", rank, "Rank (default: no rank)");

    double density = 0.1;
    fromRdm->add_option("-d", density, "Density (default: 0.1)");

    unsigned int U = 0;
    fromRdm->add_option("-U", U, "Number of line (default: size)");

    unsigned int V = 0;
    fromRdm->add_option("-V", V, "Number of column (default: size)");

    unsigned int size = 100;
    fromRdm->add_option("--size", size, "Size (default: 100)");

    fromRdm->callback( [&]() {
        if(U==0) {
            U = size;
        }
        if(V==0) {
            V = size;
        }

        if(verbosity >= 1) {
            std::cout << "Generate random matrix" << std::endl;
            std::cout << "- size: \t" << U << " x " << V << std::endl;
            if(rank) {
                std::cout << "- rank: \t" << rank << std::endl;
            }
            std::cout << "- density: \t" << (density * 100.0) << " %" << std::endl;
            std::cout << "- observate: \t" << (observateData * 100.0) << " %" << std::endl;
        }

        Moracle = generateRdmMatrix(U, V, density, rank);
    });
    ///////////////////


    CLI11_PARSE(app, argc, argv);

    if(Moracle.m() * Moracle.n() == 0) {
        std::cerr << "Empty input matrix, use subcommand \"fromFile\" or \"fromRdm\"" << std::endl;
        exit(-1);
    }

    if(optimalStrategy) {
        noFastUndercover = true;
    }

    srand(seed);
    MonRand::seed(seed);

    std::tie(M, Mremoved) = addNoise(Moracle, 0, observateData);
    const MaMatriceADJ Minit = M;   // Minit is constant to be sure that we do not modify the input data


    if(inputFile.size()) {
        exportToCSV(M, inputFile);
    }
    if(inputFileDAT.size()) {
        exportToDAT(M, inputFileDAT);
    }

    unsigned int nbIt=1;
    if(optimalStrategy) {
        if(optiblock) {
            std::cerr << "The OptiBlock strategy is not able to guarantee to find an optimal undercover. Please unactivate \"optimal\" or \"optiBlock\"." << std::endl;
            exit(-1);
        }
        if(verbosity >= 1)
            std::cout << "Search for an optimal " << k << "-undercover." << std::endl;
    } else {
        if(verbosity >= 1) {
            if(optiblock) {
                std::cout << "Search for a block-optimal " << k << "-undercover";
                if(!noFastUndercover)
                    std::cout << " with a FastUndercover initialisation";
                std::cout << "." << std::endl;
            } else {
                if(noFastUndercover) {
                    double approx = 1.0 - std::pow(1.0 - 1.0/(double)k, k);
                    std::cout << "Search for a " << approx <<"-approximation for a " << k << "-undercover." << std::endl;
                } else {
                    std::cout << "Search for a " << k << "-undercover with the FastUndercover heuristic." << std::endl;
                }
            }
        }
        nbIt = k;
        k = 1;
    }

    auto [A, B] = inferer(M, k, nbIt, !noFastUndercover, optiblock);

    unsigned int numberError = verif(A, B, Minit); // Check that the solution is an undercover and count the number of reconstruction errors made by A o B on Minit

    if(verbosity == 0) {
        std::cout << file << "\t" << B.size() << "\t" << numberError << "\t" << C_TOTAL.tac()/1000000.0 << std::endl;
    }

    if(output.size()) {
        std::string fileA = output;
        fileA.append(".A.csv");
        exportToCSV(A, fileA);

        std::string fileB = output;
        fileB.append(".B.csv");
        exportToCSV(B, fileB);
    }

    if(productOutput.size()) {
        std::string file = output;
        file.append(".csv");
        exportToCSV(mult(A, B), file);
    }

    return 0;
}

















