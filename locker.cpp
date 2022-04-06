#include"locker.h"
//构造函数初始化锁
locker::locker(){
    pthread_mutex_init(&my_mutex, NULL);
}
//析沟函数摧毁锁
locker::~locker(){
    pthread_mutex_destroy(&my_mutex);
}
//上锁
bool locker::lock(){
    return pthread_mutex_lock(&my_mutex) == 0;
}
//解锁
bool locker::unlock(){
    return pthread_mutex_unlock(&my_mutex) == 0;
}


sem::sem(){
    sem_init(&my_sem, 0, 0);//初始化为0，且只在线程间共享
}
sem::~sem(){
    sem_destroy(&my_sem);
}
bool sem::wait(){
    return sem_wait(&my_sem) == 0; 
}
bool sem::post(){
    return sem_post(&my_sem) == 0;
}