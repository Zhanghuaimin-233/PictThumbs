# 问题排查指南

本文档记录了 PictThumbs 项目开发过程中遇到的问题和解决方案，供后续开发参考。

## 问题状态说明

- ✅ 已解决
- ❌ 未解决

---

## 目录

1. [缩略图相关问题](#缩略图相关问题)
2. [Windows 11 兼容性问题](#windows-11-兼容性问题)
3. [注册表相关问题](#注册表相关问题)
4. [调试技巧](#调试技巧)

---

## 缩略图相关问题

### 问题 1: PSD 文件无法显示缩略图 ✅

**症状**: 某些 PSD 文件只显示文件图标，没有图像预览。

**原因**: PSD 编解码器有以下限制：
- 不支持 16 位/32 位颜色深度（只支持 8 位）
- 不支持 ZIP 压缩（只支持 RLE 和 Raw）
- 不支持 CMYK 颜色模式（只支持 RGB、Indexed、Grayscale、Bitmap）

**解决方案**: 在 Photoshop 中转换 PSD 文件：
1. 图像 → 模式 → 8 位/通道
2. 图像 → 模式 → RGB 颜色
3. 保存时选择 RLE 压缩

---

### 问题 2: 文件类型图标位置偏移 ✅

**症状**: 添加白底阴影效果后，文件类型图标从右下角移到了右侧中部。

**原因**: `OverlayFileTypeIcon` 函数只接收一个 `cx` 参数（宽度），但 `iconX` 和 `iconY` 都使用这个值计算：
```cpp
int iconX = (int)cx - iconSize - 2;
int iconY = (int)cx - iconSize - 2;  // 错误：应该用高度
```

**解决方案**: 修改函数签名，添加高度参数：
```cpp
// 头文件
void OverlayFileTypeIcon(HBITMAP hBitmap, UINT cx, UINT cy);

// 实现
int iconSize = std::max(16, (int)(std::min(cx, cy) / 4));
int iconX = (int)cx - iconSize - 2;
int iconY = (int)cy - iconSize - 2;  // 使用 cy 而不是 cx

// 调用处
OverlayFileTypeIcon(*phbmp, totalWidth, totalHeight);
```

**相关文件**:
- [cthumbprovider.h](../src/cthumbprovider.h)
- [cthumbprovider.cpp](../src/cthumbprovider.cpp)

---

### 问题 2: 图标不够醒目 ✅

**症状**: 文件类型图标太小，不够醒目。

**解决方案**: 
1. 使用 `SHGFI_LARGEICON` 替代 `SHGFI_SMALLICON` 获取更大的图标
2. 增大图标显示比例从 1/4 到 1/3
3. 增加边距从 2px 到 4px

```cpp
// 修改前
SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES
int iconSize = std::max(16, (int)(std::min(cx, cy) / 4));

// 修改后
SHGFI_ICON | SHGFI_LARGEICON | SHGFI_USEFILEATTRIBUTES
int iconSize = std::max(24, (int)(std::min(cx, cy) / 3));
```

---

### 问题 3: Windows 11 缩略图缓存 ✅

**症状**: 修改 DLL 后，资源管理器仍显示旧的缩略图。

**原因**: Windows 有持久的缩略图缓存机制，存储在：
```
%LocalAppData%\Microsoft\Windows\Explorer\thumbcache_*.db
```

**解决方案**:
1. 手动清除：运行 Disk Cleanup，选择"缩略图"
2. 命令行清除：
```cmd
taskkill /f /im explorer.exe
del /q /s %localappdata%\Microsoft\Windows\Explorer\thumbcache_*.db
start explorer.exe
```

---

## Windows 11 兼容性问题

### 问题 4: Win11 详细信息窗格显示图标而非缩略图 ✅

**症状**: Win10 正常显示缩略图，Win11 显示文件图标。

**根本原因**: 自定义的 Shell 扩展功能（自定义阴影）导致 Win11 兼容性问题。

**解决方案**: 重置为原项目 Pictus 的纯净缩略图实现。

**验证结果**:
- ✅ 缩略图正常显示
- ✅ 系统原生白底阴影效果正常
- ✅ 详细信息窗格正确显示缩略图

**结论**: Win11 对 Shell 扩展有更严格的安全限制，自定义功能可能导致兼容性问题。保持与原项目一致的简洁实现是最佳选择。

---

## 注册表相关问题

### 问题 5: DLL 卸载时 0x80070002 错误 ✅

**症状**: 卸载 DLL 时出现错误代码 0x80070002。

**原因**: 注册表键不存在时 `RegDeleteTree` 返回错误。

**解决方案**: 在删除前检查键是否存在：
```cpp
auto ret = GetHkcrRegistryKeyAndValue(currentSubKey, 0);
auto hr = std::get<0>(ret);
if (FAILED(hr)) {
    // 键不存在，跳过删除
    return S_OK;
}
```

---

### 问题 6: 注册表权限问题 ✅

**症状**: 写入注册表失败。

**原因**: 写入 `HKEY_CLASSES_ROOT` 需要管理员权限。

**解决方案**: 以管理员身份运行命令提示符。

---

## 调试技巧

### 使用 DebugView

1. 下载 [DebugView](https://learn.microsoft.com/en-us/sysinternals/downloads/debugview)
2. 以管理员身份运行
3. 捕获 → 勾选 "Capture Win32"
4. 查看实时调试输出

### 检查注册表

```cmd
# 查看缩略图处理程序
reg query "HKCR\.psd\shellex\{e357fccd-a995-4576-b01f-234630154e96}"

# 查看 CLSID 注册
reg query "HKCR\CLSID\{36FCD09A-A906-4cd0-8EC9-52EB6E097DFB}"
```

### 重启资源管理器

```cmd
taskkill /f /im explorer.exe
start explorer.exe
```

### 清除缩略图缓存

```cmd
taskkill /f /im explorer.exe
del /q /s %localappdata%\Microsoft\Windows\Explorer\thumbcache_*.db
start explorer.exe
```

---

## 参考资源

- [微软官方 - IThumbnailProvider](https://learn.microsoft.com/en-us/windows/win32/api/thumbcache/nn-thumbcache-ithumbnailprovider)
- [微软官方 - Thumbnail Handlers](https://learn.microsoft.com/en-us/windows/win32/shell/thumbnail-providers)
- [原始项目 Pictus](https://github.com/poppeman/Pictus)

---

## 更新日志

- **2026-05-04**: 初始版本，记录开发过程中遇到的问题和解决方案
