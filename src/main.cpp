#include "utils.cpp"
#include "pipe.cpp"
#include "parser.cpp"
#include <filesystem>
#include <iostream>
#include <ostream>
int main(int argc, char *argv[]) {
	Pipe p;
	int child_pid;
	temp_dir td("/tmp/chroot_env.XXXXXX");
	Parser parser("Dockerfile",td.path);
	int status=sandbox(child_pid,td.path,
	[&]() {
	},
	[&]() {
		p.setup_inpipe();
		int code=0;
		for (auto& instr:parser.getInstructions()) {
			if (!instr.empty()) {
				if (instr.back()=="ENTRYPOINT") {
					instr.pop_back();
					int code=exec(instr);
				} else if (instr[0]=="WORKDIR") {
					mkdir(instr[1].c_str(),0755);
					chdir(instr[1].c_str());
				} else {
					run(instr);
				}
			}
		}
		std::cout<<"Process exited with code: "<<code<<std::endl;
	},
	[&]() {
		p.setup_outpipe();
	});
	return status;
}
