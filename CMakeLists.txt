cmake_minimum_required(VERSION 3.15)
project(PictThumbs VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Windows-specific settings
if(WIN32)
    add_definitions(-DWIN32 -D_WINDOWS -D_UNICODE -DUNICODE)
    add_definitions(-DNOMINMAX -DHAVE_SNPRINTF)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS)
endif()

# Output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Third party libraries
add_subdirectory(third_party/zlib)
add_subdirectory(third_party/libwebp)

# Internal libraries
add_subdirectory(orz)
add_subdirectory(metadata)
add_subdirectory(illa)

# Main DLL
add_subdirectory(src)
