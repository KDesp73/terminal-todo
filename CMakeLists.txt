cmake_minimum_required(VERSION 3.5)
project(terminal-todo)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include directories
include_directories(include)

add_subdirectory(tui-kit)
include_directories(tui-kit)

# Add your source files
set(SOURCES
    src/main.cpp
)

# Add an executable target
add_executable(${PROJECT_NAME} ${SOURCES}) 
target_link_libraries(${PROJECT_NAME} TuiKit)

