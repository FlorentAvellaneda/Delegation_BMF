#include <iostream>
#include <vector>
#include <set>
#include <cassert>
#include <unordered_set>
#include <limits>
#include <algorithm>
#include <optional>

#include "lib/CLI11.hpp"

#include "Chrono.h"
#include "rand.h"
#include "src/UnderCover.h"

#include "src/Matrix.hpp"

MaLib::Chrono C;



MaLib::Chrono C_TOTAL;//("TOTAL");
int main(int argc, char *argv[]) {
    CLI::App app("BMF Through Simplified Matrix");

    app.add_option("--solver", GLOBAL_externalSolver, "External MaxSAT solver cmd (internal solver by default)");

    unsigned int seed = 42;
    app.add_option("--seed", seed, "seed (0 means random seed)");

    bool optiblock=false;
    app.add_flag("--OptiBlock", optiblock, "Activate the OptiBlock strategy");

    unsigned int k=std::numeric_limits<unsigned int>::max();
    app.add_option("-k", k, "k (default = +infinity)");

    bool simplExi = false;
    app.add_flag("--exi", simplExi, "Simplification Existentially");

    bool simplUni = false;
    app.add_flag("--uni", simplUni, "Simplification Universally");

    bool sparse = false;
    app.add_flag("--sparse", sparse, "Sparse format for the input file (each line is in the form idLine, idCol, val)");

    string outputSimplify="";
    app.add_option("-S", outputSimplify, "Save the simplified matrix");

    app.add_option("-v", verbosity, "verbosity (default: 1)");

    string output="";
    app.add_option("-o", output, "output file for A and B");

    string productOutput="";
    app.add_option("-O", productOutput, "output file for A o B");

    string file;
    app.add_option("CSV_file", file, "Matrix to factorize")->check(CLI::ExistingFile)->required();


    CLI11_PARSE(app, argc, argv);

    Matrix DATA;
    MatrixLoarder MLoader(file);



    if(!sparse) {
        auto Mtab = MLoader.getMatrix();

        for(unsigned int i=0; i<Mtab.size(); i++) {
            for(unsigned int j=0; j<Mtab[i].size(); j++) {
                if(Mtab[i][j].has_value()) {
                    DATA.add(i, j, Mtab[i][j].value());
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
            DATA.add(i, j, v);
        }
    }

    srand(seed);
    MonRand::seed(seed);

    const Matrix Minit = DATA;

    std::vector< std::tuple<unsigned int, unsigned int, unsigned int, unsigned int> > implications;

    unsigned int nbOnesAfterUni=DATA.nbOnes();
    Chrono TMP;
    if(verbosity) {
        std::cout << "Number ones: " << DATA.nbOnes() << std::endl;
    }

    if(simplUni)
    {
        if(verbosity) {
            std::cout << "Simplify Universally..." << std::endl;
        }
        auto tmp = DATA.simplify();
        implications.insert(implications.end(), tmp.begin(), tmp.end());

        nbOnesAfterUni=DATA.nbOnes();

        if(verbosity) {
            std::cout << "Number one after simplify: " << nbOnesAfterUni << std::endl;
        }
    }
    auto timeUni = TMP.tacSec();

    unsigned int nbOnesAfterExi = DATA.nbOnes();
    if(simplExi)
    {
        Chrono TMP;
        if(verbosity)
            std::cout << "Simplify Existentially..." << std::endl;
        auto tmp = DATA.simplify(true);

        implications.insert(implications.end(), tmp.begin(), tmp.end());


        nbOnesAfterExi = DATA.nbOnes();

        if(verbosity) {
            std::cout << "Number one after simplify: " << nbOnesAfterExi << std::endl;
        }
    }
    auto timeExi=TMP.tacSec();;

    if(outputSimplify.size()) {
        UnderCover::exportToCSV(DATA, outputSimplify);
    }




    std::vector<std::vector<bool>> A;
    std::vector<std::vector<bool>> B;

    {
        auto bmf = UnderCover::inferer2(
                    DATA,
                    /*nbit=*/ k,
                    /*fastUndercover=*/ true,
                    /*optiblock=*/ optiblock,
                    /*weighted=*/ false
                    );
        std::tie(A, B) = bmf.getAB();
    }


    for(auto it=implications.rbegin(); it!=implications.rend(); ++it) {
        auto [i,j,i2,j2] = *it;
        for(unsigned int k=0; k<A[i].size(); k++) {
            if(A[i][k] && B[k][j]) {
                A[i2][k] = 1;
                B[k][j2] = 1;
            }
        }
    }

    unsigned int cost = UnderCover::calculerCost(A, B, Minit);

    if(verbosity) {
        std::cout << "Accuracy (#err/size) : " << 1.0 - ( cost / (double) Minit.nbOnes() ) << std::endl;
        std::cout << "Accuracy on one (#err/#ones) : " << 1.0 - ( cost / (double) (Minit.nbOnes()+ Minit.nbZeros()) ) << std::endl;
        std::cout << "================" << std::endl;
    }


    // Dataset #line #col #ones #afterUni #afterExi timeUni timeExi rank TotalTime

    if(verbosity) {
        std::cout << "Dataset\t#line\t#col\t#ones\t#afterUni\t#afterExi\ttimeUni\ttimeExi\trank after fastBMF\t#Error\tTotalTime" << std::endl;
    }
    std::cout << file << '\t' << Minit.nbLine() << '\t' << Minit.nbCol() << '\t' << Minit.nbOnes() << '\t' << nbOnesAfterUni << '\t' << nbOnesAfterExi << '\t' << timeUni << '\t' << timeExi << '\t' << B.size() << '\t' << cost << '\t' << C_TOTAL.tacSec() << "sec" << std::endl;


    if(output.size()) {
        std::string fileA = output;
        fileA.append(".A.csv");
        UnderCover::exportToCSV(A, fileA);

        std::string fileB = output;
        fileB.append(".B.csv");
        UnderCover::exportToCSV(B, fileB);
    }

    if(productOutput.size()) {
        std::string file = output;
        file.append(".csv");
        UnderCover::exportToCSV(UnderCover::mult(A, B), file);
    }

    return 0;
}









