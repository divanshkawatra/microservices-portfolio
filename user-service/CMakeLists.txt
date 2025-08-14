cmake_minimum_required(VERSION 3.10)
# ↑ "I need at least CMake version 3.10 to work"

project(UserService)
# ↑ "This project is called UserService"

set(CMAKE_CXX_STANDARD 17)
# ↑ "Use C++17 features (like std::optional, std::string_view, etc.) (C++14 features like auto, lambdas, etc.)"

set(CMAKE_CXX_STANDARD_REQUIRED ON)
# ↑ "C++17 is mandatory, don't fall back to older versions"

# Add debug flags
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")  # -g adds debug symbols, -O0 disables optimization


# --- Project Structure ---
# Tell CMake where your header files are, so e.g. #include "Database.h" works
# This is also where we put httplib.h and json.hpp
include_directories(
    include                   # Your project's header directory
    libs/argon2/include       # Argon2's public header directory
)

# Define our executable and its source files - Create an executable called 'user_service' from mentioned source files
add_executable(user_service
    # Your project's source files
    src/main.cpp
    src/Database.cpp
    src/UserService.cpp
    src/Logger.cpp
    src/PasswordService.cpp
    # Add more source files as you create them

    # --- Definitive list of required Argon2 source files ---
    libs/argon2/src/argon2.c
    libs/argon2/src/core.c
    libs/argon2/src/encoding.c
    libs/argon2/src/thread.c
    libs/argon2/src/ref.c
    # libs/argon2/src/opt.c  # For runtime CPU optimizations - compatible with intel processors only
    libs/argon2/src/blake2/blake2b.c
)


# --- Find Dependencies ---
# CMake will find the libraries on your system (works perfectly with Homebrew on macOS)
# No more manual include_directories() needed
find_package(SQLite3 REQUIRED)
find_package(Threads REQUIRED)

# --- Link Dependencies ---
# Link against the "imported targets". This automatically adds include paths and library paths.
# No more manual link_directories() or linking "sqlite3".
# SQLite3: For database operations
# Threads: For threading support (required by httplib, Argon2)
target_link_libraries(user_service PRIVATE
    SQLite::SQLite3
    Threads::Threads
)


# This is added to make life easier in VSCode
# It creates a compile_commands.json file for better IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)