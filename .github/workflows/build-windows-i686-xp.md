# Build Windows i686 (XP Compatible) Workflow

> **[📖 English](build-windows-i686-xp.md)**
> **[📖 简体中文](build-windows-i686-xp.zh-cn.md)**

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
| `i686-windows-xp` | i686 (32-bit) | Windows XP SP3 with MSYS2 MINGW32 |

## Artifacts

After a successful run, the release (or CI artifact) contains:

```
fastfetch-windows-i686-xp.zip
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
