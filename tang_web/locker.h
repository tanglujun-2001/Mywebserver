#ifndef LOCKER_H
#define LOCKER_H

#include <exception>
#include <pthread.h>
#include <semaphore.h>

class locker{
public:
    locker();
    ~locker();

    bool lock();//上锁
    bool unlock();//解锁
private:
    pthread_mutex_t my_mutex;
};

class sem{
public:
    sem();
    ~sem();

    bool wait();
    bool post();
private:
    sem_t my_sem;
};

#endif
