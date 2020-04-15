#include "utils.h"
#include "net.h"
#include <unistd.h>

int main(int argc, char *argv[]) {
    int port = 80;
    char directory[256] = "./";

    if(ProgramParameterAnalysis(argc, argv, &port, directory)) {

        chdir(directory);
        int listen_fd = InitListenSocket(port, 1);  // 这里1端口复用 0不复用
        EpollRun(listen_fd);
    }

    return 0;
}