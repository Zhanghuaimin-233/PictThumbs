cmake_minimum_required(VERSION 3.15)

set(PICTTHUMBS_SOURCES
    dllmain.cpp
    cthumbprovider.cpp
    ClassFactory.cpp
    DllRef.cpp
    codecsetup.cpp
    regsetup.cpp
    regutils.cpp
)

set(PICTTHUMBS_HEADERS
    dllmain.h
    cthumbprovider.h
    ClassFactory.h
    DllRef.h
    codecsetup.h
    regsetup.h
    regutils.h
    res_pictthumbs.h
    res_version.h
)

set(PICTTHUMBS_RESOURCES
    PictThumbs.def
    res_pictthumbs.rc
    res_version.rc2
)

add_library(PictThumbs SHARED ${PICTTHUMBS_SOURCES} ${PICTTHUMBS_HEADERS} ${PICTTHUMBS_RESOURCES})

target_include_directories(PictThumbs PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

# Link dependencies
target_link_libraries(PictThumbs illa orz metadata shlwapi)

# Set output name
set_target_properties(PictThumbs PROPERTIES OUTPUT_NAME "PictThumbs")

# Windows-specific settings
if(WIN32)
    target_compile_definitions(PictThumbs PRIVATE PICTTHUMBS_EXPORTS WIN32_LEAN_AND_MEAN)
endif()
