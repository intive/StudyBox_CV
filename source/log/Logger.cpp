#include "Logger.h"

LogTargetImpl::LogTargetImpl(std::ostream& stream, LogConfig::Layout layout) : stream(stream), layout(std::move(layout))
{
}

void LogTargetImpl::operator ()(Message&& message)
{
    auto prepared = prepare(std::move(message));
    stream << prepared;
    if (message.flush)
        stream.flush();
}

std::string LogTargetImpl::prepare(Message&& message)
{
    Timestamp timestamp{ std::move(message.time) };
    EventLogLevel level{ message.level };
    ThreadInfo thread{ message.threadId, getThreadNumber(message.threadId) };
    LoggerInfo log{ std::move(message.logger) };
    Attributes attributes{ std::move(timestamp), std::move(level), std::move(thread), std::move(log) };
    ::Message text{ messageCounter++, message.text };
    return layout(std::move(attributes), std::move(text)) + '\n';
}

std::size_t LogTargetImpl::getThreadNumber(std::thread::id id)
{
    auto it = threadIds.find(id);
    if (it != threadIds.end())
        return it->second;
    else
        return threadIds.insert(std::make_pair(id, threadIds.size())).first->second;
}

LogTarget::LogTarget(std::ostream& stream, LogConfig::Layout layout, LogConfig::Timer timer) : impl(1, 5000, stream, std::move(layout)), timer(std::move(timer))
{
}

void LogTarget::operator ()(LogConfig::LogLevel level, std::string message, LoggerInfo info, bool flushFlag)
{
    impl.add(Message{ level, timer(), message, std::this_thread::get_id(), std::move(info), flushFlag });
}
