cmake_minimum_required(VERSION 3.13)
project(PedalPi C)

set(CMAKE_C_STANDARD 99)

find_library(bcm2835_lib bcm2835)

add_executable(PedalPi src/mainBackup.c)
target_link_libraries(PedalPi bcm2835_lib)