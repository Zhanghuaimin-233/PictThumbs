cmake_minimum_required(VERSION 3.15)

set(METADATA_SOURCES
    Metadata_Document.cpp
    Metadata_Value.cpp
    Exif_Parser.cpp
    Exif_ReadIfd.cpp
    Exif_Converter.cpp
    Exif_IO.cpp
)

set(METADATA_HEADERS
    Metadata_Document.h
    Metadata_Value.h
    Metadata_Types.h
    Exif_Parser.h
    Exif_ReadIfd.h
    Exif_Converter.h
    Exif_IO.h
    Exif.h
    Exif_Types.h
)

add_library(metadata STATIC ${METADATA_SOURCES} ${METADATA_HEADERS})

target_include_directories(metadata PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(metadata orz)
