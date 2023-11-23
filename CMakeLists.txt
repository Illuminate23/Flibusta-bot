cmake_minimum_required(VERSION 3.27)
project(MyParser)

set(CMAKE_CXX_STANDARD 17)

set(SOURCES
    Parser/parser.cpp
)

# Укажите, где находятся заголовочные файлы вашего проекта
include_directories(Parser)

# Добавьте исполняемый файл
add_executable(my_parser ${SOURCES})

# Укажите флаги компиляции (замените на свои флаги)
target_compile_options(my_parser PRIVATE -Wall -std=c++17)

#curl
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})
target_link_libraries(my_parser PRIVATE ${CURL_LIBRARIES})


# Включите заголовочные файлы htmlcxx
target_include_directories(my_parser PRIVATE /usr/include/htmlcxx/html)
target_link_libraries(my_parser PRIVATE /usr/lib/libhtmlcxx.so)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
