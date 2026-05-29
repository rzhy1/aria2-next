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
#include "Log.h"

#include <algorithm>
#include <csignal>
#include <cstring>
#include <memory>
#include <vector>

#ifndef __MINGW32__
#  include <fcntl.h>
#  include <unistd.h>
#endif // !__MINGW32__

#include <spdlog/sinks/null_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "A2STR.h"
#include "Exception.h"
#include "Option.h"
#include "prefs.h"
#include "util.h"

namespace aria2 {
namespace log {

namespace {

std::shared_ptr<spdlog::logger> logger;
char crashLogPath[4096];

spdlog::level::level_enum toSpdlogLevel(Level level)
{
  switch (level) {
  case Level::Trace:
    return spdlog::level::trace;
  case Level::Debug:
    return spdlog::level::debug;
  case Level::Info:
    return spdlog::level::info;
  case Level::Warn:
    return spdlog::level::warn;
  case Level::Error:
    return spdlog::level::err;
  case Level::Critical:
    return spdlog::level::critical;
  case Level::Off:
    return spdlog::level::off;
  }
  return spdlog::level::info;
}

Level fromOptionValue(const std::string& value)
{
  if (value == V_TRACE) {
    return Level::Trace;
  }
  if (value == V_DEBUG) {
    return Level::Debug;
  }
  if (value == V_INFO) {
    return Level::Info;
  }
  if (value == V_WARN) {
    return Level::Warn;
  }
  if (value == V_ERROR) {
    return Level::Error;
  }
  if (value == V_CRITICAL) {
    return Level::Critical;
  }
  if (value == V_OFF) {
    return Level::Off;
  }
  return Level::Info;
}

void ensureLogger()
{
  if (!logger) {
    auto sink = std::make_shared<spdlog::sinks::null_sink_mt>();
    logger = std::make_shared<spdlog::logger>("aria2-next", sink);
    logger->set_level(spdlog::level::off);
    spdlog::set_default_logger(logger);
  }
}

void appendText(char* out, size_t outSize, size_t& pos, const char* text)
{
  while (*text && pos + 1 < outSize) {
    out[pos++] = *text++;
  }
  if (outSize > 0) {
    out[pos] = '\0';
  }
}

void appendUInt(char* out, size_t outSize, size_t& pos, unsigned int value)
{
  char digits[16];
  size_t n = 0;
  do {
    digits[n++] = static_cast<char>('0' + value % 10);
    value /= 10;
  } while (value && n < sizeof(digits));
  while (n > 0 && pos + 1 < outSize) {
    out[pos++] = digits[--n];
  }
  if (outSize > 0) {
    out[pos] = '\0';
  }
}

void appendCrashMessage(int signal)
{
  if (crashLogPath[0] == '\0') {
    return;
  }

  char message[192];
  size_t pos = 0;
  appendText(message, sizeof(message), pos,
             "\n[FATAL] aria2-next received signal ");
  appendUInt(message, sizeof(message), pos, static_cast<unsigned int>(signal));
  appendText(message, sizeof(message), pos,
             ". The process will re-raise the signal for the OS crash report "
             "or core dump.\n");

#ifdef __MINGW32__
  HANDLE file = CreateFileA(crashLogPath, FILE_APPEND_DATA,
                            FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                            OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (file != INVALID_HANDLE_VALUE) {
    DWORD written = 0;
    WriteFile(file, message, static_cast<DWORD>(pos), &written, nullptr);
    CloseHandle(file);
  }
#else  // !__MINGW32__
  int fd = ::open(crashLogPath, O_WRONLY | O_CREAT | O_APPEND, 0600);
  if (fd != -1) {
    const char* next = message;
    size_t left = pos;
    while (left > 0) {
      ssize_t written = ::write(fd, next, left);
      if (written <= 0) {
        break;
      }
      next += written;
      left -= static_cast<size_t>(written);
    }
    ::close(fd);
  }
#endif // !__MINGW32__
}

void fatalSignalHandler(int signal)
{
  appendCrashMessage(signal);
#ifdef HAVE_SIGACTION
  struct sigaction action;
  std::memset(&action, 0, sizeof(action));
  action.sa_handler = SIG_DFL;
  sigemptyset(&action.sa_mask);
  sigaction(signal, &action, nullptr);
  kill(getpid(), signal);
#else  // !HAVE_SIGACTION
  std::signal(signal, SIG_DFL);
  std::raise(signal);
#endif // !HAVE_SIGACTION
}

#ifdef __MINGW32__
LONG WINAPI unhandledExceptionHandler(EXCEPTION_POINTERS* info)
{
  int signal = SIGSEGV;
  if (info && info->ExceptionRecord) {
    switch (info->ExceptionRecord->ExceptionCode) {
    case EXCEPTION_ILLEGAL_INSTRUCTION:
      signal = SIGILL;
      break;
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
      signal = SIGFPE;
      break;
    default:
      signal = SIGSEGV;
      break;
    }
  }
  appendCrashMessage(signal);
  return EXCEPTION_CONTINUE_SEARCH;
}
#endif // __MINGW32__

void setCrashLogPath(const std::string& path)
{
  crashLogPath[0] = '\0';
  if (path.empty() || path == V_OFF) {
    return;
  }
  std::strncpy(crashLogPath, path.c_str(), sizeof(crashLogPath) - 1);
  crashLogPath[sizeof(crashLogPath) - 1] = '\0';
}

} // namespace

std::string defaultLogFilePath()
{
#ifdef __MINGW32__
  const char* localAppData = getenv("LOCALAPPDATA");
  std::string base = localAppData ? localAppData : util::getHomeDir();
  return base + "/aria2-next/logs/aria2-next.log";
#elif defined(__APPLE__)
  return util::getHomeDir() + "/Library/Logs/aria2-next/aria2-next.log";
#else
  const char* stateHome = getenv("XDG_STATE_HOME");
  std::string base = stateHome ? stateHome : util::getHomeDir() + "/.local/state";
  return base + "/aria2-next/logs/aria2-next.log";
#endif
}

Level parseLevel(const std::string& value) { return fromOptionValue(value); }

std::string levelName(Level level)
{
  switch (level) {
  case Level::Trace:
    return V_TRACE;
  case Level::Debug:
    return V_DEBUG;
  case Level::Info:
    return V_INFO;
  case Level::Warn:
    return V_WARN;
  case Level::Error:
    return V_ERROR;
  case Level::Critical:
    return V_CRITICAL;
  case Level::Off:
    return V_OFF;
  }
  return V_INFO;
}

Config configFromOption(const Option& option)
{
  Config config;
  config.file = option.get(PREF_LOG_FILE);
  if (config.file == V_AUTO) {
    config.file = defaultLogFilePath();
  }
  config.level = parseLevel(option.get(PREF_LOG_LEVEL));
  config.maxFileSize = static_cast<size_t>(option.getAsInt(PREF_LOG_MAX_SIZE));
  config.maxFiles = static_cast<size_t>(option.getAsInt(PREF_LOG_MAX_FILES));
  config.color = option.getAsBool(PREF_ENABLE_COLOR);
  config.console = !option.getAsBool(PREF_QUIET);
  return config;
}

void configure(const Config& config)
{
  std::vector<spdlog::sink_ptr> sinks;
  std::string activeLogFile;

  if (config.console && config.level != Level::Off) {
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    consoleSink->set_level(toSpdlogLevel(config.level));
    if (!config.color) {
      consoleSink->set_color_mode(spdlog::color_mode::never);
    }
    sinks.push_back(consoleSink);
  }

  if (!config.file.empty() && config.file != V_OFF && config.level != Level::Off) {
    auto slash = config.file.find_last_of("/\\");
    if (slash != std::string::npos) {
      util::mkdirs(config.file.substr(0, slash));
    }
    auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        config.file, config.maxFileSize, config.maxFiles);
    fileSink->set_level(toSpdlogLevel(config.level));
    sinks.push_back(fileSink);
    activeLogFile = config.file;
  }

  if (sinks.empty()) {
    sinks.push_back(std::make_shared<spdlog::sinks::null_sink_mt>());
  }

  logger = std::make_shared<spdlog::logger>("aria2-next", sinks.begin(),
                                            sinks.end());
  logger->set_pattern("%Y-%m-%d %H:%M:%S.%e [%l] [%s:%#] %v");
  logger->set_level(toSpdlogLevel(config.level));
  logger->flush_on(spdlog::level::warn);
  logger->enable_backtrace(128);
  spdlog::set_default_logger(logger);
  setCrashLogPath(activeLogFile);
  installCrashHandler();
}

void configureForTests()
{
  Config config;
  config.file = V_OFF;
  config.level = Level::Off;
  configure(config);
}

void shutdown()
{
  if (logger) {
    logger->flush();
  }
  spdlog::shutdown();
  logger.reset();
  setCrashLogPath("");
}

void installCrashHandler()
{
  std::signal(SIGABRT, fatalSignalHandler);
  std::signal(SIGFPE, fatalSignalHandler);
  std::signal(SIGILL, fatalSignalHandler);
  std::signal(SIGSEGV, fatalSignalHandler);
#ifdef SIGBUS
  std::signal(SIGBUS, fatalSignalHandler);
#endif // SIGBUS
#ifdef __MINGW32__
  SetUnhandledExceptionFilter(unhandledExceptionHandler);
#endif // __MINGW32__
}

bool enabled(Level level)
{
  ensureLogger();
  return logger->should_log(toSpdlogLevel(level));
}

void write(Level level, const char* file, int line, const std::string& message)
{
  ensureLogger();
  logger->log(spdlog::source_loc{file, line, ""}, toSpdlogLevel(level), "{}",
              message);
}

void write(Level level, const char* file, int line, const char* message)
{
  write(level, file, line, std::string(message));
}

void write(Level level, const char* file, int line, const std::string& message,
           const Exception& ex)
{
  write(level, file, line, message + "\n" + ex.stackTrace());
}

void write(Level level, const char* file, int line, const char* message,
           const Exception& ex)
{
  write(level, file, line, std::string(message), ex);
}

} // namespace log
} // namespace aria2
