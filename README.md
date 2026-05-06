# PictThumbs

Windows Shell Thumbnail Provider for additional image formats.

English | [简体中文](README.zh.md)

## Overview

PictThumbs is a standalone Windows Shell thumbnail provider extracted from the [Pictus](https://github.com/poppeman/Pictus) image viewer. It enables Windows Explorer to display thumbnails for image formats that are not natively supported by Windows.

## Supported Formats

| Format | Extension | Description |
|--------|-----------|-------------|
| PCX | .pcx | Z-soft PCX (PC Paintbrush) |
| TGA | .tga | Truevision Targa |
| WBMP | .wbmp, .wbm | Wireless Bitmap |
| PSD | .psd | Adobe Photoshop |
| PSP | .psp, .pspimage | Corel Paint Shop Pro |
| WebP | .webp | Google WebP |
| XYZ | .xyz | RPG Maker XYZ |

**Total**: 7 formats, 9 extensions

## Building

### Prerequisites

- Windows 10/11
- Visual Studio 2019+ (with C++ desktop development workload)
- CMake 3.15+ (included with Visual Studio)

### Build Steps

```bash
# Clone the repository
git clone https://github.com/Zhanghuaimin-233/PictThumbs.git
cd PictThumbs

# Create build directory
mkdir build
cd build

# Configure with CMake (VS2019)
cmake .. -G "Visual Studio 16 2019" -A x64

# Or for VS2022
cmake .. -G "Visual Studio 17 2022" -A x64

# Build
cmake --build . --config Release
```

The DLL will be output to `build/bin/Release/PictThumbs.dll`.

### Using Visual Studio IDE

1. Open the project folder in Visual Studio
2. Visual Studio will automatically detect CMakeLists.txt
3. Select `Release` configuration and `x64` platform
4. Build → Build Solution (Ctrl+Shift+B)

### Using CLion

1. Open the project folder in CLion
2. Configure toolchain to use Visual Studio (MSVC)
3. Build the project

## Installation

### Register the DLL

Run the following command as Administrator:

```cmd
regsvr32 PictThumbs.dll
```

### Unregister the DLL

```cmd
regsvr32 /u PictThumbs.dll
```

## Project Structure

```
PictThumbs/
├── src/                        # Main DLL source code
│   ├── dllmain.cpp/h           # DLL entry point
│   ├── cthumbprovider.cpp/h    # Thumbnail provider implementation
│   ├── ClassFactory.cpp/h      # COM class factory
│   ├── codecsetup.cpp/h        # Codec configuration
│   ├── regsetup.cpp/h          # Registry setup
│   └── regutils.cpp/h          # Registry utilities
├── illa/                       # Image codec library
│   ├── core/                   # Core framework (surface, filter, render)
│   └── codecs/                 # Individual codec implementations
├── orz/                        # Utility library
├── metadata/                   # EXIF metadata library
├── third_party/                # Third-party libraries
│   ├── libwebp/                # WebP decoder
│   └── zlib/                   # Compression library
├── CMakeLists.txt              # Top-level build configuration
└── build/                      # Build output (generated)
    └── bin/Release/
        └── PictThumbs.dll      # Output DLL
```

## Technical Details

- **COM Interface**: Implements `IThumbnailProvider` and `IInitializeWithStream`
- **Threading Model**: Single-threaded apartment (STA)
- **Image Processing**: Lanczos3 resampling for high-quality thumbnails
- **Alpha Support**: Automatic alpha channel detection and premultiplication
- **Icon Overlay**: File type icon (48x48) in bottom-right corner
- **C++ Standard**: C++17
- **Dependencies**: No external dependencies (Boost removed)

## Known Limitations

- **PSD format**: Does not support 16-bit/32-bit color depth, ZIP compression, or CMYK color mode. Convert to 8-bit RGB in Photoshop before use.

## Dependencies

- Windows SDK (shlwapi, thumbcache, propsys, ws2_32, msimg32)
- C++17 Standard Library

## License

This project is extracted from Pictus image viewer. See the original project for license information.

## Credits

- Original Pictus project by Pontus Mårdnäs
- libwebp by Google
- zlib by Jean-loup Gailly and Mark Adler
