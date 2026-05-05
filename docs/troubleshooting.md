# 问题排查指南

本文档记录了 PictThumbs 项目开发过程中遇到的问题和解决方案，供后续开发参考。

## 问题状态说明

- ✅ 已解决
- ⚠️ 部分解决 / 有 workaround
- ❌ 未解决

---

## 目录

1. [缩略图相关问题](#缩略图相关问题)
2. [预览窗格相关问题](#预览窗格相关问题)
3. [Windows 11 兼容性问题](#windows-11-兼容性问题)
4. [注册表相关问题](#注册表相关问题)
5. [调试技巧](#调试技巧)

---

## 缩略图相关问题

### 问题 1: 文件类型图标位置偏移 ✅

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
- [cthumbprovider.h](src/cthumbprovider.h)
- [cthumbprovider.cpp](src/cthumbprovider.cpp)

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
1. 在 DLL 注册时自动清除缓存
2. 手动清除：运行 Disk Cleanup，选择"缩略图"
3. 命令行清除：
```cmd
taskkill /f /im explorer.exe
del /q /s %localappdata%\Microsoft\Windows\Explorer\thumbcache_*.db
start explorer.exe
```

**相关代码**: `regsetup.cpp` 中的 `ClearThumbnailCache()` 函数

---

## 预览窗格相关问题

### 问题 4: 预览窗格不工作 ❌

**症状**: Windows 资源管理器预览窗格显示"无法预览此文件"。

**当前状态**: 
- 已实现 `IPreviewHandler` 接口
- 注册表配置正确（已验证）
- `Prevhost.exe` 进程未启动（任务管理器中未见）
- Windows 11 资源管理器未调用预览处理程序
- 日志中无预览相关输出，说明 `Initialize` 和 `DoPreview` 从未被调用

**已尝试的解决方案**:
1. ✅ 实现 `IPreviewHandler` 接口
2. ✅ 实现 `IInitializeWithStream` 和 `IInitializeWithFile`
3. ✅ 实现 `IOleWindow`、`IObjectWithSite`、`IPreviewHandlerVisuals`
4. ✅ 注册到 `HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\PreviewHandlers`
5. ✅ 配置 AppID 和 DllSurrogate (prevhost.exe)
6. ✅ 设置 `DisableProcessIsolation` 注册表值
7. ✅ 注册表配置已验证正确
8. ❌ Prevhost.exe 进程未启动

**可能原因**:
- Windows 11 可能有额外的安全限制阻止加载未签名的 DLL
- 可能需要 Windows 数字签名
- 可能需要 App Manifest 声明
- 可能需要在文件夹选项中启用"显示预览处理程序"
- 原项目 Pictus 也没有实现预览窗格功能

**对基本功能的影响**: 不影响缩略图显示功能。

**相关文件**:
- [cpreviewhandler.h](src/cpreviewhandler.h)
- [cpreviewhandler.cpp](src/cpreviewhandler.cpp)

**待办**: 需要进一步研究 Windows 11 预览处理程序的安全要求，特别是 Prevhost.exe 加载条件。

---

### 问题 5: 预览处理程序未被调用 ❌

**症状**: 日志中没有预览处理程序的消息，Prevhost.exe 进程未启动。

**排查步骤**:
1. 检查注册表是否正确注册：
```cmd
reg query "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\PreviewHandlers" /v "{CLSID}"
reg query "HKCR\CLSID\{CLSID}"
reg query "HKCR\.psd\shellex\{8895b1c6-b41f-4c1c-a562-0d564250836f}"
```

2. 检查 AppID 配置：
```cmd
reg query "HKCR\CLSID\{CLSID}" /v AppID
reg query "HKCR\AppID\{6d2b5079-2f0b-48dd-ab7f-97cec514d30b}"
```

3. 检查 Prevhost.exe 是否存在：
```cmd
dir %SystemRoot%\system32\prevhost.exe
```

4. 重启资源管理器或重启电脑：
```cmd
taskkill /f /im explorer.exe
start explorer.exe
```

5. 检查文件夹选项：
   - 文件资源管理器 → 查看 → 确保"预览窗格"已启用
   - 文件夹选项 → 查看 → 确保"显示预览处理程序"已勾选

**当前状态**: 注册表配置正确，但 Prevhost.exe 进程未启动。Windows 11 似乎不会自动启动 Prevhost.exe 来加载我们的预览处理程序。

---

### 问题 6: 预览处理程序注册表路径错误 ✅

**症状**: 预览处理程序注册到 `HKEY_CLASSES_ROOT` 但不工作。

**原因**: `HKEY_CLASSES_ROOT` 不包含 `HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\PreviewHandlers`。

**解决方案**: 直接使用 `HKEY_LOCAL_MACHINE`：
```cpp
auto previewHandlersKey = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\PreviewHandlers";
HKEY hKey;
LONG result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, previewHandlersKey, 0, KEY_SET_VALUE, &hKey);
if (result == ERROR_SUCCESS)
{
    RegSetValueExW(hKey, wideClsId.c_str(), 0, REG_SZ, ...);
    RegCloseKey(hKey);
}
```

---

## Windows 11 兼容性问题

### 问题 7: Win11 详细信息窗格显示图标而非缩略图 ✅

**症状**: Win10 正常显示缩略图，Win11 显示文件图标。

**根本原因**: 自定义的 Shell 扩展功能（图标叠加、自定义阴影）导致 Win11 兼容性问题。

**解决方案**: 重置为原项目 Pictus 的纯净缩略图实现。

**验证结果**:
- ✅ 缩略图正常显示
- ✅ 系统原生白底阴影效果正常
- ✅ 详细信息窗格正确显示缩略图

**结论**: Win11 对 Shell 扩展有更严格的安全限制，自定义功能可能导致兼容性问题。

---

### 问题 8: Prevhost.exe 未正确配置 ✅

**症状**: 预览处理程序在 `prevhost.exe` 中无法加载。

**解决方案**: 配置 AppID 和 DllSurrogate：
```cpp
// 设置 AppID
SetHkcrRegistryKeyAndValue(clsidKey, "AppID", "{6d2b5079-2f0b-48dd-ab7f-97cec514d30b}");

// 注册 AppID
auto appIDKey = "AppID\\{6d2b5079-2f0b-48dd-ab7f-97cec514d30b}";
SetHkcrRegistryKeyAndValue(appIDKey, 0, "Preview Handler Surrogate Host");
SetHkcrRegistryKeyAndValue(appIDKey, "DllSurrogate", "%SystemRoot%\\system32\\prevhost.exe");
```

**参考**: [微软官方文档 - How to Register a Preview Handler](https://learn.microsoft.com/en-us/windows/win32/shell/how-to-register-a-preview-handler)

---

## 注册表相关问题

### 问题 9: DLL 卸载时 0x80070002 错误 ✅

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

### 问题 10: 注册表权限问题 ✅

**症状**: 写入注册表失败。

**原因**: 写入 `HKEY_CLASSES_ROOT` 需要管理员权限。

**解决方案**: 以管理员身份运行命令提示符。

---

## 调试技巧

### 查看日志

日志文件位置：
```
%LOCALAPPDATA%\PictThumbs\pictthumbs.log
```

### 使用 DebugView

1. 下载 [DebugView](https://learn.microsoft.com/en-us/sysinternals/downloads/debugview)
2. 以管理员身份运行
3. 捕获 → 勾选 "Capture Win32"
4. 查看实时调试输出

### 检查注册表

```cmd
# 查看缩略图处理程序
reg query "HKCR\.psd\shellex\{e357fccd-a995-4576-b01f-234630154e96}"

# 查看预览处理程序
reg query "HKCR\.psd\shellex\{8895b1c6-b41f-4c1c-a562-0d564250836f}"

# 查看全局预览处理程序列表
reg query "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\PreviewHandlers"

# 查看 AppID 配置
reg query "HKCR\CLSID\{5A7B3F5C-4E6D-4a8b-9C1E-2F3A4B5C6D7E}" /v AppID
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

- [微软官方 - Building Preview Handlers](https://learn.microsoft.com/en-us/windows/win32/shell/building-preview-handlers)
- [微软官方 - How to Register a Preview Handler](https://learn.microsoft.com/en-us/windows/win32/shell/how-to-register-a-preview-handler)
- [微软官方 - IThumbnailProvider](https://learn.microsoft.com/en-us/windows/win32/api/thumbcache/nn-thumbcache-ithumbnailprovider)
- [微软官方 - IPreviewHandler](https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nn-shobjidl_core-ipreviewhandler)
- [PowerToys 预览处理程序示例](https://github.com/microsoft/PowerToys/tree/main/src/modules/previewpane)

---

## 更新日志

- **2026-05-04**: 初始版本，记录开发过程中遇到的问题和解决方案
- **2026-05-04**: 添加 Win11 兼容性问题排查结果
