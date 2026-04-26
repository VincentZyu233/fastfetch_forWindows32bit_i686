# 构建 Windows i686（XP 兼容）工作流

> **[📖 English](build-windows-i686-legacy.md)**
> **[📖 简体中文](build-windows-i686-legacy.zh-cn.md)**

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
| `i686-windows-xp` | i686 (32-bit) | Windows XP SP3，使用 MSYS2 MINGW32 |

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
