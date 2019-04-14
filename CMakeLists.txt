cmake_minimum_required(VERSION 3.0.0)
project(ConsoleUtils)

set (CMAKE_CPP_FLAGS "${CMAKE_CPP_FLAGS} -std=c11 -Wall")

add_executable(main main.c)
