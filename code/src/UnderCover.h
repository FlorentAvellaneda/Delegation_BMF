#ifndef UNDERCOVER_H__

#define UNDERCOVER_H__


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
#include "coutUtil.h"
#include "Chrono.h"
#include "matrixloarder.h"
#include "Moy.h"
#include "rand.h"

#include "Matrix.hpp"

using namespace MaLib;
using namespace std;

//// GLOBAL /////
std::string GLOBAL_externalSolver = "";
bool GLOBAL_noBreakSym = 0;
unsigned int verbosity = 1;
///


namespace UnderCover {

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

    virtual bool isWeighted() {
        return true;
    }

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

    virtual unsigned int newSoftVar(bool value, t_weight weight=1) {
        if(value)
            isSoftVar.push_back(weight);
        else
            isSoftVar.push_back(-weight);
        nbSoft++;

        return isSoftVar.size()-1;
    }

    void atMostKOnSoftVar(std::vector<int> clause, unsigned int k) override {
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

    virtual bool solve() override {

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
    Matrix Mone;

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
            for(unsigned int i=0; i<Mbis.nbLine(); i++) {
                for(unsigned int j: Mbis.getOneOnLine(i)) {
                    unsigned int size = (Mbis.getOneOnLine(i).size()-1)*(Mbis.getOneOnCol(j).size()-1);
                    if(sortedMap.size() <= size) {
                        sortedMap.resize( size + 1 );
                    }
                    sortedMap[size].push_back({i, j});
                }
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

        genCard();


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
        genCard();
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

    void setA(const std::vector<std::vector<bool>> & A) {
        for(unsigned int i=0; i<A.size(); i++) {
            for(unsigned int l=0; l<A[i].size(); l++) {
                if(A[i][l]) {
                    solver->addClause( { getVarM1(i,l) } );
                } else {
                    solver->addClause( { -getVarM1(i,l) } );
                }
            }
        }
    }

    void setB(const std::vector<std::vector<bool>> & B) {
        for(unsigned int l=0; l<B.size(); l++) {
            for(unsigned int j=0; j<B[l].size(); j++) {
                if(B[l][j]) {
                    solver->addClause( { getVarM2(l,j) } );
                } else {
                    solver->addClause( { -getVarM2(l,j) } );
                }
            }
        }
    }

    int nbOne=0;
    void add(unsigned int i, unsigned int j, bool value, unsigned int weight=1) {
        assert(i < Mlit.size());
        assert(j < Mlit[i].size());
        assert(Mlit[i][j].has_value() == false);

        assert(weight > 0);

        if(value) {
            nbOne++;

            int tmp = solver->newSoftVar(true, weight);
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

    void addSoft(unsigned int i, unsigned int j, bool value, unsigned int weight=1) {
        assert(i < Mlit.size());
        assert(j < Mlit[i].size());
        assert(Mlit[i][j].has_value() == false);

        if(value) {
            nbOne++;

            int tmp = solver->newSoftVar(true, weight);
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
            for(unsigned int x=0; x<k; x++) {
                std::vector<int> clause;
                clause.push_back(-getVarM1(i, x));
                clause.push_back(-getVarM2(x, j));
                solver->addWeightedClause(clause, 1);
            }
        }

        toUpdate = true;
    }

    unsigned int getNombreVar() {
        return static_cast<unsigned int>(solver->nVars());
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

std::tuple<std::vector<bool>, std::vector<bool>> FRUIT(const Matrix &M, BMF& bmf, bool weighted) {

    ////// Select a, b such that M[a][b] = 1 and |M[a][:]| * |M[:][b]| is maximal
    unsigned int a = std::numeric_limits<unsigned int>::max();
    unsigned int b = std::numeric_limits<unsigned int>::max();
    {
        unsigned int size = 0;
        for(unsigned int i=0; i<M.nbLine(); i++) {
            for(auto j: M.getOneOnLine(i)) {
                if(bmf.get(i,j) == 0) {
                    unsigned int nbOnLine = 0; // = weighted?M.getWeightLine(i):M.getOneOnLine(i).size();
                    for(auto j2: M.getOneOnLine(i)) {
                        if(bmf.get(i,j2) == 0) {
                            nbOnLine += weighted?M.getWeight(i,j2):1;
                        }
                    }

                    unsigned int nbOnCol = 0;//weighted?M.getWeightCol(j):M.getOneOnCol(j).size();
                    for(auto i2: M.getOneOnCol(j)) {
                        if(bmf.get(i2,j) == 0) {
                            nbOnCol += weighted?M.getWeight(i2,j):1;
                        }
                    }

                    if(size < nbOnLine * nbOnCol) {
                        size = nbOnLine * nbOnCol;
                        a=i;
                        b=j;
                    }
                }
            }
        }
    }

    if(a == std::numeric_limits<unsigned int>::max()) {
        return { {}, {} };
    }

    std::vector<unsigned int> selectedI;
    for(unsigned int i=0; i<M.nbLine(); i++) {
        if( M.get(i, b).value_or(true) ) {
            if(M.getOneOnLine(i).size() > 0) {      // TODO : ok ?
                selectedI.push_back(i);
            }
        }
    }
    std::vector<unsigned int> selectedJ;
    for(unsigned int j=0; j<M.nbCol(); j++) {
        if( M.get(a, j).value_or(true) ) {
            if(M.getOneOnCol(j).size() > 0) {       // TODO : ok ?
                selectedJ.push_back(j);
            }
        }
    }

    if(selectedI.size()==1 && selectedJ.size()==1) {
        std::vector<bool> A(M.nbLine(), false);
        std::vector<bool> B(M.nbCol(), false);
        A[a] = 1;
        B[b] = 1;
        return {A, B};
    }


    unsigned int nbLineActive = selectedI.size();
    unsigned int nbColActive = selectedJ.size();

    Infering infer(nbLineActive, nbColActive, 1);
    std::map<unsigned int, unsigned int> itoI;
    std::map<unsigned int, unsigned int> jtoJ;
    std::vector<unsigned int> Itoi;
    std::vector<unsigned int> Jtoj;

    for(auto i: selectedI) {
        for(auto j: selectedJ) {

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

            if(M.get(i,j).has_value()) {
                if(M.get(i,j).value() && (bmf.get(i,j)==0)) {
                    infer.add(itoI[i], jtoJ[j], 1, weighted?M.getWeight(i, j):1);
                } else {
                    infer.add(itoI[i], jtoJ[j], 0);
                }
            }
        }
    }


    infer.forceOne(itoI[a], jtoJ[b]);

    auto res = infer.inferModel();
    assert(res);


    std::vector<bool> A(M.nbLine(), false);
    std::vector<bool> B(M.nbCol(), false);


    // Retrieve results
    for(unsigned int I = 0; I < nbLineActive; I++) {
        A[ Itoi[I] ] = infer.getValueLine(I);
    }
    for(unsigned int J = 0; J < nbColActive; J++) {
        B[ Jtoj[J] ] = infer.getValueCol(J);
    }

    assert( A[a] && B[b] );

    return {A, B};

}

unsigned int calculerCost(const vector< vector<bool> > &A, const vector< vector<bool> > &B, const Matrix& M) {
    //if(verbosity >= 1)
    //    std::cout << "k = " << B.size() << std::endl;

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

    return errOnOne+errOnZero;
}

unsigned int countScore(const BMF& bmf, const Matrix& M, bool weighted) {
    unsigned int error = 0;

    for(unsigned int i=0; i<M.nbLine(); i++) {
        for(auto j: M.getOneOnLine(i)) {
            error += (weighted?M.getWeight(i,j):1) * (0 == bmf.get(i,j));
        }
    }

    return error;
}

std::tuple<std::vector<std::vector<bool>>, std::vector<std::vector<bool>>, unsigned int> RUI(const Matrix &M, const BMF &bmf, unsigned int k, bool weighted) {

    Infering infer(M.nbLine(), M.nbCol(), k);

    bool pasDeOne=true;
    for(unsigned int i=0; i<M.nbLine(); i++) {
        for(auto j: M.getOneOnLine(i)) {
            if(bmf.get(i, j) == 0) {
                pasDeOne=false;
                infer.add(i, j, 1, weighted?M.getWeight(i, j):1);
                assert(M.get(i, j).value() == 1);
            }
        }
    }
    if(pasDeOne) {
        return {{},{}, -1};
    }

    for(unsigned int i=0; i<M.nbLine(); i++) {
        for(auto j: M.getZeroOnLine(i)) {
            infer.add(i, j, 0);
        }
    }

    auto res = infer.inferModel();
    if(!res) {
        return {{},{}, -1};
    }

    std::vector<std::vector<bool>> M1;
    std::vector<std::vector<bool>> M2;
    std::tie(M1, M2) = infer.getModel();

    //std::cout << "nb error = " << infer.getCost() << std::endl;

    return {M1, M2, infer.getCost()};
}

BMF inferer2(
            const Matrix &M,
            unsigned int nbIt,
            bool fastUndercover,
            bool optiblock,
            bool weighted) {


    BMF result;
    if(M.noOne()) {
        return result;
    }

    unsigned int lastErrorLeft=-1;
    unsigned int mainIT=0;
    for( ; mainIT < nbIt ; mainIT++) {

        if( fastUndercover ) {
            auto [line, col] = FRUIT(M, result, weighted);

            if(line.size()==0) {
                if(verbosity)
                    std::cout << "STOP after "<<mainIT<<" itterations. Rank = " << (mainIT) << std::endl;
                return result;
            }

            result.addRank(line, col, M);
        } else {
            auto [M1, M2, errorsLeft] = RUI(M, result, 1, weighted);
            if(verbosity) {
                std::cout << "k="<<(mainIT+1)<<", errorsLeft=" << errorsLeft << std::endl;
            }
            lastErrorLeft=errorsLeft;
            auto line_M2 = M2[0];
            std::vector<bool> col_M1;
            for(auto &v: M1) {
                col_M1.push_back(v[0]);
            }
            result.addRank(col_M1, line_M2, M);

            if(errorsLeft==0) {
                if(verbosity)
                    std::cout << "k = " << mainIT << std::endl;
                return result;
            }
        }
    }

    if(lastErrorLeft == -1) {
        lastErrorLeft = countScore(result, M, weighted);
        if(lastErrorLeft==0)
            return result;
    }

    if(optiblock)
    if(mainIT > 1)
    {
        unsigned int newErrorLeft;
        for(unsigned int ccc=2;;ccc++) {
            if(verbosity) {
                std::cout << "Error left: " << lastErrorLeft << std::endl;
            }
            for(int k=mainIT-2; k>=0 ; k--) {

                // Remove the block k
                result.removeRank(k);

                // Add the best block
                {
                    auto [M1, M2, errorsLeft] = RUI(M, result, 1, weighted);
                    newErrorLeft = errorsLeft;
                    auto line_M2 = M2[0];
                    std::vector<bool> col_M1;
                    for(auto &v: M1) {
                        col_M1.push_back(v[0]);
                    }
                    result.addRank(col_M1, line_M2, M);
                    if(errorsLeft==0) {
                        return result;
                    }
                }
            }

            if( newErrorLeft < lastErrorLeft ) {
                lastErrorLeft = newErrorLeft;
            } else {
                break;
            }
        }
    }

    return result;
}


void exportToCSV(const Matrix &M, const std::string &inputFile) {
    std::cout << "file = " << inputFile << std::endl;
    std::cout << "nbLine = " << M.nbLine() << std::endl;
    std::cout << "nbCol = " << M.nbCol() << std::endl;
    if(inputFile.size()) {
        fstream out(inputFile, ios::out);
        for(unsigned int i=0; i<M.nbLine(); i++) {
            for(unsigned int j=0; j<M.nbCol(); j++) {
                if(j!=0)
                    out << ",";
                if(M.get(i, j).has_value()) {
                    out << M.get(i, j).value();
                }
            }
            out << "\n";
        }
    }
    std::cout << "export Fini." << std::endl;
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


}

#endif
