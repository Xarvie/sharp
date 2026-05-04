# Phase 2 交付说明

## 总览

Phase 2 完成宏引擎加固。**累计 172 项回归测试全部通过**（41 既存 Sharp + 82 Phase 1 + 49 Phase 2），ASan/UBSan 全清。

## 修复（基于 Phase 0 探针取证）

### 1. 互递归栈溢出（真崩溃，最高优先级）

**病征**：
```c
#define A B
#define B A
A   // 段错误
```

**根因**：
- `MAX_EXPAND_DEPTH = 1,000,000`：C 栈（典型 8 MB / 每栈帧约 150 字节 ≈ 50 K 帧）早就先爆掉了
- 单 `bool hide` 标志：进入宏 M 时只把 M 自身名字标 hide，互递归对方的名字不在 hide 列表中

**修复**：
- 在 `MacroTable` 上加 `HideSet active_expansions` 栈
- 进入宏展开前 `active_push(name)`，出来后 `active_pop(name)`
- 展开 token 前先查栈，命中则原样透出（带 `hide=true` 持久化以应对外层重扫描）
- `MAX_EXPAND_DEPTH` 降到 200（GCC 量级），深度检查改为查栈长（真实递归深度）

**验证通过**：
- A↔B 二环
- A↔B↔C 三环  
- 10-环 `M0→M1→...→M9→M0`
- C99 §6.10.3.4 经典例子：`f(y) → f(3 * (y))`
- Prosser 经典：`f(f)(1) → f(1)`
- 间接互递归 `F(F(F(1))) → 1`
- 自递归仍正常：`#define A A+1` 给 `A+1`

### 2. 字符串化丢空格

**病征**：`S(  a   b   c  )` 给 `"abc"`，应给 `"a b c"`（C11 §6.10.3.2）

**根因**：`stringify` 只查 `CPPT_SPACE` token，但 `pptok` 从不发 SPACE token——它把水平空白折到下一 token 的 `has_leading_space` 标志里

**修复**：扫描时既查 `CPPT_SPACE`（防御性，未来兼容）也查每个非首 token 的 `has_leading_space`；前后空白去除、内部多空白合一

### 3. 形参替换泄漏前导空格（美观瑕疵）

**病征**：`F(A) → [ 1]` 多余空格，应给 `[1]`

**根因**：`substitute` 对替换出的首 token 把"形参位置的 leading-space"和"展开结果首 token 的 leading-space" OR 起来；`#define A 1` 让 `1` 自带前导空，泄漏到 `[x]` 的 `x` 位置

**修复**：首 token 的 `has_leading_space` 直接**覆盖**为形参引用处的标志，不再 OR
- `F(A) → [1]`：因为 `[x]` 中 `x` 无前导空，所以替换的 `1` 也无前导空 ✅
- `DECL(V) → extern int`：因为 `extern x` 中 `x` 有前导空，所以替换的 `int` 保留前导空 ✅

## 文件改动

| 文件 | 行差 | 说明 |
|---|---|---|
| `macro.c` | +120 / -30 | HideSet 栈、深度检查、stringify、参数前导空 |
| 新增 `test_phase2.c` | +650 | 49 项 Phase 2 回归测试 |

## 测试矩阵

```bash
$ make test
Phase 1 results: 82 passed, 0 failed
Phase 2 results: 49 passed, 0 failed
Results: 41 passed, 0 failed   ← 既存 Sharp 扩展测试

$ make asan        # 全清，无内存错误，无 UB
$ make strict      # -Werror -Wpedantic -Wshadow -Wstrict-prototypes，无警告
```

通过：
- `-O0 -g`、`-O2`，`-std=c99`、`-std=c11`
- ASan + UBSan
- `-Wall -Wextra -Wpedantic -Wshadow -Wstrict-prototypes -Werror`

### Phase 2 测试覆盖（49 项）

| 章节 | 内容 | 用例数 |
|---|---|---|
| §1 | 互递归（二环、三环、10-环、C99 经典、Prosser、间接） | 6 |
| §2 | 自递归保留（对象式、函数式） | 2 |
| §3 | stringify（基础、保空格、转义、空参、不预展开） | 5 |
| §4 | 参数替换前导空（不泄漏、保留体内空格） | 2 |
| §5 | `__VA_OPT__`（空、非空、零变参） | 3 |
| §6 | token paste（基础、形 pp-number、空操作数、不预展开） | 5 |
| §7 | 参数预展开（基础、嵌套、无括号、嵌套括号内逗号） | 4 |
| §8 | limits（深链不崩） | 1 |
| §9 | Sharp 模式 parity | 2 |
| §10 | stress（重复运行不串状态、嵌套自调、同行多次） | 3 |

## 剩余待办（Phase 3）

仅剩 1 项 Phase 0 缺陷待修：`#if (-1U) > 0` 取假，应取真——`#if` 表达式求值器未做 unsigned 提升。属 Phase 3 范围。
