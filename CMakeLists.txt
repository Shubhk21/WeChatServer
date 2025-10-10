cmake_minimum_required(VERSION 3.10)

# Project name and language
project(MyApp LANGUAGES CXX)

# Path to PostgreSQL (use forward slashes)
set(POSTGRESQL_INCLUDE_DIR "C:/Program Files/PostgreSQL/17/include")
set(POSTGRESQL_LIB "C:/Program Files/PostgreSQL/17/lib/libpq.lib")


include(FetchContent)
FetchContent_Declare(
  httplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
  GIT_TAG master  # or pick a release tag
)
FetchContent_MakeAvailable(httplib)


#ftxiu



# Sources
set(SOURCES
    ./src/Server.cpp
    ./src/windows_server.cpp
    ./src/config.cpp
    ./src/client_auth.cpp
    ./src/db_manager.cpp
    ./src/unix_server.cpp
    ./src/db_pool.cpp
)

# Create executable from main.cpp
add_executable(MyApp ${SOURCES})

# Include PostgreSQL headers
target_include_directories(MyApp PRIVATE ${POSTGRESQL_INCLUDE_DIR})
target_include_directories(MyApp PRIVATE ${CMAKE_SOURCE_DIR}/include)
# Link PostgreSQL library
target_link_libraries(MyApp PRIVATE ${POSTGRESQL_LIB})
target_link_libraries(MyApp PRIVATE httplib::httplib)
# Set C++ standard
set_target_properties(MyApp PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# === nlohmann-json via FetchContent ===
include(FetchContent)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)

FetchContent_MakeAvailable(nlohmann_json)
target_link_libraries(MyApp PRIVATE nlohmann_json::nlohmann_json)
