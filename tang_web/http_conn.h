#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cassert>
#include <sys/epoll.h>

class http_conn{
public:
    http_conn(){}
    ~http_conn(){}

    void init(int fd, int efd, struct sockaddr_in &address);//每一个连接我们都对其初始化

    bool read_all();//读完所有的数据

    int write_all();//将write_buffer发送给客户端

    void process();

    void conn_close();

    char *getip();
private:
    int epollfd;//表示往那个epoll中modfd
    int sockfd;//此次HTTP连接的fd
    struct sockaddr_in client_address;//此次HTTP连接的客户端信息

    int read_idx;//表示当前读的指针在那里
    char read_buffer[2048];//读缓冲区，表示客户端发送来的信息存在里面
    int write_idx;
    char write_buffer[2048];//写缓冲区，表示服务端处理客户端信息后，将生成的数据写在里面
};



#endif