#ifndef _NET_H_
#define _NET_H_

// 初始化监听文件描述符，opt是设置端口复用的参数
int InitListenSocket(const int port, int opt);

// 创建epoll树并开启轮询
void EpollRun(int listen_fd);

// 接受连接请求，创建connfd并挂到epoll树上
int AcceptConnection(int listen_fd, int epfd);

// 处理客户端请求
void RequestHandler(int connect_fd);

// 发送状态行，消息报头，空行
void ResponseHeader(int connect_fd, char *protocol, int status_code, char *status_description);

#endif