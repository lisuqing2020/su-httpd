#define _BSD_SOURCE
#define NOT_USE_404_HTML 1
#include "net.h"
#include "utils.h"
#include "fcgi.h"
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <dirent.h>
#include <stdlib.h>

void ResponsePhp(char *path, int connect_fd, char *parameter, char *method, char *content_length, char *content_type) {
    FastCgi_t *c;
    c = (FastCgi_t *)malloc(sizeof(FastCgi_t));
    FastCgi_init(c);
    setRequestId(c,1);
    startConnect(c);
    sendStartRequestRecord(c);
    sendParams(c, "SCRIPT_FILENAME",path);
    if(strcasecmp(method, "get") == 0) {
        sendParams(c, "REQUEST_METHOD","GET");
        sendParams(c, "CONTENT_LENGTH", "0"); //　0 表示没有　body
        sendParams(c, "CONTENT_TYPE", "text/html");
        sendParams(c, "QUERY_STRING", parameter);
        sendEndRequestRecord(c);
    } else if(strcasecmp(method, "post") == 0) {
        sendParams(c, "REQUEST_METHOD", "POST");
        sendParams(c, "CONTENT_LENGTH", content_length); //　17 为body的长度 !!!!
        sendParams(c, "CONTENT_TYPE", content_type);
        sendEndRequestRecord(c);

        /*FCGI_Header makeHeader(int type, int requestId,
                        int contentLength, int paddingLength)*/
        //设置type==5,为了发 body
        FCGI_Header t = makeHeader(FCGI_STDIN, c->requestId_, atoi(content_length), 0); //　17 为body的长度 !!!!
        send(c->sockfd_, &t, sizeof(t), 0);

        /*发送正式的 body */
        send(c->sockfd_, parameter, atoi(content_length), 0); //　17 为body的长度 !!!!

        //制造头告诉body结束　
        FCGI_Header endHeader;
        endHeader = makeHeader(FCGI_STDIN, c->requestId_, 0, 0);
        send(c->sockfd_, &endHeader, sizeof(endHeader), 0);
    }
    readFromPhp(c, connect_fd);
    FastCgi_finit(c);
}

void ResponseDirectory(int connect_fd, char *path) {
    char buf[4096] = {0};
    DirectoryPath(path);    // 处理路径最后的/

    // 以防万一，响应目录时也将connect_fd设置为阻塞
    int flag = fcntl(connect_fd, F_GETFL);
    flag &= ~O_NONBLOCK;
    fcntl(connect_fd, F_SETFL, flag);

    sprintf(buf, "<html><head><title>%s</title></head><body><table>", path);
    write(connect_fd, buf, strlen(buf));

    // 读取目录，把目录内容写进buf
    struct dirent **ptr = NULL;
    int amount = scandir(path, &ptr, NULL, alphasort);
    struct stat status;
    for(int i = 0; i < amount; ++i) {
        char *name = ptr[i] -> d_name;
        stat(name, &status);
        // path+1跳过前面的.符号 在之前的代码可以确认前面一定有.
        sprintf(buf, "<tr><td><a href=\"%s%s\">%s</a></td><td>%ld</td>", path+1, name, name, status.st_size);
        write(connect_fd, buf, strlen(buf));
    }
    sprintf(buf, "</table></body></html>");
    write(connect_fd, buf, strlen(buf));
    return;
}

int ResponseFile(int connect_fd, char *file_name) {

#if NOT_USE_404_HTML
    if(strcmp(file_name, "./404.html") == 0) {
        char buf[256] = {0};
        sprintf(buf, "<html><head><title>404 Not Found</title></head><body><center><h1>404 Not Found</h1></center><hr><center>su-httpd</center></body></html>");
        write(connect_fd, buf, strlen(buf));
        return 0;
    }
#endif

    int fd = open(file_name, O_RDONLY);
    if(fd == -1) {
        perror("open");
        return -1;
    }

    int len = 0;
	char buf[2048];

    // 如果connect_fd是非阻塞的，那么缓冲区满了就会下一次读写，这样图片显示不完整
    // 设置为阻塞，就可以避免出错，这里不用再设置为非阻塞，因为这个函数走完，connect_fd就被关闭了
    int flag = fcntl(connect_fd, F_GETFL);
    flag &= ~O_NONBLOCK;
    fcntl(connect_fd, F_SETFL, flag);
	while ((len = read(fd, buf, sizeof(buf))) > 0) {
		write(connect_fd, buf, len);
	}
	close(fd);
    return 0;
}

void ResponseHeader(int connect_fd, char *protocol, int status_code, char *status_description, size_t size, char *path) {

    // 状态行：HTTP/1.1 200 OK
    char response_header[128] = {0};
    sprintf(response_header, "%s %d %s\r\n", protocol, status_code, status_description);

    // 获取文件类型
    char content_type[128];
    ContentType(path, content_type);

    // 报头 content-type
    sprintf(response_header + strlen(response_header), "content-type:%s\r\n", content_type);

    // 报头 content-length
    sprintf(response_header + strlen(response_header), "content-length:%ld\r\n\r\n", size);

    // 回复客户端
    write(connect_fd, response_header, strlen(response_header));
}

int RequestHandler(int connect_fd, int epfd) {
    
    // 接受客户端发送的数据 
    // char buf[2048];
    // int len = 0, count = 0;
    // while((len = read(connect_fd, buf+count, sizeof(buf)-count)) > 0) {
    //     count += len;
    // }

    // 接受客户端发送的数据 
    char buf[1024];
    int len = 0, size = 0;
    char* cntnt = malloc(1024);
    while((len = read(connect_fd, buf, sizeof(buf))) > 0) {
        if(size+len >= sizeof(cntnt)) {
            cntnt = realloc(cntnt, size+len+1024);
        }
        strcpy(cntnt+size, buf);
        size += len;
    }

    if(len == -1 && errno != EAGAIN) {
        perror("read"); 
        return -1;
    }

    // 拿到请求行
    char method[8], path[128] = ".", protocol[8], tmp[128]; 
    sscanf(buf, "%s %s %s", method, tmp, protocol);
    strcat(path, tmp);

    // 把浏览器处理成的16进制字符串转换成中文字串
    decode16(path, path);

    // 处理路径，获取文件和参数
    // char *parameter[1024];
    // for(int i = 0; i < 1024; ++i) {
    //     parameter[i] = (char*)malloc(128);  // 释放在本函数末尾
    // }
    char parameter[1024];
    int nums = UrlHandler(path, parameter);

    // 如果是get方法
    if(strcasecmp(method, "get") == 0) {
        struct stat status;
        int ret = stat(path, &status);
        if(ret == -1) {
            ResponseHeader(connect_fd, protocol, 404, "Not Found", -1, "./404.html");
            ResponseFile(connect_fd, "./404.html");
        } else {
            if(S_ISREG(status.st_mode)) {   // path是个文件
                if(IsPhp(path)) {
                    ResponseHeader(connect_fd, protocol, 200, "OK", -1, path);
                    ResponsePhp(path, connect_fd, parameter, method, NULL, NULL);
                } else {
                    ResponseHeader(connect_fd, protocol, 200, "OK", status.st_size, path);
                    ResponseFile(connect_fd, path);
                }
            } else if(S_ISDIR(status.st_mode)) {    // path是个目录
                ResponseHeader(connect_fd, protocol, 200, "OK", -1, ".html");
                ResponseDirectory(connect_fd, path);
            }
        }
    } else if(strcasecmp(method, "post") == 0) {
        // 拿到content-type
        char content_type[128];
        char* p = strstr(cntnt, "Content-Type");
        p += strlen("Content-Type: ");
        int i = 0;
        while(*p != '\r') {
            content_type[i++] = *(p++);
        }
        // printf("%s\n",content_type);
        // 拿到body
        char* body = strstr(cntnt, "\r\n\r\n") + 4;
        // printf("%s\n",body);
        decode16(body,body);
        
        struct stat status;
        int ret = stat(path, &status);
        if(ret == -1) {
            ResponseHeader(connect_fd, protocol, 404, "Not Found", -1, "./404.html");
            ResponseFile(connect_fd, "./404.html");
        } else {
            if(S_ISREG(status.st_mode)) {   // path是个文件
                if(IsPhp(path)) {
                    ResponseHeader(connect_fd, protocol, 200, "OK", -1, path);
                    char content_length[32];
                    sprintf(content_length, "%d", strlen(body));
                    ResponsePhp(path, connect_fd, body, method, content_length, content_type);
                } else {
                    ResponseHeader(connect_fd, protocol, 403, "Forbidden", -1, "./403.html");
                    ResponseFile(connect_fd, "./403.html");
                }
            } else if(S_ISDIR(status.st_mode)) {    // path是个目录
                ResponseHeader(connect_fd, protocol, 200, "OK", -1, ".html");
                ResponseDirectory(connect_fd, path);
            }
        }
    }

    // for(int i = 0; i < 1024; ++i) {
    //     free(parameter[i]); // 释放存参数的字串串数组
    // }
    close(connect_fd);
    epoll_ctl(epfd, EPOLL_CTL_DEL, connect_fd, NULL);
    return 0;
}

int AcceptConnection(int listen_fd, int epfd) {

    // 接受连接，并创建connfd
    struct sockaddr_in client_addr;
    socklen_t size = sizeof(struct sockaddr);
    int connect_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &size);
    if(connect_fd == -1) {
        perror("accept");
        return -1;
    }

    // 打印客户端ip端口
    char client_ip[32];
    inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, sizeof(client_ip));
    // printf("client ip: %s:%d\n", client_ip, ntohs(client_addr.sin_port));

    // 设置connfd非阻塞
    int flag = fcntl(connect_fd, F_GETFL);
    flag |= O_NONBLOCK;
    fcntl(connect_fd, F_SETFL, flag);

    // 将connfd挂到epoll树上，并设置边沿触发模式
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = connect_fd;
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, connect_fd, &ev);
    if(ret == -1) {
        perror("epoll_ctl");
        return -1;
    }

    return connect_fd;
}

int EpollRun(int listen_fd) {

    // 创建epoll tree
    int epfd = epoll_create(1024);

    // 将监听连接文件描述符添加到epoll树上
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = listen_fd;
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev);
    if(ret == -1) {
        perror("epoll_ctl");
        return -1;
    }

    // 监听文件描述符状态
    struct epoll_event events[1024];
    while(1) {
        ret = epoll_wait(epfd, events, sizeof(events)/sizeof(struct epoll_event), -1);
        if(ret == -1) {
            perror("epoll_wait");
            return -1;
        }

        for(int i = 0; i < ret; ++i) {
            int current_fd = events[i].data.fd;

            if(current_fd == listen_fd) {
                // 有新的客户端连接
                AcceptConnection(listen_fd, epfd);
            } else {                                   
                // 有新的客户端请求
                RequestHandler(current_fd, epfd); // 这里的current_fd是connect_fd
            }
        }
    }
    close(listen_fd);
    epoll_ctl(epfd, EPOLL_CTL_DEL, listen_fd, NULL);
    return 0;
}

int InitListenSocket(const int port, int opt) {

    // 创建一个监听用套接字
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_fd == -1) {
        perror("socket");
        return -1;
    }

    // 设置端口复用 opt = 1 / 0
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    // 绑定端口
    struct sockaddr_in addr = {
        AF_INET,
        htons(port),
        INADDR_ANY
    };
    int ret = bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr));
    if(ret == -1) {
        perror("bind");
        return -1;
    }

    // 监听端口
    ret = listen(listen_fd, 128);
    if(ret == -1) {
        perror("listen");
        return -1;
    }

    return listen_fd;
}