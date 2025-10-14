cmake_minimum_required(VERSION 3.15..3.25)

# Project name and language
project(WeChatServer LANGUAGES CXX)

# Suppress all output except errors
set(CMAKE_MESSAGE_LOG_LEVEL ERROR)
set(CMAKE_RULE_MESSAGES OFF)
set(CMAKE_VERBOSE_MAKEFILE OFF)

# Suppress warnings from dependencies
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(CMAKE_WARN_DEPRECATED OFF)
set(FETCHCONTENT_QUIET ON)

# Disable all warnings globally
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")

# Include FetchContent module
include(FetchContent)

# === PostgreSQL ===
set(PostgreSQL_ROOT /opt/homebrew/Cellar/postgresql@15/15.14)
find_package(PostgreSQL REQUIRED)

# === libpqxx ===
FetchContent_Declare(
    libpqxx
    GIT_REPOSITORY https://github.com/jtv/libpqxx.git
    GIT_TAG 7.9.2
)
FetchContent_MakeAvailable(libpqxx)

# === nlohmann-json ===
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

# === cpp-httplib ===
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.14.3  # Use a stable release tag instead of master
)
FetchContent_MakeAvailable(httplib)

# Source files
set(SOURCES
    ./src/server.cpp
    ./src/config.cpp
    ./src/client_auth.cpp
    ./src/db_manager.cpp
    ./src/unix_server.cpp
    ./src/db_pool.cpp
)

# Create executable
add_executable(WeChatServer ${SOURCES})

# Link libraries
target_link_libraries(WeChatServer 
    PRIVATE 
        ${PostgreSQL_LIBRARIES}
        pqxx
        httplib::httplib
        nlohmann_json::nlohmann_json
)

# Include directories (if needed)
target_include_directories(WeChatServer 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${PostgreSQL_INCLUDE_DIRS}
)

set_target_properties(WeChatServer PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_compile_options(WeChatServer PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-w>  
)
