cmake_minimum_required(VERSION 3.20)
project(TelegramHybridAssistant LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(CURL REQUIRED)

add_executable(tg_assistant
    src/main.cpp
    src/HttpClient.cpp
    src/TelegramBot.cpp
)

target_include_directories(tg_assistant PRIVATE include ${CURL_INCLUDE_DIRS})
target_link_libraries(tg_assistant PRIVATE CURL::libcurl)

target_compile_options(tg_assistant PRIVATE -Wall -Wextra -Wpedantic -O3)
