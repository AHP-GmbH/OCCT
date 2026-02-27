#pragma once
#include <string>
#include <mutex>
#include <atomic>
#include <sstream>
#include <iostream>

enum class LogLevel
{
  DEBUG = 0,
  INFO  = 1,
  WARN  = 2,
  ERR   = 3
};

class Logger
{
public:
  static Logger& instance();

  void setLevel(LogLevel level);
  void log(LogLevel level, const std::string& message);

  // Variadic Templates für bequemes Loggen mit mehreren Argumenten
  template <typename... Args>
  void debug(Args... args)
  {
    log_format(LogLevel::DEBUG, args...);
  }

  template <typename... Args>
  void info(Args... args)
  {
    log_format(LogLevel::INFO, args...);
  }

  template <typename... Args>
  void warn(Args... args)
  {
    log_format(LogLevel::WARN, args...);
  }

  template <typename... Args>
  void error(Args... args)
  {
    log_format(LogLevel::ERR, args...);
  }

private:
  Logger()                         = default;
  ~Logger()                        = default;
  Logger(const Logger&)            = delete;
  Logger& operator=(const Logger&) = delete;

  // Hilfsmethode, um beliebige Typen in einen String zu streamen
  template <typename... Args>
  void log_format(LogLevel level, Args... args)
  {
    if (static_cast<int>(level) < static_cast<int>(currentLevel.load()))
    {
      return;
    }
    std::ostringstream oss;
    (oss << ... << args); // Fold Expression (C++17)
    log(level, oss.str());
  }

  std::atomic<LogLevel> currentLevel{LogLevel::INFO};
  std::mutex            mtx;

  std::string levelToString(LogLevel level);
  std::string getTimestamp();
};