#ifndef __THREAD_HPP__
#define __THREAD_HPP__

#include <iostream>
#include <string>
#include <functional>
#include <pthread.h>
#include <unistd.h>
#include "Logger.hpp"

namespace ThreadModule
{
    using namespace LogModule;

    // 获取当前线程名称
    inline std::string GetThreadNameFromNptl()
    {
        char buffer[64];
        pthread_t tid = pthread_self();
        snprintf(buffer, sizeof(buffer), "Thread[%lu]", (unsigned long)tid);
        return std::string(buffer);
    }

    class Thread
    {
    public:
        using ThreadFunc = std::function<void()>;

        Thread(ThreadFunc func)
            : _func(func)
            , _tid(0)
            , _started(false)
            , _joined(false)
        {
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "Thread-%lu", (unsigned long)pthread_self());
            _name = buffer;
        }

        Thread(ThreadFunc func, const std::string& name)
            : _func(func)
            , _tid(0)
            , _name(name)
            , _started(false)
            , _joined(false)
        {}

        ~Thread()
        {
            if (_started && !_joined)
            {
                pthread_detach(_tid);
            }
        }

        void Start()
        {
            if (_started)
            {
                LOG(LogLevel::WARNING) << "Thread " << _name << " already started";
                return;
            }

            int ret = pthread_create(&_tid, nullptr, ThreadRoutine, this);
            if (ret != 0)
            {
                LOG(LogLevel::FATAL) << "Thread " << _name << " create failed";
                exit(1);
            }
            _started = true;
            LOG(LogLevel::DEBUG) << "Thread " << _name << " started, tid: " << _tid;
        }

        void Join()
        {
            if (!_started || _joined)
            {
                return;
            }

            int ret = pthread_join(_tid, nullptr);
            if (ret != 0)
            {
                LOG(LogLevel::ERROR) << "Thread " << _name << " join failed";
            }
            _joined = true;
            LOG(LogLevel::DEBUG) << "Thread " << _name << " joined";
        }

        void Detach()
        {
            if (!_started || _joined)
            {
                return;
            }

            int ret = pthread_detach(_tid);
            if (ret != 0)
            {
                LOG(LogLevel::ERROR) << "Thread " << _name << " detach failed";
            }
            _joined = true;
            LOG(LogLevel::DEBUG) << "Thread " << _name << " detached";
        }

        std::string Name() const
        {
            return _name;
        }

        pthread_t GetTid() const
        {
            return _tid;
        }

        bool IsStarted() const
        {
            return _started;
        }

        bool IsJoined() const
        {
            return _joined;
        }

    private:
        static void* ThreadRoutine(void* arg)
        {
            Thread* self = static_cast<Thread*>(arg);
            if (self && self->_func)
            {
                self->_func();
            }
            return nullptr;
        }

    private:
        ThreadFunc _func;
        pthread_t _tid;
        std::string _name;
        bool _started;
        bool _joined;
    };
}

#endif // __THREAD_HPP__