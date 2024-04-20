#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <ostream>
#include <unordered_set>
#include <vector>
#include <set>
#include <limits>
#include <optional>



template <class T>
class SortableContainer {
    std::vector<T> _vData;
    T max = std::numeric_limits<T>::min();
public:
    void insert(const T& e) {
        _vData.push_back(e);
        if(max < e)
            max = e;
    }

    void sortSet() {
        std::vector<T> tmp(max+1);

        for(auto &e: _vData) {
            tmp[e] = 1;
        }

        _vData.clear();
        for(unsigned int i=0; i<tmp.size(); i++) {
            if(tmp[i]) {
                _vData.push_back(i);
            }
        }
    }

    auto begin() {
        return _vData.begin();
    }

    auto end() {
        return _vData.end();
    }
};


template <class T>
class VectSet {

    struct Iterator {

        std::optional< typename std::vector<T>::const_iterator> it_vec;
        std::optional< typename std::unordered_set<T>::const_iterator > it_set;

        Iterator(typename std::vector<T>::const_iterator it_vec) : it_vec(it_vec) {

        }
        Iterator(typename std::unordered_set<T>::const_iterator it_set) : it_set(it_set) {

        }

        const T& operator*() {
            if(it_vec.has_value())
                return (*(it_vec.value()));
            return *(it_set.value());
        }

        void operator++() {
            if(it_vec.has_value())
                ++(it_vec.value());
            else
                ++(it_set.value());
        }

        bool operator!=(const Iterator & it2) {
            if(it_vec.has_value())
                return it_vec.value() != it2.it_vec.value();
            else
                return it_set.value() != it2.it_set.value();
        }
    };

    std::vector<T> _vData;
    std::unordered_set<T> _sData;
public:
    void insert(const T &e) {
        if(_sData.size()) {
            _sData.insert(e);
            return;
        }
        _vData.push_back(e);

        if( _vData.size() > 1000) {
            _sData.insert(_vData.begin(), _vData.end());
            _vData.clear();
        }
    }

    void erase(const T &e) {
        if(_vData.size()) {
            for(unsigned int i=0; i<_vData.size(); i++) {
                if(e == _vData[i]) {
                    _vData[i] = _vData.back();
                    _vData.pop_back();
                    return;
                }
            }
            return;
        }
        _sData.erase(e);
    }

    int count(const T &e) const {
        if(_sData.size()) {
            return _sData.count(e);
        }
        for(auto &e2: _vData) {
            if(e == e2)
                return 1;
        }
        return 0;
    }

    Iterator begin() const {
        if(_vData.size())
            return Iterator(_vData.begin());
        return Iterator(_sData.begin());
    }

    Iterator end() const {
        if(_vData.size())
            return Iterator(_vData.end());
        return Iterator(_sData.end());

    }

    unsigned int size() const {
        if(_sData.size()) {
            return _sData.size();
        }
        return _vData.size();
    }
};


class Matrix
{
    std::vector< VectSet<unsigned int> > zeroOnLine; // zeroOnLine[i] = {all columns that have a 0 on the line i}
    std::vector< VectSet<unsigned int> > zeroOnCol; // zeroOncol[j] = {all lines that have a 0 in the column j}

    std::vector< VectSet<unsigned int> > oneOnLine; // zeroOnLine[i] = {all columns that have a 1 on the line i}
    std::vector< VectSet<unsigned int> > oneOnCol; // zeroOncol[j] = {all lines that have a 1 in the column j}

    std::map< std::tuple<unsigned int, unsigned int>, unsigned int> weight;
    std::vector< std::tuple<unsigned int, unsigned int, unsigned int, unsigned int> > inclusionDone;

public:
    Matrix() {

    }
    Matrix(unsigned int m, unsigned int n) {
        zeroOnLine.resize(m);
        oneOnLine.resize(m);


        zeroOnCol.resize(n);
        oneOnCol.resize(n);
    }
    ~Matrix() {

    }

    const unsigned int getWeight(unsigned int i, unsigned j) const {
        return weight.at({i,j});
    }

    const VectSet<unsigned int>& getZeroOnLine(unsigned int i) const {
        return zeroOnLine[i];
    }

    const VectSet<unsigned int>& getOneOnLine(unsigned int i) const {
        return oneOnLine[i];
    }

    const unsigned int getWeightLine(unsigned int i) const {
        unsigned int result=0;
        for(auto j: oneOnLine[i]) {
            result += getWeight(i,j);
        }
        return result;
    }

    const unsigned int getWeightCol(unsigned int j) const {
        unsigned int result=0;
        for(auto i: oneOnCol[j]) {
            result += getWeight(i,j);
        }
        return result;
    }

    const VectSet<unsigned int>& getZeroOnCol(unsigned int j) const {
        return zeroOnCol[j];
    }

    const VectSet<unsigned int>& getOneOnCol(unsigned int j) const {
        return oneOnCol[j];
    }

    bool noOne() const {
        for(unsigned int i=0; i<nbLine(); i++) {
            if(getOneOnLine(i).size())
                return false;
        }

        return true;
    }

    std::optional<bool> get(unsigned int i, unsigned int j) const {
        if(i >= oneOnLine.size())
            return {};
        if(oneOnLine[i].count(j)) {
            return 1;
        }
        if(zeroOnLine[i].count(j)) {
            return 0;
        }
        return {};
    }

    unsigned int nbLine() const {
        return oneOnLine.size();
    }
    unsigned int nbCol() const {
        return oneOnCol.size();
    }

    unsigned int numberWithValue() const {
        unsigned int result = 0;

        for(unsigned int i=0; i<nbLine(); i++) {
            result += getOneOnLine(i).size() + getZeroOnLine(i).size();
        }

        return result;
    }

    unsigned int nbWeightedOnes() const {
        unsigned int result = 0;

        for(unsigned int i=0; i<nbLine(); i++) {
            for(unsigned int j: getOneOnLine(i)) {
                result += getWeight(i,j);
            }
        }

        return result;
    }

    unsigned int nbOnes() const {
        unsigned int result = 0;

        for(unsigned int i=0; i<nbLine(); i++) {
            for(unsigned int j: getOneOnLine(i)) {
                result += 1;//getWeight(i,j);
            }
        }

        return result;
    }

    unsigned int nbZeros() const {
        unsigned int result = 0;

        for(unsigned int i=0; i<nbLine(); i++) {
            for(unsigned int j: getZeroOnLine(i)) {
                result += 1;//getWeight(i,j);
            }
        }

        return result;
    }

    void erase(unsigned int i, unsigned int j) {
        if(zeroOnLine.size() <= i) {
            zeroOnLine.resize(i+1);
            oneOnLine.resize(i+1);
        }
        if(zeroOnCol.size() <= j) {
            zeroOnCol.resize(j+1);
            oneOnCol.resize(j+1);
        }


        oneOnLine[i].erase(j);
        oneOnCol[j].erase(i);
        zeroOnLine[i].erase(j);
        zeroOnCol[j].erase(i);

        weight.erase({i,j});
    }

    void remove(unsigned int i, unsigned int j) {
        if(zeroOnLine.size() <= i) {
            zeroOnLine.resize(i+1);
            oneOnLine.resize(i+1);
        }
        if(zeroOnCol.size() <= j) {
            zeroOnCol.resize(j+1);
            oneOnCol.resize(j+1);
        }

        oneOnLine[i].erase(j);
        oneOnCol[j].erase(i);
        zeroOnLine[i].erase(j);
        zeroOnCol[j].erase(i);
    }

    void add(unsigned int i, unsigned int j, bool val, unsigned int w=1) {
        if(zeroOnLine.size() <= i) {
            zeroOnLine.resize(i+1);
            oneOnLine.resize(i+1);
        }
        if(zeroOnCol.size() <= j) {
            zeroOnCol.resize(j+1);
            oneOnCol.resize(j+1);
        }

        if(val == 0) {
            zeroOnLine[i].insert(j);
            zeroOnCol[j].insert(i);

            oneOnLine[i].erase(j);
            oneOnCol[j].erase(i);
        } else {
            oneOnLine[i].insert(j);
            oneOnCol[j].insert(i);

            zeroOnLine[i].erase(j);
            zeroOnCol[j].erase(i);
        }

        weight[{i,j}] = w;
    }

    std::set< std::tuple<unsigned int, unsigned int> > verifyOnePossible;

    std::vector< std::tuple<unsigned int, unsigned int, unsigned int, unsigned int> > simplifyCol(bool approx=false) {
        std::vector< std::tuple<unsigned int, unsigned int, unsigned int, unsigned int> > result;

        for(unsigned int i=0; i<oneOnLine.size(); i++) {
            SortableContainer<unsigned int> all;
            for(auto j: oneOnLine[i]) {
                    for(auto &e: oneOnCol[j])
                        if(e != i)
                            all.insert(e);
            }
            all.sortSet();

            SortableContainer<unsigned int> line_not_biger_that_line_i;
            line_not_biger_that_line_i.insert(i);
            for(auto j: oneOnLine[i]) {
                for(auto &e: zeroOnCol[j])
                    line_not_biger_that_line_i.insert(e);
            }
            line_not_biger_that_line_i.sortSet();

            std::vector<unsigned int> included;
            std::set_difference(all.begin(), all.end(), line_not_biger_that_line_i.begin(), line_not_biger_that_line_i.end(), std::back_inserter(included));


            if(!approx) {
                // Remove \empty / 0
                for(unsigned int k=0; k<included.size(); ) {
                    bool incrementerK = true;
                    for(auto j: zeroOnLine[included[k]]) {
                        if(zeroOnLine[i].count(j) == 0) {

                            verifyOnePossible.insert({i,j});

                            included[k] = included.back();
                            included.pop_back();
                            incrementerK=false;
                            break;
                        }
                    }
                    if(incrementerK)
                        k++;
                }
            }

            if(included.size()) {

                for(auto i2: included) {
                    for(auto j: zeroOnLine[i2]) {
                        assert( oneOnLine[i].count(j) == 0 ); // because inclusion
                        if(zeroOnLine[i].count(j) == 0) {
                            zeroOnLine[i].insert(j);
                            zeroOnCol[j].insert(i);
                        }
                    }
                }

                for(auto j: oneOnLine[i]) {
                    for(auto i2: included) {
                        oneOnLine[i2].erase(j);
                        oneOnCol[j].erase(i2);

                        weight[{i,j}] += weight[{i2,j}];
                        weight[{i2,j}] = 0;
                                                        // If we replace by zero (as Iteress)
                                                        // zeroOnLine[i2].insert(j);
                                                        // zeroOnCol[j].insert(i2);

                        result.push_back({i,j,i2,j});
                    }
                }
            }
        }
        return result;
    }

    std::vector< std::tuple<unsigned int, unsigned int, unsigned int, unsigned int> > simplifyLine(bool approx=false) {
        std::vector< std::tuple<unsigned int, unsigned int, unsigned int, unsigned int> > result;

        for(unsigned int j=0; j<oneOnCol.size(); j++) {

            SortableContainer<unsigned int> all;
            for(auto i: oneOnCol[j]) {
                for(auto &e: oneOnLine[i])
                    if(e != j)
                        all.insert( e );
            }
            all.sortSet();

            SortableContainer<unsigned int> col_not_biger_that_col_j;
            col_not_biger_that_col_j.insert(j);
            for(auto i: oneOnCol[j]) {
                for(auto &e: zeroOnLine[i])
                    col_not_biger_that_col_j.insert(e);
            }
            col_not_biger_that_col_j.sortSet();

            std::vector<unsigned int> included;
            std::set_difference(all.begin(), all.end(), col_not_biger_that_col_j.begin(), col_not_biger_that_col_j.end(), std::back_inserter(included));
            if(!approx) {
                // Remove \empty / 0
                for(unsigned int k=0; k<included.size(); ) {
                    bool incrementerK = true;
                    for(auto i: zeroOnCol[included[k]]) {
                        if(zeroOnCol[j].count(i) == 0) {

                            verifyOnePossible.insert({i,j});


                            included[k] = included.back();
                            included.pop_back();
                            incrementerK=false;
                            break;
                        }
                    }
                    if(incrementerK)
                        k++;
                }
            }

            if(included.size()) {

                for(auto j2: included) {
                    for(auto i: zeroOnCol[j2]) {
                        assert( oneOnCol[j].count(i) == 0 ); // because inclusion
                        if(zeroOnCol[j].count(i) == 0) {
                            zeroOnCol[j].insert(i);
                            zeroOnLine[i].insert(j);
                        }
                    }
                }

                for(auto i: oneOnCol[j]) {
                    for(auto j2: included) {
                        oneOnCol[j2].erase(i);
                        oneOnLine[i].erase(j2);


                        weight[{i,j}] += weight[{i,j2}];
                        weight[{i,j2}] = 0;
                                                        // If we replace by zero (as Iteress)
                                                        // zeroOnCol[j2].insert(i);
                                                        // zeroOnLine[i].insert(j2);
                        result.push_back({i,j,i,j2});
                    }
                }
            }

        }

        return result;
    }

    std::vector< std::tuple<unsigned int, unsigned int, unsigned int, unsigned int> > simplify(bool approx=false) {

        std::vector< std::tuple<unsigned int, unsigned int, unsigned int, unsigned int> > implication;

        for(bool modify = true; modify; ) {
            modify = false;

            verifyOnePossible.clear();
            {
                auto tmp = simplifyLine(approx);
                if( tmp.size() ) {
                    modify = true;
                    implication.insert(implication.end(), tmp.begin(), tmp.end());
                }
            }

            {
                auto tmp = simplifyCol(approx);
                if( tmp.size() ) {
                    modify = true;
                    implication.insert(implication.end(), tmp.begin(), tmp.end());
                }
            }

        }

        return implication;
    }

    void afficher(bool all=true) {
        unsigned int nbOne = 0;

        std::cout << "Matrix:" << std::endl;
        for(unsigned int i=0; i<zeroOnLine.size(); i++) {
            for(unsigned int j=0; j<oneOnCol.size(); j++) {
                if(oneOnLine[i].count(j)) {
                    assert( zeroOnLine[i].count(j) == 0 );
                    if(all)
                        std::cout << weight[{i,j}] << "\t";
                    nbOne++;
                } else if(zeroOnLine[i].count(j)) {
                    if(all)
                        std::cout << "0\t";
                } else {
                    if(all)
                        std::cout << ".\t";
                }
            }
            if(all)
            std::cout << std::endl;
        }
        std::cout << "Number of ones = " << nbOne << std::endl;
    }
};


class BMF {
    std::vector< std::vector<bool> > Atr;
    std::vector< std::vector<bool> > B;

    std::vector<std::vector<unsigned int>> covered;
public:
    std::tuple< std::vector<std::vector<bool>>, std::vector<std::vector<bool>> > getAB() const {
        std::vector<std::vector<bool>> A(Atr[0].size());

        for(unsigned int k=0; k<Atr.size(); k++) {
            for(unsigned int i=0; i<Atr[k].size(); i++) {
                A[i].push_back( Atr[k][i] );
            }
        }

        return {A,B};
    }

    void addRank(const std::vector<bool> &colA, const std::vector<bool> &lineB, const Matrix &M) {

        assert(colA.size());
        assert(lineB.size());

        if(covered.size()==0) {
            covered.resize(colA.size(), std::vector<unsigned int>(lineB.size()));
        }

        Atr.push_back(colA);
        B.push_back(lineB);

        for(unsigned int i=0; i<colA.size(); i++) {
            if(colA[i]) {
                for(unsigned int j=0; j<lineB.size(); j++) {
                    if(lineB[j]) {
                        covered[i][j]++;
                    }
                }
            }
        }

        std::vector< std::tuple<unsigned int, bool> > tmp;
        for(unsigned int i=0; i<colA.size(); i++) {
            if(colA[i] == 0) {
                tmp.push_back({i,0});
            }
        }
        for(unsigned int j=0; j<lineB.size(); j++) {
            if(lineB[j] == 0) {
                tmp.push_back({j,1});
            }
        }

        std::random_shuffle(tmp.begin(), tmp.end());

        for(auto [a, m]: tmp) {
            if(!m) {
                unsigned int i = a;
                bool utile = false;
                bool possible = true;

                for(unsigned int j=0; j<B.back().size(); j++) {
                    if(B.back()[j]) {
                        if(M.get(i,j).value_or(false) == 1) {
                            if(get(i,j) <= 1 ) {
                                utile = true;
                            }
                        }

                        if(M.get(i,j).value_or(true) == 0) {
                            possible = false;
                            break;
                        }
                    }
                }
                if(utile && possible) {
                    Atr.back()[i] = 1;
                    for(unsigned int j=0; j<lineB.size(); j++) {
                        if(lineB[j]) {
                            covered[i][j]++;
                        }
                    }
                }

            } else {
                unsigned int j = a;

                bool utile = false;
                bool possible = true;
                for(unsigned int i=0; i<Atr.back().size(); i++) {
                    if(Atr.back()[i]) {
                        if(M.get(i,j).value_or(false) == 1) {
                            if(get(i,j) <= 1 ) {
                                utile = true;
                            }
                        }

                        if(M.get(i,j).value_or(true) == 0) {
                            possible = false;
                            break;
                        }
                    }
                }

                if(utile && possible) {
                    B.back()[j] = 1;
                    for(unsigned int i=0; i<colA.size(); i++) {
                        if(colA[i]) {
                            covered[i][j]++;
                        }
                    }
                }

            }
        }
    }


    void removeRank(unsigned int k) {
        for(unsigned int i=0; i<Atr[k].size(); i++) {
            if(Atr[k][i]) {
                for(unsigned int j=0; j<B[k].size(); j++) {
                    if(B[k][j]) {
                        covered[i][j]--;
                    }
                }
            }
        }

        Atr[k] = Atr.back();
        Atr.pop_back();
        B[k] = B.back();
        B.pop_back();
    }

    int get(unsigned int i, unsigned int j) const {
        if(i>=covered.size())
            return 0;
        if(j>=covered[i].size())
            return 0;
        return covered[i][j];
    }

    unsigned int rank() const {
        return Atr.size();
    }
};

#endif // MATRIX_HPP
