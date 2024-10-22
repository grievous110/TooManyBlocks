cmake_minimum_required(VERSION 3.10)
project(GLEW)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)

include_directories(include)

add_definitions(-DGLEW_BUILD)
file(GLOB SOURCES "src/*.c")
add_library(GLEW ${SOURCES})

# If you want to install the library
install(TARGETS GLEW DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
