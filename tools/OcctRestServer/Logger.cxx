#include "Logger.hxx"
#include <chrono>
#include <iomanip>
#include <ctime>

Logger& Logger::instance()
{
  static Logger inst;
  return inst;
}

void Logger::setLevel(LogLevel level)
{
  currentLevel.store(level);
}

std::string Logger::getTimestamp()
{
  auto now        = std::chrono::system_clock::now();
  auto time_t_now = std::chrono::system_clock::to_time_t(now);
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

  std::tm bt{};
#if defined(_MSC_VER)
  localtime_s(&bt, &time_t_now);
#else
  localtime_r(&time_t_now, &bt);
#endif

  std::ostringstream oss;
  oss << std::put_time(&bt, "%H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << ms.count();
  return oss.str();
}

std::string Logger::levelToString(LogLevel level)
{
  switch (level)
  {
    case LogLevel::DEBUG:
      return "DEBUG";
    case LogLevel::INFO:
      return "INFO ";
    case LogLevel::WARN:
      return "WARN ";
    case LogLevel::ERR:
      return "ERROR";
    default:
      return "UNKNOWN";
  }
}

void Logger::log(LogLevel level, const std::string& message)
{
  // Doppelte Prüfung (einmal in log_format für Performance, hier für direkte Aufrufe)
  if (static_cast<int>(level) < static_cast<int>(currentLevel.load()))
  {
    return;
  }

  std::lock_guard<std::mutex> lock(mtx);
  std::cout << "[" << getTimestamp() << "] "
            << "[" << levelToString(level) << "] " << message << std::endl;
}