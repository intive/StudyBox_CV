#ifndef PATR_LOG_CONFIG_H
#define PATR_LOG_CONFIG_H

#include <functional>

#include <type_traits>
#include <limits>

#include <ctime>

class Attributes;
class Message;

namespace LogConfig {

    typedef std::function<std::string(Attributes&&, Message&&)> Layout;
    typedef std::function<std::time_t()> Timer;

    enum class LogLevel : unsigned int
    {
        Trace = 1,
        Debug = 2,
        Info = 4,
        Warn = 8,
        Error = 16,
        Fatal = 32
    };

    struct Severity
    {
    public:
        typedef std::underlying_type<LogLevel>::type LevelType;

        static constexpr LevelType all = std::numeric_limits<LevelType>::max();
        static constexpr LevelType disable = 0;

        constexpr LevelType operator <(LogLevel level) const
        {
            return static_cast<LevelType>(level) - 1;
        }
        constexpr LevelType operator <=(LogLevel level) const
        {
            return (static_cast<LevelType>(level) << 1) - 1;
        }
        constexpr LevelType operator >(LogLevel level) const
        {
            return all ^ operator <=(level);
        }
        constexpr LevelType operator >=(LogLevel level) const
        {
            return all ^ operator <(level);
        }
        constexpr LevelType operator ==(LogLevel level) const
        {
            return static_cast<LevelType>(level);
        }
        constexpr LevelType operator !=(LogLevel level) const
        {
            return all ^ static_cast<LevelType>(level);
        }

    } constexpr severity = {};

    constexpr Severity::LevelType operator |(Severity::LevelType lhs, LogLevel rhs)
    {
        return lhs | static_cast<Severity::LevelType>(rhs);
    }

    constexpr Severity::LevelType operator |(LogLevel lhs, Severity::LevelType rhs)
    {
        return rhs | lhs;
    }

    constexpr Severity::LevelType operator &(Severity::LevelType lhs, LogLevel rhs)
    {
        return lhs & static_cast<Severity::LevelType>(rhs);
    }

    constexpr Severity::LevelType operator &(LogLevel lhs, Severity::LevelType rhs)
    {
        return rhs & lhs;
    }

    constexpr LogConfig::Severity::LevelType GetIndexForLevel(LogConfig::Severity::LevelType level)
    {
        return (level > 1) ? 1 + GetIndexForLevel(level >> 1) : 0;
    }

    constexpr LogConfig::Severity::LevelType GetIndexForLevel(LogConfig::LogLevel level)
    {
        return GetIndexForLevel(static_cast<LogConfig::Severity::LevelType>(level));
    }

    static const std::string LogLevelStrings[GetIndexForLevel(LogLevel::Fatal) + 1] = {
        "Trace",
        "Debug",
        "Info",
        "Warn",
        "Error",
        "Fatal" };

} // namespace Config

class StringStreamFormatter
{
public:
    template<typename... Ts>
    std::string format(Ts&&... ts) const
    {
        std::ostringstream ss;
        add(ss, std::forward<Ts>(ts)...);
        return ss.str();
    }

private:
    template<typename T, typename... Ts>
    void add(std::ostringstream& stream, T&& t, Ts&&... ts) const
    {
        stream << std::forward<T>(t);
        add(stream, std::forward<Ts>(ts)...);
    }

    void add(std::ostringstream&) const
    {
    }
};

#endif // PATR_LOG_CONFIG_H
