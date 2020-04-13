#include "net.h"
#include "utils.h"
#include <arpa/inet.h>

int InitListenSocket(const int port, int opt) {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    ErrorHandler(listen_fd, -1, "socket");

    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    struct sockaddr_in addr = {
        AF_INET,
        htons(port),
        INADDR_ANY
    };
    int ret = bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr));
    ErrorHandler(ret, -1, "bind");

    ret = listen(listen_fd, 128);
    ErrorHandler(ret, -1, "listen");

    return listen_fd;
}