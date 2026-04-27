# Build Windows i686 (XP Compatible) Workflow

> **[📖 English](build-windows-i686-legacy.md)** · **[📖 简体中文](build-windows-i686-legacy.zh-cn.md)**  
> **[📖 Main README (English)](../../README.md)** · **[📖 主 README (简体中文)](../../README-cn.md)**

## Overview

Cross-compiles `fastfetch` for **Windows XP SP3 (i686)** via GitHub Actions using MSYS2 MINGW32. Execution is controlled entirely by commit message keywords.

## Keywords

| Keyword in commit message | Stage 1 — Check | Stage 2 — Build | Stage 3 — GitHub Release |
|---------------------------|:---:|:---:|:---:|
| `build action`            | ✅  | ✅  | ❌ |
| `build release`           | ✅  | ✅  | ✅  |
| *(none)*                  | ✅  | ❌  | ❌ |

> PRs are not supported by this workflow. Use `workflow_dispatch` in the GitHub UI to manually trigger any stage.

## Usage

```bash
# Stage 1 only (commit message check, no build)
git commit --allow-empty -m "docs: update README"

# Stages 1 + 2 (build only, no release)
git commit --allow-empty -m "ci: build Windows XP binary (build action)"

# Stages 1 + 2 + 3 (build + create GitHub Release)
git commit -m "release: v2.99.0 (build release)"
```

## Build Target

| Target | Architecture | Description |
|--------|:---:|------------|
| `i686-windows-xp` | i686 (32-bit) | Windows XP SP3 / Vista / 7, MSYS2 MINGW32 |

## CI Toolchain

| Component | Version / Detail |
|-----------|-----------------|
| CI Runner | `windows-latest` (GitHub Actions) |
| Shell     | MSYS2 MINGW32 |
| Compiler  | `mingw-w64-i686-gcc` (GCC i686) |
| C Standard | `gnu11` |
| C++ Standard | `gnu++17` |
| CMake     | `mingw-w64-i686-cmake` |
| Linker    | GNU ld via GCC (with LTO: `-flto=auto`) |
| Archiver  | `mingw-w64-i686-ninja` (Ninja build system) |
| Packager  | `mingw-w64-i686-7zip` |
| Subsystem | `console`, major OS version = 5, minor = 1 (XP) |

## Key CMake Options

| Option | Value | Description |
|--------|:-----:|-------------|
| `ENABLE_WINXP_COMPAT` | `ON` | Sets `_WIN32_WINNT=0x0501`, defines `FF_WINXP_COMPAT=1` |
| `ENABLE_WIN81_COMPAT` | `OFF` | Disables Windows 8.1+ API usage |
| `CMAKE_SYSTEM_PROCESSOR_OVERRIDE` | `i686` | Forces 32-bit target |

## Compiler Flags Explained

The following flags are used to ensure XP compatibility:

```
-Wall -Wextra -Wconversion -Werror=uninitialized -Werror=return-type
-Werror=vla -Werror=incompatible-pointer-types -Werror=implicit-function-declaration
-Werror=int-conversion -O2 -g -DNDEBUG -std=gnu11 -flto=auto
```

| Flag | Purpose |
|------|---------|
| `_WIN32_WINNT=0x0501` | Targets Windows XP (latest API available: XP SP3) |
| `FF_WINXP_COMPAT=1` | Enables runtime fallback code paths for XP |
| `FF_WIN81_COMPAT=0` | Disables Windows 8.1+ specific features |
| `WIN32_LEAN_AND_MEAN` | Excludes less common Windows headers |
| `NOMINMAX` | Prevents `min`/`max` macro conflicts |
| `UNICODE` | Enables Unicode (`W` suffixed Win32 APIs) |
| `-flto=auto` | Link-time optimization across translation units |

## Local Build (MSYS2 MINGW32)

```bash
# Install toolchain
pacman -S mingw-w64-i686-cmake mingw-w64-i686-gcc mingw-w64-i686-ninja \
          mingw-w64-i686-7zip mingw-w64-i686-pkg-config mingw-w64-i686-zlib

# Configure (32-bit XP compat build)
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

# Build
cmake --build . --verbose -j4
```

## Artifacts

After a successful run, the release (or CI artifact) contains:

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

## Known Limitations

- GPU detection may not work (D3DKMT not available on XP)
- Detection modules relying on `winbrand.dll` fall back to registry
- Some I/O stats require Vista+ kernel APIs
