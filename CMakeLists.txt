project(dp)
set(CMAKE_CXX_STANDARD 17)

add_executable(dp main.cpp)

configure_file(config.json ${CMAKE_CURRENT_BINARY_DIR}/config.json COPYONLY)
