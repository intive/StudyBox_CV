#ifndef PATR_LOGGER_H
#define PATR_LOGGER_H

#include "BasicLogger.h"
#include "Config.h"
#include "Layout.h"

#include <ostream>
#include <unordered_map>

#include "../utility/ThreadPool.h"

class LogTargetImpl
{
public:
    struct Message
    {
        LogConfig::LogLevel level;
        std::time_t time;
        std::string text;
        std::thread::id threadId;
        LoggerInfo logger;
        bool flush;
    };

    LogTargetImpl(std::ostream& stream, LogConfig::Layout layout);

    void operator ()(Message&& message);

private:

    std::string prepare(Message&& message);

    std::size_t getThreadNumber(std::thread::id id);

    std::size_t messageCounter;
    std::unordered_map<std::thread::id, std::size_t> threadIds;
    std::ostream& stream;
    LogConfig::Layout layout;
};


class LogTarget
{
private:
    typedef LogTargetImpl::Message Message;

public:
    LogTarget(std::ostream& stream, LogConfig::Layout layout, LogConfig::Timer timer);

    void notify(LogConfig::LogLevel level, std::string message, LoggerInfo info, bool flushFlag);

private:
    Utility::ThreadPool<Message, LogTargetImpl> impl;
    LogConfig::Timer timer;
};

typedef BasicLogManager<LogTarget> LogManager;


#endif // PATR_LOGGER_H
