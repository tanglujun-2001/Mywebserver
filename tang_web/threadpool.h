#ifndef THREADPOLL_H
#define THREADPOLL_H

#include<pthread.h>
#include<list>
#include"locker.h"
#include<exception>
#include<cstdio>

template<typename T>
class threadpool{
public:
    threadpool(int thread_number);
    ~threadpool();

    bool append(T*);//增加任务至请求任务队列中

    //工作线程相关
    static void *worker(void*);
    void gotorun();

private:
    pthread_t *my_threadpool;//线程池的指针
    int my_thread_number;//需要创建的线程的数量

    //请求任务队列
    std::list<T*> my_work_queue;
    sem my_work_queue_sem;//该信号量用于记录任务队列中的任务数量
    locker my_work_queue_lock;//保证工作线程从请求任务队列取任务是原子操作
};

template<typename T>
threadpool<T>::threadpool(int thread_number): my_thread_number(thread_number){

    my_threadpool = new pthread_t[my_thread_number];
    for(int i = 0; i < my_thread_number; i ++){
        pthread_create(my_threadpool + i, NULL, worker, this);//创建线程
        printf("创建第%d个线程成功\n", i);
        pthread_detach(my_threadpool[i]);//将线程分离，让系统回收
    }
}

template<typename T>
threadpool<T>::~threadpool(){
    delete [] my_threadpool;
}

template<typename T>
bool threadpool<T>::append(T *NewWork){
    my_work_queue.push_back(NewWork);//增加任务至任务队列中
    my_work_queue_sem.post();//任务队列中的数量信号post
    return true;
}

//工作线程的运行函数
template<typename T>
void *threadpool<T>::worker(void *arg){
    threadpool *tmp = (threadpool *)arg;
    tmp->gotorun();
    return NULL;
}
template<typename T>
void threadpool<T>::gotorun(){
    while(true){
        my_work_queue_sem.wait();
        my_work_queue_lock.lock();
        if(!my_work_queue.size()){
            my_work_queue_lock.unlock();
            continue;
        }
        T *request = my_work_queue.front();//取出任务
        my_work_queue.pop_front();//从工作队列中删除任务
        my_work_queue_lock.unlock();//解锁
        request->process();//做工作
    }
}

#endif
