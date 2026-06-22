#include "logger.h"
#include <fstream>
#include <ctime>
#include <cstdarg>

void Logger::init(const fs::path& log_path) {
    std::lock_guard<std::mutex> lock(get_instance().m_mutex);
    get_instance().m_log_path = log_path;
}

void Logger::log(Level level, const char* format, ...) {
    Logger& instance = get_instance();
    std::lock_guard<std::mutex> lock(instance.m_mutex);
    if (instance.m_log_path.empty()) return;

    char buf[4096];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);

    std::ofstream ofs(instance.m_log_path, std::ios::app);
    if (ofs.is_open()) {
        std::time_t now = std::time(nullptr);
        char t_buf[26];
        ctime_s(t_buf, sizeof(t_buf), &now);
        t_buf[24] = '\0';

        const char* level_str = "INFO";
        if (level == WARN) level_str = "WARN";
        else if (level == ERR) level_str = "ERR ";

        ofs << "[" << t_buf << "] [" << level_str << "] " << buf << std::endl;
    }
}
