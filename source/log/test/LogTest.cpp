#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <map>
#include "../Logger.h"

BOOST_AUTO_TEST_SUITE(LogTests)

struct LogManagerHelper
{
    LogManagerHelper(std::ostringstream& sink,
                     std::function<std::string(Attributes&&, Message&&)> layout =
                     [] (Attributes&& attributes, Message&& message)
                     {
                         return message.what();
                     },
                     std::function<std::time_t()> time =
                     []
                     {
                        return std::time(nullptr);
                     })
    : manager(sink, 
              LogConfig::Severity::all,
              std::move(layout),
              std::move(time)
             )
    {
    }

    LogManager manager;
};

BOOST_AUTO_TEST_CASE(MessageFiltering)
{
    for (LogConfig::Severity::LevelType c = 0;
         c < static_cast<LogConfig::Severity::LevelType>(LogConfig::LogLevel::Fatal) + 1;
         ++c)
    {
        std::ostringstream stream, test;
        {
            LogManagerHelper helper(stream);

            auto logger = helper.manager.get("", c);
            for (LogConfig::Severity::LevelType i = 1;
                 i < LogConfig::Severity::all >> 1;
                 i <<= 1)
            {
                logger.log(static_cast<LogConfig::LogLevel>(i), i);
                if (i & c)
                    test << i << '\n';
            }
        }

        {
            LogManagerHelper helper(stream);

            auto logger = helper.manager.get("", c);
            {
                int i;
                i = 1;
                logger.trace(i);
                if (i & c)
                    test << i << '\n';
                i = 2;
                logger.debug(i);
                if (i & c)
                    test << i << '\n';
                i = 4;
                logger.info(i);
                if (i & c)
                    test << i << '\n';
                i = 8;
                logger.warn(i);
                if (i & c)
                    test << i << '\n';
                i = 16;
                logger.error(i);
                if (i & c)
                    test << i << '\n';
                i = 32;
                logger.fatal(i);
                if (i & c)
                    test << i << '\n';
            }
        }
        BOOST_TEST((stream.str() == test.str()));
    }
}


BOOST_AUTO_TEST_CASE(AttributeValidity)
{
    bool called = false;
    {
        std::ostringstream stream;
        auto level = LogConfig::LogLevel::Info;
        std::string levelString = "Info";
        std::time_t val = 10;
        std::tm tm(*std::localtime(&val));
        std::thread::id id = std::this_thread::get_id();
        constexpr int loggerCount = 2;
        std::array<int, 6> loggerIds = {1, 1, 0, 0, 1, 0};
        std::vector<std::string> logNames(loggerCount);

        for (int i = 0; i < loggerCount; ++i)
        {
            logNames[i] = "Attribute logger " + std::to_string(i);
        }

        LogManagerHelper helper(stream, [&](Attributes&& attributes, Message&& message)
        {
            called = true;
            BOOST_TEST((attributes.thread.number() == 0));
            BOOST_TEST((attributes.thread.id() == id));
            BOOST_TEST((attributes.timestamp.ticks() == val));
            BOOST_TEST((attributes.timestamp.date.year() == tm.tm_year + 1900));
            BOOST_TEST((attributes.timestamp.date.month() == tm.tm_mon));
            BOOST_TEST((attributes.timestamp.date.day() == tm.tm_wday));
            BOOST_TEST((attributes.timestamp.date.time.hour() == tm.tm_hour));
            BOOST_TEST((attributes.timestamp.date.time.minute() == tm.tm_min));
            BOOST_TEST((attributes.timestamp.date.time.second() == tm.tm_sec));
            BOOST_TEST((attributes.level.value() == level));
            BOOST_TEST((attributes.level.name() == levelString));
            BOOST_TEST((attributes.logger.name() == logNames[attributes.logger.id()]));
            BOOST_TEST((message.what() == logNames[attributes.logger.id()] + '0'));

            return "";
        },
        [&]
        {
          return val;
        });

        std::vector<BasicLogger<StringStreamFormatter>> loggers;

        for (const auto& logName : logNames)
            loggers.emplace_back(helper.manager.get(logName));

        for (int i = 0; i < loggerIds.size(); ++i)
            loggers[loggerIds[i]].log(level, logNames[loggerIds[i]] + '0');

    }

    BOOST_TEST(called);
}

BOOST_AUTO_TEST_CASE(LoggerModes)
{
    std::ostringstream stream;
    std::string testMessage = "text";
    {
        LogManagerHelper helper(stream);
        auto logger = helper.manager.get("", LogConfig::severity > LogConfig::LogLevel::Trace);
        logger.log(LogConfig::LogLevel::Trace, testMessage);
        logger(LogConfig::LogLevel::Trace) << testMessage;
        logger.log(LogConfig::LogLevel::Trace) << testMessage;
        logger.trace(testMessage);
    }
    BOOST_REQUIRE(stream.str().empty());

    {
        LogManagerHelper helper(stream);
        auto logger = helper.manager.get("", LogConfig::severity > LogConfig::LogLevel::Trace);
        logger.log(LogConfig::LogLevel::Debug, testMessage);
        logger(LogConfig::LogLevel::Debug) << testMessage;
        logger.log(LogConfig::LogLevel::Debug) << testMessage;
        logger.debug(testMessage);
    }

    auto testMessageMass = testMessage;
    for (int i = 0; i < 3; ++i)
        testMessageMass += '\n' + testMessage;
    testMessageMass += '\n';
    BOOST_REQUIRE(stream.str() == testMessageMass);

}

constexpr auto dummyMessage = "dummy";
constexpr auto nonDummyMessage = "non dummy";
struct Formatter
{
    template<typename... Args>
    std::string format(Args&&...)
    {
        return dummyMessage;
    }
};

BOOST_AUTO_TEST_CASE(LogManagerFunctionality)
{

    std::ostringstream sink;

    std::map<char, std::pair<std::string, bool>> expectedLines = {
        {'1', {dummyMessage, false}},
        {'2', {dummyMessage, false}},
        {'3', {dummyMessage, false}},
        {'4', {nonDummyMessage, false}},
        {'5', {dummyMessage, false}},
        {'6', {nonDummyMessage, false}} };
    
    {
        LogManagerHelper helper(sink, [](Attributes&& attributes, Message&& message)
        {
            return attributes.logger.name() + message.what();
        });
        auto& manager = helper.manager;
        {
            auto logger = manager.get("1", LogConfig::severity == LogConfig::LogLevel::Debug, Formatter());
            logger.trace("");
            logger.debug("");
        }
        {
            auto logger = manager.get("2", Formatter());
            logger.trace("");
        }
        {
            auto logger = manager.get<Formatter>("3", LogConfig::severity == LogConfig::LogLevel::Debug);
            logger.trace("");
            logger.debug("");
        }
        {
            auto logger = manager.get("4", LogConfig::severity == LogConfig::LogLevel::Debug);
            logger.trace("");
            logger.debug(nonDummyMessage);
        }
        {
            auto logger = manager.get<Formatter>("5");
            logger.trace("");
        }
        {
            auto logger = manager.get("6");
            logger.trace(nonDummyMessage);
        }
    }
    std::istringstream in(sink.str());
    for (std::string line; std::getline(in, line); )
    {
        BOOST_TEST(std::string(line.c_str() + 1) == expectedLines[line[0]].first);
        expectedLines[line[0]].second = true;
    }

    for (const auto& pp : expectedLines)
    {
        BOOST_TEST(pp.second.second == true);
    }

}

BOOST_AUTO_TEST_SUITE_END()
