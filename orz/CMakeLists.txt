cmake_minimum_required(VERSION 3.15)

set(ORZ_SOURCES
    types.cpp
    logger.cpp
    stream.cpp
    file_reader.cpp
    file_cache.cpp
    file_writer.cpp
    exception.cpp
    streamconv.cpp
    intl.cpp
    Win32/stream_windows.cpp
)

set(ORZ_HEADERS
    types.h
    logger.h
    stream.h
    file_reader.h
    file_cache.h
    file_writer.h
    exception.h
    streamconv.h
    intl.h
    intl_table_entry.h
    geom.h
    Win32/stream_windows.h
)

add_library(orz STATIC ${ORZ_SOURCES} ${ORZ_HEADERS})

target_include_directories(orz PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

if(WIN32)
    target_link_libraries(orz shlwapi)
endif()
