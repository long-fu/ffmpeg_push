# CMake lowest version requirement
cmake_minimum_required(VERSION 3.0.0)

# CMake lowest version requirement
project(push_stream VERSION 0.1.0 LANGUAGES C CXX)

# Compile options
add_compile_options(-std=c++11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  "../out")
set(CMAKE_CXX_FLAGS_DEBUG "-fPIC -O0 -g -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-fPIC -O2 -Wall")

# Header path
include_directories(
    ../inc/
    /usr/local/include/
)

# add host lib path
# link_directories(

# )

add_executable(push_stream
        rtsp_stream.cpp
        rtmp_stream.cpp
        main.cpp)

target_link_libraries(push_stream stdc++ pthread avcodec avutil swscale avformat avcodec avformat avdevice avutil swresample avfilter swscale)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
# install(TARGETS push_stream)
