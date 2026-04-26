cmake_minimum_required(VERSION 3.14)
project(tAI
    VERSION 1.0.0
    DESCRIPTION "Terminal AI Assistant - CLI tool for multiple AI backends"
    HOMEPAGE_URL "https://github.com/example/tAI"
    LANGUAGES CXX
)

# ============================================================================
# Project Settings
# ============================================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable stricter warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ============================================================================
# Dependencies
# ============================================================================

# Find CURL (required for all backends)
find_package(CURL REQUIRED)

if(NOT CURL_FOUND)
    message(FATAL_ERROR "CURL library not found. Please install libcurl development files.")
endif()

message(STATUS "Found CURL: ${CURL_INCLUDE_DIRS}")

# ============================================================================
# Source Files
# ============================================================================

set(TAI_HEADERS
    include/tAI/Config.h
    include/tAI/IApiClient.h
    include/tAI/OpenAIClient.h
    include/tAI/OllamaClient.h
    include/tAI/OllamaCloudClient.h
    include/tAI/HuggingfaceClient.h
    include/tAI/GrokClient.h
    include/tAI/OpenRouterClient.h
    include/tAI/Utils.h
)

set(TAI_SOURCES
    src/main.cpp
    src/Config.cpp
    src/OpenAIClient.cpp
    src/OllamaClient.cpp
    src/OllamaCloudClient.cpp
    src/HuggingfaceClient.cpp
    src/GrokClient.cpp
    src/OpenRouterClient.cpp
    src/Utils.cpp
)

# ============================================================================
# Executable Target
# ============================================================================

add_executable(tAI ${TAI_SOURCES} ${TAI_HEADERS})

# Include directories
target_include_directories(tAI
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CURL_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(tAI
    PRIVATE
        CURL::libcurl
)

# ============================================================================
# Compiler Options
# ============================================================================

if(MSVC)
    target_compile_options(tAI PRIVATE /W4 /WX)
else()
    target_compile_options(tAI PRIVATE -Wall -Wextra -Wpedantic -Werror=unused-result)
endif()

# ============================================================================
# Installation Rules
# ============================================================================

# Install the executable
install(TARGETS tAI
    RUNTIME DESTINATION bin
    COMPONENT binaries
)

# Install headers (optional, for library usage)
install(FILES ${TAI_HEADERS}
    DESTINATION include/tAI
    COMPONENT development
)

# Install documentation
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
    install(FILES README.md
        DESTINATION share/doc/tAI
        COMPONENT documentation
    )
endif()

# Install man page if available
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/docs/tAI.1")
    install(FILES docs/tAI.1
        DESTINATION share/man/man1
        COMPONENT documentation
    )
endif()

# ============================================================================
# Summary
# ============================================================================

message(STATUS "")
message(STATUS "tAI Build Summary")
message(STATUS "=================")
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "CXX Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "CURL: ${CURL_LIBRARIES}")
message(STATUS "")
message(STATUS "Supported Engines:")
message(STATUS "  - OpenAI (GPT-3.5/GPT-4)")
message(STATUS "  - Ollama (local models)")
message(STATUS "  - Ollama Cloud (cloud models via API)")
message(STATUS "  - Hugging Face (Inference API)")
message(STATUS "  - xAI Grok")
message(STATUS "  - OpenRouter (model aggregator)")
message(STATUS "")
message(STATUS "Installation prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
message(STATUS "Build with: cmake --build . --config Release")
message(STATUS "Install with: cmake --install . --prefix /usr/local")
message(STATUS "")
