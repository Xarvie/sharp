# Phase 4 交付说明

## 总览

Phase 4 完成指令层打磨。**累计 309 项回归测试全部通过**（41 既存 Sharp + 82 Phase 1 + 49 Phase 2 + 90 Phase 3 + 47 Phase 4），ASan/UBSan 全清，`-Werror -Wpedantic -Wshadow -Wstrict-prototypes` 通过。

Phase 0 取证报告的 10 项缺陷在 Phase 1-3 已悉数关闭。Phase 4 的 9 项探针未发现新崩溃或行为错误，但识别出 1 个真 bug 和若干"应有但缺失"的诊断改进。**Phase 4 修复 1 个 bug（`#line "name"` 不更新 `__FILE__`）+ 4 项主动加固。**

## 修复

### 1. `#line N "filename"` 现在正确更新 `__FILE__`（真 bug）

**病征**：
```c
#line 50 "renamed.c"
__FILE__   // 仍然展开成原文件名而不是 "renamed.c"
```

**根因**：`handle_line` 调用 `reader_set_line(rd, N)` 更新行号，但**不更新文件名**——只把新文件名写进 linemarker 输出和该行的 `loc.file`，不写回 reader 的状态。下一 token 由 reader 用旧 `rd->raw.filename` 打 loc 标签，所以 `__FILE__` 还是老名字。

**修复**：
- `pptok.c/h` 增加 `reader_set_file(CppReader*, const char*)` API
- `handle_line` 在更新 line 后立刻调用 `reader_set_file(rd, new_file)`
- 文件名是 `intern_cstr` 出的，生命周期由 `CppState` 管理，安全

### 2. 尾随 token 警告（GCC 风格）

GCC 对 `#endif foo`、`#else garbage`、`#ifdef X extra`、`#ifndef X extra`、`#undef X extra` 都发警告（`-Wendif-labels` 一族），本实现之前**静默接受**。

**修复**：
- 增加 `warn_trailing(st, dname, line, consumed_tok, loc)` 助手函数：从 `consumed_tok->next` 起跳过空白/换行/注释，碰到任何 `pp-token` 则发 `CPP_DIAG_WARNING`（不阻断编译）
- 在 `handle_endif`、`handle_else`、`handle_ifdef`/`#ifndef`、`handle_undef` 中调用
- 注释（`/* … */`、`//`）**不算**尾随 token——保留 `#endif /* CLOSE */` 这种合法用法
- 把 `handle_else`、`handle_endif` 签名从 `(st, loc)` 改成 `(st, line, loc)`，dispatcher 同步更新

### 3. CPP_DIAG_NOTE 助手

公开 API 早就有 `CPP_DIAG_NOTE` 等级，但 `directive.c` 没有 helper。

**修复**：增加 `emit_note(st, loc, fmt, ...)`，与 `emit_diag` 配套。约定：先发主诊断（warning/error），再用 `emit_note` 追加 `note: …` 说明。

### 4. `#include` 失败时列出已搜索路径

之前 `#include "foo.h"` 找不到只发一句 `error: file not found: foo.h`。用户得自己想是哪几个路径搜过了。

**修复**：在 `handle_include` 的 not-found 分支后追加 4 类 note：
- `searched (relative to including file): /dir/of/main.c/`（仅 `"…"` 形式）
- `searched (user, -I): /path/from/cpp_add_user_include`（每个 -I 一条）
- `searched (system): /path/from/cpp_add_sys_include`（每个 sys 一条）
- 注意 `<…>` 形式不发"relative to including file"（它本不该走那条路径）

这是 Clang 的 "<…> search starts here:" 风格的简化版。

### 5. 既有正确性**确认无回归**

下列功能 Phase 4 探针确认**仍然正确**：
- 自包含 `#include` 命中深度限制并 fatal，不崩溃
- `#line` 接受宏展开的行号（`#define LN 100 / #line LN`）
- `#line N` 后 `__LINE__` 给 N（标准要求）
- 重复 `#else` → 错误
- `#elif` 跟在 `#else` 后 → 错误
- EOF 处未闭合 `#if` → 错误
- 未知指令 `#foozle` → 错误，loc 行号准确
- 重复 slash 路径（`/tmp//foo.h`）能解析（access(2) 自然 canonicalize）
- `#include` 用宏展开成头文件名
- 跨包含 linemarker 正确（`# 4 "main.c"` 在 `#include` 之后正确恢复）
- `#pragma once` 与 `#ifndef` 包含守卫都正确
- 非守卫头文件包含两次 → 内容出现两次

### 6. `#error` / `#warning` 不展开宏的行为是**对的**

探针之前提示 `#error msg VER` 不展开 `VER`。复测实机 GCC，确认 GCC 也**不展开**——这是 ISO C11 §6.10.5 的标准行为。无需修改。

## 文件改动

| 文件 | 行差 | 说明 |
|---|---|---|
| `pptok.c` | +5 | `reader_set_file` 实现 |
| `pptok.h` | +3 | `reader_set_file` 声明 |
| `directive.c` | +60 / -10 | `emit_note`、`warn_trailing`、include not-found notes、`handle_line` 更新文件名、`handle_else/endif` 签名变化、wire 尾随检查 |
| 新增 `test_phase4.c` | +600 | 47 项 Phase 4 回归 |

## 测试矩阵

```bash
$ make test
Phase 1 results: 82 passed, 0 failed
Phase 2 results: 49 passed, 0 failed
Phase 3 results: 90 passed, 0 failed
Phase 4 results: 47 passed, 0 failed
Results: 41 passed, 0 failed   ← 既存 Sharp 扩展测试
                                   合计 309 项

$ make asan       # 全清
$ make strict     # 无警告
```

通过：
- `-O0 -g`、`-O2`，`-std=c99`、`-std=c11`
- ASan + UBSan
- `-Wall -Wextra -Wpedantic -Wshadow -Wstrict-prototypes -Werror`

### Phase 4 测试覆盖（47 项）

| 章节 | 内容 | 用例数 |
|---|---|:---:|
| §1 | 尾随 token 警告（endif/else/ifdef/ifndef/undef + 注释/空白不警） | 7 |
| §2 | `#include` 失败时的 search-path note（`""` 与 `<>` 区分） | 2 |
| §3 | `#include` 实际行为（基础、`#pragma once`、传统 guard、自递归、宏-as-头名、深度限制） | 6 |
| §4 | `#line`（基础、宏展开行号、文件名更新 `__FILE__`） | 3 |
| §5 | 条件结构错误（dangling endif/else/elif、duplicate else、elif after else、unterminated） | 6 |
| §6 | linemarker 正确性（include 后回到主文件、嵌套 include） | 2 |
| §7 | Sharp 模式 parity | 2 |

## 下一步（Phase 5、6 仍未做）

- **Phase 5**：Sharp 扩展打磨——`#elif @…` 含 `@` 内省的延迟处理、`@ident` 与 `#`/`##` 的交互（`@ident` 永不参与 stringify/paste）、内置宏完备性（`__SHARP_TARGET_*` 等）
- **Phase 6**：规模验证——跑通真实 `<stdio.h>`/`<stdlib.h>`/`<string.h>`、Linux/macOS/Windows CI matrix、性能基准对比 `gcc -E`
