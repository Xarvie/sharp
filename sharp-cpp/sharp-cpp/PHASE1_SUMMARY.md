# Sharp/C 预处理器 — Phase 1 交付说明

## 总览

商业级 Sharp 预处理器分 6 阶段实施。本次交付完成 **Phase 1：词法层加固与诊断基础**。Sharp 是 C 的超集，整套系统是**单一**预处理器（带 `CPP_LANG_SHARP` 模式开关），不是两份代码。

## 实施阶段规划（决定不变）

| 阶段 | 范围 | 触及文件 |
|---|---|---|
| **Phase 1（本次完成）** | 词法层加固：宽串/宇符前缀（L/u/U/u8）、未闭合诊断、UCN 占位、`keep_whitespace` 真正生效、行拼接边角 | `pptok.c`, `directive.c`, `expr.c` 小修, 新增 `test_phase1.c` |
| Phase 2 | 宏引擎：互递归蓝染色（修崩溃）、`#` 字符串化保留空格、`##` 边角、`__VA_OPT__` 完整 C23 语义 | `macro.c` |
| Phase 3 | `#if` 常量表达式：无符号算术、整型提升、宽字符常量 | `expr.c` |
| Phase 4 | 指令打磨：路径归一化、跨包含 linemarker、`#error`/`#warning` 中宏展开、诊断 note 提示 | `directive.c` |
| Phase 5 | Sharp 扩展打磨：`#elif @…` 延迟、`@ident` 与 `#`/`##` 交互、内置宏完备性 | `directive.c`, `macro.c` |
| Phase 6 | 规模验证：真实 `<stdio.h>`/`<stdlib.h>`、跨平台 CI、性能基准 | 仅构建/测试 |

## Phase 1 — 修复与新增

### 修复（基于 50 次定向探针的取证）

1. **宽/Unicode 字符串与字符前缀**（`L"…"`, `u"…"`, `U"…"`, `u8"…"`, 同样适用字符常量）
   - **病征**：`L"foo"` 被切成 `L`（标识符）+ `"foo"`（无前缀串）。原因是 `pptok.c` 标识符分支在前缀分支之前匹配 `L`/`u`/`U`，致使前缀分支成为死代码。
   - **修复**：在标识符分支**之前**插入前缀检测，先看一字符（或 `u8` 看二字符）；若紧跟 `"` 或 `'`，吃成单一 `CPPT_STRING_LIT`/`CPPT_CHAR_CONST`；否则恢复 reader 状态，让标识符分支按原样处理（`Lvalue`、`u8type`、`u8_var` 仍是普通标识符）。

2. **未闭合字面量与块注释静默通过**
   - **病征**：`"hello<EOF>` 与 `/* foo<EOF>` 不报诊断；甚至 `"hello\n` 这种行内未闭合还会**吃掉换行**，破坏后续 `#define` 的行首识别。
   - **修复**：`lex_quoted` 与 `skip_block_comment` 改为返回 `bool`；`lex_quoted` 用 peek-then-getc 模式，**保留**致命换行，让后一轮 token 化看到 `\n` 并维持 `at_bol`。失败时调用方在开引号位置发 `CPP_DIAG_ERROR("unterminated …")`。

3. **`cpp_keep_whitespace(true)` 形同虚设**
   - **病征**：选项可设但 `out_tokens` 数组里永远没有 `CPPT_SPACE`/`CPPT_NEWLINE`。原因是 `pptok.c` 把水平空白吃掉转而置于下一 token 的 `has_leading_space` 标志，从未真正产生 SPACE token；`process_buf` 中 SPACE 分支为死代码。
   - **修复**：在 `emit_tok_text` 处合成——遇到 `t->has_leading_space==true && ctx->keep_whitespace` 时，向 `out_tokens` 推入一个指向 `out_text` 中刚写入空格字节的 `CPPT_SPACE`。`CPPT_NEWLINE` 是真实 token，由 `process_buf` 直接推。新增的"重建"测试验证：把 `out_tokens` 全部 token 文字按序拼接，等于 `out_text`，做到无损往返。

4. **遗留编译警告**（既存，顺手清掉）
   - `expr.c:425` 中 `then_start` 未使用 → 删除。
   - `directive.c:413` 中 `used_expansion` 设而未读 → 删除。

### 文件修改

| 文件 | 行差 | 说明 |
|---|---|---|
| `pptok.c` | +160 / -50 | 重写 `lex_quoted` 与 `skip_block_comment` 返回布尔；新增前缀-引号探测块；删除死代码前缀块；为四处未闭合点插入诊断 |
| `directive.c` | +50 / -10 | `emit_tok_text` 加合成 SPACE；NEWLINE/SPACE/COMMENT 三处分支统一向 `out_tokens` 推；移除未用变量 |
| `expr.c` | -2 | 移除未用变量 |
| 新增 `test_phase1.c` | +650 | 82 个 Phase 1 回归测试 |
| 新增 `probe.c` | +500 | 50 探针，本阶段从 10 个 ISSUE 减到 4 个（后 4 个属 Phase 2/3 范围） |
| 新增 `Makefile` | +50 | `make test` / `make asan` / `make strict` |

## 测试矩阵

```
$ make test
Phase 1 results: 82 passed, 0 failed
Results: 41 passed, 0 failed   ← 既存 Sharp 扩展测试

$ make asan
Phase 1 results: 82 passed, 0 failed   ← ASan + UBSan 全清
Results: 41 passed, 0 failed

$ make strict   # -Werror -Wpedantic -Wshadow -Wstrict-prototypes
Phase 1 results: 82 passed, 0 failed
Results: 41 passed, 0 failed
```

测试矩阵覆盖：

- **`-O0 -g`** 调试构建
- **`-O2`** 优化构建
- **`-std=c99` / `-std=c11`** 跨标准
- **`-fsanitize=address,undefined`** 内存与 UB 检测
- **`-Wall -Wextra -Wpedantic -Wshadow -Wstrict-prototypes -Werror`** 严格警告

### Phase 1 测试覆盖（82 项）

| 章节 | 内容 | 用例数 |
|---|---|---|
| §1 | 编码前缀 L/u/U/u8 字符串与字符；前缀作为普通标识符；前缀+宏体；前缀+转义 | 13 |
| §2 | 未闭合 `"…\n`、`"…<EOF>`、`'a\n`、`/* …<EOF>`、`L"…\n`；不吞末尾换行；正常字面量不诊断 | 7 |
| §3 | 行拼接：标识符内、`#define` 前、pp-number 内、Windows CRLF 拼接 | 4 |
| §4 | `keep_whitespace=true` 产生 SPACE/NEWLINE token；默认无；token 序列重建 = `out_text` | 3 |
| §5 | CRLF 行尾、混合 LF + CRLF | 2 |
| §6 | 十六进制浮点 `0x1.fp10` / `0x1.8p-3`、首点 pp-number `.5e-3`、带号指数 `1e+10` | 4 |
| §7 | 三字符 `??=` → `#`、`??<` `??>` → `{` `}` | 2 |
| §8 | 1 万字符标识符；1000 个 `#define` + 使用 | 2 |
| §9 | 空输入、纯空白输入、纯注释输入 | 3 |
| §10 | Sharp 模式下前缀仍可用；`@ident` 不受前缀改动影响 | 2 |
| §11 | 未闭合诊断的 `loc.line` 指向开引号处 | 1 |

## 跨平台保证

- **POSIX**：`access(2)` 通过 `<unistd.h>`；包含搜索路径用 `/`。
- **Windows**：`directive.c` 已为 `_WIN32` 走 `<io.h>` + `_access`、双向斜杠搜索；linemarker 中 `\` 转 `/` 以避免被 C 字符串解释为转义。
- **C99 + C11 双兼容**：`-std=c99 -D_POSIX_C_SOURCE=200809L` 与 `-std=c11` 均通过。
- **大端/小端**：词法层不依赖字节序，宏与表达式以 `intmax_t` 计算。
- **CRLF / LF 混合**：测试矩阵已覆盖。

## 仍存在的 4 个已知 ISSUE（属后续阶段）

| ISSUE | 阶段 | 说明 |
|---|---|---|
| `F(A)` 输出 `[ 1]` 而非 `[1]`（多一空格） | Phase 2 | 函数式宏参数预展开后保留前导空格。属外观瑕疵，不影响语义 |
| `S(  a   b   c  )` 字符串化得 `"abc"` 而非 `"a b c"` | Phase 2 | `#` 算子未保留 token 间空格 |
| `(-1U) > 0` 取假 | Phase 3 | `#if` 表达式未做无符号提升 |
| 互递归 `#define A B` / `#define B A` 栈溢出 | Phase 2 | 蓝染色（hide-set）对互递归失效 — **此项是真实崩溃，Phase 2 首要修复** |

## 文件清单

```
sharp-cpp-phase1/
├── cpp.h               公开 API
├── cpp.c               公开 API + Phase 6 字符串拼接
├── cpp_internal.h      内部类型 (DA, StrBuf, InternTable, …)
├── pptok.h, pptok.c    Phase 1-3 词法（本阶段重点修改）
├── macro.h, macro.c    宏表与展开（待 Phase 2）
├── expr.h, expr.c      `#if` 常量表达式（待 Phase 3）
├── directive.h, directive.c  指令分发（本阶段小改）
├── test_sharp_cpp.c    既存 Sharp 扩展回归测试（41 项）
├── test_phase1.c       Phase 1 新增回归测试（82 项）
├── probe.c             50 个探针（gap 取证）
└── Makefile            构建/测试入口
```

## 下一步

进入 Phase 2，首要修复互递归崩溃。计划用经典 hide-set per-token 策略（每 token 携带"已展开宏集合"），替换当前的全局蓝染色标志位。Phase 1 的探针 `probe.c` 与 `test_phase1.c` 作为不变量回归基线。
