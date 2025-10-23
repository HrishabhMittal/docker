#include <fstream>
#include <ios>
#include <string>
#include <sched.h>
#include <functional>
#include <sys/wait.h>
#include <iostream>
#include <unistd.h>
#include <filesystem>
int sandbox(int& child_pid,const std::string& chroot_dir,const std::function<void()>& setup_func,const std::function<void()>& child_func,const std::function<void()>& parent_func) {
    unshare(CLONE_NEWUSER|CLONE_NEWPID|CLONE_NEWNS);
    setup_func();
    child_pid=fork();
    if (child_pid==-1) {
        std::cerr<<"Fork failed"<<std::endl;
        return 1;
    } else if (child_pid == 0) {
        if (chroot(chroot_dir.c_str())) {
            perror("Error, could not chroot to specified directory");
            exit(1);
        }
        if (chdir("/")) {
            perror("Error, could not change directory to / after chroot");
            exit(1);
        }
        child_func();
        exit(0);
    } else {
        int status;
        parent_func();
        waitpid(child_pid,&status,0);
        return status;
    }
}
int exec(const std::vector<std::string>& args) {
    if (args.empty()) return -1;
    std::vector<char*> cstrs;
    for (auto& s:args) {
        cstrs.push_back(const_cast<char*>(s.c_str()));
    }
    cstrs.push_back(nullptr);
    if (execv(cstrs[0],cstrs.data())==-1) {
        perror("execv failed");
        std::cout<<"Failed to execute: "<<args[0]<<std::endl;
        return 1;
    }
    return 0;
}
class temp_dir {
public:
    std::string path;
    temp_dir(std::string template_path) {
        path=mkdtemp(template_path.data());
        if (path.empty()) {
            throw std::runtime_error("Failed to create temporary directory");
        }
    }
    ~temp_dir() {
        // std::filesystem::remove_all(path);
    }    
};