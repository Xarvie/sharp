# Sharp 代码库全面死代码检查 — 10 阶段计划

> 代码库规模：35,736 行 / ~529 函数 / 18 个 .c 文件
> 基线：回归测试 442/442 全部通过

---

## 阶段 1：删除显式标记的 `__attribute__((unused))` 死函数 ✅ 已完成

**目标**：移除开发者已承认不再使用的函数。

| 文件 | 位置 | 函数 | 状态 |
|------|------|------|------|
| `sharp-fe/cg.c` | :380 | `ast_type_outer_is_volatile` | ✅ 已删除 |
| `sharp-fe/parse.c` | :406 | `ps_node` | ✅ 已删除 |
| `sharp-fe/parse.c` | :604 + :3250 | `parse_var_decl_list`（声明+定义） | ✅ 已删除 |
| `sharp-fe/parse.c` | :5899 | `is_user_file` | ✅ 已删除 |
| `sharp-fe/sema.c` | :1704 | `recv_is_const` | ✅ 已删除 |

---

## 阶段 2：清理垃圾文件 & 非代码文档 ✅ 已完成

**目标**：移除不属于代码库的文件。

| 文件 | 原因 | 状态 |
|------|------|------|
| `fred.txt` | 调试/临时垃圾文件 | ✅ 已删除 |
| `sharp-test/fred.txt` | 同上 | ✅ 已删除 |
| `DESIGN.md` | spkg 包管理器设计文档 — 与 sharpc 无关 | ✅ 已删除 |

---

## 阶段 3：头文件死声明审计（sharp-fe 前端）✅ 已完成

**已删除的声明**（`sharp.h`）：
- `sharp_ctx_compile_file`、`sharp_ctx_compile_buf`、`sharp_ctx_output`、`sharp_ctx_output_len`、`sharp_ctx_diags`、`sharp_ctx_diags_count`、`sharp_ctx_free`

**已删除的文件**：`sharp-fe/sharp.c`（7 个函数均为 0 调用者）

**已更新**：`build.py` 中 `SHARP_SOURCES` 移除 `sharp-fe/sharp.c`

**保留**：`ast_print`（内部自递归，作为调试工具保留）、`ast_clone_type`（有外部调用者）

---

## 阶段 4：头文件死声明审计（sharp-cpp 预处理器）✅ 已完成

**结论**：`cpp.h` / `cpp_internal.h` 中无死声明。`DA()` / `da_push` / `da_free` / `sb_*` 等宏和 inline 函数均有实际调用方。`cpp_*` 公共 API 函数即使当前不被 sharpc.c 直接调用，也是对外公开的库接口，予以保留。

---

## 阶段 5：未使用的 Storage Class 枚举值 ✅ 已完成

**已删除**：
- `SC_AUTO_C` 枚举值（`ast.h:86-93`）
- `cg.c` 中 `case SC_AUTO_C: break;` 的无效 switch case

**保留**：`SC_TYPEDEF` — 确认在 AST 构建中有语义用途，非死代码

---

## 阶段 6：struct 字段使用检查 ✅ 已完成

对 `ast.h` 中所有 AstNode union 字段进行全代码库交叉验证，确认每个字段均有至少 1 处写入和至少 1 处读取。未发现死字段。已知可疑字段（`func_def.name_paren` 等）均已确认有实际使用。

---

## 阶段 7：函数级死代码检测（cross-file call graph）🔴 进行中

**目标**：构建完整的跨文件函数调用图，找出定义了但从未被调用的函数。

### 已完成的删除

| 文件 | 函数 | 说明 |
|------|------|------|
| `sharp-fe/lex.c` | `lex_is_keyword` | 0 调用者 |
| `sharp-fe/lex.h` | `lex_is_keyword` 声明 | 已移除 |
| `sharp-fe/scope.c` | `scope_kind_name` | 0 调用者 |
| `sharp-fe/scope.h` | `scope_kind_name` 声明 | 已移除 |
| `sharp-cpp/macro.c` | `macro_lookup_is_func` | 0 调用者 |
| `sharp-cpp/macro.h` | `macro_lookup_is_func` 声明 | 已移除，注释也修正为 `macro_def_is_func` |
| `sharp-cpp/directive.c` | `macro_state_table` | 0 调用者，函数体已删除 |
| `sharp-cpp/pptok.c` | `reader_current_col` | 0 调用者，函数体已删除 |

### 待完成的删除

| 文件 | 行号 | 内容 | 操作 |
|------|------|------|------|
| `sharp-cpp/directive.h` | :45 | `MacroTable *macro_state_table(CppState *st);` | 删除声明 |
| `sharp-cpp/pptok.h` | :38 | `int reader_current_col(const CppReader *rd);` | 删除声明 |
| `sharp-cpp/cpp.c` | :557-561 | `macro_state_table` 的注释块 | 清理孤儿注释 |

### 保留的函数（确认为公共 API）
- `cpp_run`、`cpp_define_func`、`cpp_define_obj` 等 `cpp.h` 中的函数 — 对外公共 API，即使 sharpc.c 不走这条路径，也应保留
- `parse_int_literal_pub`（`directive.h:42`）— 仍在 `expr.c` 中有实质调用

---

## 阶段 8：重复代码 & 冗余模式检测 ⏳ 待执行

**目标**：找出可以合并或简化的重复代码。

**方法**：
1. 搜索相似的结构化代码段（相同结构的 switch case、相同模式的 if-else 链）
2. 搜索重复的字面量字符串（相同的错误消息、相同的格式字符串）
3. 检查 `cg.c` 中 type emission 是否有重复逻辑（此文件最大，8574 行）
4. 检查 `parse.c` 中的声明解析代码是否有重复

**已知可疑点**：
- `cg.c` 中 struct body emission 出现在 3 个位置（cg_struct、cg_emit_decl_sharp、Phase 3a）— 代码几乎相同
- `parse.c` 中多处 `ast_clone_type` + `parse_declarator` 模式

**注意**：此阶段聚焦于识别和记录重复模式，不轻易执行重构。重构风险较高，需逐个评估。

---

## 阶段 9：构建系统 & 测试基础设施审计 ⏳ 待执行

**目标**：检查构建脚本和测试中的死代码。

### 9a: `build.py`
- `build_probe_cpp()` — 输出 `probe_cpp` 二进制，确认是否有 CI/测试使用
- `build_probe_sharp()` — 输出 `probe_sharp`，确认使用方
- `sharp-cpp/probe.c` — 678 行独立程序，确认是否在测试流程中使用
- `sharp-cpp/sharp_cli.c` — 137 行 CLI 测试夹具，确认是否被任何测试引用
- `sharp-test/probe.c` — 确认用途和使用方
- `sharp-test/tokcmp.c` — 确认用途

### 9b: `CMakeLists.txt` vs `build.py`
- 2 个构建系统并存 → 确认哪个是主构建系统 → 考虑移除未使用的那个

### 9c: 测试目录
- `sharp-test/run_regression.py` — 确认是否依赖所有 probe 二进制
- `sharp-test/c_superset_probes/` — 扫描 `.ref.c` 文件和 `.sp` 文件是否一一对应（无孤立引用文件）
- `sharp-test/cpp_probes/` — 确认测试覆盖完整性

### 9d: `clibs/` 目录
- `clibs/wg/webgui.c` — 确认是否属于 sharpc 项目
- `clibs/todos.c` — 确认用途
- `clibs/counter.c` — 确认用途
- `clibs/smoke.c` — 确认用途
- `clibs/wg/cson.h` — 确认用途

---

## 阶段 10：最终回归验证 & 文档化 ⏳ 待执行

**目标**：确保所有变更零回归，输出死代码审计报告。

**操作**：
1. 运行完整回归：`python3 sharp-test/run_regression.py` → 确认 442/442
2. 编译 sharpc、probe_cpp、probe_sharp 全部通过
3. 运行 `-Wall -Wextra` 确认无新增 warning
4. 审查 diff 大小，确保变更最小化
5. git 提交（分阶段或合并提交）
6. （可选）在 `.trae/rules/project_rules.md` 中添加死代码预防规则：
   - 新增函数需在 commit message 中说明调用方
   - `__attribute__((unused))` 禁止合并
   - 删除功能时同步删除相关字段/枚举值

---

## 优先级总结

| 阶段 | 状态 | 风险 | 收益 |
|------|------|------|------|
| 1 | ✅ 完成 | 极低 | 清理 5 个已知死函数 |
| 2 | ✅ 完成 | 极低 | 清理 3 个垃圾文件 |
| 3 | ✅ 完成 | 低 | 删除 sharp.c 及 7 个 API 声明 |
| 4 | ✅ 完成 | 低 | 确认 cpp 头文件无死声明 |
| 5 | ✅ 完成 | 低 | 删除 SC_AUTO_C |
| 6 | ✅ 完成 | 中 | 确认 struct 字段无死字段 |
| 7 | 🔴 进行中 | 中 | 5 个死函数 + 3 个声明待删 |
| 8 | ⏳ 待执行 | 中 | 识别重复代码模式 |
| 9 | ⏳ 待执行 | 中 | 构建/测试系统清理 |
| 10 | ⏳ 待执行 | 低 | 验证 + 文档化 |