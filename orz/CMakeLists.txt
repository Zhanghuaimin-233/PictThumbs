cmake_minimum_required(VERSION 3.15)

set(ORZ_SOURCES
    types.cpp
    logger.cpp
    stream.cpp
    stream_file.cpp
    stream_mem.cpp
    file_reader.cpp
    file_cache.cpp
    file_writer.cpp
    fileops.cpp
    exception.cpp
    streamconv.cpp
    ByteOrder.cpp
    bitconvert.cpp
    threadpool.cpp
    intl.cpp
    Win32/stream_windows.cpp
)

set(ORZ_HEADERS
    types.h
    logger.h
    stream.h
    stream_file.h
    stream_mem.h
    file_reader.h
    file_cache.h
    file_writer.h
    fileops.h
    exception.h
    streamconv.h
    ByteOrder.h
    bitconvert.h
    threadpool.h
    intl.h
    intl_language.h
    intl_table_entry.h
    geom.h
    Win32/stream_windows.h
)

add_library(orz STATIC ${ORZ_SOURCES} ${ORZ_HEADERS})

target_include_directories(orz PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

if(WIN32)
    target_link_libraries(orz shlwapi)
endif()
