#include "logger.h"

#include <fstream>
#include <mutex>
#include <ctime>

static std::mutex log_mutex;
static std::ofstream log_file;

void init_logger(const std::string& logfile) {
    std::lock_guard<std::mutex> lock(log_mutex);

    if (!log_file.is_open()) {
        log_file.open(logfile, std::ios::app);
    }
}

void log_event(const std::string& message) {
    std::lock_guard<std::mutex> lock(log_mutex);

    if (!log_file.is_open()) {
        return; // fail silently
    }

    std::time_t now = std::time(nullptr);
    char ts[32];
    std::strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S",
                  std::localtime(&now));

    log_file << "[" << ts << "] " << message << "\n";
    log_file.flush();
}
