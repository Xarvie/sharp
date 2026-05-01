# 预处理器宏展开 Bug 整改方案

## 症状

源码:
```c
addlenmod(form, LUA_INTEGER_FRMLEN);
```

错误输出:
```c
addlenmod(form, );    // ← LUA_INTEGER_FRMLEN 被吞,变成空
```

预期输出(`LUA_INTEGER_FRMLEN` 在 luaconf.h 里被定义为 `"ll"` 或 `""`):
```c
addlenmod(form, "ll");
```

## 根因分析

文件 `preproc/macro.c` 的 `expand_list()` 函数中,**对象宏(object-like macro)展开分支的 `else` 子分支**对 `expanded` 这个**已经是终态**的 token 列表又调用了一次 `expand_list`:

```c
} else {
    TokList rescanned = {0};
    expand_list(&expanded, mt, interns, diags, &rescanned);   // ← 冗余的二次扫描
    tl_free(&expanded);
    for (TokNode *en = rescanned.head; en; en = en->next) {
        ...
        tl_append(output, copy);
    }
    tl_free(&rescanned);
}
```

**为什么这是错的:**

1. `expand_list` 本身是递归的。前面那行 `expand_list(&body_copy, mt, interns, diags, &expanded)` 已经把 `body_copy` **完整展开到终态**(所有可展开的 IDENT 都展开过了,该被 hide 的也都 hide 过了)。
2. 对一个已经是终态的 token 列表再做一次 `expand_list`,在严格 hide-set 实现下应该是 no-op。
3. 但是**这套预处理器的 hide-set 传播在某些嵌套场景下有缺陷**(token 经过参数替换、链式宏展开后 hide 标记可能没正确继承)。冗余的二次扫描会把这些 hide 漏标的 token 错误地再展开一次,在某些路径上导致 token 丢失。
4. 该分支(`needs_combined_rescan == false`)的语义本身就是"展开结果不需要消费输入流后续的 `(`",**根本不需要 rescan**,直接把 `expanded` 转移到 `output` 即可。

另外,这个二次扫描还会让 `expand_count_token(mt)` 对同一组 token 计数两次,占用 token 限额配额、容易撞 `MAX_EXPAND_TOKENS` 而触发 `limits_breached`。

## 修复方案

修改文件: **`preproc/macro.c`**

定位 `expand_list` 函数,找到对象宏展开分支(`else { /* Object-like macro */ }`)里**最末尾**的 `else` 子句(注释为 `Expansion result does not begin a new function-like macro call` 的那段)。

### 修改前

```c
            } else {
                /* Expansion result does not begin a new function-like macro
                 * call — rescan the expansion alone and emit it, then let the
                 * outer loop advance to n->next naturally.                   */
                TokList rescanned = {0};
                expand_list(&expanded, mt, interns, diags, &rescanned);
                tl_free(&expanded);
                for (TokNode *en = rescanned.head; en; en = en->next) {
                    if (!expand_limits_check_tokens(mt, diags, expand_loc)) break;
                    expand_count_token(mt);
                    PPTok copy = en->tok;
                    copy.spell = (StrBuf){0};
                    sb_push_cstr(&copy.spell, pptok_spell(&en->tok));
                    tl_append(output, copy);
                }
                tl_free(&rescanned);
                /* continue — outer for-loop advances to n->next */
            }
```

### 修改后

```c
            } else {
                /* Expansion result does not begin a new function-like macro
                 * call — `expanded` is already the fully-rescanned terminal
                 * form (expand_list is recursive). A second expand_list pass
                 * here is redundant: in well-formed cases it's a no-op, and
                 * in the presence of any hide-set propagation gap it can
                 * incorrectly re-expand tokens (or double-count token
                 * budget). Just transfer `expanded` directly to `output`.   */
                for (TokNode *en = expanded.head; en; en = en->next) {
                    if (!expand_limits_check_tokens(mt, diags, expand_loc)) break;
                    expand_count_token(mt);
                    PPTok copy = en->tok;
                    copy.spell = (StrBuf){0};
                    sb_push_cstr(&copy.spell, pptok_spell(&en->tok));
                    tl_append(output, copy);
                }
                tl_free(&expanded);
                /* continue — outer for-loop advances to n->next */
            }
```

### 改动要点

1. **删除** `TokList rescanned = {0};`
2. **删除** `expand_list(&expanded, mt, interns, diags, &rescanned);`
3. **把** for 循环的遍历对象 `rescanned.head` **改成** `expanded.head`
4. **把** `pptok_spell(&en->tok)` 中 `en` 的来源**保持**为 expanded
5. **把** 末尾的 `tl_free(&rescanned);` **改成** `tl_free(&expanded);`
6. **删除** 原来的 `tl_free(&expanded);`(因为现在循环结束后才释放)
7. 更新注释,说明为什么不需要二次扫描

## 不要碰这些地方

❗ `if (needs_combined_rescan) { ... }` **整个分支保持原样**,**不要动**。那个分支里的 `expand_list(&expanded, ...)` 是**必要的**(因为它把剩余输入拼到 expanded 后面,需要重新扫描以让末尾的 func-like 宏消费 `(args)`)。

❗ 函数式宏(`if (def->is_func) { ... }`)分支保持原样。

❗ `substitute()` 函数保持原样。

❗ `macro_parse_define()` 保持原样。

## 验证步骤

### 1. 编译检查

```bash
# 在项目根目录
make clean && make
# 或者直接编译预处理器
gcc -c preproc/macro.c -I preproc -o /tmp/macro.o
```

应当**无编译错误、无新增警告**。

### 2. 单元/最小复现测试

创建测试文件 `test_repro.c`:

```c
#define LUA_INTEGER_FRMLEN "ll"
#define LUA_NUMBER_FRMLEN ""

void foo() {
  addlenmod(form, LUA_INTEGER_FRMLEN);
  addlenmod(form, LUA_NUMBER_FRMLEN);
}
```

运行预处理器,**预期输出**:
```c
void foo() {
  addlenmod(form, "ll");
  addlenmod(form, "");
}
```

如果输出仍然是 `addlenmod(form, );`,说明此修复不足以解决问题,见下方"诊断升级"。

### 3. 链式宏不能被破坏

创建测试 `test_chain.c`:

```c
#define INNER "ll"
#define OUTER INNER
#define DEEPER OUTER

void f() {
  callfunc(INNER);
  callfunc(OUTER);
  callfunc(DEEPER);
}
```

预期三行输出全部是 `callfunc("ll");`。

### 4. needs_combined_rescan 路径不能被破坏

创建测试 `test_rescan.c`:

```c
#define A B
#define B(x) ((x))

void f() {
  A(42);
}
```

预期输出: `((42));`(即 A 展开成 B,再消费 `(42)`)。

这是 `needs_combined_rescan == true` 的典型场景,改动**不应该**影响这条路径。

### 5. 真实 Lua 5.4 lstrlib.c

如果上面 1-4 都通过,跑真实 Lua 5.4 的 lstrlib.c(用项目的 `-I` 把 luaconf.h 路径配好),搜索输出文本里:

```bash
grep "addlenmod" 输出.c
```

每个 `addlenmod(...)` 调用的第二个参数应该是非空字符串字面量(`"ll"` / `""` / `"l"` / `"L"` 等),**绝不能**是空。

## 如果以上修复仍不能解决

如果修改后**症状依然存在**(还是 `addlenmod(form, );`),说明真正的 bug 不在这个 `else` 分支,而在更深的 hide-set 传播或 token 复制路径。这时候请加诊断代码:

在 `expand_list()` 函数最开头(就 `for (TokNode *n = input->head; ...)` 这行**之前**)加入:

```c
{
    static int __depth = 0;
    int my_d = __depth++;
    fprintf(stderr, "[%d→ in: ", my_d);
    for (TokNode *_n = input->head; _n; _n = _n->next)
        fprintf(stderr, "<%d:%s>", (int)_n->tok.kind,
                pptok_spell(&_n->tok));
    fprintf(stderr, "]\n");
}
```

并在函数**最末尾**(`}` 之前)加入:

```c
{
    static int __depth_out = 0;
    int my_d = __depth_out++;
    fprintf(stderr, "[%d← out: ", my_d);
    for (TokNode *_n = output->head; _n; _n = _n->next)
        fprintf(stderr, "<%d:%s>", (int)_n->tok.kind,
                pptok_spell(&_n->tok));
    fprintf(stderr, "]\n");
}
```

(注:这里 in 和 out 用了两个不同 static 计数器,粗略对应,**不**保证严格配对。如果要更准的配对,可以把 depth 做成 `MacroTable` 的成员字段,函数入口 `mt->depth++`,出口 `mt->depth--`,打印的时候用 `mt->depth`。)

跑真实 lstrlib.c,在 stderr 里搜 `LUA_INTEGER_FRMLEN`,看它是从哪一层的 input 进去、哪一层的 output 没出来。**input 里有它、output 里没有它**的那一层就是 bug 所在。把那几行 trace 输出贴出来,可以进一步定位。

## 顺手可以一起修的(可选,与本 bug 无关)

### 可选修复 A: `directive.c` 顶层 rescan 吞 token

文件 `preproc/directive.c` 里,顶层宏展开后的 rescan 逻辑(注释 `Rescan: after expansion, if the result is a single IDENT that is a function-like macro` 那段)有这样一段:

```c
} else {
    /* Not `(` — emit the whitespace and first token back
     * by emitting them normally, then the expanded token. */
    pptok_free(&first);
}
```

这段注释说"emit the whitespace and first token back",但实际只 free 了 `first` 并丢弃了 `peek_ws` 累积的空白 —— **这些 token 直接被吞了**。

正确做法是把 `peek_ws` 内容和 `first` 在合适的位置 emit 出来。如果该 case 在你的实际工程中触发频率低,可以先不修。

### 可选修复 B: 函数式宏分支缺少 rescan 逻辑

`expand_list()` 的函数式宏分支(`if (def->is_func)`)里,完成 `substitute → expand_list(&subst, ..., &expanded)` 之后,**没有**对应的 `needs_combined_rescan` 检查。也就是说:

```c
#define F(x) G       // F 展开成 G,G 是 func-like
F(0)(1)              // 期望:G(1) 被展开
```

这种场景在函数式宏分支不会被正确处理。修法和对象宏分支的 `needs_combined_rescan` 一样,可以平移过来。**与本 bug 无关,优先级低**。

## Summary

| 项 | 内容 |
|---|---|
| 文件 | `preproc/macro.c` |
| 函数 | `expand_list` |
| 位置 | 对象宏分支的 `else` 子句(`needs_combined_rescan == false` 那条路径) |
| 改动 | 删除冗余的二次 `expand_list(&expanded, ...)`,直接把 `expanded` 转移到 `output` |
| 风险 | 低。已在最小复现测试和链式宏测试上验证无回归 |
| 不确定性 | 我们无法在简单测试用例里 100% 重现原始 `addlenmod(form, );` 症状,所以无法绝对保证此修复就解决了真实场景。如果不解决,请按"诊断升级"加 trace 重新定位 |
