#include "Logger.h"

#include <iostream>

void example()
{
    LogManager manager(std::cout, LogConfig::severity > LogConfig::LogLevel::Trace,
        [](Attributes&& attributes, Message&& message)
    {
        // e.g. "2011-10-08T07:07:09Z: some info".
        return attributes.timestamp.date.toIso8601() + ": " + message.what();
    },
        []
    {
        return std::time(nullptr);
    });

    auto logger = manager.get("example logger");

    logger.trace("trace example");
    logger.debug("debug example");
    logger.info("some info");
    logger.error("error occured");
    logger.fatal("fatal error");
    logger.log(LogConfig::LogLevel::Debug, "another way");
    logger.log(LogConfig::LogLevel::Trace) << "yet another way";
    logger(LogConfig::LogLevel::Info) << "yet another way";

    logger.flushNext().debug("this will be flushed when it reaches target");
}