#ifndef _NET_H_
#define _NET_H_
#include <stddef.h>

// 初始化监听文件描述符，opt是设置端口复用的参数
int InitListenSocket(const int port, int opt);

// 创建epoll树并开启轮询
int EpollRun(int listen_fd);

// 接受连接请求，创建connfd并挂到epoll树上
int AcceptConnection(int listen_fd, int epfd);

// 处理客户端请求
int RequestHandler(int connect_fd, int epfd);

// 发送状态行，消息报头，空行
void ResponseHeader(int connect_fd, char *protocol, int status_code, char *status_description, size_t size, char *path);

// 响应文件内容
int ResponseFile(int connect_fd, char *filename);

// 响应目录内容
void ResponseDirectory(int connect_fd, char *path);

// fcgi里的内容，不了解直接用的
void ResponsePhp(char *path, int connect_fd, char *parameter);
#endif