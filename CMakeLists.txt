cmake_minimum_required(VERSION 3.15)
project(image_compressor CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -O2)
endif()

add_executable(imgcomp
    src/main.cpp
    src/bitmap.cpp
)

add_executable(make_test_image
    src/make_test_image.cpp
    src/bitmap.cpp
)
