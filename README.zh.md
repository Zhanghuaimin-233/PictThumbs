# PictThumbs

Windows 资源管理器缩略图预览扩展，支持更多图像格式。

[English](README.md) | 简体中文

## 概述

PictThumbs 是从 [Pictus](https://github.com/poppeman/Pictus) 图像查看器中提取的独立 Windows Shell 缩略图提供程序。它能让 Windows 资源管理器显示原生不支持的图像格式的缩略图预览。

## 支持格式

| 格式 | 扩展名 | 说明 |
|------|--------|------|
| PCX | .pcx | PC Paintbrush 图像 |
| TGA | .tga | Truevision Targa 图像 |
| WBMP | .wbmp | 无线位图 |
| PSD | .psd | Adobe Photoshop 文档 |
| PSP | .psp | PaintShop Pro 文档 |
| WebP | .webp | Google WebP 图像 |
| XYZ | .xyz | Rolander XYZ 图像 |

## 构建

### 环境要求

- Windows 10/11
- CMake 3.15+
- Visual Studio 2019+ 或兼容的 C++17 编译器

### 构建步骤

```bash
# 克隆仓库
git clone https://github.com/Zhanghuaimin-233/PictThumbs.git
cd PictThumbs

# 创建构建目录
mkdir build
cd build

# 使用 CMake 配置
cmake .. -G "Visual Studio 17 2022" -A x64

# 构建
cmake --build . --config Release
```

构建完成后，DLL 文件位于 `build/bin/Release/PictThumbs.dll`。

## 安装

### 注册 DLL

以管理员身份运行以下命令：

```cmd
regsvr32 PictThumbs.dll
```

### 卸载 DLL

```cmd
regsvr32 /u PictThumbs.dll
```

## 项目结构

```
PictThumbs/
├── src/                    # 主 DLL 源代码
│   ├── dllmain.cpp/h       # DLL 入口点
│   ├── cthumbprovider.cpp/h # 缩略图提供程序实现
│   ├── ClassFactory.cpp/h  # COM 类工厂
│   ├── codecsetup.cpp/h    # 编解码器配置
│   ├── regsetup.cpp/h      # 注册表设置
│   └── regutils.cpp/h      # 注册表工具
├── illa/                   # 图像编解码库（精简版）
│   ├── core/               # 核心框架
│   └── codecs/             # 各格式编解码器实现
├── orz/                    # 工具库（精简版）
├── metadata/               # EXIF 元数据解析库
├── third_party/            # 第三方库
│   ├── libwebp/            # WebP 解码器
│   └── zlib/               # 压缩库
└── CMakeLists.txt          # 构建配置
```

## 技术细节

- **COM 接口**: 实现 `IThumbnailProvider` 和 `IInitializeWithStream`
- **线程模型**: 单线程单元 (STA)
- **图像处理**: 使用 Lanczos3 重采样算法生成高质量缩略图
- **Alpha 支持**: 自动检测 Alpha 通道并进行预乘处理

## 依赖

- Windows SDK (shlwapi, thumbcache, propsys)
- C++17 标准库

## 许可证

本项目提取自 Pictus 图像查看器。请参阅原项目了解许可证信息。

## 致谢

- 原 Pictus 项目作者 Pontus Mårdnäs
- libwebp by Google
- zlib by Jean-loup Gailly and Mark Adler
