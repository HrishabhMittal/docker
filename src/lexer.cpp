#pragma once
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

class Lexer {
    bool gettoken(std::ifstream& in) {
        std::string token;
        char c;
        while (in.get(c)) {
            if (isspace(c)) {
                if (!token.empty()) {
                    tokens.push_back(token);
                    token.clear();
                }
                if (c=='\n'&&(tokens.empty()||tokens.back()!="\n")) {
                    tokens.push_back("\n");
                }
                return true;
            } else if (c=='#') {
                while (in.get(c) && c!='\n');
                if (c=='\n'&&(tokens.empty()||tokens.back()!="\n")) {
                    tokens.push_back("\n");
                }
                if (!token.empty()) {
                    tokens.push_back(token);
                    token.clear();
                }
                return true;
            } else if (c=='"'||c=='\'') {
                if (!token.empty()) {
                    tokens.push_back(token);
                    token.clear();
                }
                char quote_char=c;
                std::string quoted_token;
                while (in.get(c)) {
                    if (c==quote_char&&(token.empty()||token.back()!='\\')) break;
                    quoted_token+=c;
                }
                tokens.push_back(quoted_token);
            } else if (c=='['||c==']'||c==','||c=='='||c==':'||c==';') {
                if (!token.empty()) {
                    tokens.push_back(token);
                    token.clear();
                }
                tokens.push_back(std::string(1,c));
            } else {
                token+=c;
            }
        }
        if (!token.empty()) {
            tokens.push_back(token);
            return true;
        }
        return false;
    }
public:
    std::vector<std::string> tokens;
    Lexer(const std::string& filename) {
        std::ifstream file(filename);
        while (gettoken(file));
        for (auto i:tokens) std::cout<<"\""<<i<<"\""<<std::endl;
        file.close();
    }
    
};
