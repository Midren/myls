cmake_minimum_required(VERSION 3.13)
project(myls)

set(CMAKE_CXX_STANDARD 17)
find_package(Boost COMPONENTS program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
add_executable(myls main.cpp config.cpp config.h util.cpp util.h file.cpp file.h)
target_link_libraries(myls ${Boost_LIBRARIES})