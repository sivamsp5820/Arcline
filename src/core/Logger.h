#pragma once

#include <string>
#include <mutex>
#include <fstream>
#include <iostream>
#include <memory>

namespace sonora::core {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

class Logger {
public:
    static Logger& getInstance() noexcept;

    void initialize(const std::string& logFilePath);
    void log(LogLevel level, const std::string& message);

    void debug(const std::string& message) { log(LogLevel::Debug, message); }
    void info(const std::string& message)  { log(LogLevel::Info, message); }
    void warn(const std::string& message)  { log(LogLevel::Warning, message); }
    void error(const std::string& message) { log(LogLevel::Error, message); }

private:
    Logger() = default;
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::mutex mutex_;
    std::ofstream fileStream_;
    bool initialized_{false};
};

} // namespace sonora::core
