cmake_minimum_required(VERSION 3.5)

project(cg)

include_directories(include)
add_executable(cg main.cpp)
target_sources(cg PRIVATE ${HEADER_FILES})
target_link_libraries(cg X11)
