#pragma once
#include <stdexcept>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include "lexer.cpp"
#include "curl.cpp"
#include "utils.cpp"
class Parser {
    Lexer lexer;
    std::string chroot_dir,work_dir="/";
    int cur=0;
    std::vector<std::vector<std::string>> instructions;
    void expect(const std::string& token) {
        if (cur>=lexer.tokens.size()) {
            throw std::runtime_error("expected "+token+" instead of EOF");
        }
        if (lexer.tokens[cur++]!=token) {
            throw std::runtime_error("expected "+token+" instead of "+lexer.tokens[cur]);
        }
    }
    bool match(const std::string& token) {
        if (cur>=lexer.tokens.size()) return false;
        return lexer.tokens[cur]==token;
    }
    void next() {
        cur++;
    }
    bool match_and_next(const std::string& token) {
        if (cur>=lexer.tokens.size()) return false;
        bool out=lexer.tokens[cur]==token;
        if (out) next();
        return out;
    }
    std::string get() {
        if (cur>=lexer.tokens.size()) return "";
        return lexer.tokens[cur++];
    }
    void parseFrom() {
        expect("FROM");
        auto image=get();
        expect(":");
        auto tag=get();
        match_and_next("\n");
        std::string repository="library";
        if (image.find('/')!=std::string::npos) {
            repository=image.substr(0,image.find('/'));
            image=image.substr(image.find('/')+1);
        }
        std::cout<<"Pulling image "<<repository<<"/"<<image<<":"<<tag<<std::endl;
        PullDockerImage(repository+"/"+image+":"+tag,chroot_dir);
    }
    void parseWorkDir() {
        expect("WORKDIR");
        auto dir=get();
        match_and_next("\n");
        if (dir[0]=='/') {
            work_dir=dir;
        } else {
            if (work_dir.back()!='/') work_dir+="/";
            work_dir+=dir;
        }
        instructions.push_back({"WORKDIR",work_dir});
        std::cout<<"Setting working directory to "<<dir<<std::endl;
    }
    void parseCopy() {
        expect("COPY");
        auto src=get();
        auto dest=get();
        std::string full_dest;
        match_and_next("\n");
        if (dest[0]=='/') {
            full_dest=chroot_dir+dest;
        } else {
            if (work_dir.back()!='/') work_dir+="/";
            full_dest=chroot_dir+work_dir+dest;
        }
        std::filesystem::create_directories(std::filesystem::path(full_dest).parent_path());
        std::filesystem::copy(src,full_dest,std::filesystem::copy_options::recursive|std::filesystem::copy_options::overwrite_existing);
        std::cout<<"Copied "<<src<<" to "<<full_dest<<std::endl;
    }
    void parseRun() {
        expect("RUN");
        std::vector<std::string> cmd;
        while (cur<lexer.tokens.size()&&!match("\n")) {
            cmd.push_back(get());
        }
        match_and_next("\n");
        instructions.push_back(cmd);
    }
    void parseEntryPoint() {
        expect("ENTRYPOINT");
        std::vector<std::string> cmd;
        while (cur<lexer.tokens.size()&&!match("\n")) {
            cmd.push_back(get());
        }
        match_and_next("\n");
        cmd.push_back("ENTRYPOINT");
        instructions.push_back(cmd);
    }
    void parseStatement() {
        match_and_next("\n");
        if (match("FROM")) parseFrom();
        else if (match("WORKDIR")) parseWorkDir();
        else if (match("COPY")) parseCopy();
        else if (match("RUN")) parseRun();
        else if (match("ENTRYPOINT")) parseEntryPoint();
        else throw std::runtime_error("unknown statement: "+lexer.tokens[cur]);
    }
public:
    std::vector<std::vector<std::string>>& getInstructions() {
        return instructions;
    }
    Parser(const std::string& filename,const std::string& chroot_dir): lexer(filename),chroot_dir(chroot_dir) {
        while (cur<lexer.tokens.size()) {
            parseStatement();
        }
    }
};
