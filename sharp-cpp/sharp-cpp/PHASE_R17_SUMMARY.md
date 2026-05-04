# Phase R17 — `_Pragma("once")` guard 修复 + musl stress + 152/152 glibc 全覆盖

> Status: **610/610 baseline ✓ + ASAN ✓ + strict ✓ + self_smoke 20/20 零诊断 ✓**
> **gcc-diff 累计：/usr/include/*.h 152/152 (100%) + 190+ corpus 文件 全 100%**

## 修复的 3 个 Bug

### Bug R17-1：`_Pragma("once")` 不触发 include-guard

**症状**：
```c
// header.h
_Pragma("once")
int x;
// main.c
#include "header.h"
#include "header.h"  // sharp-cpp: int x 出现两次
```

**根本原因**：新的 `emit_pragma_text` 直接向 `st->out_text` 写 `#pragma once\n`，绕过了 `handle_pragma` 里的 `guard_set` 调用。

**修法**：重写 `emit_pragma_text` — 把 de-stringized 文本重新 tokenize 后，调用 `handle_pragma(st, &tl, loc, current_file)`，与 `#pragma once` 走同一路径。两种形式（直接 `_Pragma`、宏展开后的 `_Pragma`）都需要传入 `current_file`，为此给 `emit_expanded_list` 增加 `current_file` 参数，三处调用点同步更新。

### Bug R17-2：`#pragma` 输出不在行首（二次修复）

**症状**：glib.h 的 `GLIB_DEPRECATED_IN_*` 宏展开后，`#pragma GCC diagnostic push` 紧贴前一行 typedef 内容，导致 token stream 中出现 `; # pragma` 的混排。

**根本原因**：`handle_pragma` 的 `flush_pending_lm` 只处理 pending linemarker，不检查 `out_text` 最后一个字符是否是 `\n`。

**修法**：在 `handle_pragma` 的 emit 路径加 `if (out_text.buf[last] != '\n') push '\n'`，确保 pragma 始终独占一行。

### Bug R17-3：`__indirect_return__` 不在属性表

**症状**：`ucontext.h` 里的 `__glibc_has_attribute(__indirect_return__)` 返回 0，fallback 到 `__returns_twice__`，token 流与 gcc 不一致。

**修法**：在 `expr.c` 的 `gcc_attrs[]` 表末尾加 `"indirect_return"`（GCC 13 为 CET/shadow-stack 新增）。

## 新增 Corpus

| 组          | 文件数 | 代表性                                  |
|-----------|-------|----------------------------------------|
| musl libc  | 25    | stdio/stdlib/string/pthread/socket/… 全套 |
| 更多库      | 11    | hiredis / jansson / yaml / sodium / amqp / openssl-ext |
| glibc sweep | 152  | /usr/include/*.h 全量（R17 前 1 失败）|

## 验证矩阵

```
make test           610/610 ✓
make asan           610/610 ✓
make strict         610/610 ✓
self_smoke          20/20   零诊断 ✓

gcc-diff corpus (累计):
  R13-16 旧 corpus    149/149  ✓
  R17 musl             25/25  ✓
  R17 更多库           11/11  ✓
  glib+dbus(含flags)    6/ 6  ✓
  /usr/include/*.h全量 152/152 ✓  ← 新
  ──────────────────────────────
  可量化总计          343/343  100% ✓
```

## 关键洞察

`_Pragma` 的修复需要三层一致性：输出文字、触发 guard、确保行首。R16 修了第一层，R17 修了后两层。教训：预处理器指令（pragma/include/define）不能只发文字——必须执行语义副作用。

## R18+ 候选

1. **性能优化**：bench 1.55×，TokList 节点池化可显著减少 malloc 调用
2. **`__GNUC_MINOR__` 精度**：当前 2 vs gcc 的 3，影响极少数版本检查
3. **Windows MSVC 兼容**：`_WIN32`/`_MSC_VER` 目标下的 corpus
4. **更大扫描**：/usr/include 全部子目录（sys/bits/netinet/arpa 等数百个头）
