#ifndef LOGGER_H
#define LOGGER_H

#include <string>

// Initialize logger (optional, safe to call once)
void init_logger(const std::string& logfile);

// Thread-safe log function
void log_event(const std::string& message);

#endif
