# Sharp 测试合并实施计划

> 制定日期: 2026-05-27
> 目标: 将 395 个测试文件精简至 180-220 个，同时保持测试覆盖率不降低

---

## 一、总体原则

1. **先建后删**: 在新目录创建合并后的测试，确认通过后再删除旧文件
2. **功能等价**: 合并后的测试必须覆盖原文件的所有测试场景
3. **注释溯源**: 每个合并文件顶部添加注释，说明来源文件
4. **渐进实施**: 按功能域分批实施，每批独立验证
5. **回归验证**: 每批完成后运行 `run_regression.py` 验证

---

## 二、目录结构设计

### 2.1 新目录树

```
sharp-test/
├── unit/                          # 单元测试 (目标: ~80 个文件)
│   ├── generics/                  # 泛型 (29 → 12)
│   │   ├── struct.sp
│   │   ├── struct.ref.c
│   │   ├── function.sp
│   │   ├── function.ref.c
│   │   ├── nested.sp
│   │   ├── nested.ref.c
│   │   ├── selfref.sp
│   │   ├── selfref.ref.c
│   │   ├── typedef.sp
│   │   ├── typedef.ref.c
│   │   ├── operator.sp
│   │   └── operator.ref.c
│   ├── defer/                     # defer (5 → 3)
│   │   ├── return.sp
│   │   ├── return.ref.c
│   │   ├── loop.sp
│   │   ├── loop.ref.c
│   │   ├── complex.sp
│   │   └── complex.ref.c
│   ├── operators/                 # 运算符重载 (13 → 6)
│   │   ├── basic.sp
│   │   ├── basic.ref.c
│   │   ├── index.sp
│   │   ├── index.ref.c
│   │   ├── unary.sp
│   │   ├── chain.sp
│   │   ├── chain.ref.c
│   │   ├── precedence.sp
│   │   ├── precedence.ref.c
│   │   ├── extension.sp
│   │   └── extension.ref.c
│   ├── auto/                      # auto 推导 (3 → 2)
│   │   ├── basic.sp
│   │   ├── basic.ref.c
│   │   └── complex.sp
│   ├── const/                     # const/限定符 (9 → 5)
│   │   ├── method.sp
│   │   ├── pointer.sp
│   │   ├── overload.sp
│   │   ├── overload.ref.c
│   │   ├── generic.sp
│   │   └── volatile.sp
│   ├── modules/                   # 模块导入 (11 → 5 + 8 个 .sph)
│   │   ├── basic.sp
│   │   ├── basic.sph
│   │   ├── cross_ref.sp
│   │   ├── cross_ref.sph
│   │   ├── stdlib.sp
│   │   ├── generic_inst.sp
│   │   └── generic_inst.sph
│   ├── introspection/             # 编译期内省 (7 → 4)
│   │   ├── has_operator.sp
│   │   ├── has_method.sp
│   │   ├── static_assert.sp
│   │   └── v013_intrinsics.sp
│   ├── extensions/                # 扩展方法 (9 → 5)
│   │   ├── basic.sp
│   │   ├── basic.ref.c
│   │   ├── method_call.sp
│   │   ├── dispatch.sp
│   │   ├── dispatch.ref.c
│   │   └── composition.sp
│   ├── cinterop/                  # C 互操作 (4 → 3)
│   │   ├── to_cstr.sp
│   │   ├── std_include.sp
│   │   └── std_include.ref.c
│   └── controlflow/               # 控制流 (6 → 4)
│       ├── switch.sp
│       ├── loops.sp
│       ├── goto.sp
│       └── string_literal.sp
├── integration/                   # 集成测试 (目标: ~30 个文件)
│   ├── codegen/                   # 代码生成 (8 → 5)
│   │   ├── typedef_chain.sp
│   │   ├── enum.sp
│   │   ├── struct_array.sp
│   │   ├── struct_array.ref.c
│   │   ├── sizeof.sp
│   │   ├── sizeof.ref.c
│   │   ├── compound_literal.sp
│   │   └── compound_literal.ref.c
│   ├── runtime/                   # 运行时验证 (若干 → 5)
│   │   ├── generic_monomorph.sp
│   │   ├── generic_monomorph.ref.c
│   │   ├── multi_inst.sp
│   │   ├── multi_inst.ref.c
│   │   ├── struct_layout.sp
│   │   ├── extmethod_chain.sp
│   │   └── extmethod_chain.ref.c
│   ├── stdlib/                    # 标准库集成
│   │   ├── cjson_test.sp
│   │   └── raw_string_test.sp
│   └── static_var.sp              # 静态变量
├── regression/                    # 回归测试 (目标: ~15 个文件)
│   ├── bugs/                      # Bug 回归 (5 个保持不变)
│   │   ├── bug001_stddef_preamble.sp
│   │   ├── bug002_const_auto.sp
│   │   ├── bug003_ptr_return_method.sp
│   │   ├── bug004_typedef_return.sp
│   │   └── bug005_local_func_decl.sp
│   ├── typedef/                   # typedef 回归 (3 个 .c)
│   │   ├── alias_ptr.c
│   │   ├── alias_qual.c
│   │   └── alias_no_expand.c
│   └── parser/                    # 解析器回归
│       └── (从 bugs 中提取相关)
├── compatibility/                 # 兼容性测试 (目标: ~80 个文件)
│   ├── c_baseline/                # C 基础语法 (pc00001-pc00220 → ~50)
│   │   ├── types/                 # ~5
│   │   ├── operators/             # ~8
│   │   ├── controlflow/           # ~8
│   │   ├── functions/             # ~8
│   │   ├── pointers/              # ~8
│   │   ├── structs/               # ~8
│   │   ├── type_system/           # ~8
│   │   └── preprocessor/          # ~5
│   ├── c99/                       # C99 特性 (~10)
│   ├── gcc_extensions/            # GCC 扩展 (~15)
│   └── cpp_probes/                # 预处理器宏 (20 → 8)
│       ├── sequence.c
│       ├── dispatch.c
│       ├── variadic.c
│       ├── foo.c
│       ├── availability.c
│       ├── simple.c
│       ├── ifdef.c
│       └── test_runner.c
├── special/                       # 特殊测试 (2 个保持不变)
│   ├── p160_cross_compile_linux.c
│   └── p160b_pthread_cross.c
├── tools/                         # 测试工具
│   ├── run_regression.py          # 回归测试运行器
│   ├── probe.c                    # 探针测试运行器
│   └── tokcmp.c                   # Token 比较工具
├── demo/                          # 演示测试 (4 个 WebGUI)
│   ├── webgui_counter.sp
│   ├── webgui_todo.sp
│   ├── webgui_settings.sp
│   └── webgui_clock.sp
└── LEGACY/                        # 旧文件归档 (合并后移入)
    └── (所有被合并的旧文件)
```

### 2.2 文件数量预估

| 目录 | 当前数量 | 目标数量 | 缩减 |
|------|----------|----------|------|
| unit/ | 约 100 | ~80 | 20% |
| integration/ | 约 20 | ~30 | 新增分类 |
| regression/ | 8 | ~15 | 补充分类 |
| compatibility/ | 约 250 | ~80 | 68% |
| special/ | 2 | 2 | 不变 |
| demo/ | 4 | 4 | 不变 |
| tools/ | 3 | 3 | 不变 |
| **总计** | **~395** | **~180-220** | **45-55%** |

---

## 三、分阶段实施计划

### Phase 1: 泛型测试合并 (29 → 12)

**预估工作量**: 1-2 小时
**风险等级**: 低

#### 1.1 struct.sp — 合并 6 个文件

**来源**: p101, p103, p104, p105, p115, p107

**内容规划**:
```sharp
// 来源: p101_generic_struct.sp, p103_generic_nested.sp,
//       p104_generic_ptr.sp, p105_generic_method.sp,
//       p115_generic_struct_lit.sp, p107_generic_multitype.sp

// 1. 单参数泛型结构体 (原 p101)
struct Vec<T> { T* data; long len; };
// 测试: Vec<int> 实例化，字段访问

// 2. 双参数泛型结构体 (原 p103)
struct Pair<A, B> { A first; B second; };
// 测试: Pair<int, float> 实例化

// 3. 含指针字段的泛型 (原 p104)
struct Box<T> { T* value; };
// 测试: Box<int> 指针字段访问

// 4. 泛型方法 (原 p105)
struct Stack<T> {
    T* items;
    long size;
    void push(T item) { ... }
    T peek() const { ... }
}
// 测试: 方法调用

// 5. 泛型结构体字面量 (原 p115)
Pair<int, float> p = { .first = 1, .second = 2.5 };

// 6. 多类型参数 (原 p107)
struct Map<K, V> { K* keys; V* values; long count; };
```

#### 1.2 selfref.sp — 合并 2 个文件

**来源**: p106, p238_selfref

**内容规划**:
```sharp
// 来源: p106_generic_selfref.sp, p238_generic_selfref.sp

// 1. 链表节点 (原 p106)
struct Node<T> {
    T value;
    Node<T>* next;
}

// 2. 交叉引用节点 (原 p238_crossref)
struct LinkNode<T> {
    T data;
    void link(LinkNode<T>* other) { ... }
}
```

#### 1.3 function.sp — 合并 4 个文件

**来源**: p102, p117, p118, p119

#### 1.4 nested.sp — 合并 3 个文件

**来源**: p233, p153, p152

#### 1.5 typedef.sp — 合并 3 个文件

**来源**: p114, p224, p149

#### 1.6 operator.sp — 合并 2 个文件

**来源**: p236, p154(泛型部分)

### Phase 2: defer 测试合并 (5 → 3)

**预估工作量**: 30-45 分钟
**风险等级**: 低

#### 2.1 return.sp — 合并 3 个文件

**来源**: p108, p257, p266(return 部分)

#### 2.2 loop.sp — 合并 2 个文件

**来源**: p109, p266(loop 部分)

#### 2.3 complex.sp — 保留 p155

**来源**: p155(保留独立，场景复杂)

### Phase 3: 运算符重载合并 (13 → 6)

**预估工作量**: 1-2 小时
**风险等级**: 中

#### 3.1 basic.sp — 合并 2 个文件

**来源**: p110, p154(基础部分)

#### 3.2 index.sp — 保留 p112

**来源**: p112(独立功能)

#### 3.3 unary.sp — 合并 2 个文件

**来源**: p113, p249

#### 3.4 chain.sp — 合并 2 个文件

**来源**: p239, p246

#### 3.5 precedence.sp — 合并 2 个文件

**来源**: p247, p248

#### 3.6 extension.sp — 合并 2 个文件

**来源**: p225, p267

### Phase 4: auto/const/modules 合并 (22 → 12)

**预估工作量**: 1-2 小时
**风险等级**: 低

#### 4.1 auto: 3 → 2

- basic.sp: p111 + p157(基础)
- complex.sp: p157(边界) + p231

#### 4.2 const: 9 → 5

- method.sp: p141 + p147 + p148
- pointer.sp: p226
- overload.sp: p156 + p244
- generic.sp: p142 + p146
- volatile.sp: p271 + p278

#### 4.3 modules: 11 → 5 + 8 .sph

- basic.sp + basic.sph: p158
- cross_ref.sp + cross_ref.sph: p159 + p261
- stdlib.sp: p161
- generic_inst.sp + generic_inst.sph: p163
- (p162, p164 合并入上述)

### Phase 5: introspection/extensions 合并 (16 → 9)

**预估工作量**: 1-2 小时
**风险等级**: 低

#### 5.1 introspection: 7 → 4

- has_operator.sp: p50
- has_method.sp: p250 + p251 + p252
- static_assert.sp: p253 + p220
- v013_intrinsics.sp: p219

#### 5.2 extensions: 9 → 5

- basic.sp: p223 + p228
- method_call.sp: p240
- dispatch.sp: p241 + p242 + p243
- composition.sp: p245
- runtime.sp: p264 + p265

### Phase 6: cinterop/controlflow 合并 (10 → 7)

**预估工作量**: 30-45 分钟
**风险等级**: 低

#### 6.1 cinterop: 4 → 3

- to_cstr.sp: p218
- std_include.sp: p221 + p227
- (pc00220 移至 compatibility)

#### 6.2 controlflow: 6 → 4

- switch.sp: p254
- loops.sp: p255 + p256 + p258
- goto.sp: p259
- string_literal.sp: p274

### Phase 7: integration 重组 (8 → 10)

**预估工作量**: 1 小时
**风险等级**: 低

#### 7.1 codegen: 8 → 5

- typedef_chain.sp: p269
- enum.sp: p272
- struct_array.sp: p273
- sizeof.sp: p275
- compound_literal.sp: p276

#### 7.2 runtime: 新增分类

- generic_monomorph.sp: p268
- multi_inst.sp: p270
- struct_layout.sp: p264
- extmethod_chain.sp: p265

### Phase 8: compatibility 大幅精简 (250 → 80)

**预估工作量**: 4-6 小时
**风险等级**: 高

这是最大的工作量，需要仔细分析每个 pc 文件。

#### 8.1 c_baseline: pc00001-pc00220 → ~50

按功能分组，每组分析并保留代表性文件:

**types/ (5 个)**:
```
从 pc00001-pc00020 中选择:
- pc00001.c (int 基础)
- pc00006.c (字面量)
- pc00011.c (字符串)
- pc00016.c (数组初始化)
- pc00020.c (浮点)
```

**operators/ (8 个)**:
```
从 pc00021-pc00050 中选择:
- pc00021.c (算术)
- pc00025.c (比较)
- pc00031.c (逻辑)
- pc00041.c (位运算)
- pc00044.c (复合赋值)
- pc00046.c (三元)
- pc00048.c (sizeof)
- pc00050.c (移位)
```

**controlflow/ (8 个)**:
```
从 pc00051-pc00080 中选择:
- pc00051.c (if/else)
- pc00055.c (switch)
- pc00061.c (for)
- pc00065.c (while)
- pc00071.c (do-while)
- pc00075.c (break/continue)
- pc00077.c (return)
- pc00080.c (goto)
```

**functions/ (8 个)**:
```
从 pc00081-pc00110 中选择:
- pc00081.c (声明)
- pc00085.c (调用)
- pc00091.c (参数传递)
- pc00095.c (返回值)
- pc00101.c (递归)
- pc00105.c (变参)
- pc00108.c (函数指针)
- pc00110.c (inline)
```

**pointers/ (8 个)**:
```
从 pc00111-pc00140 中选择:
- pc00111.c (指针算术)
- pc00115.c (指针数组)
- pc00121.c (多维数组)
- pc00125.c (字符串指针)
- pc00131.c (指针与数组)
- pc00135.c (函数指针数组)
- pc00138.c (void 指针)
- pc00140.c (const 指针)
```

**structs/ (8 个)**:
```
从 pc00141-pc00170 中选择:
- pc00141.c (结构体定义)
- pc00145.c (结构体初始化)
- pc00151.c (嵌套结构体)
- pc00155.c (位域)
- pc00161.c (联合)
- pc00165.c (枚举)
- pc00168.c (匿名结构体)
- pc00170.c (结构体指针)
```

**type_system/ (8 个)**:
```
从 pc00171-pc00200 中选择:
- pc00171.c (typedef)
- pc00175.c (const)
- pc00181.c (volatile)
- pc00185.c (sizeof)
- pc00191.c (类型转换)
- pc00195.c (typeof)
- pc00198.c (alignof)
- pc00200.c (auto)
```

**preprocessor/ (5 个)**:
```
从 pc00201-pc00220 中选择:
- pc00201.c (#include)
- pc00205.c (#define)
- pc00211.c (#if/#ifdef)
- pc00215.c (#error)
- pc00220.c (混合)
```

#### 8.2 c99/: 新增 ~10 个

从 p05-p49, p50-p99 中选择 C99 特性:
- p29_init_list.c (指定初始化器)
- p39_compound_lit.c (复合字面量)
- p71_stmt_expr.c (语句表达式)
- p72_atomic.c (原子类型)
- p85_int128.c (__int128)
- p87_typeof_type.c (typeof)
- p88_auto_type.c (C11 auto)
- p92_vla_restrict.c (VLA)
- p98_typeof_expr.c (typeof 表达式)
- p100_offsetof_unexpanded.c (offsetof)

#### 8.3 gcc_extensions/: 新增 ~15 个

从 p05-p132 中选择 GCC 扩展:
- p43_attribute.c (属性)
- p45_gcc_aliases.c (函数别名)
- p51_computed_goto.c (计算 goto)
- p63_attribute_stmt.c (语句属性)
- p67_tentative_def.c (试探性定义)
- p70_builtin_va_arg.c (内置 va_arg)
- p73_attribute_passthrough.c (属性透传)
- p76_asm_stmt.c (内联汇编)
- p78_anon_union.c (匿名联合)
- p82_nested_fnptr_decl.c (嵌套函数指针)
- p89_multi_decl_bitfield.c (多声明位域)
- p94_transparent_union.c (透明联合)
- p122_alignas.c (alignas)
- p123_typeof_stmt_expr.c (typeof+语句表达式)
- p132_struct_leading_attribute.c (前导属性)

#### 8.4 cpp_probes: 20 → 8

- sequence.c: probe_seq_1/2/3
- dispatch.c: probe_dispatch_*
- variadic.c: probe_va_*
- foo.c: probe_foo_*
- availability.c: probe_availability*
- simple.c: probe_simple*, probe_two_simple, probe_call_first
- ifdef.c: probe_ifdef_1
- test_runner.c: 保留

### Phase 9: 更新测试基础设施

**预估工作量**: 1-2 小时
**风险等级**: 中

#### 9.1 更新 run_regression.py

```python
# 修改目录常量
PROBES_DIR_NAME = "unit"           # 原 c_superset_probes
SPECIAL_DIR_NAME = "special"       # 保持不变

# 新增目录支持
INTEGRATION_DIR = "integration"
REGRESSION_DIR = "regression"
COMPATIBILITY_DIR = "compatibility"

# 更新测试发现逻辑
def discover_tests(base_dir):
    tests = {}
    for category in ['unit', 'integration', 'regression', 'compatibility']:
        tests[category] = find_tests_in_dir(base_dir / category)
    return tests
```

#### 9.2 更新文档

- 更新 TEST_CLASSIFICATION.md
- 更新 README (如有)

### Phase 10: 验证与归档

**预估工作量**: 1-2 小时
**风险等级**: 低

#### 10.1 回归测试

```bash
# 运行所有测试
python3 run_regression.py -v

# JSON 输出用于比对
python3 run_regression.py --json > results_new.json
```

#### 10.2 旧文件归档

```bash
# 创建归档目录
mkdir LEGACY

# 移动所有被合并的文件
mv c_superset_probes/p101*.sp LEGACY/
mv c_superset_probes/p102*.sp LEGACY/
# ... (按合并计划移动)

# 确认测试通过后，可以选择删除
# rm -rf LEGACY
```

---

## 四、风险与缓解

| 风险 | 概率 | 影响 | 缓解措施 |
|------|------|------|----------|
| 合并后测试覆盖率下降 | 中 | 高 | 每步运行回归测试验证 |
| pc 系列误删重要用例 | 中 | 高 | 仔细审查每个文件，保留边界情况 |
| run_regression.py 适配失败 | 低 | 中 | 保留旧目录作为备份 |
| 合并文件过大难以维护 | 低 | 中 | 每个文件不超过 200 行 |
| .ref.c 文件同步问题 | 中 | 中 | 合并后重新生成 .ref.c |

---

## 五、时间估算

| Phase | 内容 | 预估时间 |
|-------|------|----------|
| Phase 1 | 泛型合并 | 1-2h |
| Phase 2 | defer 合并 | 0.5-1h |
| Phase 3 | 运算符合并 | 1-2h |
| Phase 4 | auto/const/modules | 1-2h |
| Phase 5 | introspection/extensions | 1-2h |
| Phase 6 | cinterop/controlflow | 0.5-1h |
| Phase 7 | integration 重组 | 1h |
| Phase 8 | compatibility 精简 | 4-6h |
| Phase 9 | 基础设施更新 | 1-2h |
| Phase 10 | 验证与归档 | 1-2h |
| **总计** | | **12-21 小时** |

---

## 六、执行检查清单

### Phase 1 检查项
- [ ] 创建 unit/generics/ 目录
- [ ] 编写 struct.sp 并生成 struct.ref.c
- [ ] 编写 selfref.sp 并生成 selfref.ref.c
- [ ] 编写 function.sp 并生成 function.ref.c
- [ ] 编写 nested.sp 并生成 nested.ref.c
- [ ] 编写 typedef.sp 并生成 typedef.ref.c
- [ ] 编写 operator.sp 并生成 operator.ref.c
- [ ] 运行 `run_regression.py` 验证泛型测试
- [ ] 确认通过率 100%

### Phase 2 检查项
- [ ] 创建 unit/defer/ 目录
- [ ] 编写 return.sp 并生成 return.ref.c
- [ ] 编写 loop.sp 并生成 loop.ref.c
- [ ] 复制 complex.sp (p155)
- [ ] 运行回归测试验证
- [ ] 确认通过率 100%

### Phase 3-10 检查项
- [ ] (同上，每个 Phase 完成后验证)

### 最终检查项
- [ ] 所有 Phase 完成
- [ ] 总体回归测试通过率 100%
- [ ] 旧文件归档到 LEGACY/
- [ ] 更新 TEST_CLASSIFICATION.md
- [ ] 更新 run_regression.py
- [ ] 代码审查 (如有需要)

---

## 七、回滚方案

如果合并过程中出现问题:

```bash
# 1. 恢复旧目录结构
git checkout HEAD -- c_superset_probes/
git checkout HEAD -- bugs/
git checkout HEAD -- cpp_probes/
git checkout HEAD -- regress/

# 2. 删除新目录
rm -rf unit/ integration/ regression/ compatibility/ demo/ tools/

# 3. 恢复 run_regression.py
git checkout HEAD -- run_regression.py
```

---

*本计划为 Sharp 测试合并的详细实施指南，建议按 Phase 顺序逐步执行。*
