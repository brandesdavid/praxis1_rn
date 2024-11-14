project(RN-Praxis1)
cmake_minimum_required(VERSION 3.5)
set(CMAKE_C_STANDARD 11)


file(GLOB SOURCES "src/*.c")

add_executable(webserver ${SOURCES})

include_directories("includes")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CPACK_SOURCE_GENERATOR "TGZ")

set(CPACK_SOURCE_IGNORE_FILES ${CMAKE_BINARY_DIR} /\\..*$)
set(CPACK_VERBATIM_VARIABLES YES)

include(CPack)
