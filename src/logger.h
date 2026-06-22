#pragma once
#include <string>
#include <filesystem>
#include <mutex>

namespace fs = std::filesystem;

class Logger {
public:
    enum Level { INFO, WARN, ERR };

    static void init(const fs::path& log_path);
    static void log(Level level, const char* format, ...);

private:
    Logger() = default;
    static Logger& get_instance() {
        static Logger instance;
        return instance;
    }
    fs::path m_log_path;
    std::mutex m_mutex;
};

#define LOG_INFO(...) Logger::log(Logger::INFO, __VA_ARGS__)
#define LOG_WARN(...) Logger::log(Logger::WARN, __VA_ARGS__)
#define LOG_ERR(...)  Logger::log(Logger::ERR, __VA_ARGS__)
