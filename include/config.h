#pragma once
#include <string>
#include <nlohmann/json.hpp>

namespace CONFIG {
    extern const std::string SERVER_IP;
    extern const unsigned short SERVER_PORT_API;
    extern const unsigned short SERVER_PORT_SOCKET;
    extern const std::string secret_key;
    extern const std::string DB_URL;
}