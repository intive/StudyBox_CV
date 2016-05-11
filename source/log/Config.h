#ifndef PATR_LOG_CONFIG_H
#define PATR_LOG_CONFIG_H

#include <sstream>

#include <functional>

#include <type_traits>
#include <limits>

#include <ctime>

class Attributes;
class Message;

namespace LogConfig {

    /// Typ, za pomocą którego formatowane jest wyjście loggera.
    typedef std::function<std::string(Attributes&&, Message&&)> Layout;
    /// Typ, za pomocą którego zwracany jest obecny czas.
    typedef std::function<std::time_t()> Timer;

    /// Określa poziomy zgłaszanych informacji przez logger.
    enum class LogLevel : unsigned int
    {
        Trace = 1,
        Debug = 2,
        Info = 4,
        Warn = 8,
        Error = 16,
        Fatal = 32
    };

    /// Określa zbiór poziomów obsługiwanych przez logger.
    struct Severity
    {
    public:
        typedef std::underlying_type<LogLevel>::type LevelType;

        /// Określa wszystkie poziomy jako aktywne.
        static constexpr LevelType all = std::numeric_limits<LevelType>::max();
        static constexpr LevelType disable = 0;

        /// Zwraca poziomy mniejsze niż określony.
        constexpr LevelType operator <(LogLevel level) const
        {
            return static_cast<LevelType>(level) - 1;
        }

        /// Zwraca poziomy mniejsze lub równe określonemu.
        constexpr LevelType operator <=(LogLevel level) const
        {
            return (static_cast<LevelType>(level) << 1) - 1;
        }

        /// Zwraca poziomy większe od określonego.
        constexpr LevelType operator >(LogLevel level) const
        {
            return all ^ operator <=(level);
        }

        /// Zwraca poziomy większe lub równe określonemu.
        constexpr LevelType operator >=(LogLevel level) const
        {
            return all ^ operator <(level);
        }

        /// Zwraca poziom równy określonemu.
        constexpr LevelType operator ==(LogLevel level) const
        {
            return static_cast<LevelType>(level);
        }

        /// Zwraca poziomy różne od określonego.
        constexpr LevelType operator !=(LogLevel level) const
        {
            return all ^ static_cast<LevelType>(level);
        }

    } constexpr severity = {};

    /// Poniższe funkcje mają na celu umożliwienie operacji bitowych na LogLevel i jego typie bazowym.

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

    /// Zwraca numer pierwszego bitu określonego przez maskę.
    constexpr LogConfig::Severity::LevelType GetIndexForLevel(LogConfig::Severity::LevelType level)
    {
        return (level > 1) ? 1 + GetIndexForLevel(level >> 1) : 0;
    }

    constexpr LogConfig::Severity::LevelType GetIndexForLevel(LogConfig::LogLevel level)
    {
        return GetIndexForLevel(static_cast<LogConfig::Severity::LevelType>(level));
    }

    /// Zawiera stringi odpowiadające kolejnym wartościom LogLevel.
    static const std::string LogLevelStrings[GetIndexForLevel(LogLevel::Fatal) + 1] = {
        "Trace",
        "Debug",
        "Info",
        "Warn",
        "Error",
        "Fatal" };

} // namespace Config

/// Typ podstawowy wykorzystywany domyślnie przez BasicLogger.
class StringStreamFormatter
{
public:
    /// Zwraca string z wartościami przekonwertowanymi za pomocą operatora << dla ostream&.
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
