#pragma once

#include <iostream>
#include <vector>
#include <queue>
#include <memory>
#include <functional>
#include <pthread.h>
#include "Logger.hpp"
#include "Thread.hpp"
#include "Mutex.hpp"
#include "Cond.hpp"

using namespace ThreadModule;
using namespace CondModule;
using namespace LockModule;
using namespace LogModule;

const static int gdefaultthreadnum = 10;

// 线程池
template <typename T>
class ThreadPool
{
private:
    // 构造函数私有化（单例模式）
    ThreadPool(int threadnum = gdefaultthreadnum) : _threadnum(threadnum), _waitnum(0), _isrunning(false)
    {
        LOG(LogLevel::INFO) << "ThreadPool Construct()";
    }
    
    void InitThreadPool()
    {
        // 构建出所有的线程，并不启动
        for (int num = 0; num < _threadnum; num++)
        {
            _threads.emplace_back(std::bind(&ThreadPool::HandlerTask, this));
            LOG(LogLevel::INFO) << "init thread " << _threads.back().Name() << " done";
        }
    }
    
    void Start()
    {
        _isrunning = true;
        for (auto &thread : _threads)
        {
            thread.Start();
            LOG(LogLevel::INFO) << "start thread " << thread.Name() << " done";
        }
    }
    
    void HandlerTask() // 类的成员方法，也可以成为另一个类的回调方法
    {
        std::string name = GetThreadNameFromNptl();
        LOG(LogLevel::INFO) << name << " is running ...";
        while (true)
        {
            // 1. 保证队列安全
            _mutex.lock();
            // 2. 队列中不一定有数据
            while (_task_queue.empty() && _isrunning)
            {
                _waitnum++;
                _cond.Wait(_mutex);
                _waitnum--;
            }
            // 2.1 如果线程池已经退出了 && 任务队列是空的
            if (_task_queue.empty() && !_isrunning)
            {
                _mutex.unlock();
                break;
            }
            // 2.2 如果线程池不退出 && 任务队列不是空的
            // 2.3 如果线程池已经退出 && 任务队列不是空的 --- 处理完所有的任务，然后再退出
            // 3. 一定有任务，处理任务
            T t = _task_queue.front();
            _task_queue.pop();
            _mutex.unlock();
            LOG(LogLevel::DEBUG) << name << " get a task";
            // 4. 处理任务，这个任务属于线程独占的任务
            t();
        }
    }
    
    // 复制拷贝禁用
    ThreadPool<T>& operator=(const ThreadPool<T>&) = delete;
    ThreadPool(const ThreadPool<T>&) = delete;

public:
    static ThreadPool<T> *GetInstance()
    {
        // 如果是多线程获取线程池对象下面的代码就有问题了！！
        // 只有第一次会创建对象，后续都是获取
        // 双判断的方式，可以有效减少获取单例的加锁成本，而且保证线程安全
        if (nullptr == _instance) // 保证第二次之后，所有线程，不用再加锁，直接返回 _instance 单例对象
        {
            LockGuard lockguard(_lock);
            if (nullptr == _instance)
            {
                _instance = new ThreadPool<T>();
                _instance->InitThreadPool();
                _instance->Start();
                LOG(LogLevel::DEBUG) << "创建线程池单例";
                return _instance;
            }
        }
        LOG(LogLevel::DEBUG) << "获取线程池单例";
        return _instance;
    }

    void Stop()
    {
        _mutex.lock();
        _isrunning = false;
        _cond.NotifyAll();
        _mutex.unlock();
        LOG(LogLevel::DEBUG) << "线程池退出中...";
    }
    
    void Wait()
    {
        for (auto &thread : _threads)
        {
            thread.Join();
            LOG(LogLevel::INFO) << thread.Name() << "退出...";
        }
    }
    
    bool Enqueue(const T &t)
    {
        bool ret = false;
        _mutex.lock();
        if (_isrunning)
        {
            _task_queue.push(t);
            if (_waitnum > 0)
            {
                _cond.Notify();
            }
            LOG(LogLevel::DEBUG) << "任务入队列成功";
            ret = true;
        }
        _mutex.unlock();
        return ret;
    }
    
    ~ThreadPool()
    {}

private:
    int _threadnum;
    std::vector<Thread> _threads;
    std::queue<T> _task_queue;
    Mutex _mutex;
    Cond _cond;
    
    int _waitnum;
    bool _isrunning;
    
    // 添加单例模式
    static ThreadPool<T> *_instance;
    static Mutex _lock;
};

template <typename T>
ThreadPool<T> *ThreadPool<T>::_instance = nullptr;

template <typename T>
Mutex ThreadPool<T>::_lock;