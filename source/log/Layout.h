#ifndef PATR_LOG_LAYOUT_H
#define PATR_LOG_LAYOUT_H

#include "Config.h"
#include <thread>
#include <ctime>
#include <locale>

/// Zawiera informacje o obiekcie wysyłającym wiadomość.
class LoggerInfo
{
public:
    LoggerInfo(std::size_t id, std::string name);

    std::size_t id() const;
    const std::string& name() const;

private:
    std::size_t loggerId;
    std::string loggerName;
};

/// Zawiera informacje o źródłowym wątku, z którego wysłana została wiadomość.
class ThreadInfo
{
public:
    ThreadInfo(std::thread::id id, std::size_t number);

    std::thread::id id() const;
    std::size_t number() const;

private:
    std::thread::id threadId;
    std::size_t threadNumber;
};

/// Zawiera informacje o poziomie zgłaszanej wiadomości.
class EventLogLevel
{
public:
    EventLogLevel(LogConfig::LogLevel level);

    const std::string& name() const;
    LogConfig::LogLevel value() const;

private:
    LogConfig::LogLevel level;
};

/// Zawiera informacje o czasie w danym dniu.
class Time
{
public:
    Time(std::tm* time);

    int hour() const;
    int minute() const;
    int second() const;
    int millisecond() const;

private:
    std::tm* time;
};

/// Zawiera informacje o dacie.
class Date
{
private:
    std::tm* tmtime;

public:
    Date(std::time_t timeTicks);

    int year() const;
    int month() const;
    int day() const;

    std::string toString(const std::string& format, const std::locale& locale = std::locale()) const;

    std::string toIso8601() const;

    const Time time;
};

/// Zawiera datę i dane podstawowe.
class Timestamp
{
private:
    std::time_t timeTicks;

public:
    Timestamp(std::time_t timeTicks);

    std::time_t ticks() const;
    const Date date;
};

/// Zawiera informacje o źródle wiadomości.
class Attributes
{
public:
    const Timestamp timestamp;
    const EventLogLevel level;
    const ThreadInfo thread;
    const LoggerInfo logger;
};

/// Zawiera informacje o wiadomości i jej zawartość.
class Message
{
public:
    Message(std::size_t id, std::string message);

    std::size_t id() const;
    std::string what() const;

private:
    std::size_t messageId;
    std::string message;
};


#endif // PATR_LOG_LAYOUT_H
