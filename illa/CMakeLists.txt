cmake_minimum_required(VERSION 3.15)

# Core framework sources
set(ILLA_CORE_SOURCES
    core/types.cpp
    core/color.cpp
    core/color_io.cpp
    core/palette.cpp
    core/surface.cpp
    core/surfacemgr.cpp
    core/surface_lock.cpp
    core/surface_lock_exclusive.cpp
    core/surface_lock_swmr.cpp
    core/surface_locked_area.cpp
    core/swsurface.cpp
    core/codec.cpp
    core/codec_static.cpp
    core/c_factory.cpp
    core/codecmgr.cpp
    core/imagecomposer.cpp
    core/imagecomposer_static.cpp
    core/render.cpp
    core/filter.cpp
    core/filter_int.cpp
    core/filter_buffer.cpp
    core/filter_lanczos3.cpp
    core/filter_nearest.cpp
    core/filter_bilinear.cpp
    core/filter_directcopy.cpp
    core/filter_premultiply.cpp
    core/filter_setalpha.cpp
    core/filterjob.cpp
)

# PCX codec sources
set(ILLA_PCX_SOURCES
    codecs/pcx/c_pcx.cpp
    codecs/pcx/f_pcx.cpp
    codecs/pcx/pcx_header.cpp
)

# TGA codec sources
set(ILLA_TGA_SOURCES
    codecs/tga/c_tga.cpp
    codecs/tga/c_tga_decoder.cpp
    codecs/tga/c_tga_header.cpp
    codecs/tga/c_tga_raw.cpp
    codecs/tga/c_tga_rle.cpp
    codecs/tga/f_tga.cpp
)

# WBMP codec sources
set(ILLA_WBMP_SOURCES
    codecs/wbmp/c_wbmp.cpp
    codecs/wbmp/f_wbmp.cpp
)

# PSD codec sources
set(ILLA_PSD_SOURCES
    codecs/psd/c_psd.cpp
    codecs/psd/f_psd.cpp
    codecs/psd/psd_header.cpp
    codecs/psd/psd_shared.cpp
)

# PSP codec sources
set(ILLA_PSP_SOURCES
    codecs/psp/c_psp.cpp
    codecs/psp/f_psp.cpp
    codecs/psp/psp_chunkskipper.cpp
)

# WebP codec sources
set(ILLA_WEBP_SOURCES
    codecs/webp/c_webp.cpp
    codecs/webp/f_webp.cpp
    codecs/webp/webp_imagecomposer.cpp
)

# XYZ codec sources
set(ILLA_XYZ_SOURCES
    codecs/xyz/c_xyz.cpp
    codecs/xyz/f_xyz.cpp
)

# Combine all sources
set(ILLA_SOURCES
    ${ILLA_CORE_SOURCES}
    ${ILLA_PCX_SOURCES}
    ${ILLA_TGA_SOURCES}
    ${ILLA_WBMP_SOURCES}
    ${ILLA_PSD_SOURCES}
    ${ILLA_PSP_SOURCES}
    ${ILLA_WEBP_SOURCES}
    ${ILLA_XYZ_SOURCES}
)

add_library(illa STATIC ${ILLA_SOURCES})

target_include_directories(illa PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

# Link dependencies
target_link_libraries(illa orz metadata webp zlib)
