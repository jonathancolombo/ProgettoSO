cmake_minimum_required(VERSION 3.25)
project(project_adas)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(src)

add_executable(project_adas
        data/frontCamera.data
        data/randomARTIFICIALE.binary
        data/urandomARTIFICIALE.binary
        src/esempio-processi.c
        src/hmi.c src/ecu.c src/throttlebycontrol.c src/throttlebycontrol.c src/throttlebycontrol.c src/brakebywire.c)
