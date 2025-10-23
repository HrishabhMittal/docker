#include <unistd.h>
enum class IO {
    IN,
    OUT,
    ERR
};
struct Pipe {
    int pipefd[2];
    Pipe() {
        pipe(pipefd);
    }
    void bind(IO s) {
        if (s==IO::IN) {
            dup2(pipefd[0], STDIN_FILENO);
        } else if (s==IO::OUT) {
            dup2(pipefd[1], STDOUT_FILENO);
        } else if (s==IO::ERR) {
            dup2(pipefd[1], STDERR_FILENO);
        }
    }
    void end_close(IO s) {
        if (s==IO::IN) {
            close(pipefd[0]);
        } else if (s==IO::OUT) {
            close(pipefd[1]);
        } else if (s==IO::ERR) {
            close(pipefd[1]);
        }
    }
    void setup_inpipe() {
        bind(IO::IN);
        end_close(IO::OUT);
    }
    void setup_outpipe() {
        end_close(IO::IN);
        bind(IO::OUT);
    }
};
