#ifndef _NET_H_
#define _NET_H_

// 初始化监听文件描述符，opt是设置端口复用的参数
int InitListenSocket(const int port, int opt);

#endif