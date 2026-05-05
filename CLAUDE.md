# CLAUDE.md

## 项目概述

PictThumbs 是一个独立的 Windows Shell 缩略图提供程序，从 Pictus 图像查看器项目中提取。它能让 Windows 资源管理器显示原生不支持的图像格式的缩略图。

**仓库地址**: https://github.com/Zhanghuaimin-233/PictThumbs

**支持格式**: PCX, TGA, WBMP, PSD, PSP, WebP, XYZ

**功能**:
- 缩略图显示 (IThumbnailProvider)
- 文件类型图标叠加 (48x48, 右下角)
- 系统原生白底阴影效果

## 构建环境

### 系统要求
- Windows 10/11
- Visual Studio 2019+ (需要 C++ 桌面开发工作负载)
- CMake 3.15+ (VS2019 自带)

### 构建命令

```bash
# 进入项目目录
cd E:\Dev\Projects\PictThumbs

# 创建构建目录
mkdir build
cd build

# 配置 (VS2019)
cmake .. -G "Visual Studio 16 2019" -A x64

# 构建 Release 版本
cmake --build . --config Release
```

### 输出文件
- DLL: `build/bin/Release/PictThumbs.dll`
- 大小: ~468KB

## 注册/卸载 DLL

**必须以管理员身份运行命令提示符！**

```cmd
# 注册
regsvr32 "E:\Dev\Projects\PictThumbs\build\bin\Release\PictThumbs.dll"

# 卸载
regsvr32 /u "E:\Dev\Projects\PictThumbs\build\bin\Release\PictThumbs.dll"
```

## 支持格式

| 格式 | 扩展名 | 描述 |
|------|--------|------|
| PCX | .pcx | Z-soft PCX (PC Paintbrush) |
| TGA | .tga | Truevision Targa |
| WBMP | .wbmp, .wbm | Wireless Bitmap |
| PSD | .psd | Adobe Photoshop |
| PSP | .psp, .pspimage | Corel Paint Shop Pro |
| WebP | .webp | Google WebP |
| XYZ | .xyz | RPG Maker XYZ |

**总计**: 7 种格式, 9 个扩展名

## 项目结构

```
PictThumbs/
├── src/                        # 主 DLL 源代码
│   ├── dllmain.cpp/h           # DLL 入口点
│   ├── cthumbprovider.cpp/h    # 缩略图提供程序 (核心)
│   ├── ClassFactory.cpp/h      # COM 类工厂
│   ├── codecsetup.cpp/h        # 编解码器配置
│   ├── regsetup.cpp/h          # 注册表操作
│   └── regutils.cpp/h          # 注册表工具
├── illa/                       # 图像编解码库
│   ├── core/                   # 核心框架
│   └── codecs/                 # 各格式编解码器
├── orz/                        # 工具库
├── metadata/                   # EXIF 元数据
├── third_party/                # 第三方库 (libwebp, zlib)
├── docs/                       # 文档目录
│   └── troubleshooting.md      # 问题排查指南
├── CMakeLists.txt              # 顶层构建配置
├── README.md                   # 英文文档
└── README.zh.md                # 中文文档
```

## 关键文件说明

### cthumbprovider.cpp
缩略图提供程序的核心实现：
- `Initialize()`: 接收文件流
- `GetThumbnail()`: 生成缩略图, 叠加文件类型图标
- `LoadSurface()`: 加载图像
- `FindCodec()`: 自动检测格式

### codecsetup.cpp
编解码器配置：
- `CodecManagerSetup()`: 初始化编解码器工厂, 注册 7 种格式

## 技术细节

### COM 接口
- `IInitializeWithStream`: 接收文件流
- `IThumbnailProvider`: 生成缩略图
- 线程模型: 单线程单元 (STA)

### 图像处理
- 缩放算法: Lanczos3 高质量重采样
- Alpha 处理: 自动检测并预乘
- 输出格式: 32bpp ARGB DIB

### 注册表结构
注册时写入：
- `HKCR\CLSID\{36FCD09A-...}`: COM 类注册
- `HKCR\.ext\shellex\{e357fccd-...}`: 文件扩展名关联

## 依赖关系

### 外部依赖
- Windows SDK (shlwapi, thumbcache, propsys, ws2_32, msimg32)
- C++17 标准库

### 内部依赖链
```
PictThumbs.dll
├── illa.lib (图像编解码)
│   ├── orz.lib (工具库)
│   ├── metadata.lib (EXIF)
│   ├── webp.lib (WebP 解码)
│   └── zlib.lib (压缩)
├── orz.lib
├── metadata.lib
├── webp.lib
├── webpdemux.lib
└── zlib.lib
```

## 重要发现

### 2026-05-04 Win11 兼容性问题排查

**问题**: 之前的版本（带自定义阴影、预览窗格）在 Win11 上有以下问题：
- 详细信息窗格显示文件图标而非缩略图
- 缩略图加载缓慢

**解决方案**: 重置为原项目 Pictus 的纯净缩略图实现后，所有功能正常：
- ✅ 缩略图显示
- ✅ 系统原生白底阴影效果
- ✅ 详细信息窗格正确显示缩略图

**结论**: Win11 对 Shell 扩展有更严格的安全限制，自定义的阴影效果可能导致兼容性问题。

## 已知问题

1. XYZ 格式在某些情况下可能不工作
2. 没有安装对应软件时，文件类型图标显示为空白
3. 需要管理员权限才能注册/卸载 DLL

## 后续改进方向

1. 添加更多格式支持 (如 AVIF, HEIF)
2. 创建安装程序自动注册 DLL
3. 支持 Windows 11 新版缩略图 API

## 注意事项

1. **Boost 已完全移除**: 所有 Boost 依赖已替换为 C++17 标准库
2. **Include 路径**: 编解码器文件使用 `illa/core/xxx.h` 和 `illa/codecs/xxx/xxx.h` 路径
3. **注册表权限**: 写入 HKCR 需要管理员权限
4. **文件编码**: 源代码使用 UTF-8 with BOM

## 快速上手

1. 打开项目: `E:\Dev\Projects\PictThumbs`
2. 构建: `cmake --build build --config Release`
3. 测试: 以管理员身份注册 DLL
4. 验证: 在资源管理器中查看 PSD/TGA/WebP 文件的缩略图

## 相关链接

- 原始项目: https://github.com/poppeman/Pictus
- 本项目: https://github.com/Zhanghuaimin-233/PictThumbs
