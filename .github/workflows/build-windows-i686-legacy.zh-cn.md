# 构建 Windows i686（XP 兼容）工作流

> **[📖 English](build-windows-i686-legacy.md)** · **[📖 简体中文](build-windows-i686-legacy.zh-cn.md)**  
> **[📖 Main README (English)](../../README.md)** · **[📖 主 README (简体中文)](../../README-cn.md)**

## 概述

通过 GitHub Actions 使用 MSYS2 MINGW32 环境将 `fastfetch` 交叉编译为 **Windows XP SP3 (i686)** 版本。执行与否完全由 commit 信息中的关键词控制。

## 关键词

| Commit 信息中的关键词 | 阶段一 — 校验 | 阶段二 — 构建 | 阶段三 — GitHub Release |
|----------------------|:---:|:---:|:---:|
| `build action`        | ✅  | ✅  | ❌ |
| `build release`       | ✅  | ✅  | ✅  |
| *(无关键词)*          | ✅  | ❌  | ❌ |

> 此工作流不支持 PR。可在 GitHub UI 中使用 `workflow_dispatch` 手动触发任意阶段。

## 用法

```bash
# 仅阶段一（校验 commit 信息，不构建）
git commit --allow-empty -m "docs: update README"

# 阶段一 + 阶段二（仅构建，不发布 Release）
git commit --allow-empty -m "ci: 构建 Windows XP 二进制 (build action)"

# 阶段一 + 阶段二 + 阶段三（构建 + 创建 GitHub Release）
git commit -m "release: v2.99.0 (build release)"
```

## 构建目标

| 目标 | 架构 | 说明 |
|--------|:---:|------|
| `i686-windows-xp` | i686 (32-bit) | Windows XP SP3 / Vista / 7，使用 MSYS2 MINGW32 |

## CI 工具链

| 组件 | 版本 / 详情 |
|------|------------|
| CI 运行环境 | `windows-latest` (GitHub Actions) |
| Shell | MSYS2 MINGW32 |
| 编译器 | `mingw-w64-i686-gcc` (GCC i686) |
| C 标准 | `gnu11` |
| C++ 标准 | `gnu++17` |
| CMake | `mingw-w64-i686-cmake` |
| 链接器 | GNU ld (via GCC，开启 LTO: `-flto=auto`) |
| 构建系统 | `mingw-w64-i686-ninja` (Ninja) |
| 打包工具 | `mingw-w64-i686-7zip` |
| 子系统 | `console`，主 OS 版本 = 5，次 = 1 (XP) |

## 关键 CMake 选项

| 选项 | 值 | 说明 |
|------|:---:|------|
| `ENABLE_WINXP_COMPAT` | `ON` | 设定 `_WIN32_WINNT=0x0501`，定义 `FF_WINXP_COMPAT=1` |
| `ENABLE_WIN81_COMPAT` | `OFF` | 禁用 Windows 8.1+ 专用 API |
| `CMAKE_SYSTEM_PROCESSOR_OVERRIDE` | `i686` | 强制 32 位目标 |

## 编译器标志说明

使用以下编译标志确保 XP 兼容性：

```
-Wall -Wextra -Wconversion -Werror=uninitialized -Werror=return-type
-Werror=vla -Werror=incompatible-pointer-types -Werror=implicit-function-declaration
-Werror=int-conversion -O2 -g -DNDEBUG -std=gnu11 -flto=auto
```

| 标志 | 作用 |
|------|------|
| `_WIN32_WINNT=0x0501` | 目标 Windows XP（可用 API 截止至 XP SP3） |
| `FF_WINXP_COMPAT=1` | 启用 XP 兼容的运行时回退代码路径 |
| `FF_WIN81_COMPAT=0` | 禁用 Windows 8.1+ 专用功能 |
| `WIN32_LEAN_AND_MEAN` | 排除不常用的 Windows 头文件 |
| `NOMINMAX` | 防止 `min`/`max` 宏冲突 |
| `UNICODE` | 启用 Unicode（`W` 后缀的 Win32 API） |
| `-flto=auto` | 跨编译单元链接时优化 |

## 本地构建（MSYS2 MINGW32）

```bash
# 安装工具链
pacman -S mingw-w64-i686-cmake mingw-w64-i686-gcc mingw-w64-i686-ninja \
          mingw-w64-i686-7zip mingw-w64-i686-pkg-config mingw-w64-i686-zlib

# 配置（32 位 XP 兼容构建）
cmake -G Ninja \
  -DCMAKE_SYSTEM_PROCESSOR_OVERRIDE=i686 \
  -DSET_TWEAK=Off \
  -DBUILD_TESTS=Off \
  -DENABLE_WINXP_COMPAT=ON \
  -DENABLE_WIN81_COMPAT=OFF \
  -DENABLE_VULKAN=OFF \
  -DENABLE_OPENCL=OFF \
  -DENABLE_DBUS=OFF \
  -DENABLE_GIO=OFF \
  -DENABLE_DCONF=OFF \
  -DENABLE_SQLITE3=OFF \
  -DENABLE_RPM=OFF \
  -DENABLE_IMAGEMAGICK7=OFF \
  -DENABLE_IMAGEMAGICK6=OFF \
  -DENABLE_CHAFA=OFF \
  -DENABLE_EGL=OFF \
  -DENABLE_GLX=OFF \
  -DENABLE_FREETYPE=OFF \
  -DENABLE_PULSE=OFF \
  -DENABLE_DDCUTIL=OFF \
  -DENABLE_ELF=OFF \
  -DENABLE_LIBZFS=OFF \
  -DENABLE_WORDEXP=OFF \
  .

# 构建
cmake --build . --verbose -j4
```

## 产物

运行成功后，Release（或 CI artifact）中包含：

```
fastfetch-windows-i686-legacy.zip
  ├── fastfetch.exe
  ├── flashfetch.exe
  ├── libgcc_s_dw2-1.dll
  ├── libwinpthread-1.dll
  ├── libstdc++-6.dll
  ├── presets/
  └── LICENSE
```

## 已知限制

- GPU 检测可能无法正常工作（XP 没有 D3DKMT API）
- 依赖 `winbrand.dll` 的检测模块会回退到注册表读取
- 部分 I/O 统计信息需要 Vista+ 内核 API
