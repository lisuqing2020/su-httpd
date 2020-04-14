#include "net.h"
#include "utils.h"
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

void ResponseHeader(int connect_fd, char *protocol, int status_code, char *status_description) {

    // 状态行：HTTP/1.1 200 OK
    char response_header[128];
    sprintf(response_header, "%s %d %s\r\n", protocol, status_code, status_description);

    // 报头 content-type
    sprintf(response_header + strlen(response_header), "content-type:%s\r\n", "text/html");

    // 报头 content-length
    sprintf(response_header + strlen(response_header), "content-length:%ld\r\n\r\n", -1);

    sprintf(response_header + strlen(response_header), "<h1>hello client!</h1>");

    // 回复客户端
    write(connect_fd, response_header, strlen(response_header));
}

void RequestHandler(int connect_fd) {
    
    // 接受客户端发送的数据 
    char buf[2048];
    int len = 0, count = 0;
    while((len = read(connect_fd, buf+count, sizeof(buf)-count)) > 0) {
        count += len;
    }

    // 如果不是读完了数据，返回
    if(!(len == -1 && errno == EAGAIN)) {
        return;
    }

    // 拿到请求行
    char method[8], path[128] = ".", protocol[8], tmp[128]; 
    sscanf(buf, "%s %s %s", method, tmp, protocol);
    strcat(path, tmp);

    // 如果是get方法
    if(strcasecmp(method, "get") == 0) {
        struct stat status;
        int ret = stat(method, &status);
        if(ret == -1) {
            ResponseHeader(connect_fd, protocol, 404, "Not Found");
            return;
        }

        if(S_ISREG(status.st_mode)) {   // path是个文件
            ResponseHeader(connect_fd, protocol, 200, "OK");
        } else if(S_ISDIR(status.st_mode)) {    // path是个目录
            printf("send directory\n");
        }
    }
}

int AcceptConnection(int listen_fd, int epfd) {

    // 接受连接，并创建connfd
    struct sockaddr_in client_addr;
    socklen_t size = sizeof(struct sockaddr);
    int connect_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &size);
    ErrorHandler(connect_fd, -1, "accept");

    // 打印客户端ip端口
    char client_ip[32];
    inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, sizeof(client_ip));
    printf("client ip: %s:%d\n", client_ip, ntohs(client_addr.sin_port));

    // 设置connfd非阻塞
    int flag = fcntl(connect_fd, F_GETFL);
    flag |= O_NONBLOCK;
    fcntl(connect_fd, F_SETFL, flag);

    // 将connfd挂到epoll树上，并设置边沿触发模式
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = connect_fd;
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, connect_fd, &ev);
    ErrorHandler(ret, -1, "epoll_ctl");

    return connect_fd;
}

void EpollRun(int listen_fd) {

    // 创建epoll tree
    int epfd = epoll_create(1024);

    // 将监听连接文件描述符添加到epoll树上
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = listen_fd;
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev);
    ErrorHandler(ret, -1, "epoll_ctl");

    // 监听文件描述符状态
    struct epoll_event events[1024];
    while(1) {
        ret = epoll_wait(epfd, events, sizeof(events)/sizeof(struct epoll_event), -1);
        ErrorHandler(ret, -1, "epoll_wait");

        for(int i = 0; i < ret; ++i) {
            int current_fd = events[i].data.fd;

            if(current_fd == listen_fd) {
                // 有新的客户端连接
                AcceptConnection(listen_fd, epfd);
            } else {                                   
                // 有新的客户端请求
                RequestHandler(current_fd); // 这里的current_fd是connect_fd
            }
        }
    }
}

int InitListenSocket(const int port, int opt) {

    // 创建一个监听用套接字
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    ErrorHandler(listen_fd, -1, "socket");

    // 设置端口复用 opt = 1 / 0
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    // 绑定端口
    struct sockaddr_in addr = {
        AF_INET,
        htons(port),
        INADDR_ANY
    };
    int ret = bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr));
    ErrorHandler(ret, -1, "bind");

    // 监听端口
    ret = listen(listen_fd, 128);
    ErrorHandler(ret, -1, "listen");

    return listen_fd;
}