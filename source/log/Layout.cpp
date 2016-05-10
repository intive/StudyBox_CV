#define _CRT_SECURE_NO_WARNINGS
#include "Layout.h"

#include <iomanip>
#include <sstream>

LoggerInfo::LoggerInfo(std::size_t id, std::string name) : loggerId(id), loggerName(name)
{
}

std::size_t LoggerInfo::id() const
{
    return loggerId;
}
const std::string& LoggerInfo::name() const
{
    return loggerName;
}

ThreadInfo::ThreadInfo(std::thread::id id, std::size_t number) : threadId(id), threadNumber(number)
{
}

std::thread::id ThreadInfo::id() const
{
    return threadId;
}
std::size_t ThreadInfo::number() const
{
    return threadNumber;
}


ErrorLevel::ErrorLevel(LogConfig::LogLevel level) : level(level)
{
}

const std::string& ErrorLevel::name() const
{
    return LogConfig::LogLevelStrings[LogConfig::GetIndexForLevel(level)];
}
LogConfig::LogLevel ErrorLevel::value() const
{
    return level;
}

Time::Time(std::tm* time) : time(time)
{
}

int Time::hour() const
{
    return time->tm_hour;
}
int Time::minute() const
{
    return time->tm_min;
}
int Time::second() const
{
    return time->tm_sec;
}
int Time::millisecond() const
{
    return 0; // TODO
}

Date::Date(std::time_t timeTicks) : tmtime(std::localtime(&timeTicks)), time(tmtime)
{
}

int Date::year() const
{
    return 1900 + tmtime->tm_year;
}
int Date::month() const
{
    return tmtime->tm_mon;
}
int Date::day() const
{
    return tmtime->tm_wday;
}

std::string Date::toString(const std::string& format, const std::locale& locale) const
{
    std::ostringstream ss;
    ss.imbue(locale);
    ss << std::put_time(tmtime, format.c_str());
    return ss.str();
}

std::string Date::toIso8601() const
{
    return toString("%FT%TZ");
}


Timestamp::Timestamp(std::time_t timeTicks) : timeTicks(timeTicks), date(timeTicks)
{
}

std::time_t Timestamp::ticks() const
{
    return timeTicks;
}

Message::Message(std::size_t id, std::string message) : messageId(id), message(message)
{
}

std::size_t Message::id() const
{
    return messageId;
}
std::string Message::what() const
{
    return message;
}
