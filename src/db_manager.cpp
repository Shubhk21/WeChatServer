#include "db_manager.h"


std::set<std::string> LDB::currently_active;

std::mutex LDB::is_active_mutex;