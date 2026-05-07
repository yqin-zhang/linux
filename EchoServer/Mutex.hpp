#pragma once

#include <pthread.h>

namespace LockModule
{
    class Mutex
    {
    public:
        Mutex()
        {
            pthread_mutex_init(&_mutex, nullptr);
        }
        ~Mutex()
        {
            pthread_mutex_destroy(&_mutex);
        }
        void lock()
        {
            pthread_mutex_lock(&_mutex);
        }
        void unlock()
        {
            pthread_mutex_unlock(&_mutex);
        }
        pthread_mutex_t* getMutex()
        {
            return &_mutex;
        }
    private:
        pthread_mutex_t _mutex;
    };

    class LockGuard
    {
    public:
        LockGuard(Mutex& mutex) : _mutex(mutex)
        {
            _mutex.lock();
        }
        ~LockGuard()
        {
            _mutex.unlock();
        }
    private:
        Mutex& _mutex;
    };
}