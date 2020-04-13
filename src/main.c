#include "utils.h"
#include "net.h"

int main(int argc, char *argv[]) {
    int port = 80;
    char directory[256] = "./";

    ProgramParameterAnalysis(argc, argv, &port, directory);

    int listen_fd = InitListenSocket(port, 1);

    //epoll_create(1999); todo:epoll
    
    return 0;
}