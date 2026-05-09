#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <memory>
#include <ctime>
#include <sstream>
#include <filesystem> // C++17, 需要高版本编译器和-std=c++17
#include <unistd.h>
#include "Mutex.hpp"

namespace LogModule
{
    using namespace LockModule; //使用我们自己封装的锁，也可以采用C++11的锁.

    // 默认路径和日志名称
    const std::string defaultpath = "./log/";
    const std::string defaultname = "log.txt";

    // 日志等级
    enum class LogLevel
    {
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        FATAL
    };

    // 日志转换成为字符串
    std::string LogLevelToString(LogLevel level)
    {
        switch (level)
        {
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARNING:
            return "WARNING";
        case LogLevel::ERROR:
            return "ERROR";
        case LogLevel::FATAL:
            return "FATAL";
        default:
            return "UNKNOWN";
        }
    }

    // 根据时间戳，获取可读性较强的时间信息
    std::string GetCurrTime()
    {
        time_t tm = time(nullptr);
        struct tm curr;
        localtime_r(&tm, &curr);
        // 这里如果不好看，可以考虑sprintf
        // 方法1
        // std::stringstream ss;
        // ss << curr.tm_year + 1900 << "-" << curr.tm_mon << "-" << curr.tm_mday << " "
        //   << curr.tm_hour << ":" << curr.tm_min << ":" << curr.tm_sec;
        // return ss.str();
        // 方法2
        char timebuffer[64];
        snprintf(timebuffer, sizeof(timebuffer), "%4d-%02d-%02d %02d:%02d:%02d",
                 curr.tm_year + 1900,
                 curr.tm_mon + 1,  // 注意：月份需要 +1
                 curr.tm_mday,
                 curr.tm_hour,
                 curr.tm_min,
                 curr.tm_sec);
        return timebuffer;
    }

    // 策略模式，策略接口
    class LogStrategy
    {
    public:
        virtual ~LogStrategy() = default;                        // 策略的析构函数
        virtual void SyncLog(const std::string &message) = 0;    // 不同模式核心是刷新方式的不同
    };

    // 控制台日志策略,就是日志只向显示器打印，方便我们debug
    class ConsoleLogStrategy : public LogStrategy
    {
    public:
        void SyncLog(const std::string &message) override
        {
            LockGuard LockGuard(_mutex);
            std::cerr << message << std::endl;
        }
        ~ConsoleLogStrategy()
        {
            // std::cout << "~Console LogStrategy " << std::endl; // for debug
        }

    private:
        Mutex _mutex; // 显示器也是临界资源，保证输出线程安全
    };

    // 文件日志策略
    class FileLogStrategy : public LogStrategy
    {
    public:
        // 构造函数，建立出来指定的目录结构和文件结构
        FileLogStrategy(const std::string logpath = defaultpath, std::string logfilename = defaultname)
            : _logpath(logpath), _logfilename(logfilename)
        {
            LockGuard lockguard(_mutex);
            if (std::filesystem::exists(_logpath))
                return;
            try
            {
                std::filesystem::create_directories(_logpath);
            }
            catch (const std::filesystem::filesystem_error &e)
            {
                std::cerr << e.what() << '\n';
            }
        }
        // 将一条日志信息写入到文件中
        void SyncLog(const std::string &message) override
        {
            LockGuard lockguard(_mutex);
            std::string log = _logpath + _logfilename;
            std::ofstream out(log.c_str(), std::ios::app); // 追加方式
            if (!out.is_open())
                return;
            out << message << "\n";
            out.close();
        }
        ~FileLogStrategy()
        {
            // std::cout << "~File LogStrategy " << std::endl;  // for debug
        }

    public:
        std::string _logpath;
        std::string _logfilename;
        Mutex _mutex; //保证输出线程安全，粗狂方式下，可以不用
    };

    // 具体的日志类
    class Logger
    {
    public:
        Logger()
        {
            // 默认使用显示器策略，如果用户二次指明了策略，会释放在申请，测试的时候注意析构次数
            UseConsoleStrategy();
        }
        ~Logger()
        {
        }
        void UseConsoleStrategy()
        {
            _strategy = std::make_unique<ConsoleLogStrategy>();
        }
        void UseFileStrategy()
        {
            _strategy = std::make_unique<FileLogStrategy>();
        }
        // 内部类，实现RAII风格的日志格式化和刷新
        // 这个LogMessage，表示一条完整的日志对象
        class LogMessage
        {
        private:
            LogLevel _type;          // 日志等级
            std::string _curr_time;  // 日志时间
            pid_t _pid;              // 写入日志的时间
            std::string _filename;   // 对应的文件名
            int _line;               // 对应的文件行号
            Logger &_logger;         // 引用外部logger类, 方便使用策略进行刷新
            std::string _loginfo;    // 一条合并完成的，完整的日志信息

        public:
            // RAII风格，构造的时候构建好日志头部信息
            LogMessage(LogLevel type, std::string &filename, int line, Logger &logger)
              : _type(type),
                _curr_time(GetCurrTime()),
                _pid(getpid()),
                _filename(filename),
                _line(line),
                _logger(logger)
            {
                // stringstream不允许拷贝,所以这里就当做格式化功能使用
                std::stringstream ssbuffer;
                ssbuffer << "[" << _curr_time << "] "
                      << "[" << LogLevelToString(type) << "] "
                      << "[" << _pid << "] "
                      << "[" << _filename << "] "
                      << "[" << _line << "]"
                      << " - ";
                _loginfo = ssbuffer.str();
            }
            // 重载<< 支持C++风格的日志输入，使用模版，表示支持任意类型
            template <typename T>
            LogMessage &operator<<(const T &info)
            {
                std::stringstream ssbuffer;
                ssbuffer << info;
                _loginfo += ssbuffer.str();
                return *this; // 返回当前LogMessage对象，方便下次继续进行<<
            }
            // RAII风格，析构的时候进行日志持久化，采用指定的策略
            ~LogMessage()
            {
                if (_logger._strategy)
                {
                    _logger._strategy->SyncLog(_loginfo);
                }
                // std::cout << "~LogMessage " << std::endl;
            }
        };

        // 故意拷贝，形成LogMessage临时对象，后续在被<<时，会被持续引用，
        // 直到完成输入，才会自动析构临时LogMessage，至此也完成了日志的显示或者刷新
        // 同时，形成的临时对象内包含独立日志数据
        // 未来采用宏替换，进行文件名和代码行数的获取
        LogMessage operator()(LogLevel type, std::string filename, int line)
        {
            return LogMessage(type, filename, line, *this);
        }

    private:
        std::unique_ptr<LogStrategy> _strategy; // 写入日志的策略
    };

    // 定义全局的logger对象
    Logger logger;

    // 使用宏，可以进行代码插入，方便随时获取文件名和行号
    #define LOG(type) logger(type, __FILE__, __LINE__)

    // 提供选择使用何种日志策略的方法
    #define ENABLE_CONSOLE_LOG_STRATEGY() logger.UseConsoleStrategy()
    #define ENABLE_FILE_LOG_STRATEGY() logger.UseFileStrategy()
}