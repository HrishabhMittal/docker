#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include "lexer.cpp"

class Parser {
    Lexer lexer;
    std::string chroot_dir;
    int cur=0;
    void parseStatement() {

    }
public:
    Parser(const std::string& filename,const std::string& chroot_dir): lexer(filename),chroot_dir(chroot_dir) {
        while (cur<lexer.tokens.size()) {
            parseStatement();
        }
    }
};