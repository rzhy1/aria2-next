/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2026 AnInsomniacy
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
/* copyright --> */
#ifndef D_LOG_H
#define D_LOG_H

#include "common.h"

#include <cstddef>
#include <string>

namespace aria2 {

class Exception;
class Option;

namespace log {

enum class Level {
  Trace,
  Debug,
  Info,
  Warn,
  Error,
  Critical,
  Off,
};

struct Config {
  std::string file;
  Level terminalLevel = Level::Info;
  Level fileLevel = Level::Info;
  size_t maxFileSize = 10 * 1024 * 1024;
  size_t maxFiles = 5;
  bool color = true;
  bool console = true;
};

Config configFromOption(const Option& option);
void configure(const Config& config);
void configureForTests();
void shutdown();
void installCrashHandler();

std::string defaultLogFilePath();
Level parseLevel(const std::string& value);
std::string levelName(Level level);

bool enabled(Level level);
void write(Level level, const char* file, int line, const std::string& message);
void write(Level level, const char* file, int line, const char* message);
void write(Level level, const char* file, int line, const std::string& message,
           const Exception& ex);
void write(Level level, const char* file, int line, const char* message,
           const Exception& ex);

} // namespace log
} // namespace aria2

#define ARIA2_LOG_ENABLED(level) aria2::log::enabled(level)
#define ARIA2_LOG_DEBUG_ENABLED ARIA2_LOG_ENABLED(aria2::log::Level::Debug)

#define ARIA2_LOG_AT(level, message)                                            \
  do {                                                                         \
    if (ARIA2_LOG_ENABLED(level)) {                                             \
      aria2::log::write(level, __FILE__, __LINE__, (message));                 \
    }                                                                          \
  } while (false)

#define ARIA2_LOG_EX_AT(level, message, ex)                                     \
  do {                                                                         \
    if (ARIA2_LOG_ENABLED(level)) {                                             \
      aria2::log::write(level, __FILE__, __LINE__, (message), (ex));           \
    }                                                                          \
  } while (false)

#define ARIA2_LOG_TRACE(message) ARIA2_LOG_AT(aria2::log::Level::Trace, message)
#define ARIA2_LOG_DEBUG(message) ARIA2_LOG_AT(aria2::log::Level::Debug, message)
#define ARIA2_LOG_INFO(message) ARIA2_LOG_AT(aria2::log::Level::Info, message)
#define ARIA2_LOG_WARN(message) ARIA2_LOG_AT(aria2::log::Level::Warn, message)
#define ARIA2_LOG_ERROR(message) ARIA2_LOG_AT(aria2::log::Level::Error, message)
#define ARIA2_LOG_CRITICAL(message)                                            \
  ARIA2_LOG_AT(aria2::log::Level::Critical, message)

#define ARIA2_LOG_DEBUG_EX(message, ex)                                        \
  ARIA2_LOG_EX_AT(aria2::log::Level::Debug, message, ex)
#define ARIA2_LOG_INFO_EX(message, ex)                                         \
  ARIA2_LOG_EX_AT(aria2::log::Level::Info, message, ex)
#define ARIA2_LOG_WARN_EX(message, ex)                                         \
  ARIA2_LOG_EX_AT(aria2::log::Level::Warn, message, ex)
#define ARIA2_LOG_ERROR_EX(message, ex)                                        \
  ARIA2_LOG_EX_AT(aria2::log::Level::Error, message, ex)
#define ARIA2_LOG_CRITICAL_EX(message, ex)                                     \
  ARIA2_LOG_EX_AT(aria2::log::Level::Critical, message, ex)

#endif // D_LOG_H
