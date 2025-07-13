cmake_minimum_required(VERSION 3.10)
# ↑ "I need at least CMake version 3.10 to work"

project(UserService)
# ↑ "This project is called UserService"

set(CMAKE_CXX_STANDARD 14)
# ↑ "Use C++14 features (like auto, lambdas, etc.)"

set(CMAKE_CXX_STANDARD_REQUIRED ON)
# ↑ "C++14 is mandatory, don't fall back to older versions"

include_directories(include)
# ↑ "Look for header files in the 'include' folder"
# This is where we put httplib.h and json.hpp

add_executable(user_service src/main.cpp)
# ↑ "Create an executable called 'user_service' from main.cpp"

target_link_libraries(user_service sqlite3 pthread)
# ↑ "Link these libraries when building user_service"
# sqlite3: For database operations
# pthread: For threading support (required by httplib)

# If you installed SQLite via Homebrew, uncomment these:
include_directories(/opt/homebrew/opt/sqlite/include)
link_directories(/opt/homebrew/opt/sqlite/lib)