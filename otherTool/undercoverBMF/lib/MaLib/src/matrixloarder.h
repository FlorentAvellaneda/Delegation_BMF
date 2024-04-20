#ifndef MATRIXLOARDER_H
#define MATRIXLOARDER_H

#include <vector>
#include <set>
#include <regex>

#include "CSV.h"


class MatrixLoarder {

    std::filebuf * fb=nullptr;

    std::istream * _is=nullptr;
    boost::escaped_list_separator<char> _els;

    std::vector<std::string> vLine;

    bool end=false;

    public:

        MatrixLoarder(std::string fileName, char sep=',', char quote='\"', char escape='\\')
            : _els(escape, sep, quote) {

              fb = new std::filebuf();
              if (fb->open (fileName,std::ios::in))
              {
                _is = new std::istream(fb);
              } else {
                std::cerr << "Lecture du fichier " << fileName << " imposible." << std::endl;
              }
        }

        ~MatrixLoarder() {
            delete _is;
            delete fb;
        }

        std::vector<std::vector<std::optional<bool>>> getMatrix() {
            std::vector<std::vector<std::optional<bool>>> result;

            bool noMoreLine=false;
            while(next()) {
                if(vLine.size() == 0) {
                    noMoreLine=true;
                    continue;
                }
                if(noMoreLine == true) {
                    std::cerr << "Error in the format of the input matrix. A row was detected an empty string." << std::endl;
                    exit(0);
                }
                result.push_back(std::vector<std::optional<bool>>());
                for(auto s: vLine) {
                    if(std::regex_replace(s, std::regex(" "), "").size() == 0) {
                        result.back().push_back({}); // Missing value
                        continue;
                    }

                    std::istringstream iss (s);
                    int number;
                    iss >> number;
                    if (!iss.fail()) {
                        if(number == 0) {
                            result.back().push_back(0); // False value
                            continue;
                        } else if(number == 1) {
                            result.back().push_back(1); // True value
                            continue;
                        } else {
                            std::cerr << "The input matrix must be binary. The number \"" << s << "\" is not accepted." << std::endl;
                            exit(0);
                        }
                    } else {
                        std::cerr << "The input matrix must be binary. The input \"" << s << "\" is not accepted." << std::endl;
                        exit(0);
                    }

                }
                if(result.back().size() != result[0].size()) {
                    std::cerr << "Error in the dimensions of the matrix. Some rows have more entries than others." << std::endl;
                    exit(0);
                }
                assert(result.back().size() == result[0].size());
            }

            return result;
        }

        std::vector<std::tuple<unsigned int, unsigned int, unsigned int>> getADJ() {
            std::vector<std::tuple<unsigned int, unsigned int, unsigned int>> result;

            while(next()) {
                if(vLine.size() == 0) {
                    continue;
                }
                assert(vLine.size() >= 3);

                unsigned int i = std::stoi(vLine[0]);
                unsigned int j = std::stoi(vLine[1]);
                unsigned int v = std::stoi(vLine[2]);

                result.push_back({i, j, v});
            }

            return result;
        }




        std::vector<std::string> getLine() {
            return vLine;
        }

        bool next() {
            std::string line;

            if(!std::getline(*_is, line)) {
                end = true;
                return false;
            }

            vLine = parseLine(line);

            end = false;
            return true;
        }



private:
    std::vector<std::string> parseLine(std::string &s) {
        std::vector<std::string> result;

        for(unsigned int i=1; i<s.size(); ) {
            if(s[i] == '"') {
                if( s[i-1] == '"') {
                    s[i-1] = '\\';
                    i+=2;
                } else {
                    i+=1;
                }
            } else {
                i+=2;
            }
        }

        boost::tokenizer<boost::escaped_list_separator<char>> tok(s, _els);

        for(auto c: tok)
            result.push_back(c);

        return result;
    }


};


template<class T>
class MonIterator {
    T _begin;
    T _end;
public:
    MonIterator(T begin) : _begin(begin), _end(begin.end()){

    }

    T begin() const {
        return _begin;
    }
    T end() const {
        return _end;
    }
};


/*

class MaMatriceADJ {

    std::vector<std::set<unsigned int>> oneOnLine;
    std::vector<std::set<unsigned int>> zeroOnLine;

    std::vector<std::set<unsigned int>> oneOnCol;
    std::vector<std::set<unsigned int>> zeroOnCol;


    class Iterator {
       const std::vector<std::set<unsigned int>>* data;
       std::set<unsigned int>::iterator it;
       unsigned int position;

    public:

       Iterator(const std::vector<std::set<unsigned int>>* _data)
           : data(_data), position(0) {

           for(position = 0; position < data->size(); position++) {
               if( (*data)[position].size() ) {
                   it = (*data)[position].begin();
                   break;
               }
           }
       }

       Iterator end() {
           Iterator result = *this;
           result.position = data->size();
           return result;
       }

       const std::tuple<unsigned int, unsigned int> operator*() const {
           return {position, *it};
       }

       Iterator& operator++() {

           if(++it == (*data)[position].end()) {
               do {
                   position++;
                   if( position >= (*data).size() )
                       break;
                   it = (*data)[position].begin();
               } while(it == (*data)[position].end());
           }

           return *this;
       }

       bool operator!=(const Iterator& it) const {
           if(this->position != it.position)
               return true;
           if( position >= data->size() )
               return false;

           return (this->position != it.position);
       }

    };


public:


    MonIterator<Iterator> Ones() const {
        return MonIterator<Iterator>( Iterator(&oneOnLine) );
    }

    MonIterator<Iterator> Zeros() const {
        return MonIterator<Iterator>( Iterator(&zeroOnLine) );
    }

    auto& OneOnLine(unsigned int i) const {
        assert(i < oneOnLine.size());
        return oneOnLine[i];
    }

    auto& ZeroOnLine(unsigned int j) const {
        assert(j < zeroOnLine.size());
        return zeroOnLine[j];
    }

    auto& OneOnCol(unsigned int i) const {
        assert(i < oneOnCol.size());
        return oneOnCol[i];
    }

    auto& ZeroOnCol(unsigned int j) const {
        assert(j < zeroOnCol.size());
        return zeroOnCol[j];
    }

    std::optional<bool> get(unsigned int i, unsigned int j) {
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

    void add(unsigned int i, unsigned int j, bool val) {
        if(i >= oneOnLine.size()) {
            oneOnLine.resize(i+1);
            zeroOnLine.resize(i+1);
//            activeLine.resize(i+1, true);
        }

        if(j >= oneOnCol.size()) {
            oneOnCol.resize(j+1);
            zeroOnCol.resize(j+1);
//            activeCol.resize(j+1, true);
        }


        if(val) {
            oneOnLine[i].insert(j);
            oneOnCol[j].insert(i);
        } else {
            zeroOnLine[i].insert(j);
            zeroOnCol[j].insert(i);
        }
    }

    void remove(unsigned int i, unsigned int j) {
        assert(i < oneOnLine.size());
        assert(j < oneOnCol.size());

        oneOnLine[i].erase(j);
        zeroOnLine[i].erase(j);
        oneOnCol[j].erase(i);
        zeroOnCol[j].erase(i);
    }

    unsigned int m() const {
//        return activeLine.nbActive();
        return oneOnLine.size();
    }

    unsigned int n() const {
//        return activeCol.nbActive();
        return oneOnCol.size();
    }




};

*/



class MaMatriceADJ {

    std::vector<std::set<unsigned int>> oneOnLine;
    std::vector<std::set<unsigned int>> zeroOnLine;

    std::vector<std::set<unsigned int>> oneOnCol;
    std::vector<std::set<unsigned int>> zeroOnCol;

    unsigned int nbOne=0;
    unsigned int nbZero=0;

//    std::map< std::tuple<unsigned int, unsigned int>, unsigned int> weight;

    class Iterator {
       const std::vector<std::set<unsigned int>>* data;
       std::set<unsigned int>::iterator it;
       unsigned int position;

    public:

       Iterator(const std::vector<std::set<unsigned int>>* _data)
           : data(_data), position(0) {

           for(position = 0; position < data->size(); position++) {
               if( (*data)[position].size() ) {
                   it = (*data)[position].begin();
                   break;
               }
           }
       }

       Iterator end() {
           Iterator result = *this;
           result.position = data->size();
           return result;
       }

       const std::tuple<unsigned int, unsigned int> operator*() const {
           return {position, *it};
       }

       Iterator& operator++() {

           if(++it == (*data)[position].end()) {
               do {
                   position++;
                   if( position >= (*data).size() )
                       break;
                   it = (*data)[position].begin();
               } while(it == (*data)[position].end());
           }

           return *this;
       }

       bool operator!=(const Iterator& it) const {
           if(this->position != it.position)
               return true;
           if( position >= data->size() )
               return false;

           return (this->position != it.position);
       }

    };


public:


    unsigned int numberOfOne() const {
        return nbOne;
    }

    unsigned int numberOfZero() const {
        return nbZero;
    }

    void print() {
        std::cout << "M:\n";
        for(unsigned int i=0; i<m(); i++) {
            for(unsigned int j=0; j<n(); j++) {
                if(oneOnLine[i].count(j)) {
                    std::cout << "1";
                } else if(zeroOnLine[i].count(j)) {
                    std::cout << "0";
                } else {
                    std::cout << ".";
                }
            }
            std::cout << "\n";
        }
        std::cout << std::endl;
    }

    MonIterator<Iterator> Ones() const {
        return MonIterator<Iterator>( Iterator(&oneOnLine) );
    }

    MonIterator<Iterator> Zeros() const {
        return MonIterator<Iterator>( Iterator(&zeroOnLine) );
    }

    auto& OneOnLine(unsigned int i) const {
        assert(i < oneOnLine.size());
        return oneOnLine[i];
    }

    auto& ZeroOnLine(unsigned int j) const {
        assert(j < zeroOnLine.size());
        return zeroOnLine[j];
    }

    auto& OneOnCol(unsigned int i) const {
        assert(i < oneOnCol.size());
        return oneOnCol[i];
    }

    auto& ZeroOnCol(unsigned int j) const {
        assert(j < zeroOnCol.size());
        return zeroOnCol[j];
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

    void resize(unsigned int m, unsigned int n) {
        oneOnLine.resize(m);
        zeroOnLine.resize(m);

        oneOnCol.resize(n);
        zeroOnCol.resize(n);
    }

/*
    void add(unsigned int i, unsigned int j) {
        if(i >= oneOnLine.size()) {
            oneOnLine.resize(i+1);
            zeroOnLine.resize(i+1);
//            activeLine.resize(i+1, true);
        }

        if(j >= oneOnCol.size()) {
            oneOnCol.resize(j+1);
            zeroOnCol.resize(j+1);
//            activeCol.resize(j+1, true);
        }
    }
*/
    void add(unsigned int i, unsigned int j, bool val) {
        if(i >= oneOnLine.size()) {
            oneOnLine.resize(i+1);
            zeroOnLine.resize(i+1);
//            activeLine.resize(i+1, true);
        }

        if(j >= oneOnCol.size()) {
            oneOnCol.resize(j+1);
            zeroOnCol.resize(j+1);
//            activeCol.resize(j+1, true);
        }




        if(val) {
            assert(zeroOnCol[j].count(i) == 0);
            assert(zeroOnLine[i].count(j) == 0);

            if(oneOnLine[i].count(j) == 0) {
                oneOnLine[i].insert(j);
                oneOnCol[j].insert(i);
                nbOne++;
            }
        } else {
            assert(oneOnLine[i].count(j) == 0);
            assert(oneOnCol[j].count(i) == 0);

            if(zeroOnLine[i].count(j) == 0) {
                zeroOnLine[i].insert(j);
                zeroOnCol[j].insert(i);
                nbZero++;
            }
        }

        //weight[{i,j}] = 1;//1024; //////// HyperParametre
    }

    void substractWeight(unsigned int i, unsigned int j, unsigned int w) {
        /*
        if( w > weight[{i,j}]) {
            assert(false);
        }
        weight[{i,j}] -= w;
        */

        //if(weight[{i,j}] == 0) {
            if(oneOnLine[i].count(j)) {
                nbOne--;
            }
            if(zeroOnLine[i].count(j)) {
                nbZero--;
            }

            oneOnLine[i].erase(j);
            zeroOnLine[i].erase(j);
            oneOnCol[j].erase(i);
            zeroOnCol[j].erase(i);
        //}
    }

    void remove(unsigned int i, unsigned int j) {
        assert(i < oneOnLine.size());
        assert(j < oneOnCol.size());

        /*
        if( weight[{i,j}] == 0 ) {
            assert(oneOnLine[i].count(j) == 0);
            return;
        }
        weight[{i,j}] /= 2;
        if(weight[{i,j}] > 0)
            return;
        */

        if(oneOnLine[i].count(j)) {
            nbOne--;
        }
        if(zeroOnLine[i].count(j)) {
            nbZero--;
        }

        oneOnLine[i].erase(j);
        zeroOnLine[i].erase(j);
        oneOnCol[j].erase(i);
        zeroOnCol[j].erase(i);
    }

    void removeCompletly(unsigned int i, unsigned int j) {
        assert(i < oneOnLine.size());
        assert(j < oneOnCol.size());
        /*
        if( weight[{i,j}] == 0 ) {
            assert(oneOnLine[i].count(j) == 0);
            return;
        }
        weight[{i,j}] = 0;
        */

        if(oneOnLine[i].count(j)) {
            nbOne--;
        }
        if(zeroOnLine[i].count(j)) {
            nbZero--;
        }

        oneOnLine[i].erase(j);
        zeroOnLine[i].erase(j);
        oneOnCol[j].erase(i);
        zeroOnCol[j].erase(i);
    }

    unsigned int getWeight(unsigned i, unsigned j) {
        ///return weight[{i,j}];
        return 1;
    }


    unsigned int m() const {
//        return activeLine.nbActive();
        return oneOnLine.size();
    }

    unsigned int n() const {
//        return activeCol.nbActive();
        return oneOnCol.size();
    }

    void clearLine(unsigned int i) {
        for(auto j: oneOnLine[i]) {
            assert( oneOnCol[j].count(i) );
            oneOnCol[j].erase(i);
        }
        nbOne -= oneOnLine[i].size();
        oneOnLine[i].clear();

        for(auto j: zeroOnLine[i]) {
            assert( zeroOnCol[j].count(i) );
            zeroOnCol[j].erase(i);
        }
        zeroOnLine[i].clear();
    }

    void clearCol(unsigned int j) {
        for(auto i: oneOnCol[j]) {
            assert( oneOnLine[i].count(j) );
            oneOnLine[i].erase(j);
            nbOne--;
        }
        nbZero -= oneOnCol[j].size();
        oneOnCol[j].clear();

        for(auto i: zeroOnCol[j]) {
            assert( zeroOnLine[i].count(j) );
            zeroOnLine[i].erase(j);
        }
        zeroOnCol[j].clear();
    }

    void clear() {
        oneOnLine.clear();
        zeroOnLine.clear();
        oneOnCol.clear();
        zeroOnCol.clear();
        nbOne=0;
        nbZero=0;
    }
};




class MaMatriceADJXXX {

    std::vector<std::set<unsigned int>> oneOnLine;
    std::vector<std::set<unsigned int>> zeroOnLine;

    std::vector<std::set<unsigned int>> oneOnCol;
    std::vector<std::set<unsigned int>> zeroOnCol;

    unsigned int nbOne=0;
    unsigned int nbZero=0;


    class Iterator {
       const std::vector<std::set<unsigned int>>* data;
       std::set<unsigned int>::iterator it;
       unsigned int position;

    public:

       Iterator(const std::vector<std::set<unsigned int>>* _data)
           : data(_data), position(0) {

           for(position = 0; position < data->size(); position++) {
               if( (*data)[position].size() ) {
                   it = (*data)[position].begin();
                   break;
               }
           }
       }

       Iterator end() {
           Iterator result = *this;
           result.position = data->size();
           return result;
       }

       const std::tuple<unsigned int, unsigned int> operator*() const {
           return {position, *it};
       }

       Iterator& operator++() {

           if(++it == (*data)[position].end()) {
               do {
                   position++;
                   if( position >= (*data).size() )
                       break;
                   it = (*data)[position].begin();
               } while(it == (*data)[position].end());
           }

           return *this;
       }

       bool operator!=(const Iterator& it) const {
           if(this->position != it.position)
               return true;
           if( position >= data->size() )
               return false;

           return (this->position != it.position);
       }

    };


public:

    unsigned int numberOfOne() {
        return nbOne;
    }

    unsigned int numberOfZero() {
        return nbZero;
    }


    unsigned int getWeight(unsigned int i, unsigned int j) {
        return 1;
    }

    void print() {
        std::cout << "M:\n";
        for(unsigned int i=0; i<m(); i++) {
            for(unsigned int j=0; j<n(); j++) {
                if(oneOnLine[i].count(j)) {
                    std::cout << "1";
                } else if(zeroOnLine[i].count(j)) {
                    std::cout << "0";
                } else {
                    std::cout << ".";
                }
            }
            std::cout << "\n";
        }
        std::cout << std::endl;
    }

    MonIterator<Iterator> Ones() const {
        return MonIterator<Iterator>( Iterator(&oneOnLine) );
    }

    MonIterator<Iterator> Zeros() const {
        return MonIterator<Iterator>( Iterator(&zeroOnLine) );
    }

    auto& OneOnLine(unsigned int i) const {
        assert(i < oneOnLine.size());
        return oneOnLine[i];
    }

    auto& ZeroOnLine(unsigned int j) const {
        assert(j < zeroOnLine.size());
        return zeroOnLine[j];
    }

    auto& OneOnCol(unsigned int i) const {
        assert(i < oneOnCol.size());
        return oneOnCol[i];
    }

    auto& ZeroOnCol(unsigned int j) const {
        assert(j < zeroOnCol.size());
        return zeroOnCol[j];
    }

    std::optional<bool> get(unsigned int i, unsigned int j) {
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

    void add(unsigned int i, unsigned int j, bool val) {
        if(i >= oneOnLine.size()) {
            oneOnLine.resize(i+1);
            zeroOnLine.resize(i+1);
//            activeLine.resize(i+1, true);
        }

        if(j >= oneOnCol.size()) {
            oneOnCol.resize(j+1);
            zeroOnCol.resize(j+1);
//            activeCol.resize(j+1, true);
        }


        assert(oneOnLine[i].count(j) == 0);
        assert(zeroOnLine[i].count(j) == 0);
        assert(oneOnCol[j].count(i) == 0);
        assert(zeroOnCol[j].count(i) == 0);


        if(val) {
            oneOnLine[i].insert(j);
            oneOnCol[j].insert(i);
            nbOne++;
        } else {
            zeroOnLine[i].insert(j);
            zeroOnCol[j].insert(i);
            nbZero++;
        }

    }

    void remove(unsigned int i, unsigned int j) {
        assert(i < oneOnLine.size());
        assert(j < oneOnCol.size());

        if(oneOnLine[i].count(j)) {
            nbOne--;
        }
        if(zeroOnLine[i].count(j)) {
            nbZero--;
        }

        oneOnLine[i].erase(j);
        zeroOnLine[i].erase(j);
        oneOnCol[j].erase(i);
        zeroOnCol[j].erase(i);
    }

    unsigned int m() const {
//        return activeLine.nbActive();
        return oneOnLine.size();
    }

    unsigned int n() const {
//        return activeCol.nbActive();
        return oneOnCol.size();
    }

    void clearLine(unsigned int i) {
        for(auto j: oneOnLine[i]) {
            assert( oneOnCol[j].count(i) );
            oneOnCol[j].erase(i);
        }
        nbOne -= oneOnLine[i].size();
        oneOnLine[i].clear();

        for(auto j: zeroOnLine[i]) {
            assert( zeroOnCol[j].count(i) );
            zeroOnCol[j].erase(i);
        }
        zeroOnLine[i].clear();
    }

    void clearCol(unsigned int j) {
        for(auto i: oneOnCol[j]) {
            assert( oneOnLine[i].count(j) );
            oneOnLine[i].erase(j);
            nbOne--;
        }
        nbZero -= oneOnCol[j].size();
        oneOnCol[j].clear();

        for(auto i: zeroOnCol[j]) {
            assert( zeroOnLine[i].count(j) );
            zeroOnLine[i].erase(j);
        }
        zeroOnCol[j].clear();
    }

};


class Matrix {
    std::vector<std::vector<std::optional<bool>>> data;
public:

    Matrix(const std::vector<std::vector<std::optional<bool>>> &data) : data(data) {

    }

    std::optional<bool> operator()(unsigned int i, unsigned int j) const {
        return data[i][j];
    }

    void substract(const std::vector<std::vector<bool>> &m) {
        assert(m.size() == data.size());

        for(unsigned int i=0; i<m.size(); i++) {
            assert(m[i].size() == data[i].size());
            for(unsigned int j=0; j<m[i].size(); j++) {
                if(m[i][j]) {
                    data[i][j] = {};
                }
            }
        }
    }

    void remove(unsigned int i, unsigned int j) {
        data[i][j] = {};
    }

    unsigned int m() const {
        return data.size();
    }

    unsigned int n() const {
        return data[0].size();
    }

    void print() {
        for(unsigned int i=0; i<data.size(); i++) {
            for(unsigned int j=0; j<data[i].size(); j++) {
                if(data[i][j].has_value()) {
                    std::cout << data[i][j].value();
                } else {
                    std::cout << ".";
                }
            }
            std::cout << std::endl;
        }
    }
};


#endif // MATRIXLOARDER_H
