#ifndef PATR_LOGGER_H
#define PATR_LOGGER_H

#include "BasicLogger.h"
#include "Config.h"
#include "Layout.h"

#include <ostream>
#include <unordered_map>

#include "../utility/ThreadPool.h"

/// Odpowiedzialny za obsługę wiadomości wysyłanych przez loggery.
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

    /// Tworzy nowy obiekt wykorzystujący podany strumień oraz funkcję formatującą rozkład wiadomości.
    LogTargetImpl(std::ostream& stream, LogConfig::Layout layout);

    /// Dokonuje obsługi wiadomości.
    void operator ()(Message&& message);

private:
    /// Przygotowuje wiadomość do zapisu.
    std::string prepare(Message&& message);

    /// Zwraca zarejestrowany numer wątku.
    std::size_t getThreadNumber(std::thread::id id);

    std::size_t messageCounter;
    std::unordered_map<std::thread::id, std::size_t> threadIds;
    std::ostream& stream;
    LogConfig::Layout layout;
};

/// Odpowiedzialny za demultipleksację wiadomości wysyłanych przez loggery.
class LogTarget
{
private:
    typedef LogTargetImpl::Message Message;

public:
    /// Przekierowuje wartości do implementacji.
    LogTarget(std::ostream& stream, LogConfig::Layout layout, LogConfig::Timer timer);

    /// Informuje implementację o nowej wiadomości.
    void operator ()(LogConfig::LogLevel level, std::string message, LoggerInfo info, bool flushFlag);

private:
    Utility::ThreadPool<Message, LogTargetImpl> impl;
    LogConfig::Timer timer;
};

typedef BasicLogManager<LogTarget> LogManager;


#endif // PATR_LOGGER_H
