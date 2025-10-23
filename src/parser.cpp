#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include "lexer.cpp"

class Parser {
    Lexer lexer;
public:
    Parser(const std::string& filename): lexer(filename) {
        for (const auto& token : lexer.tokens) {
            std::cout << "Token: \'" << token <<'\''<< std::endl;
        }
    }
};