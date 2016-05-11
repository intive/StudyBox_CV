#ifndef PATR_BASIC_LOGGER_H
#define PATR_BASIC_LOGGER_H

#include "Config.h"
#include "Layout.h"
#include <utility>
#include <sstream>

/// Klasa pozwalająca na opóźnienie wysłania wiadomości do loggera.
template<typename Target>
class LoggerOstream
{
public:
    LoggerOstream(LogConfig::Severity::LevelType severity, LogConfig::LogLevel level, Target& target, LoggerInfo& info, bool flushFlag) : target(target), severity(severity), level(level), info(info), flushFlag(flushFlag)
    {
    }

    LoggerOstream(LoggerOstream&&) = default;

    /// Powoduje dodanie wartości przekonwertowanej do stringa do bufora.
    template<typename T>
    LoggerOstream& operator <<(T&& t)
    {
        if (severity & level)
        {
            stream << std::forward<T>(t);
        }

        return *this;
    }

    /// Powoduje przeniesienie bufora do loggera.
    ~LoggerOstream()
    {
        if (severity & level)
        {
            target.notify(level, std::move(stream.str()), info, flushFlag);
        }
    }

private:
    std::ostringstream stream;
    Target& target;
    LogConfig::Severity::LevelType severity;
    LogConfig::LogLevel level;
    LoggerInfo& info;
    bool flushFlag;
};

/// Podstawowa klasa odpowiedzialna za stworzenie front-endu dla użytkownika do logowania wiadomości.
/**
 * Target musi implementować funkcję void notify(LogConfig::LogLevel level, std::string message, LoggerInfo info, bool flushFlag).
 * Formatter musi implementować funkcję std::string format(Args...).
 * Wykorzystuje EBCO w celu zaoszczędzenia miejsca w przypadku klasy nie mającej stanu.
 */
template<typename Target, typename Formatter>
class BasicLogger : private Formatter
{
public:

    /// Tworzy obiekt do logowania zdarzeń.
    /**
     * @param target obiekt odpowiedzialny za back-end logowania.
     * @param level maska poziomów obsługiwanych przez logger.
     * @param info informacje dot. loggera.
     * @param formatterArgs... opcjonalne argumenty do obiektu formatującego logowaną wiadomość.
     */
    template<typename LoggerInfoU, typename... U>
    BasicLogger(Target& target, LogConfig::Severity::LevelType level, LoggerInfoU&& info, U&&... formatterArgs) : Formatter(std::forward<U>(formatterArgs)...), target(target), severity(level), loggerInfo(std::forward<LoggerInfoU>(info)), flushFlag(false)
    {
    }

    template<typename... Ts>
    void trace(Ts&&... ts)
    {
        log(LogConfig::LogLevel::Trace, std::forward<Ts>(ts)...);
    }
    template<typename... Ts>
    void debug(Ts&&... ts)
    {
        log(LogConfig::LogLevel::Debug, std::forward<Ts>(ts)...);
    }
    template<typename... Ts>
    void info(Ts&&... ts)
    {
        log(LogConfig::LogLevel::Info, std::forward<Ts>(ts)...);
    }
    template<typename... Ts>
    void warn(Ts&&... ts)
    {
        log(LogConfig::LogLevel::Warn, std::forward<Ts>(ts)...);
    }
    template<typename... Ts>
    void error(Ts&&... ts)
    {
        log(LogConfig::LogLevel::Error, std::forward<Ts>(ts)...);
    }
    template<typename... Ts>
    void fatal(Ts&&... ts)
    {
        log(LogConfig::LogLevel::Fatal, std::forward<Ts>(ts)...);
    }
    template<typename... Ts>
    void log(LogConfig::LogLevel level, Ts&&... ts)
    {
        if (severity & level)
        {
            target.notify(level, std::move(Formatter::format(std::forward<Ts>(ts)...)), loggerInfo, flushFlag);
            flushFlag = false;
        }
    }

    LoggerOstream<Target> log(LogConfig::LogLevel level)
    {
        return LoggerOstream<Target>(severity, level, target, loggerInfo, flushFlag);
    }

    LoggerOstream<Target> operator ()(LogConfig::LogLevel level)
    {
        return log(level);
    }

    /// Oznacza następną wiadomość jako wiadomość opróżniającą bufor obiektu target.
    BasicLogger& flushNext()
    {
        flushFlag = true;
        return *this;
    }

private:
    Target& target;
    LogConfig::Severity::LevelType severity;
    LoggerInfo loggerInfo;
    bool flushFlag;
};

/// Klasa odpowiedzialna za przechowywanie celu logowania oraz tworzenie loggerów.
template<typename Target>
class BasicLogManager
{
public:
    /// Tworzy nowy obiekt wykorzystując konstruktor Target::Target(Sink&& sink, LogConfig::Severity::LevelType globalLevel, Layout&& layout).
    /**
     * @param sink obiekt do którego logowane będą zdarzenia.
     * @param globalLevel domyślny poziom obsługi zdarzeń.
     * @param layout układ logowanych wiadomości.
     * @param timer funkcja wyznaczająca obecny czas.
     */
    template<typename Sink, typename Layout, typename Timer>
    BasicLogManager(Sink&& sink, LogConfig::Severity::LevelType globalLevel, Layout&& layout, Timer&& timer) : target(std::forward<Sink>(sink), std::forward<Layout>(layout), std::forward<Timer>(timer)), globalLevel(globalLevel), loggerCount(0)
    {
    }

    /// Tworzy nowy obiekt loggera z podaną nazwą, poziomem obsługi zdarzeń oraz funkcją formatującą.
    template<typename Formatter, typename String>
    BasicLogger<Target, Formatter> get(String&& name, LogConfig::Severity level, Formatter&& formatter)
    {
        LoggerInfo info = { loggerCount++, std::forward<String>(name) };
        return BasicLogger<Target, Formatter>(target, level, std::move(info), std::forward<Formatter>(formatter));
    }

    /// Tworzy nowy obiekt loggera z podaną nazwą, poziomem obsługi zdarzeń równym podanemu w konstruktorze obiektu oraz funkcją formatującą.
    template<typename Formatter, typename String>
    BasicLogger<Target, Formatter> get(String&& name, Formatter&& formatter)
    {
        LoggerInfo info = { loggerCount++, std::forward<String>(name) };
        return BasicLogger<Target, Formatter>(target, globalLevel, std::move(info), std::forward<Formatter>(formatter));
    }

    /// Tworzy nowy obiekt loggera z podaną nazwą, poziomem obsługi zdarzeń oraz domyślnym obiektem typu Formatter.
    template<typename Formatter, typename String>
    BasicLogger<Target, Formatter> get(String&& name, LogConfig::Severity::LevelType level)
    {
        LoggerInfo info = { loggerCount++, std::forward<String>(name) };
        return BasicLogger<Target, Formatter>(target, level, std::move(info));
    }

    /// Tworzy nowy obiekt loggera z podaną nazwą, poziomem obsługi zdarzeń oraz domyślnym obiektem formatującym.
    template<typename String>
    BasicLogger<Target, StringStreamFormatter> get(String&& name, LogConfig::Severity::LevelType level)
    {
        return get<StringStreamFormatter>(std::forward<String>(name), level);
    }

    /// Tworzy nowy obiekt loggera z podaną nazwą, poziomem obsługi zdarzeń równym podanemu w konstruktorze obiektu oraz domyślnym obiektem typu Formatter.
    template<typename Formatter, typename String>
    BasicLogger<Target, Formatter> get(String&& name)
    {
        LoggerInfo info = { loggerCount++, std::forward<String>(name) };
        return BasicLogger<Target, Formatter>(target, globalLevel, std::move(info));
    }

    /// Tworzy nowy obiekt loggera z podaną nazwą, poziomem obsługi zdarzeń równym podanemu w konstruktorze obiektu oraz domyślnym obiektem formatującym.
    template<typename String>
    BasicLogger<Target, StringStreamFormatter> get(String&& name)
    {
        return get<StringStreamFormatter>(std::forward<String>(name));
    }

    /// Zwraca poziom obsługi zdarzeń.
    const LogConfig::Severity& level() const
    {
        return globalLevel;
    }

private:
    Target target;
    LogConfig::Severity::LevelType globalLevel;
    std::size_t loggerCount;
};

#endif // PATR_BASIC_LOGGER_H
