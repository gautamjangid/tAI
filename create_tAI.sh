#!/usr/bin/env bash
# create_tAI.sh - Generates the complete tAI project
set -e

ROOT="tAI_Root"

echo "Creating $ROOT ..."
mkdir -p "$ROOT"/{docs,scripts,include/tAI,src}

# --- CMakeLists.txt ---
cat > "$ROOT/CMakeLists.txt" << 'EOF'
cmake_minimum_required(VERSION 3.14)
project(tAI VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)

add_executable(tAI
    src/main.cpp
    src/Config.cpp
    src/DuckClient.cpp
    src/OllamaClient.cpp
    src/OllamaCloudClient.cpp
    src/HuggingfaceClient.cpp
    src/GrokClient.cpp
    src/OpenRouterClient.cpp
    src/Utils.cpp
)

include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(nlohmann_json)

target_include_directories(tAI PRIVATE include)
target_link_libraries(tAI PRIVATE CURL::libcurl nlohmann_json::nlohmann_json)

# Install rules
install(TARGETS tAI RUNTIME DESTINATION bin)
EOF
