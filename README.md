# PictThumbs

Windows Shell Thumbnail Provider for additional image formats.

English | [简体中文](README.zh.md)

## Overview

PictThumbs is a standalone Windows Shell thumbnail provider extracted from the [Pictus](https://github.com/poppeman/Pictus) image viewer. It enables Windows Explorer to display thumbnails for image formats that are not natively supported by Windows.

## Supported Formats

| Format | Extension | Description |
|--------|-----------|-------------|
| PCX | .pcx | PC Paintbrush |
| TGA | .tga | Truevision Targa |
| WBMP | .wbmp | Wireless Bitmap |
| PSD | .psd | Adobe Photoshop |
| PSP | .psp | PaintShop Pro |
| WebP | .webp | Google WebP |
| XYZ | .xyz | Rolander XYZ |

## Building

### Prerequisites

- Windows 10/11
- CMake 3.15+
- Visual Studio 2019+ or compatible C++17 compiler

### Build Steps

```bash
# Clone the repository
git clone https://github.com/yourusername/PictThumbs.git
cd PictThumbs

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -G "Visual Studio 17 2022" -A x64

# Build
cmake --build . --config Release
```

The DLL will be output to `build/bin/Release/PictThumbs.dll`.

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
├── src/                    # Main DLL source code
│   ├── dllmain.cpp/h       # DLL entry point
│   ├── cthumbprovider.cpp/h # Thumbnail provider implementation
│   ├── ClassFactory.cpp/h  # COM class factory
│   ├── codecsetup.cpp/h    # Codec configuration
│   ├── regsetup.cpp/h      # Registry setup
│   └── regutils.cpp/h      # Registry utilities
├── illa/                   # Image codec library (simplified)
│   ├── core/               # Core framework
│   └── codecs/             # Individual codec implementations
├── orz/                    # Utility library (simplified)
├── metadata/               # EXIF metadata library
├── third_party/            # Third-party libraries
│   ├── libwebp/            # WebP decoder
│   └── zlib/               # Compression library
└── CMakeLists.txt          # Build configuration
```

## Technical Details

- **COM Interface**: Implements `IThumbnailProvider` and `IInitializeWithStream`
- **Threading Model**: Single-threaded apartment (STA)
- **Image Processing**: Lanczos3 resampling for high-quality thumbnails
- **Alpha Support**: Automatic alpha channel detection and premultiplication

## Dependencies

- Windows SDK (shlwapi, thumbcache, propsys)
- C++17 Standard Library

## License

This project is extracted from Pictus image viewer. See the original project for license information.

## Credits

- Original Pictus project by Pontus Mårdnäs
- libwebp by Google
- zlib by Jean-loup Gailly and Mark Adler
