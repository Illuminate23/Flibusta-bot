cmake_minimum_required(VERSION 3.27)
project(Bot)

set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_STANDARD 17)
set(Boost_USE_MULTITHREADED ON)

set(SOURCES
    Parser/parser.cpp
    Bot/bot.cpp
)



include_directories(Parser)

add_executable(bot ${SOURCES})

target_compile_options(bot PRIVATE -Wall -std=c++17)

#curl
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})
target_link_libraries(bot PRIVATE ${CURL_LIBRARIES})


#htmlcxx
target_include_directories(bot PRIVATE /usr/include/htmlcxx/html)
target_link_libraries(bot PRIVATE /usr/lib/libhtmlcxx.so)

#tgbot
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost COMPONENTS system REQUIRED)
include_directories(/usr/local/include ${OPENSSL_INCLUDE_DIR} ${Boost_INCLUDE_DIR})
target_link_libraries(bot PRIVATE /usr/local/lib/libTgBot.a ${CMAKE_THREAD_LIBS_INIT} ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
