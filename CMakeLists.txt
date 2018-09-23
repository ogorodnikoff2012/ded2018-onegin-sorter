cmake_minimum_required(VERSION 3.0)
project(OneginSorter)

set(SRC src/main.c unicode/unicode_tables.c src/document.c src/unicode.c)
include_directories(include)
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -std=c11 -Wall -Wextra -g")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -std=c11 -Wall -Wextra")

add_executable(sorter ${SRC})
