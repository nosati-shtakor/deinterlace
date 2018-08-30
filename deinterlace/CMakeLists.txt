cmake_minimum_required(VERSION 3.7.2)
set (CMAKE_CXX_STANDARD 11)

project(deinterlace)

# include_directories(. "/usr/local/include")
include_directories(. "/usr/include/x86_64-linux-gnu")
# link_directories("usr/local/lib")
link_directories("/usr/lib/x86_64-linux-gnu")

add_executable(deinterlace Deinterlacer.cpp deinterlace.cpp LibJpeg.cpp pch.cpp)

target_link_libraries(deinterlace libjpeg.a)

