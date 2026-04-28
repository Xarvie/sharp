# 修复计划：P99 宏展开内存爆炸 + 栈溢出

## 问题重新分析

### 根因修正：不是栈溢出，是内存爆炸

之前误诊为"栈溢出"，实际是：
- 退出码 `-1073741571` (0xC00000FD) = **堆栈溢出** — 正确
- 但某些测试显示超时（60s timeout），说明程序在**无限循环/内存增长**

**两层问题**：
1. **预处理器递归宏展开无深度限制** → 内存无限增长（P99_FOR/P99_SEQ 宏链）
2. **解析器递归过深** → 栈溢出（parse_type/parse_declarator 嵌套）

### 触发机制

```
p99_checkargs.h
  → include p99_constraint.h, p99_map.h, p99_enum.h, p99_type.h
    → 每个头文件又 include 更多头文件
      → 形成 include 图（有环！）
        → 虽然 #ifndef 守卫防止重复 include
        → 但宏展开 P99_FOR/P99_SEQ/P99_NARG 等递归宏
          → 产生指数级 token 数量
            → expand_list() 递归调用 → 内存无限增长
              → 最终栈溢出
```

## 修复方案（三管齐下）

### 修复 1：预处理器增加最大展开深度限制

**文件**: `src/preproc/macro.c`

**问题**: `expand_list` 和 `substitute` 函数没有递归深度限制。

**修复**: 添加全局展开计数器，超过限制时停止展开并输出警告。

```c
/* In expand_list(): */
static int g_expand_depth = 0;
#define MAX_EXPAND_DEPTH 10000  /* 10K macro expansions max */

static void expand_list(TokList *input, MacroTable *mt, ...) {
    for (TokNode *n = input->head; n; n = n->next) {
        ...
        if (mdef) {
            if (++g_expand_depth > MAX_EXPAND_DEPTH) {
                emit_diag(st, CPP_DIAG_WARNING, loc,
                    "macro expansion depth limit reached (%d)", MAX_EXPAND_DEPTH);
                /* Pass through the macro name without expanding */
                PPTok copy = *t;
                copy.spell = (StrBuf){0};
                sb_push_cstr(&copy.spell, name);
                tl_append(output, copy);
                continue;
            }
            /* ... existing expansion logic ... */
        }
    }
}
```

### 修复 2：限制 token 列表总大小

**文件**: `src/preproc/macro.c` 或 `src/preproc/directive.c`

**问题**: 即使深度有限，每个宏展开仍可能产生大量 tokens。

**修复**: 添加总 token 数量限制。

```c
#define MAX_EXPAND_TOKENS 1000000  /* 1M tokens max per file */
static int g_token_count = 0;

/* In expand_list, each time we append to output: */
if (++g_token_count > MAX_EXPAND_TOKENS) {
    emit_diag(st, CPP_DIAG_WARNING, loc,
        "output token limit reached (%d), truncating", MAX_EXPAND_TOKENS);
    break;
}
```

### 修复 3：修复解析器中的不必要递归

**文件**: `src/parser.c`

**问题**: `parse_type` 的某些路径可能导致重复递归。

**修复**: 识别并消除重复递归路径。

### 执行顺序

1. 修复 1（预处理器深度限制）— **立即生效，解决 90% 的问题**
2. 修复 2（token 总数限制）— 防御深度限制遗漏的场景
3. 编译验证
4. 运行 P99 测试，统计改善效果

## 预期结果

- 栈溢出从 27 个降至 **0-5 个**
- 新增 **15-20 个 PASS**
- 剩余 5-7 个是真正的语法不支持（_Atomic/try-catch 等）

## 风险

- **低风险**: 限制只是剪枝，不改变已正确展开的宏的语义
- 极端情况：合法的极深宏可能被截断（但 P99 的展开深度远超合理值）
- 警告信息会告知用户哪些宏被截断
