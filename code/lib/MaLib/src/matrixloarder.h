#ifndef MATRIXLOARDER_H
#define MATRIXLOARDER_H

#include <optional>
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


#endif // MATRIXLOARDER_H
