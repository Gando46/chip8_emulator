cmake_minimum_required(VERSION 3.15)
project(chip8_emulator VERSION 1.0 LANGUAGES CXX)

# C++17 standard required
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find Raylib
# Option 1: If Raylib is installed system-wide
find_package(raylib 4.5 QUIET)

# Option 2: If not found, fetch it from GitHub
if(NOT raylib_FOUND)
    message(STATUS "Raylib not found, fetching from GitHub...")
    include(FetchContent)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.0
    )
    FetchContent_MakeAvailable(raylib)
endif()

# Source files
set(SOURCES
    src/chip8.cpp
    src/main.cpp
)

set(HEADERS
    src/chip8.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link Raylib
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Platform-specific settings
if(WIN32)
    # Windows-specific settings
    target_link_libraries(${PROJECT_NAME} PRIVATE winmm)
elseif(UNIX AND NOT APPLE)
    # Linux-specific settings
    target_link_libraries(${PROJECT_NAME} PRIVATE m pthread dl rt)
elseif(APPLE)
    # macOS-specific settings
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework IOKit" "-framework Cocoa" "-framework OpenGL")
endif()

# Install target
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# Print configuration summary
message(STATUS "")
message(STATUS "====================================")
message(STATUS "CHIP-8 Emulator Configuration")
message(STATUS "====================================")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Raylib Found: ${raylib_FOUND}")
message(STATUS "====================================")
message(STATUS "")
