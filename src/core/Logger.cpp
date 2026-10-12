#include "core/Logger.h"
#include <chrono>
#include <iomanip>
#include <sstream>

namespace sonora::core {

namespace {
std::string formatTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::tm bt{};
#if defined(_WIN32)
    localtime_s(&bt, &in_time_t);
#else
    localtime_r(&in_time_t, &bt);
#endif

    std::ostringstream ss;
    ss << std::put_time(&bt, "%Y-%m-%d %H:%M:%S")
       << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

const char* levelToString(LogLevel level) noexcept {
    switch (level) {
        case LogLevel::Debug:   return "DEBUG";
        case LogLevel::Info:    return "INFO ";
        case LogLevel::Warning: return "WARN ";
        case LogLevel::Error:   return "ERROR";
    }
    return "UNKNOWN";
}
} // namespace

Logger& Logger::getInstance() noexcept {
    static Logger instance;
    return instance;
}

Logger::~Logger() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (fileStream_.is_open()) {
        fileStream_.flush();
        fileStream_.close();
    }
}

void Logger::initialize(const std::string& logFilePath) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (fileStream_.is_open()) {
        fileStream_.close();
    }
    fileStream_.open(logFilePath, std::ios::out | std::ios::app);
    initialized_ = fileStream_.is_open();
}

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string line = "[" + formatTimestamp() + "] [" + levelToString(level) + "] " + message;

    std::cout << line << std::endl;

    if (fileStream_.is_open()) {
        fileStream_ << line << "\n";
        fileStream_.flush();
    }
}

} // namespace sonora::core
