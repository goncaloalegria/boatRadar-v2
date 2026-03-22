cmake_minimum_required(VERSION 3.30)
project(ProjetoFinalRecurso C)

set(CMAKE_C_FLAGS "-Wall -Wextra -g -Wvla -Wpedantic -Wdeclaration-after-statement")

set(CMAKE_C_STANDARD 11)

add_executable(ProjetoFinalRecurso main.c)

target_link_libraries(ProjetoFinalRecurso m)