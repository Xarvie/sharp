# Phase R20 — 代码优雅性整理

> Status: **610/610 baseline ✓ + ASAN ✓ + self_smoke 20/20 ✓ + corpus 167/167 ✓**

## 变更摘要

### 1. Sharp 语言模式统一（R19 延续）
Sharp 是 C 的超集，`sharp_cli` 默认改为 `CPP_LANG_SHARP`。
`#if @has_destructor(T)` 等在 `#if` 里的 `@` 内省整体透传给语言前端，不再报错。

### 2. `expr.c` — eval_primary 从 351 行缩减到 168 行
将内嵌于 `eval_primary` 中的两张静态表提升为文件级常量：
- `kGccBuiltins[]` — 303 个 GCC 13 built-in 名称
- `kGccAttrs[]`    — 81 个 GCC 13 attribute 名称

新增 `table_contains()` 辅助函数统一查表。`eval_primary` 本体现在只含控制流，不含数据。

### 3. `cpp_targets.c` — 提取 `install_common()`
三个 target 函数（linux/darwin/msvc）共有 21 个完全相同的宏定义，
提取到 `install_common(ctx)` 中，每个 `install_*` 函数在开头调用一次。
文件从 722 行减到 688 行，三个函数各减少约 21 行重复。

### 4. `cpp_internal.h` — `is_has_family()` 内联辅助
`directive.c` 和 `expr.c` 中有三处完全相同的 8 行 `__has_*` 名称检测链，
统一为 `is_has_family(name)` 内联函数，消除复制粘贴。

### 5. 全文件 LF 规范化
`pptok.c`、`cpp.c` 混有 CRLF 行尾，统一转为 LF。

### 6. `directive.c` — 补充函数注释 + 删除噪声 Phase 注释
- 12 个 `static` 函数补充了缺失的单行文档注释
- 删除 3 处仅描述"做了什么"而非"为什么"的机械性 Phase 注释

### 7. CLI 补全（-P / -o / --tokens / -std=c17）
`sharp_cli` 新增：
- `-P` 抑制 linemarker（对应 `gcc -E -P`）
- `-o <file>` 写文件（默认 stdout）
- `--tokens` token dump 到 stderr（调试用）
- `-std=c17` / `-std=c18` 显式设 C17 标准

## 不变的设计决策
- `reader_next_tok`（447 行）保持完整——它是一个有 10 个清晰 section 的词法分派器，
  每个 section 用 `/* --- */` 分隔，提取 helper 会让每个 section 都依赖外层状态，
  反而增加复杂度。
- Phase 注释主体保留——它们记录了非显然的标准引用（§6.10.x）和边界条件，
  是真正的架构文档而非开发日志。
