#include "http_conn.h"

//对文件描述符设置非阻塞
void setnonblocking(int fd){
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
}

void addfd(int epollfd, int fd, bool oneshot){
    struct epoll_event event;
    event.data.fd = fd;
    //EPOLLRDHUP表示的是客户端关闭连接或者客户端关闭了写操作
    //此标志对于编写简单代码以在使用边缘触发监视时检测对等关闭特别有用。
    event.events = EPOLLIN | EPOLLET| EPOLLRDHUP;//ET模式
    if(oneshot) event.events |= EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);//设置文件描述符为非阻塞
}

void modfd(int efd, int fd, int ev){
    struct epoll_event event;
    event.data.fd = fd;
    event.events = ev | EPOLLET | EPOLLRDHUP | EPOLLONESHOT;
    epoll_ctl(efd, EPOLL_CTL_MOD, fd, &event);
}

void http_conn::init(int fd, int efd, struct sockaddr_in &address){
    sockfd = fd;
    epollfd = efd;
    client_address = address;
}

bool http_conn::read_all(){
    read_idx = 0;
    while (true)
    {
        int ret = recv(sockfd, read_buffer + read_idx, 2048 - read_idx, 0);
        if (ret == -1)
        {
            //ret为-1有可能是因为数据读完了
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            return false;
        }
        else if (ret == 0)
        {
            return false;
        }
        read_idx += ret;
    }
    //printf("%s\n", read_buffer);
    return true;
}

int http_conn::write_all(){
    while(1){
        int ret = send(sockfd, write_buffer + write_idx, 2048 - write_idx, 0);
        if(ret == -1){
            if(errno == EAGAIN){
                break;
            }
            return false;
        } else if(ret == 0){
            return false;
        }
        write_idx += ret;
    }
    return false;
}

void http_conn::process(){
    printf("我在处理客户端任务\n");
    printf("客户端任务处理完毕\n");
    modfd(epollfd, sockfd, EPOLLIN);
}

void http_conn::conn_close(){
    close(sockfd);//关闭连接
}

char *http_conn::getip(){
    return inet_ntoa(client_address.sin_addr);
}