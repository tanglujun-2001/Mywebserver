#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<errno.h>
#include<fcntl.h>
#include<sys/epoll.h>
#include"locker.h"
#include"threadpool.h"
#include"http_conn.h"
#include<signal.h>

#define MAX_EVENT_NUMBER 10000
#define MAX_FD 65535

extern void addfd(int epollfd, int fd, bool oneshot);
extern void modfd(int efd, int fd, int ev);

int main(int argc, char *argv[])
{
    if(argc <= 1){
        //说明只有一个参数（程序的路径）
        printf("请指定端口号!\n");
        return 0;
    }
    int port = atoi(argv[1]);//指定端口

    threadpool<http_conn> *MyPool = NULL;
    MyPool = new threadpool<http_conn>(8);//创建线程池

    http_conn *user = NULL;
    user = new http_conn[MAX_FD];//申请最大套接字大小的http_conn类，用于接受客户端的HTTP连接

    //创建监听套接字
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);//TCP连接
    printf("listenfd = %d\n", listenfd);
    //socket基本操作
    struct sockaddr_in address;
    memset(&address, 0, sizeof address);
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    //绑定
    bind(listenfd, (struct sockaddr *)&address, sizeof(address));
    //监听
    listen(listenfd, 5);//指定半连接队列长度
    
    //创建epoll红黑树根节点
    int epollfd = epoll_create(5);//这里的5无关紧要
    addfd(epollfd, listenfd, false);//将监听fd添加进epoll中，同时指定oneshot为false
    struct epoll_event events[MAX_EVENT_NUMBER];//创建一个事件表，等一下epoll_wait要用

    while(1){
        //得到活跃的事件数量
        int dump_num = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if(dump_num < 0 && errno != EINTR){
            printf("epoll_wait调用失败\n");
            break;
        }
        for(int i = 0; i < dump_num; i ++){
            int fd = events[i].data.fd;
            if(fd == listenfd){
                //有新客户端连接
                struct sockaddr_in client_address;
                socklen_t len = sizeof(client_address);
                int clientfd = accept(listenfd, (struct sockaddr *)&client_address, &len);
                addfd(epollfd, clientfd, true);//将客户端的监听加入到epoll中
                user[clientfd].init(clientfd, epollfd, client_address);//初始化客户端连接
                printf("客户端ip:%s已连接\n", user[clientfd].getip());

            } else if(events[i].events & EPOLLRDHUP){
                //关闭连接
                user[fd].conn_close();//关闭客户端连接
                printf("客户端ip:%s已经断开连接\n", user[fd].getip());
                break;
            } else if(events[i].events & EPOLLIN){
                //客户端数据来了
                int ret = user[fd].read_all();
                if(ret == -1){
                    //客户端终止连接
                    user[fd].conn_close();//关闭客户端连接

                } else if(ret == 0){
                    //读取错误了
                    user[fd].conn_close();//关闭客户端连接

                } else{
                    MyPool->append(user + fd);//将需要处理的请求放到请求队列中让工作线程去做

                }
            }
        }
    }    
}
