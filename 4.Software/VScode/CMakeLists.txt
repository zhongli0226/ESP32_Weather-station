cmake_minimum_required(VERSION 3.25)
project(LVGL)

#  SET(CMAKE_C_COMPILER "D:/ProgramFiles/LLVM/x86/bin/clang.exe")
#  SET(CMAKE_CXX_COMPILER "D:/ProgramFiles/LLVM/x86/bin/clang++.exe")

message("-- " ${PROJECT_NAME} " " ${CMAKE_CXX_COMPILER})

set(CMAKE_CXX_STANDARD 20) #1

file(GLOB_RECURSE HEADERS lv_examples/*.h lv_drivers/*.h lvgl/*.h lv_conf.h lv_ex_conf.h lv_drv_conf.h)
file(GLOB_RECURSE SOURCES lv_examples/*.c image/*.c lv_drivers/*.c lvgl/*.c main/*.c)
include_directories(.  ./lvgl sdl2/include)
link_directories(sdl2/lib)
#message(${HEADERS})
#message(${SOURCES})
add_executable(LVGL  ${HEADERS} ${SOURCES})

target_link_libraries(LVGL libSDL2)
