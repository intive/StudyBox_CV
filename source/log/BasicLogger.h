#ifndef PATR_BASIC_LOGGER_H
#define PATR_BASIC_LOGGER_H

#include "Config.h"
#include "Layout.h"
#include <utility>
#include <sstream>

/// Klasa pozwalająca na opóźnienie wysłania wiadomości do loggera.
class LoggerOstream
{
public:
    typedef std::function<void(LogConfig::LogLevel, std::string, LoggerInfo, bool)> Target;

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
            target(level, std::move(stream.str()), info, flushFlag);
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
 * Target musi implementować funkcję void operator ()(LogConfig::LogLevel level, std::string message, LoggerInfo info, bool flushFlag).
 * Formatter musi implementować funkcję std::string format(Args...).
 * Wykorzystuje EBCO w celu zaoszczędzenia miejsca w przypadku klasy nie mającej stanu.
 */
template<typename Formatter>
class BasicLogger : private Formatter
{
public:
    typedef LoggerOstream::Target Target;
    /// Tworzy obiekt do logowania zdarzeń.
    /**
     * @param target obiekt odpowiedzialny za back-end logowania.
     * @param level maska poziomów obsługiwanych przez logger.
     * @param info informacje dot. loggera.
     * @param formatterArgs... opcjonalne argumenty do obiektu formatującego logowaną wiadomość.
     */
    template<typename TargetU, typename LoggerInfoU, typename... U>
    BasicLogger(TargetU&& target, LogConfig::Severity::LevelType level, LoggerInfoU&& info, U&&... formatterArgs) : Formatter(std::forward<U>(formatterArgs)...), target(std::forward<TargetU>(target)), severity(level), loggerInfo(std::forward<LoggerInfoU>(info)), flushFlag(false)
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
            target(level, std::move(Formatter::format(std::forward<Ts>(ts)...)), loggerInfo, flushFlag);
            flushFlag = false;
        }
    }

    LoggerOstream log(LogConfig::LogLevel level)
    {
        return LoggerOstream(severity, level, target, loggerInfo, flushFlag);
    }

    LoggerOstream operator ()(LogConfig::LogLevel level)
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
    Target target;
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
    BasicLogger<Formatter> get(String&& name, LogConfig::Severity::LevelType level, Formatter&& formatter)
    {
        using namespace std::placeholders;
        LoggerInfo info = { loggerCount++, std::forward<String>(name) };
        return BasicLogger<Formatter>(std::bind(&Target::operator(), std::ref(target), _1, _2, _3, _4), level, std::move(info), std::forward<Formatter>(formatter));
    }

    /// Tworzy nowy obiekt loggera z podaną nazwą, poziomem obsługi zdarzeń równym podanemu w konstruktorze obiektu oraz funkcją formatującą.
    template<typename Formatter, typename String>
    BasicLogger<Formatter> get(String&& name, Formatter&& formatter)
    {
        using namespace std::placeholders;
        LoggerInfo info = { loggerCount++, std::forward<String>(name) };
        return BasicLogger<Formatter>(std::bind(&Target::operator(), std::ref(target), _1, _2, _3, _4), globalLevel, std::move(info), std::forward<Formatter>(formatter));
    }

    /// Tworzy nowy obiekt loggera z podaną nazwą, poziomem obsługi zdarzeń oraz domyślnym obiektem typu Formatter.
    template<typename Formatter, typename String>
    BasicLogger<Formatter> get(String&& name, LogConfig::Severity::LevelType level)
    {
        using namespace std::placeholders;
        LoggerInfo info = { loggerCount++, std::forward<String>(name) };
        return BasicLogger<Formatter>(std::bind(&Target::operator(), std::ref(target), _1, _2, _3, _4), level, std::move(info));
    }

    /// Tworzy nowy obiekt loggera z podaną nazwą, poziomem obsługi zdarzeń oraz domyślnym obiektem formatującym.
    template<typename String>
    BasicLogger<StringStreamFormatter> get(String&& name, LogConfig::Severity::LevelType level)
    {
        return get<StringStreamFormatter>(std::forward<String>(name), level);
    }

    /// Tworzy nowy obiekt loggera z podaną nazwą, poziomem obsługi zdarzeń równym podanemu w konstruktorze obiektu oraz domyślnym obiektem typu Formatter.
    template<typename Formatter, typename String>
    BasicLogger<Formatter> get(String&& name)
    {
        using namespace std::placeholders;
        LoggerInfo info = { loggerCount++, std::forward<String>(name) };
        return BasicLogger<Formatter>(std::bind(&Target::operator(), std::ref(target), _1, _2, _3, _4), globalLevel, std::move(info));
    }

    /// Tworzy nowy obiekt loggera z podaną nazwą, poziomem obsługi zdarzeń równym podanemu w konstruktorze obiektu oraz domyślnym obiektem formatującym.
    template<typename String>
    BasicLogger<StringStreamFormatter> get(String&& name)
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
