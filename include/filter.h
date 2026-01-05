#ifndef FILTER_H
#define FILTER_H

#include "http_parser.h"
#include <string>

// Initialize filter system (safe to call once)
void init_filter(const std::string& blocklist_path);

// Returns true if request should be blocked
bool is_blocked(const HttpRequest& req);

#endif
