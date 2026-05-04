# Phase R18 — 性能优化 + __GNUC_MINOR__/C17/`__STDC_VERSION__` 精确对齐 + 全量 glibc 扫描

> Status: **610/610 baseline ✓ + ASAN ✓ + strict ✓ + self_smoke 20/20 零诊断 ✓**
> **glibc 全量扫描 152/152 ✓ + boost/predef 修复 ✓**
> **性能：sharp-cpp 1.53× gcc -E（从 1.60× 降低 5%，gcc 含 subprocess 开销）**

## 修复的 4 个 Bug

### Bug R18-1：`__GNUC_MINOR__` 版本号不匹配

gcc 13.3 在 Ubuntu 24.04，`__GNUC_MINOR__` = 3，而 sharp-cpp 定义为 2。
**修法**：x86_64-linux-gnu target 中 `__GNUC_MINOR__` 从 `"2"` 改为 `"3"`。

### Bug R18-2：默认 lang_std 是 C11 而不是 C17

gcc 默认以 gnu17 (C17) 运行，`__STDC_VERSION__ = 201710L`，而 sharp-cpp 默认 C11 (`201112L`)。导致 `boost/predef/detail/test_def.h` 的 `BOOST_LANG_STDC` 宏选择错误版本号，token 流出现 `201112` vs `201710L` 差异。
**修法**：`cpp.c` 中 `ctx->lang_std` 默认从 `201112L` 改为 `201710L`（对应 C17）；同步更新测试断言。

### Bug R18-3：`__STDC_VERSION__` 缺少 `L` 后缀

即使 lang_std 正确，`install_builtin_num` 以 `%lld` 格式输出 `201710` 而 gcc 输出 `201710L`。`boost/predef` 通过除法算出版本号，整数字面量格式差异导致 token-diff。
**修法**：改用 `install_builtin_str` 输出 `"%ldL"` 格式，如 `"201710L"`。

### Bug R18-4：`__indirect_return__` 不在属性表

`ucontext.h` 通过 `__glibc_has_attribute(__indirect_return__)` 检测 GCC 13 新增的 CET/shadow-stack 属性，返回 0 导致 fallback 到 `__returns_twice__`。
**修法**：`expr.c` 的 `gcc_attrs[]` 表末加 `"indirect_return"`。

## 性能优化（三项）

| 优化 | 原理 | 效果 |
|------|------|------|
| `skip_block_comment` memchr | `keep=false` 时用 `memchr` 直接扫 `*` 字符，跳过 ph2 的逐字符循环 | comment 处理时间从 16.6% → 6.4% |
| `macro_lookup_is_func` 消除 | 主循环的 rescan 路径原先做两次 hash 查找（lookup + is_func）；改为一次 `macro_lookup` 后用 `macro_def_is_func` | 消除第二次 hash 查找 |
| 空白跳过快路径 | `reader_next_tok` 中跳过 `' '`/`'\t'` 直接读原始缓冲区；`\r/\f/\v` 及 `\<newline>` 后的空白仍走 ph2 | 空白密集代码受益 |
| **总计** | | **1.60× → 1.53×（改善 ~5%）** |

> 注：gcc -E 计时包含 subprocess 启动开销（约 10ms）。纯计算时间 sharp-cpp 约 44ms vs gcc 纯处理约 19ms（估算），意味着实际算法层面约 2.3× 差距仍有优化空间。

## 全量 /usr/include 扫描

```
/usr/include/*.h           152/152 ✓  (152 pass, 0 fail, 5 skip-C++)
sys/ bits/ netinet/ …      146/146 ✓
gcc-include/*.h             45/ 45 ✓ (82 skip-C++)
full /usr/include/** 递归  2962/2966 ✓ (3 skip-C++, 1 boost-predef 已修)
```

## 验证矩阵

```
make test     610/610 ✓
make asan     610/610 ✓
self_smoke    20/20   零诊断 ✓
glibc sweep   152/152 100% ✓
gcc-diff corpus 343+  100% ✓
性能          1.53× gcc-E (含 subprocess overhead)
```

## R19+ 候選

1. **进一步性能**：TokList 节点池化（减少 malloc）、macro_expand 内联展开
2. **`-P` 抑制 linemarker 模式**：gcc -E -P 的对应功能，让输出更简洁
3. **Windows/MSVC corpus**：在 Wine/cross 环境下测试 `<windows.h>` 子集
4. **musl+glibc 混合头**：musl 的 `__attribute__` 扩展与 glibc 兼容性
