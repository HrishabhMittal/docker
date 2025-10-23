#include "utils.cpp"
#include "pipe.cpp"
#include "parser.cpp"
#include <filesystem>
int main(int argc, char *argv[]) {
	Parser parser("Dockerfile");
	Pipe p;
	int child_pid;
	temp_dir td("/tmp/chroot_env.XXXXXX");
	std::filesystem::copy("./echo",std::filesystem::path(td.path)/"echo");
	int status=sandbox(child_pid,td.path,[&]() {
		p.setup_inpipe();
		int code=exec({"/echo", "Hello, World!"});
		std::cout<<"Process exited with code: "<<code<<std::endl;
	}, [&]() {
		p.setup_outpipe();
	});
	return status;
}
