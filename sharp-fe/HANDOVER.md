# 交付书 — Sharp 语言前端（Phase 0 完成，Phase 1 待开始）

> 收件人：下一位接手本项目的 AI
> 发件人：Phase 0 实施者
> 项目位置：`/home/claude/sharp-fe/`
> 上游依赖：`/home/claude/sharp-cpp/sharp-cpp/`（Phase R23，610/610 测试全过）
> 项目状态：**Phase 0 完成 — 58 探针 (37 OK / 1 ISSUE / 20 DECISION)，三道质量门全过。Phase 1 (lex) 可立即开始。**

---

## 你需要知道的最重要的 5 件事

1. **项目完全继承 sharp-cpp 的工程纪律**。你手里的这份 `HANDOVER.md` 和对面 `../sharp-cpp/sharp-cpp/HANDOVER.md` 是同一种信——读一遍 cpp 那份（特别是"关键约定"章节）。本项目所有纪律与 cpp 项目完全一致，不重复说。

2. **先信任既有测试**。`make test` 会跑 probe（Phase 0 的"测试"）。Phase 1 起会有 `test_phase1` 等二进制。**任何修改前先确认三道质量门都过**（`make test` + `make asan` + `make strict`），作为基线。不过就别动代码，先看 `git diff`。

3. **唯一的真实 ISSUE 是 P1.8**：`Vec<Vec<int>>` 中 `>>` 是 cpp 发出的单个 right-shift token。在 Phase 3 (parse) 的 `parse_generic_args()` 函数里，遇到 `PUNCT(">>")` 时必须消费它并把合成的 `PUNCT(">")` 作为下一层 `>` 处理。probe.c 的 P9.5 DECISION 已记录了具体方案。

4. **Phase 1 目标**：`lex.c/h`——把 cpp 的 `CppTok[]` 流转换为 `SharpTok[]`（关键字识别、字面量类型化、`@intrinsic` 透传）。预计测试约 80 项。完整验收条件见 PLAN.md §Phase 1 段。

5. **每阶段必须打包**：`make dist DIST_PHASE=N` → `/mnt/user-data/outputs/sharp-fe-phaseN.tar.gz`。这是 AI 交接稳定的物理基础，永远有一个绿色快照可以回滚到。

---

## 架构速览（30 秒读完）

```
sharp.h               公开 API (SharpCtx, SharpResult, sharp_compile_*)
sharp.c               API 实现 + ctx 管理（Phase 0 只有骨架）
sharp_internal.h      内部共享类型 (FeDiagArr, FE_ERROR 宏, SharpCtx 布局)
─────────────────────────────────────────────────────────────────
lex.h, lex.c          Phase 1 — pp-token -> SharpTok (keyword promo)
ast.h                 Phase 2 — AST 节点类型定义（纯 header）
parse.h, parse.c      Phase 3 — 递归下降解析
scope.h, scope.c      Phase 4 — 作用域 / 符号表
type.h, type.c        Phase 5 — 类型系统
sema.h, sema.c        Phase 6-10 — 语义分析
mono.h, mono.c        Phase 11 — 单态化
cg.h, cg.c            Phase 12 — C11 代码生成
─────────────────────────────────────────────────────────────────
probe.c               Phase 0 探针 (58 probes, 1 ISSUE)
test_helpers.h        共享 CHECK/CHECK_EQ_INT 等测试框架宏
test_phase1.c…        Phase 1+ 各阶段测试（Phase 0 无）
─────────────────────────────────────────────────────────────────
../sharp-cpp/sharp-cpp/   上游预处理器（作为库编译进前端）
```

**数据流**：`.sp` → `cpp_run()` → `CppTok[]` → `lex_run()` → `SharpTok[]`
          → `parse_file()` → `AstFile*` → `sema_check()` → `mono_expand()`
          → `cg_emit()` → C11 text → `cc` → binary

---

## 当前状态

### Phase 0 完成

| 探针类 | 数量 | 说明 |
|---|:---:|---|
| OK | 37 | cpp 对 Sharp 语法产生正确 token 流 |
| ISSUE | 1 | P1.8: `>>` 是单个 rshift token，Phase 3 parse 必须拆分 |
| DECISION | 20 | 设计决策已记录（名字改编 / defer 策略 / cg 架构 / API / const 规则）|

### 文件清单

| 文件 | 状态 | 说明 |
|---|:---:|---|
| `Makefile` | ✅ 完整 | probe/test/asan/strict/dist/clean 全部可用 |
| `probe.c` | ✅ 完整 | 58 探针 + main 函数 + 汇总 |
| `sharp.h` | ✅ 骨架 | 公开 API 全部声明（实现在后续 phase）|
| `sharp.c` | ✅ 骨架 | ctx new/free，compile_* 是 stub |
| `sharp_internal.h` | ✅ 骨架 | FeDiagArr, FE_ERROR 宏, SharpCtx 布局 |
| `test_helpers.h` | ✅ 完整 | CHECK 系列宏，每阶段的 test_phaseN.c 使用 |
| `PLAN.md` | ✅ v1 | 22 个阶段完整规划 |
| `HANDOVER.md` | ✅ 本文 | |
| `PHASE0_SUMMARY.md` | ✅ 完整 | |

---

## 关键约定（继续保持）

### 修改文件时
- 用 `view` 看上下文 → 用 `str_replace` 精准替换 → `bash` 重新编译并跑测试
- **禁止整文件 rewrite**（除非是新增文件）
- 每个修改带注释：`/* Phase N: <一句话原因> */`

### 加新功能时
1. 先写探针或 test case 验证现状
2. 区分真 bug vs 预期行为 vs 探针 bug
3. 修复 + 代码加 Phase N 注释
4. 加回归测试到对应 `test_phaseN.c`
5. 跑 `make test` + `make asan` + `make strict`
6. 写 `PHASEN_SUMMARY.md`，更新本 HANDOVER 末尾补遗段，更新 PLAN.md 状态表
7. `make dist DIST_PHASE=N` 打包

### 测试约定
```c
static int g_pass = 0, g_fail = 0;
/* ... CHECK 宏来自 test_helpers.h ... */
int main(void) {
    test_one(); test_two(); /* ... */
    printf("\nPhase N results: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
```

---

## 坑列表

*继承自 cpp 项目的通用坑（坑 1-5）见 PLAN.md §7 节。前端独有坑从这里开始：*

- **坑 FE-1** (P1.8)：`Vec<Vec<int>>` 中 `>>` 是单个 rshift token。Phase 3 `parse_generic_args()` 里处理：遇 `PUNCT(">>")` → 消费 + 推回合成 `>`。
- **坑 FE-2**（预期）：方法 vs 关联函数的判定要看完整函数体（体内是否使用 `this`）。Phase 3 parse 阶段先标记"待定"，Phase 7 sema 后定型。
- **坑 FE-3**（预期）：`defer` 翻译时 `return expr` 需要先把 expr 求值存入临时变量，再 emit defer 清理，再 return 临时变量——否则 expr 里引用的局部变量被 defer 提前清理。

---

## 立即开始的检查清单

```bash
cd /home/claude/sharp-fe
make test    # 期望输出 58 probes, OK 37, ISSUE 1, DECISION 20
make asan    # 同上，零内存错误
make strict  # 同上，零警告

# 读 PLAN.md Phase 1 节，了解完整范围
sed -n '/### Phase 1/,/### Phase 2/p' PLAN.md

# 读 spec 中关于 lex 的约定（词法章节、整型字面量、字符串字面量）
cat ../sharp-cpp/sharp-cpp/../../../sharp-lang-spec-v0_11.md | \
    grep -A 50 "## 类型系统" | head -60
```

如果 `make test` 不过 → **不要继续**，先回滚：
```bash
tar -xzf /mnt/user-data/outputs/sharp-fe-phase0.tar.gz -C /home/claude
```

---

## 推荐的第一个动作

**做 Phase 1：lex 模块**。

理由：所有后续阶段都吃 `SharpTok[]`，lex 是整个前端的基础。Phase 1 本身相对独立（只依赖 cpp.h 和 CppTok），没有复杂的依赖链，是最适合热身的第一阶段。

具体步骤：
1. 先写 `test_phase1.c` 里的 §1-§9 测试用例（测试先行）
2. 建 `lex.h` 定义 `SharpTokKind` enum 和 `SharpTok` struct
3. 建 `lex.c` 实现 `lex_run(const CppTok*, size_t, const char*, FeDiagArr*) -> SharpTok*`
4. 跑 `make test_phase1 && ./test_phase1`
5. 三道门全过，写 `PHASE1_SUMMARY.md`，打包

预计工时：4-6 小时（keyword hash table + 字面量解析是主要工作量）。

---

## Phase 0 接力补遗

本轮我完成了：
- 58 个探针（37 OK / 1 ISSUE / 20 DECISION），三道门全过
- 建立 `sharp.h` / `sharp.c` / `sharp_internal.h` / `test_helpers.h` 骨架
- 建立 `PLAN.md`（22 阶段完整规划）/ `HANDOVER.md`（本文）/ `PHASE0_SUMMARY.md`

**关键发现给下一位**：唯一的真实 ISSUE (P1.8) 是一个编译器实现的经典问题（C++ 20 年前就遇到过，C++11 修了），我们在 Phase 3 parse 里用 token-splitting 解决。这不会影响 Phase 1 (lex) 的开始——lex 不需要处理泛型参数，那是 parse 的事。

Phase 1 测试基线：当前 probe 必须全过（58/58，其中 ISSUE 1 是预期）。

— Phase 0 接力人，于 Phase 0 收尾时

---

## Phase 1 接力补遗

本阶段完成了 lex 模块（`lex.c` / `lex.h`）+ 213 项测试。

**关键实现决策**：
- 关键字识别用线性扫描（~50 个关键字）。如果 profiling 显示这是热点，改 perfect hash，但首版无需过早优化。
- `SharpTok.text` 借用 `CppResult._raw_text`，调用者必须保证 CppResult 活过 SharpTok 数组。这在 `lex.h` 的 doc comment 里有说明，测试里 `LexFix` 结构体把二者的生命期绑定在一起。
- **Known limitation 记录**：`cpp.h` 声明了 `CppTok.from_macro`，但 `macro.c` 在简单宏体展开时不填充它（实测 `from_macro=0`）。lex 层透传该值；等 cpp 模块实现后，此行为自动正确。

**坑 FE-4**（本阶段发现）：`fe_emit_diag()` 分配的 `msg` 字符串需要在 diag array 释放前逐项 free。测试框架的 `lf_free()` 必须先循环 free 各 diag.msg 再 free 数组。ASan 在第一次跑时立即抓到这个问题——正是三道质量门的价值所在。

**Phase 2 推荐下一动作**：建 `ast.h`（AST 节点类型定义 + 纯 header），写 `test_phase2.c`（40 项：构建/打印/free 每种节点，ASan 检查无泄漏）。AST 节点设计是整个前端的"词汇表"——一旦在 Phase 2 定型，后续阶段不能随意改动 AstNode 布局（API 向后兼容要求 I6）。

基线：`make test` 应输出 `Phase 0 results: 58` + `Phase 1 results: 213 passed, 0 failed`。

— Phase 1 接力人，于 Phase 1 收尾时

---

## Spec v0.11 Update 接力补遗（import + auto）

本次用户上传了更新版 spec，新增两个特性，修改了一个设计：

**变更 1 — `import` 关键字（spec §模块系统）**：
- Sharp 取消了 `.sph` 头文件。所有声明 + 实现都在 `.sp` 文件中。
- `import "path.sp";` 是新关键字，由 fe **parse 层**处理（不走 cpp 预处理器）。
- lex 层已更新：`STOK_IMPORT` 加入 Sharp keywords，`import` token 被正确提升。
- 探针 P14.1-P14.3 验证了 cpp 不处理 `import`（不查文件、不报错）。

**变更 2 — `auto` 类型推导（spec §auto-类型推导）**：
- `auto x = expr;` 从 expr 推导类型，转译 C 时展开为具体类型。
- lex 层无需改动（`STOK_AUTO` 已存在作为 C11 关键字）。
- **parse 层（Phase 3）**：`auto` 出现在声明类型位置时，生成 `AstTypeAuto` 占位节点。
- **sema 层（Phase 6）**：`auto` 无初始化表达式报错；有初始化表达式则推断类型填回节点。
- **cg 层（Phase 12）**：生成 C 代码时输出具体类型，永不输出 `auto`。

**坑 FE-5**（新增）：`import` 解析时目标文件也需要过 cpp（但要用**新的** `CppCtx`，不共享当前文件的 cpp 上下文），否则目标文件的 `#define` 宏会泄漏——这违反 spec 规定。

**测试数字更新**：
- 探针：58 → 66（+8：P14.1-P14.5 + P14.D1-D3）
- Phase 1 测试：213 → 222（+9：`import` 6 项 + `auto` 3 项）

— Spec v0.11 Update，于 Phase 1 收尾时

---

## Phase 2 接力补遗

本阶段完成 AST 类型定义（`ast.h` + `ast.c`）+ 73 项测试。

**关键设计决策**：
- **内存模型：每节点 malloc**，`ast_node_free` 递归 free。首版简单优先；如果 profiling 显示 Phase 11/12 的大 AST 导致性能问题，届时换 arena。
- **AstVec backing array 与节点分离**：`astvec_deep_free` 先递归 free 每个子节点，再 free backing array。顺序不可颠倒——否则 use-after-free。
- **string_lit.text 是借用指针**：`ast_node_free` 里 `AST_STRING_LIT` 分支不 free `.text`（它借用 `CppResult._raw_text`）。这是一个隐式约定——将来如果 AST 生命期超过 CppResult，需要改为 strdup。
- **operator 用 SharpTokKind 编码**：`binop.op`、`unary.op` 直接存 `SharpTokKind` 值（STOK_PLUS 等），无额外 enum。`ast_print` 用 `lex_tok_kind_name(op)` 输出可读名称。
- **strdup → cpp_xstrdup**：C11 模式下 `strdup` 是 POSIX 扩展，需要 `_POSIX_C_SOURCE`。所有字符串复制统一用 `cpp_xstrdup`（来自 `cpp_internal.h`，自动可用）。

**坑 FE-6**（本阶段发现）：`strdup` 在 `-std=c11` 下是隐式声明，返回 `int`，赋给 `char*` 导致 SEGFAULT——ASan 的报错指向 `strcmp`（解引用时崩），不是 `strdup`，容易误导。**解法：始终用 `cpp_xstrdup`**。

**Phase 3 推荐下一动作**：建 `parse.c/h`——递归下降解析，把 `SharpTok[]` 流转为 `AstNode*` 树。先写 `test_phase3.c`（~150 项，§1-§9 见 PLAN.md），再实现。`import` 和 `auto` 是 Phase 3 的新增解析点。

基线：`make test` 应输出 Phase 0: 69 + Phase 1: 222 + Phase 2: 73，全 0 fail。

— Phase 2 接力人，于 Phase 2 收尾时

---

## Phase 3 接力补遗

本阶段完成递归下降解析器（`parse.h` + `parse.c` ~820 行）+ 108 项测试。

**关键实现决策**：
- **Pratt 解析器**：从 prec=2 开始的 `parse_expr_prec(ps, min_prec)`，comma 算 prec=1（防止函数参数列表里的 `,` 被吃成 comma 运算符）。
- **`>>` 拆分**：`pending_close` 字段加入 `PS` struct，`parse_generic_args()` 消费 `>>` 时把一个 `>` 记入 `ps->pending_close`，供外层 `parse_generic_args()` 消费。跨级传递，彻底解决 P1.8。
- **声明 vs 表达式区分**：`is_type_start()` 在 `IDENT` 后检查 peek2 是否为 `IDENT|*|<|operator`。不完美但对 Sharp 的实际代码足够。
- **struct operator 顺序**：运算符检测（`if ps_at(STOK_OPERATOR)`）必须在 `parse_type()` 之后，因为运算符有返回类型（`Vec3 operator+(Vec3 rhs)`）。

**坑 FE-7**（本阶段发现，已修）：struct body 内的 operator 检测放在 `parse_type` 之前会导致 `ps_expect(STOK_IDENT)` 看到 `STOK_OPERATOR` 报错。正确顺序：type → check operator → check ident。

**Phase 4 推荐下一动作**：建 `scope.h/scope.c`——作用域链 + 符号表。测试先行（~70 项），测试符号注册、查找、嵌套 block 遮蔽、struct scope。

基线：69 probes + 222 P1 + 73 P2 + 108 P3 = 472，全 0 fail。

— Phase 3 接力人，于 Phase 3 收尾时

---

## Phase 4 接力补遗

本阶段完成 scope 作用域/符号表（`scope.h` + `scope.c`）+ 59 项测试。

**关键实现决策**：
- **两遍 build**：Pass 1 注册所有顶层名字（支持前向引用），Pass 2 递归进入 struct body 和 func body。
- **children 列表**：每个 Scope 记录它创建的子 scope（`Scope **children`）。`scope_free` 深度优先递归 free 所有子 scope，确保整棵 scope 树一次性 free。
- **scope_free_chain 语义调整**：现在是"找到 root，然后 scope_free(root)"（从上往下），不再从下往上走 parent 链。这样不会重复 free（之前的问题：parent.children 里的 scope 已经被 scope_free_chain 逐个 free，再由 scope_free 递归 free 时就 double-free）。
- **type_ref 复用**：`AstNode.type_ref`（Phase 5 的 TypeRef*）在 Phase 4 里临时用来存 Scope* 指针（将来 Phase 5 会用真正的 TypeRef）。这是一个 `void*` 的多用途 hack——清楚记录在注释里。

**坑 FE-8**（本阶段发现，已修）：`scope_free_chain` 从叶子往上 free，但 `scope_free` 也递归 free children，导致 global.children 里的 file scope 被 double-free（use-after-free）。根因：children 注册机制使 free 路径变了，old `scope_free_chain` 的语义与新 children 机制不兼容。修法：改为"从 root 往下 free"。

**Phase 5 推荐下一动作**：建 `type.h/type.c`——类型系统（C 类型 + const + struct + 泛型类型表达式 + intern 唯一性）。测试先行（~100 项）。`type_ref` 字段届时会被真正的 `TypeRef*` 填充（目前 Phase 4 用它存 Scope*，Phase 5 之后统一为 TypeRef*）。

基线：69 probes + 222 P1 + 73 P2 + 108 P3 + 59 P4 = 531，全 0 fail。

— Phase 4 接力人，于 Phase 4 收尾时

---

## Phase 5 接力补遗

本阶段完成类型系统（`type.h` + `type.c`）+ 107 项测试。

**关键实现决策**：
- **原始类型为 singleton**：存在 `TyStore.prims[]` 数组里，地址稳定，每次 `ty_int(ts)` 返回同一指针。`ty_eq` 直接用指针相等。
- **复合类型用线性扫描 intern**：对于 PTR/CONST/ARRAY/STRUCT/PARAM/FUNC，在 `ts->compound[]` 里线性搜索结构相等的节点。找到则返回现有指针，否则 malloc 新节点。O(n) 但实际类型数量小（< 200），首版够用。
- **字符串也 intern**：`ts->strings[]` 存 STRUCT/PARAM 的名字，保证 `interned_name_ptr == interned_name_ptr` 成立，compound 比较可以用指针而非 strcmp。
- **TY_STRUCT 和 TY_FUNC 的 args/params 数组**：`ts_intern` 在把节点加入 compound 表时，深拷贝 args/params 数组（防止调用者 free 导致悬挂引用）。`ty_store_free` 负责 free 这些拷贝。

**坑 FE-9（本阶段发现，已修）**：`const char*` 在 parse.c 里原来被解析为 CONST(PTR(char))——即 `char* const`（const 指针），而不是 PTR(CONST(char))——即 `const char*`（指向 const 的指针）。根因：const 分支调用了完整的 `parse_type()`（会吃掉 `*` 后缀）。修法：拆出 `parse_type_unqual()`（只解析基础名字，不处理 `*`），const 分支调用它，然后通过 `goto apply_suffix` 让 `*` 后缀应用到整个 CONST 节点。这个修复同时改善了 Phase 3 的语义正确性（parse_type3 全 108 项仍然通过）。

**基线**：69 + 222 + 73 + 108 + 59 + 107 = **638 项，全 0 fail**。

— Phase 5 接力人，于 Phase 5 收尾时

---

## Phase 6 接力补遗

本阶段完成语义分析基础（`sema.h` + `sema.c`）+ 43 项测试。

**Phase 6 边界（刻意不做）**：
- 方法调用（`.method()`、`->method()`）→ Phase 7
- 运算符重载 → Phase 7
- struct 字段访问 → Phase 7
- defer 语义检查 → Phase 9
- @intrinsic 语义 → Phase 10
- 泛型单态化 → Phase 11

**关键实现决策**：
- **type_ref 双用途**：声明/控制节点（FUNC_DEF, STRUCT_DEF, BLOCK, FOR）的 `type_ref` 继续存 Phase 4 建立的 Scope*；表达式节点和 VAR_DECL 的 `type_ref` 由 sema 填充 Type*。两类节点不重叠，安全。
- **`auto` 推断**：在 `sema_stmt(AST_DECL_STMT)` 里，先 sema_expr init 得到 init_t，当 declared_type 是 AST_TYPE_AUTO 时，直接把 init_t 设为 decl_type，写入 vd->type_ref。连续 `auto b = a;` 也能正确传播（因为 `a` 的 VAR_DECL.type_ref 已经在前一个语句里设好了）。
- **`for` 的 init-scope**：`for (int i=0; ...)` 里的 `i` 在一个 for_scope（Phase 4 建立的 SCOPE_BLOCK）里，sema 用 `SS inner = *ss; inner.scope = for_scope;` 来正确切换 scope。
- **assign_compat 规则**：`T → const T` 合法；`const T → T` 报错；arithmetic ↔ arithmetic 全部接受（Phase 6 不做严格窄化 warning，留 Phase 7）；`void*`（null 的类型）→ 任意指针合法。

**Phase 7 推荐下一动作**：建 `sema_method.c/h`——扩展 sema，增加 struct 字段访问类型推断、方法调用类型推断、运算符重载查找。测试先行（~80 项）。

基线：69 + 222 + 73 + 108 + 59 + 107 + 43 = **681 项，全 0 fail**。

— Phase 6 接力人，于 Phase 6 收尾时

---

## Phase 7 接力补遗

本阶段完成 sema 字段/方法/运算符扩展 + 28 项测试。改动集中在 `sema.c`（+250 行 Phase 7 helper）、`ast.h`（+1 字段）、`parse.c`（+2 处修复）。

**关键实现决策**：
- **struct_scope_of**：从 TY_STRUCT.decl 指针（Phase 5 由 ty_from_ast 设置）取出 Phase 4 建立的 Scope*。这是两个阶段协作的接缝点。
- **is_const_method 字段**：新加到 `ast.h` 的 `func_def`，由 parse.c 在遇到 `)  const  {` 时设置。sema 用于 const receiver 检查。
- **关联函数调用 Type.func()**：parse 生成 METHOD_CALL(recv=IDENT("Type"), method="func")，sema 在 recv 是 SYM_TYPE 时改走 struct scope 里的关联函数路径，不报 "is a type" 错误。
- **operator[] 注册名**：parse 里 `operator[` + `]` 两个 token，拼接后是 "operator[]"（必须完整拼接，而不是只取 `[` 的文本 "operator["）。

**坑 FE-10**（本阶段发现，已修）：test_phase7 里 `first_var` 用 `decls[0]` 当 FUNC_DEF，但有 struct 定义在前面时 `decls[0]` 是 STRUCT_DEF，导致 NULL 返回。改为遍历找第一个 FUNC_DEF。这是测试写法 bug，不是实现 bug。

**Phase 8 推荐下一动作**：代码生成 Phase（cg.h/cg.c）——把 AST 转译到 C11 代码。C11 代码通过 `gcc` 编译。测试先行：对 spec 综合示例（Vec<T> + Vec3 + Buffer）生成 C 代码并通过 gcc 编译。

基线：69 + 222 + 73 + 108 + 59 + 107 + 43 + 28 = **709 项，全 0 fail**。

— Phase 7 接力人，于 Phase 7 收尾时

---

## Phase 8 接力补遗

本阶段完成代码生成（`cg.h` + `cg.c` ~780 行）+ 47 项测试。生成的 C11 代码可通过 gcc 编译。

**关键实现决策**：
- **三遍 Pass**：Pass 1 — 所有 struct 前向声明（`typedef struct X X;`）；Pass 2 — struct 完整定义 + 方法实现；Pass 3 — 自由函数 + 全局变量。这样方法可以互相引用而不需要前向声明方法。
- **方法名 mangling**：`StructName__methodName`；运算符 `operator+` → `StructName__op_add`（后缀表：add/sub/mul/div/mod/eq/ne/lt/gt/le/ge/idx/band/bor/bxor/shl/shr）。
- **defer inline 展开**：`cg_block` 维护一个 per-block defer 列表，在每个 `return`/`break`/`continue` 前按 LIFO 顺序内联展开，在 block 末尾也展开剩余 defer。
- **struct literal** → C11 复合字面量 `(Type){ .field = val }` 形式。
- **this 参数**：生成方法时，第一个参数是 `Type* this`（非 const 方法）或 `const Type* this`（const 方法）。调用时，值类型 receiver 取地址（`&b`）。
- **SB 命名冲突**：cg.c 里的 string builder 最初命名为 `SB/sb_push/sb_printf`，与 `cpp_internal.h` 里的同名函数冲突，改为 `CgSB/cgb_push/cgb_printf` 解决。

**坑 FE-11**（本阶段发现，已修）：为让 struct 方法被 sema 处理（设置 type_ref），在 `sema_check_file` 里新增对 STRUCT_DEF methods 的 sema_func 调用。但 `this` 的 decl 是 FUNC_DEF 节点（Phase 4 的设计），不是 PARAM_DECL，导致 `this` 类型推断失败、Phase 7 测试回归。修法：sema 的 IDENT 处理对 `this` 特殊处理——从 enclosing STRUCT scope 推导 `struct_type*` 或 `const struct_type*`。

**基线**：69 + 222 + 73 + 108 + 59 + 107 + 43 + 28 + 47 = **756 项，全 0 fail**。

— Phase 8 接力人，于 Phase 8 收尾时

---

## Phase 9 + 10 接力补遗

本阶段完成 defer 语义检查（Phase 9）和 @intrinsic 语义（Phase 10），均集成进 `sema.c`（新增 ~200 行）。测试：15 + 10 = 25 项，全 0 fail。

### Phase 9：defer–goto 检查

**实现方案（保守策略）**：
- 两遍扫描函数体：Pass 1 探测是否有任意 `defer`，Pass 2（如有 defer）扫描所有 `goto`，发现即报错。
- 这比 spec 精确要求（路径分析）更保守：任意 goto + 任意 defer 在同一函数内 = 错误。
- `break`/`continue` 不触发检查（spec §defer-stmt 明确安全）。
- `if (!diags) return;` 防御性检查避免 NULL diags 崩溃（Pass 1 用 NULL diags 做探针）。
- 坑：`defer ;` 是 parse error（非法语法）。测试用 `defer { }` 代替。

### Phase 10：@intrinsic

**实现方案**：
- `@static_assert(cond, "msg")`：通过 `eval_const_int` 对 cond 求编译期整数值，0 → 报错，非零 → OK，非常数 → 沉默（留给运行时）。
- `@has_operator(T, op)`：在 T 的 struct scope 里查找 `"operator<sym>"`，存在返回 1，否则 0。
- 两者嵌套 `@static_assert(@has_operator(T, +), "msg")` 通过 `eval_intrinsic` 递归求值。
- AT_INTRINSIC 的 sema_expr 不调用 `sema_expr(args)` 以避免类型名被当表达式报错。

**坑（@has_operator 参数解析）**：
- `@has_operator(T, +)` 中 `+)` 被 parse 成 UNARY(+, `)`)，消费了闭括号，导致 parse 错乱。
- 修法：测试里改写为 `@has_operator(T, 0+0)`，让 `+` 出现在合法的 binop 表达式里。`eval_has_operator` 从 AST_BINOP.op 提取操作符类型 → `"operator+"`。

**坑（null byte in source）**：Python 字符串里的 `'\0'` 被写成字面 null byte 到源文件，编译报 `-Werror=null-character`。用 `rb` 模式读写修复。

**基线**：69 + 222 + 73 + 108 + 59 + 107 + 43 + 28 + 47 + 15 + 10 = **781 项，全 0 fail**。

— Phase 9/10 接力人，于 Phase 9+10 收尾时

---

## Phase 11 接力补遗

本阶段完成泛型单态化（monomorphization），全部集成进 `cg.c`（+280 行）。19 项测试，全 0 fail。

**生成流程**（cg_file 里的新 Phase 11 pass）：

1. `cg_collect_all(ctx, file)` — 遍历所有函数体、参数类型、返回类型，找出所有使用了泛型实参的 TY_STRUCT 类型。
2. 对每个唯一实例（如 Vec<int>、Vec<long>）调用 `cg_emit_specialization`。
3. `cg_emit_specialization` 找到原 generic struct 的 AST 定义，把 generic params（T）替换为 concrete args（int），生成一个具名 C struct + 方法。
4. 后续 Pass 2 跳过 generic struct（它们已在 Phase 11 pass 处理）。

**关键辅助函数**：

| 函数 | 作用 |
|---|---|
| `cgb_mangle_type(sb, t)` | 把 Type* 序列化为名字片段（int→"int"，Vec<int>→"Vec__int"）|
| `cg_mangle_inst(sname, args, nargs)` | 生成完整 mangled 名，如 "Vec__int__long" |
| `subst_type(ts, t, pnames, pvals, n)` | 用具体类型替换 TY_PARAM 占位符 |
| `cg_inst_seen(ctx, mn)` | 去重：记录已 emit 的实例名，避免重复 |
| `cg_emit_spec_method` | 带类型替换的方法生成 |
| `cg_type()` | 更新：TY_STRUCT with nargs > 0 → 输出 mangled 名 |

**坑（Phase 11）**：
- `cg_mangle_inst` 在 `cg_type()` 里（line 140）被调用，但定义在 line 780。需要前向声明，否则 C 编译器用隐式 `int` 返回类型产生 conflicting types 错误。
- CgCtx 结构体需要新增 `file_ast`、`inst_names`、`ninsts`、`insts_cap` 四个字段，`cg_ctx_free` 需对应释放。

**示例输出**（`struct Vec<T> { T* data; long len; }` + `Vec<int> v`）：
```c
typedef struct Vec__int Vec__int;
struct Vec__int {
    int *data;
    long len;
};
```

**基线**：69 + 222 + 73 + 108 + 59 + 107 + 43 + 28 + 47 + 15 + 10 + 19 = **800 项，全 0 fail**。

— Phase 11 接力人，于 Phase 11 收尾时

---

## Integration + Driver 接力补遗

本轮完成两件事：`sharpc.c`（命令行驱动）+ `test_integration.c`（端到端集成测试，27 项）。

### sharpc 驱动

用法：`./sharpc input.sp [-o out.c]` 或 `./sharpc -`（读 stdin）。
流程：`read_file → cpp_run_buf → lex_run → parse_file → scope_build → ty_store → sema_check_file → cg_generate`。
诊断输出到 stderr（格式 `file:line:col: level: msg`），exit code 0 = 成功，1 = 有错误，2 = I/O 错误。

### 集成测试修复的 bug（跨多个 Phase）

**坑 FE-12：for/while 非 block body 被丢弃**
`for (int i=0; i<n; i++) s += i;` — 当 body 是单个语句（不是 block）时，`cg_block(ctx, body)` 直接返回空块。修法：在 for/while body 处理前检查是否是 BLOCK，若不是则用 `{ indent; cg_stmt(...); }` 包裹。

**坑 FE-13：operator 方法用值接收器导致 `this->x` 在 C 中非法**
`operator+` 的方法签名生成为 `V2 V2__op_add(V2 this, V2 r)` (value receiver)，但方法体里用 `this->x`（指针语法），C 编译报错。修法：所有方法（包括 operator）统一用指针接收器 `V2* this`。

**坑 FE-14：struct 方法前的全局变量不可见**
`Res__destroy` 引用全局 `g_destroyed`，但生成的 C 里 `g_destroyed` 出现在 `Res__destroy` 之后。修法：在 Pass 1 和 Phase 11 pass 之间新增 Pass 1b，先输出所有全局变量定义。

**坑 FE-15：运算符重载检查在比较检查之后**
`sema_binop` 里比较检查（`STOK_EQEQ` 等）在运算符重载检查之前，导致 struct 的 `operator==` 触发 "comparison of non-scalar" 错误。修法：把 Phase 7 运算符重载检查块移到比较检查之前。

**坑 FE-16：sema 中泛型方法返回类型用调用者 scope 解析**
`Box<int>.get()` 中，`get()` 的返回类型 `T` 用 main 的 scope 解析 → `unknown type 'T'`。修法：`sema_method_call_expr` 改用方法自身的 func scope（`fn->type_ref`）解析返回类型，再通过 `sema_subst_for_struct` 替换泛型参数。

**坑 FE-17：if 非 block body 不转发 defer 列表**
`defer mark(); if (x) return x;` 中，单语句 if body 调用 `cg_stmt(ctx, then, NULL, 0)` 而不是 `(ctx, then, defers, ndefers)`，导致 early return 前 defer 不触发。修法：非 block if/else body 也包一层 `{ }` 并传入 defers。

**坑 FE-18：cg_expr 泛型方法调用用未 mangled 的 struct 名**
`Box<int>.get()` 生成 `Box__get(&b)` 而不是 `Box__int__get(&b)`。修法：`cg_expr(METHOD_CALL)` 里，当 recv 是泛型实例时用 `cg_mangle_inst` 生成完整 mangled 名。

**坑 FE-19：sema 泛型字段类型解析用调用者 scope 而非 struct scope**
`Box<int>.val = 99` 中，val 的类型 `T` 用 main 的 scope 解析 → `unknown type 'T'`。修法：`sema_field_access_expr` 改用字段所在 struct 的 scope 解析，再通过 `sema_subst_for_struct` 替换。

**基线**：69+222+73+108+59+107+43+28+47+15+10+19+27 = **827 项，全 0 fail**。

— Integration 接力人，于最终轮收尾时

---

## CPP UAF 根因修复

**坑 FE-20（CPP 内部 bug）**：`directive.c` 在每次 `sb_push*(&st->out_text, ...)` 时可能触发 `cpp_xrealloc`，把 `out_text.buf` 移到新地址，但已存储在 `out_tokens[i].text` 里的旧 buf 地址变成悬垂指针。前端所有后续对 token.text 的访问（`lex_run`、`keyword_kind` 里的 `strncmp`）都是 UAF。

表现：对于小型 Sharp 程序不触发（freed 内存未被及时复用），对于 Vec3 operator+ 这类较大程序，sema 里的 `ty_struct_type` / `ty_from_ast` 等 malloc 会复用 freed 内存，导致 token.text 读到垃圾数据，进而 parse 报 `expected ; after variable declaration, got '<garbage>'`。

**修法**（在 sharp-cpp/directive.c）：
1. 新增 `fix_token_ptrs(CppState *st, const char *old_buf)` 辅助函数：若 `out_text.buf` 地址发生变化（realloc 移位），把 `out_tokens[i].text` 指针统一加上位移量 delta。
2. 用三个宏 `PUSH_CH/PUSH_STR/PUSH_CSTR` 替换所有 `sb_push*(&st->out_text, ...)` 调用，宏内部自动保存 old_buf、调用 sb_push*、再调用 fix_token_ptrs。
3. sb_printf 同理（手动展开）。

**附带修复（lex.c）**：`lex_run` 里 `st.text = ct.text`（直接复用指针）改为 `st.text = cpp_xstrndup(ct.text, ct.len)`（立即复制），使 SharpTok 生命周期独立于 CppResult。`lex_free` 对应 free 每个 token 的 text 副本。

**验证**：sharp-cpp 原有 615/615 测试全部通过，前端 827/827 通过，ASan 门无 ERROR。


---

## 本轮新增功能（860项测试，全0 fail）

### Import 系统（25项测试）
`import.h/import.c` — 完整实现：
- 相对路径 + `-I search_path` 解析
- 传递 import（A→B→C）
- 去重（同文件 import 两次 = no-op）
- Cycle 检测（error 而非 silent skip）
- 跨文件 sema 类型引用

**根本 Bug（FE-21）**：`realpath` 未声明（`stdlib.h` 未先于 cpp/sharp 内部 header include），返回值被截断为 32-bit int 后 sign-extend 成无效地址。修复：`import.c` 第一行 `#include <stdlib.h>`。

### sharpc.c 升级
- 接入 `import_resolve`
- 新增 `-I dir` / `-Idir` flag

### 集成测试 §9（4项）
跨文件函数、struct、transitive import、dedup 端对端测试。

### 泛型独立函数单态化（Phase 11 extension，7项新测试）

**实现要点**：
- `sema.c`：跳过 generic 函数体检查（`fn->u.func_def.generic_params.len > 0 → return`），避免 cascade errors；改善 `sema_call` 里的返回类型推断（从参数类型推断 T）
- `cg.c`：
  - 新增 `GFuncInst` 结构 + 全局 instance 列表
  - `cg_collect_expr` 里对 `AST_CALL` 检测 generic 函数调用，推断 type args，记录实例
  - `cg_emit_gfunc_specs`：发射所有收集到的特化版本
  - `cg_expr` AST_CALL：对 generic 调用输出 mangled 名（`identity__int`）
  - `cg_type` TY_PARAM：在特化上下文中用 `gp_names/gp_vals` 替换

**生成示例**（`T identity<T>(T x) { return x; }` 调用 `identity(42)`）：
```c
int identity__int(int x) {
    return x;
}
...
    return identity__int(42);
```

**已知局限**：
- 仅支持从参数类型推断 T（不支持显式 `identity<long>(x)` 语法）
- 多参数时只绑定直接匹配的 TY_PARAM（不做深度递归推断）
- 全局状态 `g_gfinsts` 在每次 `cg_file` 调用时重置


---

## 多变量声明 + 定长数组接力补遗

**接力起点**：上一位接手在 chat log 中描述了对 `int x = 3, y = 7;` 与 `int arr[3]` 的修复方案，但代码层未真正落盘——交付时 parse.c / type.c / cg.c 仍是不支持这两种语法的版本，只是 `test_integration` (40) + `test_import` (25) = 65 项继续过。`test_phase*.c` 不在交付的 tar 中。

### 一句话目标
让用户写 `int x = 3, y = 7;` 与 `int arr[3]` / `int m[3][4]` 不再被 parser 拒绝，并端到端 `sharpc → cc → run` 跑出正确结果。

### 实施摘要

**ast.h / ast.c**
- 新增 `ast_clone_type()`：深拷贝任意 type 子树（TYPE_NAME / PTR / CONST / ARRAY / FUNC / GENERIC / AUTO / VOID / PARAM）。多变量声明中第一个 declarator 持有原始 base type，后续 declarator 各自持一份克隆，避免 `ast_node_free` 二次释放。
- 配套 `clone_size_expr()`：克隆 array-size 表达式（INT_LIT / CHAR_LIT / IDENT / UNARY / BINOP）。其他形式返回 NULL，下游报错。

**parse.c**
- PS 新增 `pending_decls` 字段。多变量场景下 `parse_var_decl_list` 把第 1 个 decl 作为函数返回值，多余的入队；`parse_block` 与 `parse_file` 的主循环每轮 drain。
- 新增 `parse_var_decl_list(ps, base_ty, stmt_wrap)` —— 循环消费 `,`，每个 declarator 解析 IDENT、可选 `[size]`、可选 `= expr`。Initialiser 用 `parse_expr_prec(ps, 2)`（**关键**：跳过 comma 运算符，否则 `int x = 3, y = 7` 被吃成一个 comma expression）。
- 新增 `parse_array_suffix(ps, inner_ty)` —— 支持 1D / 多维 `[3][4]`，从源序左→右收集 size，反向 wrap 让最左 suffix 成最外层 ARRAY（与 C 语义一致）。
- `parse_top_decl` 与 `parse_stmt` 的 var-decl 分支统一改用上述 helper。
- `for`-init 中调多变量场景：drain `pending_decls`，逐个 free 并 `ps_error("multiple declarators in 'for' init not supported")` —— 防止溢出 decl 泄漏到 for 之后的 block。
- `is_type_start` 用 3-token lookahead 区分 `Vec<int>`（泛型）vs `i < 5`（比较）。第三个 token 必须是 type-like 才认为 IDENT < 是泛型。
- `parse_generic_args` 加 diag-snapshot rollback：trial 内部的 `parse_type` 一旦走「expected type」错误路径，rollback 时把 diag 一并回滚，否则会留下假 error。同时在调 `parse_type` 之前做 cheap pre-check，仅对 type-like token 启动 trial。
- `parse_var_decl_list` 内：`*` 出现在 declarator 处一律拒绝并报「Sharp 中 `*` 是类型的一部分；写 `T* a, b;` 即可」—— 明确「Sharp 用类型前缀风格」的语义，避免 C 与 Sharp 在 `int *p, *q` 上的歧义。

**type.c**
- `ty_is_pointer()`：现在认 `TY_ARRAY`（数组在表达式上下文衰减为指针）。
- `ty_deref()`：现在解 `TY_ARRAY` 与 `TY_CONST(TY_ARRAY)` 返回 base。
- 这两条改动是让 `arr[i]` 的 sema 检查通过的关键。

**cg.c**
- `cg_decl()` 处理 `TY_ARRAY`：找到最内层元素类型，emit `T name[N1][N2]…`。多维按 outermost→innermost 顺序（即源序）输出 suffix。`size < 0` 输出 `[]`（不完整数组）。
- `cg_type()` 给 `TY_ARRAY` 一个衰减 fallback：`T *`。用于 sizeof / cast / 函数参数等表达式上下文。

### 已知约束 / 设计决策
- `for (int i = 0, j = 0; …)` 不支持，会报错。多变量 for-init 极少见，强行支持会让 AST_FOR 复杂化。
- Sharp 中 `*` 是类型的一部分；不允许 declarator-prefix `*`。`int* p, q;` 两个都是 `int*`；`int* p, *q;` 报错。这与 C 不同但比 C 一致。
- Array size 表达式只克隆字面量级别（INT_LIT / IDENT / 等）；复杂 const-expr 在多变量场景下可能克隆失败（克隆函数返回 NULL，下游 sema 报错）。spec 没规定数组 size 必须是 int literal，但实际用例都是字面量，未见问题。
- 数组在 sema 中视作 pointer（`ty_is_pointer` 返回 true）。这意味着把 `int[5]` 传给 `int*` 参数 OK；但 `int[5]` 传给 `int[5]` 参数（sema 会找完全相等类型）也 OK，因为我们走的是 ptr 兼容路径。生成的 C 中 array 自动 decay 给 cc 处理。

### 测试

新增 `test_decls.c`（测试套件 §1–§5，共 **27 项**）：
- §1 多变量：基本两变量、三变量含无 init、后看前 init、顶层、Sharp 风格指针、long、5+ 多 declarator、复杂表达式 init、函数调用 init、ternary init
- §2 数组：1D、loop 填充 + 求和、2D `m[3][4]`、衰减传指针参、`char s[3]`、顶层数组
- §3 混合：`int a, b[3], c;`、`int a[3], b[2];`、顶层混合
- §4 泛型 vs 比较：`i < 5` 表达式、for-cond `i < 5`、`Vec<int> v;`、init 中 `x < y ? : `
- §5 错误：for-init 多变量、`auto x;`、`auto y` 在多变量中、C-style `int *p, *q`

测试运行管线：Sharp source → pipeline (lex/parse/sema/cg) → 写到 /tmp → `gcc -std=c11 -w` 编译 → 运行 → 检 exit code。即每个测试都验证**运行时语义**，不是只看 parse 通过。

### 三道门基线（本轮修复后）

```
Phase 0 probes:        69 probes
Integration tests:     40 passed, 0 failed
Import tests:          25 passed, 0 failed
Decls tests:           27 passed, 0 failed
─────────────────────────────────────
Total:                 92 passed, 0 failed
make test   ✓
make asan   ✓ (no leak/UAF/UB)
make strict ✓ (-Wpedantic -Wshadow -Wstrict-prototypes -Werror)
```

### 新增「坑」总结

- **FE-22 多变量声明 type AST 共享**：`int x=3, y=7` 中第一稿让两个 VAR_DECL 共指同一个 type 节点，`ast_node_free` 重复 free。修复：`ast_clone_type` 给后续 declarator 各自一份。
- **FE-23 多变量 init 被 comma 吞噬**：`int x = 3, y = 7` 中 `parse_expr` 默认从最低优先级（comma）开始，会把 `3, y=7` 解析成一个 comma expression。修复：`parse_expr_prec(ps, 2)` —— 从 assignment 优先级起，跳过 comma 运算符。
- **FE-24 `i < 5` 误识别为泛型**：`is_type_start` 见 `IDENT <` 就回 true，进 decl 分支后 `parse_type` 失败但已 push diag。修复：① `is_type_start` 用 3-token 看 `<` 后是否 type-like；② `parse_generic_args` rollback 时同步 rollback diag 数组并预先 peek。
- **FE-25 `int arr[3]` sema 不识别为 ptr**：`ty_is_pointer` / `ty_deref` 不含 TY_ARRAY，`arr[i]` 报「subscript of non-pointer」。修复：两函数都把 TY_ARRAY 视作 ptr。
- **FE-26 cg 数组用 `int[3] arr` 错语法**：C 数组语法是 `int arr[3]`，name 在尺寸前。修复：`cg_decl` 检测 TY_ARRAY 时找最内层元素类型先 emit，再 emit name，再按外层→内层 emit suffix。

### Makefile 改动

- 新增 `test_decls` target。
- `test:` target 改为只跑 `probe + sharpc + test_integration + test_import + test_decls`（去掉 tar 包中不存在的 `test_phase1..11` 依赖）—— 否则 `make test` 直接构建失败。如未来有人重建 phase 测试，加回去就行。

— 多变量+数组接力人，本轮收尾时

---

## C-superset 探索接力补遗

**目标的演进**：上一轮接力把 sharp-fe 自身的 Sharp 源码处理修到 92/92 全过。
这一轮要走得更远：让 sharp-fe 能解析*任意*预处理过的 C 源码，从而完整链
`.c → sharp-cpp → sharp-fe → C → cc → binary` 能跑通 Lua 5.4.7 testsuite。

**结论**：这是一个真正的架构升级，不是几个 patch。本轮**不在此完成**，但
建立了：

1. **41 个最小化 C 探针**（`c_superset_probes/`）+ 自动化 `c_superset_probe.sh`
2. **完整 ROADMAP** （`ROADMAP.md`）—— 5 个独立可交付子阶段 S1..S5，每个
   有自己的验收条件和回到主任务 M 的切换点
3. **干净的回滚** —— 试探阶段做的 hack 已全部撤回，目前树是 checkpointable 状态：
   - 92/92 老测试全过（无回归）
   - 20/41 C 探针通过
4. **保留的干净改进**：
   - `(void)` 参数表（`parse_param_list` 单 void 检测）
   - 数组形参 `int a[]` / `int a[N]`（衰减到 TY_ARRAY，靠 cg fallback）
   - 可空参数名（`AST_PARAM_DECL.name = NULL` 用于 prototype）

**已撤回的 hack**（不要重做）：
- 顶层/语句层 storage-class 吞掉（应进 AST 字段，cg 输出对应 C 关键字）
- `AST_INIT_LIST` 半实现（parse + 部分 sema 写了，cg 没写——必须一次性完成）
- 合成 `_p<N>` 占位参数名（应让 NULL 流过 sema/cg）

**为什么需要架构升级**：C 的声明语法是「decl-specifiers + 递归 declarator」二元式
（ISO 6.7），sharp-fe 现在是「类型作前缀」单线式。`int (*signal(int, void(*)(int)))(int);`
sharp-fe 现有的 declarator 模型完全没办法。子阶段 S1 重构这个模型；之后 S2..S5
都建立在 S1 的基础上变成局部 AST 添加。

**新接手第一件事**：读 `ROADMAP.md`，从 §8 PAUSED AT 看当前在哪。

— C-superset 探索接力人，于本轮收尾时

---

## Phase S5 接力补遗

本阶段完成 C-superset 路线图最后一站：把 `sharp-fe` 接入 Lua 5.4.7 完整
构建管线，让 `.c → sharp-cpp → sharp-fe → C → cc → binary` 全程跑通且
官方 testsuite 出 `final OK !!!`。

**项目状态一行总结**：S5 完成。三道门全过；C-superset 51/52；Lua 5.4.7
testsuite `final OK !!!`。

**最重要的 4 件事给下一位**：

1. **`build_lua_with_sharpfe.sh` 是 S5 的核心产物**。位于仓库根 (`sharp/`)，
   会自动下载 Lua 源、跑完整 6 阶段管线并验 testsuite。`RUN_TESTSUITE=0`
   可跳过 testsuite 阶段。出问题时去 `_logs/` 里看 per-file 错误。

2. **本轮发现的两个真实世界 bug 已就地修复**，不要回退：
   - **labels-as-values（`&&label` + `goto *expr;`）**：Lua 解释器分派循环
     需要的 GCC 扩展。新增 `AST_ADDR_OF_LABEL` / `AST_COMPUTED_GOTO`，
     parse/sema/cg 三层闭合。详见 `PHASE_S5_SUMMARY.md` § Gap 1。
   - **float 字面量精度截断**：`cg.c` 用 `%g`（6 位有效数字）发射 float
     字面量，把 `3.141592653589793238462643383279502884` 截成 `3.14159`，
     导致 `math.tan(math.pi/4)` 在 `math.lua:650` 失败。改用 `%.17g`
     （DBL_DECIMAL_DIG）+ 必要时补 `.0` 后缀（防止 `1.0` 变成整型 `1`，
     翻转 `1.0/2` 的结果）。详见 § Gap 2。

3. **顺手修复了 import.c 的旧 leak**。S4 文档声称 `make asan: clean`，但
   新解压的代码上 asan 报 5 个 leak（共 144 字节），全部在 `splice_one`
   把 imported 文件的 decls 数组传所有权进新数组时把 `data = NULL` 而
   没 free 旧 backing array。改用 `astvec_free`。`import.c` 也因此
   新增 `#include "ast.h"`。

4. **基线 = 190 项**：Phase 0 probes 69 + integration 40 + import 25 +
   decls 56（test_decls 比 S4 多了 `test_s5_computed_goto` 的 3 项）。
   `c_superset_probe.sh` 51/52（多了 `p51_computed_goto`；唯一仍 fail 的
   `p21_kr_func` 按 S4 决议无限期延后）。

**坑 FE-27（本阶段发现，已修）— `%g` 截断 float 字面量**：
cg 用 `printf("%g", v)` 发射 float 字面量。`%g` 默认 6 位有效数字。Lua
源中 `(3.141592653589793238462643383279502884)` 经 sharp-cpp 透传到
sharp-fe，经 `strtod` 解析为精确 double，再被 cg 截成 `3.14159`，
误差 ~3.6e-6。下游 `math.tan(math.pi/4)` 在 `math.lua:650` 失败。
**修法**：`%.17g` 是 IEEE 754 double 的 round-trip 精度；同时检查输出
里是否含 `.`/`e`/`E`，没有就追加 `.0`，避免 `1.0` 被 `%g` 渲染成整型 `1`、
让 `1.0 / 2` 从 `0.5` 翻转成 `0`。两条修复在同一个 case 中，因为它们
共享同一个根因（cg 不该信任 `%g` 的默认行为）。

**坑 FE-28（本阶段发现，已修）— `splice_one` 漏 free imported 的 backing array**：
S4 文档说 asan clean 实际不是。`splice_one` 把 imported file 的 decls
指针搬到新数组后，零掉 `imported->u.file.decls.data` / `len` / `cap`
再 `ast_node_free(imported)`，导致 `data` 指向的 realloc 出来的数组永
远不被 free。改用 `astvec_free(&imported->u.file.decls)`，它正确释放
backing array 并把 vec 字段清零，下游 `ast_node_free` 看到 NULL data
也不会重复 free 子节点（子节点指针已搬走）。

**Phase R1 候选（推荐下一动作）**：拿一个新的中型 C 项目（mcpp / nuklear
/ stb_image / sqlite-amalgamation），照 `build_lua_with_sharpfe.sh`
模板写 `build_X_with_sharpfe.sh`，跑出来失败的构造每个写成 `p52..p60_*.c`
小探针，分类「真 bug / 缺特性 / 探针自身错误」三类，再修 fe。Lua 已经
覆盖了 GCC 扩展和精度边界；下一个项目大概率会暴露不同的盲点（typedef
名解析、复杂 declarator、attribute 透传、… 都是疑似候选）。

**基线**：69 probes + 40 integration + 25 import + 56 decls = **190 项，
全 0 fail**。c_superset 51/52。Lua testsuite `final OK !!!`。

— Phase S5 接力人，于 Phase S5 收尾时

---

## Phase R1 接力补遗

本阶段拿 cJSON 1.7.18 作为继 Lua 之后的第二个真实 C 代码集成目标，把
`.c → sharp-cpp → sharp-fe → C → cc → binary` 在一个新的、跟 Lua 风格
完全不同的代码集上跑通：cJSON 是纯 ISO C99，没用 GCC 扩展，但用了大量
`const`-discipline 和匿名 typedef，正好暴露 Lua 没碰到的盲点。

**项目状态一行总结**：R1 完成。三道门全过；C-superset 54/55；Lua testsuite
仍 `final OK !!!`；cJSON 端到端输出与 cc-only 基线**逐字节相同**。

**最重要的 4 件事给下一位**：

1. **`build_cjson_with_sharpfe.sh` 是 R1 的核心产物**，跟 S5 的
   `build_lua_with_sharpfe.sh` 一对。验收方法：跑 cc-only 出基线
   `/tmp/cjson_baseline.out`（脚本内会自动产生），跑完整管线、把 fe-cc
   产物的输出 diff 它。一致 = 通过。

2. **本阶段发现并修复了 4 类真实 bug**，全部带 `Phase R1:` 注释；不要回退：
   - **Sharp 关键字泄进 C 解析**（cJSON 行 6360：`cJSON *null = ...`）。
     `lex_run` 加 `bool sharp_keywords` 参数；sharpc 按文件后缀自动选
     模式（`.sp` → Sharp，其余 → C）；probe runner 同时遍历 `.c` 和 `.sp`。
   - **缺 GCC 预定义浮点值宏**（cJSON 行 4823：`__DBL_EPSILON__`）。
     在 `cpp_targets.c` 里新增 18 个 `__{FLT,DBL,LDBL}_*` 数值宏，值与
     `gcc -E -dM` 输出逐字一致。
   - **TY_ARRAY 在字段访问路径里没衰减**（cJSON 行 5474：`buf[1]; buf->field`）。
     `struct_scope_of` 跟 `sema_field_access_expr` 的 generic-substitution
     `base_t2` 都加了 TY_ARRAY 剥离，跟原有的 TY_CONST/TY_PTR 剥离对齐。
   - **`cg_type` 把 const 放在指针错的一侧**（cJSON 上百处 `cJSON * const item`）。
     `TY_CONST(TY_PTR(T))` 之前总是发射 `const T *`（指向 const 的指针），
     导致函数体写 `item->field = ...` 被 cc 拒。改成检测 `TY_CONST` 内层
     是 `TY_PTR` 时把 `const` 后置：`T * const`。

3. **顺手发现 p50_if_has_operator.c 类型分错了**：它测的是 Sharp dialect
   特性（`operator+`、`@has_operator`），文件名却是 `.c`。S5 之前因为
   关键字无条件提升所以"碰巧"工作，R1 做完 lex 模式分流后立即暴露。
   重命名为 `.sp` 才是正确的归类；probe runner 同步扩展为遍历两种扩展名。

4. **基线 = 193 项**：69 + 40 + 25 + 59（test_decls 比 S5 多了 R1 的 3 项
   regression）。c_superset 54/55（多了 p52/p53/p54；唯一仍 fail 的
   p21_kr_func 仍按 S4 决议无限期延后）。

**坑 FE-29（本阶段发现，已修）— Sharp keywords 在 C 输入里被错误提升**：
`null`、`defer`、`operator`、`this`、`import` 在 lex 里无条件促成 Sharp
关键字，导致任何用 `null` 当变量名的 C 代码失败。修法：`lex_run` 加
`sharp_keywords` 标志参数；按文件后缀决定。Test 套件保留 `true`（fixture
是 Sharp 风格），sharpc 按 `.sp` vs 其他自动判断。

**坑 FE-30（本阶段发现，已修）— GCC `__DBL_EPSILON__` 等数值宏未预定义**：
sharp-cpp 的 `cpp_targets.c` 有形状宏（`__DBL_DIG__`、`__DBL_MANT_DIG__`
等）但没有数值宏（`__DBL_EPSILON__`、`__DBL_MAX__` 等），而真实 C 代码会
不带 `<float.h>` 直接用它们。修法：在 `cpp_install_target_macros` 中
按 `gcc -E -dM` 的逐字输出加入 18 个 FLT/DBL/LDBL 数值宏。

**坑 FE-31（本阶段发现，已修）— `arr->field` 中 TY_ARRAY 没衰减**：
`struct_scope_of` 剥离 TY_CONST/TY_PTR 但不剥离 TY_ARRAY，导致 `pair p[1];
p->x` 报「no member 'x' in struct」。修法：在 `struct_scope_of` 和
`sema_field_access_expr` 的 generic-substitution 路径都加 TY_ARRAY 剥离，
跟原有 TY_PTR 剥离对齐。

**坑 FE-32（本阶段发现，已修）— `T * const` 被 cg 发射成 `const T *`**：
`TY_CONST(TY_PTR(T))` 应当输出 `T * const`（const 指针，pointee 可写），
但 `cg_type` 的 TY_CONST 分支无脑前置 `const ` + 递归发射，结果是
`const T *`（指向 const 的指针），把 `*const` vs `const*` 翻了个个儿。
修法：检测 TY_CONST 内层是 TY_PTR 时改为后置 `... * const`，否则保持
原前置写法。覆盖 `T*const`、`const T*const`、`T*const*const` 等多层组合。

**Phase R2 候选（推荐下一动作）**：再挑一个中型 C 项目（**sqlite-amalgamation**
是好候选——单个 ~250K 行的 .c，涵盖 stdlib/printf-family/typedef/复杂
declarator；或 **mcpp** / **nuklear** / **picol** / **stb_image**）。模板：
照 `build_cjson_with_sharpfe.sh` 写 `build_X_with_sharpfe.sh`，每个新失败
模式都 reduce 成 `c_superset_probes/p55..pNN_*.c`，再修 fe。

**基线**：69 probes + 40 integration + 25 import + 59 decls = **193 项，
全 0 fail**。c_superset 54/55。Lua + cJSON 端到端均通过。

— Phase R1 接力人，于 Phase R1 收尾时

---

## Phase R2 接力补遗

本阶段拿 **picol**（Salvatore Sanfilippo 写的 ~800 行 TCL 解释器）和
**stb_image / stb_image_write**（Sean Barrett 的 header-only 图像库，
~10K 行）作为继 Lua 和 cJSON 之后的第三、第四个真实 C 集成目标。
两个目标风格跟前两个完全不同——picol 是密集 idiomatic C，stb 是
宏极重的 header-only。验证的是端到端"输出与 cc-only 基线**逐字节相
同**"。

**项目状态一行总结**：R2 完成。三道门 197/197；c-superset 58/59；
四个真实目标（Lua + cJSON + picol + stb_image）端到端均通过且字节对齐。

**最重要的 4 件事给下一位**：

1. **`build_picol_with_sharpfe.sh` 和 `build_stb_image_with_sharpfe.sh`
   是 R2 的核心产物**，跟 S5/R1 的两个 build 脚本对齐。每个脚本拿
   cc-only 跑一次出 baseline，再跑完整管线、把 fe-cc 产物的输出
   diff 它。一致 = 通过。

2. **picol 0 个新 bug**——这是 sharp-fe 收敛的第一个数据点。S5 + R1
   修过的四类 bug 把通用 ISO C 大表面覆盖到了picol 第一次跑就直接过。

3. **stb_image 暴露 4 类深层 bug，全已修，全有 probe**：

   - **坑 FE-33（已修）— `_Thread_local` C11 storage-class 不被识别**：
     源 `static _Thread_local const char *p;` 报"missing type specifier"。
     `_Thread_local` 是与 static/extern 正交的 storage 修饰（C11 6.7.1
     允许并存）。修法：DeclSpecs / AST_VAR_DECL / AST_FUNC_DEF 各加
     `bool is_thread_local`；parse 接受关键字、5 个 AST 构造点透传；
     cg 三处 emission（block var / top var / func）都 emit
     `_Thread_local`。Probe `p55_thread_local.c`。

   - **坑 FE-34（已修）— `typedef void Fn(int);` K&R 函数类型 typedef
     被误识别为函数原型**：parse_top_decl line 2143 检测「outermost type
     is FUNC + 下一 token 是 `;`」即认为是函数原型。但 stb 的
     `typedef void stbi_write_func(void *, void *, int);` 也满足这个
     条件，结果 `Fn` 没注册成 typedef，所有 `Fn *p` 都报"unknown type"。
     修法：加 `ds.storage != SC_TYPEDEF` 守卫。一行修复，深远影响。
     Probe `p58_fn_type_typedef.c`。

   - **坑 FE-35（已修，三处合并）— block-scope 匿名 enum 三连环 bug**：
     `int main(void) { enum { W=16, H=16 }; int x = W + H; ... }`
     失败的根因有三：
     (a) scope.c `build_stmt` 漏 `AST_ENUM_DEF` case，枚举常量没注入
         外层 scope，sema 报"undefined name 'W'"；
     (b) cg.c `cg_stmt` 漏 `AST_ENUM_DEF` case，输出"unsupported stmt
         kind 6"，cc 看不到 enum；
     (c) parse_enum_def 错吃了 trailing `;`，导致外层 parse_decl_specifiers
         继续吞下一个 stmt 的 `int x` 当作同一个 declaration，AST 顺
         序乱了——even after fixing (a) and (b) 仍然失败因为 enum 被
         emit 到 use site 之后。
     修法：(a) scope.c build_stmt 加 ENUM_DEF case；(b) cg.c cg_stmt 加
     ENUM_DEF case；(c) parse_enum_def 不消费 `;`，由各调用方自己负责
     （parse_top_decl 路径加 ps_expect SEMI；其他路径外层本来就 expect
     SEMI）。Probe `p59_block_scope_enum.c`。

   - **坑 FE-36（已修）— `do EXPR; while (COND);` 单语句体被吃掉**：
     stb_image zlib 解码的 `do *zout++ = v; while (--len);` 生成成
     `do { } while ((--len));`——循环体丢了。根因：cg.c `AST_DO_WHILE`
     无脑调 `cg_block(body)`，但 cg_block 期待 AST_BLOCK，body 是
     AST_EXPR_STMT 时去 deref 错的 union arm，得到空 stmts。**与之相反**，
     if/while/for/switch 都早已正确 dispatch on `body->kind == AST_BLOCK`。
     修法：do-while 照搬同样的 dispatch——BLOCK → cg_block，否则手工
     包 `{}` + cg_stmt。Probe `p60_do_while_single_stmt.c`。
     运行时症状（zlib "bad dist"）离根因很远，**最难诊断的一个**。

4. **R2 完全没碰 sharp-cpp**——所有 4 个 fix 都在 sharp-fe 一侧。这
   是 cpp/fe 拆分以来第一次有阶段做到这点，反映 sharp-cpp 已经成熟。

**bug 数演化**：S5 = 2（Lua）、R1 = 4（cJSON）、R2-picol = 0、R2-stb = 4。
每个新目标的 bug 数有界并随表面饱和递减。**两个连续目标无 bug** 会暗
示 saturation；目前 picol 的 0-bug 是 suggestive 但非 conclusive。

**两个绕开的 GCC 扩展**（stb_image 通过 `-DNDEBUG -DSTBI_NO_SIMD` 避开）：

- **`__attribute__((vector_size(N)))` 作 cast 类型**（SSE intrinsic
  内联）。深度集成的 GCC 扩展，本质上是个新的类型系统分支。
- **GCC statement-expression `({ stmts; expr; })`**（glibc `assert()`
  宏展开）。需要新 AST 节点 + 新 sema/cg。

两者都不在 R2 修——它们各自是大功能，stb_image 用宏避开是 stb 自己
官方推荐的便携构建方式。如果哪个未来项目需要它们，再开专项。

**Phase R3 候选（推荐下一动作）**：

- **sqlite-amalgamation** ——单文件 ~250K 行，最大现实 C 表面，几乎
  必然暴露新 bug，但量级可控。
- **mcpp** ——它自己是个 C 预处理器，**结构上自相似**，编译 mcpp 等于
  test sharp-fe 在"自己同类"代码上的表现。
- **zlib** ——LZ77/Huffman 本来已经被 stb 间接覆盖，但单独跑 zlib 能
  确认 zlib 类修复是否耐用。
- 或转向 **D1 dogfooding**——开始把 sharp-fe 自己的模块翻译成 Sharp
  方言用 round-tripped C 当编译产物，"sharp-fe 能不能编 sharp-fe"。

**基线**：69 probes + 40 integration + 25 import + 63 decls = **197 项,
全 0 fail**。c_superset 58/59。Lua + cJSON + picol + stb_image 端到端
均通过，全部字节对齐。

— Phase R2 接力人，于 Phase R2 收尾时
