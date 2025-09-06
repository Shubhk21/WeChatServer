cmake_minimum_required(VERSION 3.10)

# Project name and language
project(MyApp LANGUAGES CXX)

# Path to PostgreSQL (use forward slashes)
set(POSTGRESQL_INCLUDE_DIR "C:/Program Files/PostgreSQL/17/include")
set(POSTGRESQL_LIB "C:/Program Files/PostgreSQL/17/lib/libpq.lib")

# Create executable from main.cpp
add_executable(MyApp ./src/main.cpp)

# Include PostgreSQL headers
target_include_directories(MyApp PRIVATE ${POSTGRESQL_INCLUDE_DIR})

# Link PostgreSQL library
target_link_libraries(MyApp PRIVATE ${POSTGRESQL_LIB})

# Set C++ standard
set_target_properties(MyApp PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
