# Sharp 单元测试完整分类文档

> 生成日期: 2026-05-27
> 用途: 为未来测试合并、去重、优化提供完整细致的参考

---

## 总览统计

| 分类目录 | .ce | .c (非 .ref.i) | .ref.i | .he | 总计 |
|----------|-----|---------------|--------|------|------|
| c_superset_probes/ | 117 | 143 | 78 | 16 | 354 |
| bugs/ | 5 | 0 | 0 | 0 | 5 |
| 根目录独立文件 | 6 | 2 | 0 | 0 | 8 |
| cpp_probes/ | 0 | 20 | 0 | 0 | 20 |
| regress/ | 0 | 3 | 0 | 0 | 3 |
| special/ | 0 | 2 | 0 | 0 | 2 |
| probe.c (根) | 0 | 1 | 0 | 0 | 1 |
| tokcmp.c (根) | 0 | 1 | 0 | 0 | 1 |
| run_regression.py | 0 | 0 | 0 | 0 | 1 |
| **总计** | **128** | **172** | **78** | **16** | **395** |

---

## 一、c_superset_probes/ — 核心功能探针测试

### 1.1 泛型 (Generics) — 29 个文件

**核心泛型结构体** (12 个):

| 文件 | 功能点 | .ref.i | 描述 |
|------|--------|--------|------|
| p101_generic_struct.ce | 单参数泛型结构体 | ✅ | `Vec<T>` 基础特化与字段访问 |
| p103_generic_nested.ce | 双参数泛型结构体 | ✅ | `Pair<A, B>` 双类型参数 |
| p104_generic_ptr.ce | 泛型含指针字段 | ❌ | `Box<T>` 含 T* 指针字段 |
| p105_generic_method.ce | 泛型结构体方法 | ❌ | `Stack<T>` push/peek 方法 |
| p106_generic_selfref.ce | 自引用泛型 | ❌ | 链表 `Node<T>` 含 `Node<T>* next` |
| p107_generic_multitype.ce | 多类型参数结构体 | ❌ | `Map<K, V>` 键值对 |
| p114_generic_assoc.ce | 泛型关联函数 | ✅ | `Stack<T>.new()` 工厂模式 |
| p115_generic_struct_lit.ce | 泛型结构体字面量 | ❌ | `Pair<A, B>{ .field = val }` 语法 |
| p233_generic_nested3.ce | 三层嵌套泛型 | ✅ | `Vec<Vec<Vec<int>>>` |
| p234_generic_const_mix.ce | 泛型+const混合 | ✅ | `Pair<T,U>` const访问器 |
| p235_generic_zeroinit.ce | 泛型零初始化 | ❌ | `Maybe<T> = {0}` |
| p237_generic_embed.ce | 泛型嵌入裸结构体 | ✅ | `Labeled<T>` embed `Point` |

**泛型函数** (5 个):

| 文件 | 功能点 | .ref.i | 描述 |
|------|--------|--------|------|
| p102_generic_func.ce | 泛型函数 | ✅ | `max<T>` 函数 |
| p117_gfunc_swap.ce | 泛型交换函数 | ❌ | `swap<T>(T*, T*)` |
| p118_gfunc_infer.ce | 泛型函数类型推断 | ❌ | `max<T>` 推断类型 |
| p119_gfunc_multiparam.ce | 多参数泛型函数 | ❌ | `pass<T, U>(T, U)` |
| p120_gfunc_nested_gtgt.ce | 嵌套泛型类型签名 | ❌ | `Vec<Vec<int>>` 函数签名 |

**泛型边界与复杂场景** (5 个):

| 文件 | 功能点 | .ref.i | 描述 |
|------|--------|--------|------|
| p151_vec_template.ce | Vec泛型模板 | ❌ | `Vec<T>` size/push/at |
| p152_vec_stdlib.ce | 完整Vec库 | ❌ | `Vec<T>` 12个方法，operator[]，`<T> T` 返回语法 |
| p153_generic_edge.ce | 泛型边界情况 | ❌ | 多参数、深层嵌套、独立泛型 |
| p224_generic_typedef.ce | 泛型类型别名 | ❌ | `typedef Vec<int> IntVec`，链式别名 |
| p236_generic_operator.ce | 泛型运算符重载 | ✅ | `Vec2<T>` equals/add/sum |

**泛型跨引用与实例化** (4 个):

| 文件 | 功能点 | .ref.i | 描述 |
|------|--------|--------|------|
| p238_generic_selfref.ce | 自引用泛型(重复) | ✅ | `Node<T>` 含 `Node* next` |
| p238_generic_crossref.ce | 交叉引用泛型 | ✅ | `Node<T>` 含 `link()` 方法 |
| p268_runtime_generic.ce | 运行时泛型单态化 | ✅ | `Pair<T>` swap，int/float测试 |
| p270_codegen_multi_inst.ce | 同文件多实例化 | ✅ | `Wrap<int>`、`Wrap<float>` 共存 |

**泛型+const** (2 个):

| 文件 | 功能点 | .ref.i | 描述 |
|------|--------|--------|------|
| p142_generic_const_method.ce | 泛型+const方法 | ❌ | `Vec<T>` const size()/at() |
| p149_generic_const_access.ce | 泛型const字段访问修复 | ❌ | `Arr<T>.size()` const方法字段访问 |

**泛型extern** (1 个):

| 文件 | 功能点 | .ref.i | 描述 |
|------|--------|--------|------|
| p150_extern_func_inst.ce | extern泛型实例化 | ❌ | `extern swap<int>` 阻止代码生成 |

### 1.2 defer — 5 个文件

| 文件 | 功能点 | .ref.i | 描述 |
|------|--------|--------|------|
| p108_defer_return.ce | defer返回捕获、LIFO、void | ✅ | 返回值捕获、LIFO顺序、void defer |
| p109_defer_loop.ce | 循环中defer(break/continue) | ✅ | break/continue 每迭代触发defer |
| p155_defer_complex.ce | 复杂defer场景 | ❌ | 单次、LIFO、if、循环、嵌套作用域、返回、结构体 |
| p257_multi_return_defer.ce | 多返回路径defer | ✅ | 每个返回路径触发defer |
| p266_runtime_defer.ce | 运行时defer执行验证 | ✅ | 返回、LIFO、循环defer验证 |

### 1.3 运算符重载 (Operator Overloading) — 13 个文件

**自由函数运算符** (3 个):

| 文件 | 功能点 | .ref.i | 描述 |
|------|--------|--------|------|
| p110_free_operator.ce | 自由函数运算符 | ❌ | struct Vec2 operator+/-/= 自由函数 |
| p112_operator_index.ce | operator[]重载 | ✅ | Arr.operator[] 下标访问 |
| p113_unary_operator.ce | 一元运算符重载 | ❌ | Vec2.operator- (取反) 方法+自由函数 |

**全面运算符测试** (2 个):

| 文件 | 功能点 | .ref.i | 描述 |
|------|--------|--------|------|
| p154_operator_full.ce | 12种运算符完整组合 | ✅ | +, -, *, /, %, ==, !=, <, >, <=, >=, [] |
| p225_operator_extension.ce | 运算符作为扩展方法 | ❌ | 前向引用：先使用后定义 |

**方法链与运算符优先级** (5 个):

| 文件 | 功能点 | .ref.i | 描述 |
|------|--------|--------|------|
| p239_method_chain.ce | 方法链(返回*this) | ✅ | Counter*.inc()->inc()->add(5) |
| p246_operator_chain.ce | 运算符链式 a+b+c | ✅ | 链式二元运算符调用 |
| p247_operator_precedence.ce | 运算符优先级 | ❌ | a + b * c 尊重优先级 |
| p248_comparison_ops.ce | 比较运算符 | ❌ | ==, !=, <, >, <=, >= 扩展方法 |
| p249_unary_binary_mix.ce | 一元+二元混合 | ❌ | -a + b 表达式 |

**跨类型与运行时** (3 个):

| 文件 | 功能点 | .ref.i | 描述 |
|------|--------|--------|------|
| p222_mix_class_struct_op.ce | 跨类型运算符 | ❌ | class + struct 运算符 |
| p242_ext_vs_freefunc.ce | 扩展方法vs自由函数 | ✅ | 同名不同派发 |
| p267_runtime_operator.ce | 运行时运算符重载 | ✅ | Point operator+/- 运行时验证 |

### 1.4 auto 类型推导 — 3 个文件

| 文件 | 功能点 | .ref.i | 描述 |
|------|--------|--------|------|
| p111_auto_type.ce | 基础auto推导 | ✅ | `auto x = expr`，for循环中的auto |
| p157_auto_deduction.ce | auto边界情况 | ❌ | int, float, char, string, pointer, generic |
| p231_auto_complex.ce | auto复杂表达式 | ❌ | 算术、三元、函数返回 |

### 1.5 const 方法与限定符 — 9 个文件

| 文件 | 功能点 | .ref.i | 描述 |
|------|--------|--------|------|
| p141_const_method.ce | 基础const方法 | ❌ | `Vec.size() const` |
| p146_pointer_const_mangle.ce | const泛型命名修饰 | ❌ | `first<const int>` vs `first<float>` |
| p147_const_nonconst_mix.ce | const/non-const方法混合 | ❌ | Counter.inc() non-const, get() const |
| p148_const_method_params.ce | const方法含参数 | ❌ | BBox.area/move/at with const |
| p156_assoc_const.ce | 关联函数+const | ❌ | 工厂方法，const链式调用 |
| p226_const_pointer.ce | const指针嵌套 | ❌ | `const T*`, `T* const`, `const T* const` |
| p244_ext_const_overload.ce | const重载 | ✅ | Buffer.read() const vs write() non-const |
| p271_codegen_const_method.ce | const方法代码生成 | ❌ | const指针调用const方法 |
| p278_volatile_const.ce | volatile+const限定符 | ✅ | volatile int 读取，const int id |

### 1.6 模块与导入 (Modules & Imports) — 11 个文件

**模块基础** (4 个):

| 文件 | 功能点 | .ref.i | 描述 |
|------|--------|--------|------|
| p158_module_basic.ce | 基础模块交叉引用 | ❌ | `#include .he` 含函数、结构体、typedef、枚举 |
| p159_main.ce | 多头文件消费者 | ❌ | 多.he include (math, collections) |
| p162_main.ce | 多头条式传递包含 | ❌ | Types + strwrap headers |
| p260_module_class.ce | 模块含class+方法 | ❌ | Calculator class from module |

**模块泛型与交叉** (3 个):

| 文件 | 功能点 | .ref.i | 描述 |
|------|--------|--------|------|
| p261_module_cross.ce | 模块交叉引用 | ❌ | Point2D class with distance computation |
| p262_module_generic.ce | 模块泛型类型 | ❌ | RingBuf<int> from module |
| p163_main.ce | 泛型实例化头文件 | ❌ | Vec_int, Pair_int_double from module |

**stdlib集成** (2 个):

| 文件 | 功能点 | .ref.i | 描述 |
|------|--------|--------|------|
| p161_stdlib_integration.ce | stdlib集成 | ❌ | std.str, std.string, std.vec 集成 |
| p164_main.ce | 模块结构体方法 | ❌ | Counter class with methods |

**模块头文件 (.he)** (16 个):

| 文件 | 所属模块 | 描述 |
|------|----------|------|
| p158_module_basic.he | 模块基础 | 函数/结构体/typedef/enum 定义 |
| p159_core_math.he | 数学模块 | 数学函数 |
| p159_collections_vec.he | 集合模块 | Vec 集合 |
| p161_std_types.he | std类型 | 标准类型定义 |
| p161_std_str.he | std字符串 | 字符串处理 |
| p161_std_string.he | std string | String 库 |
| p161_std_vec.he | std Vec | Vec 容器 |
| p162_types.he | 类型定义 | 类型声明 |
| p162_strwrap.he | 字符串包装 | Str 包装器 |
| p163_genvec.he | 泛型Vec | Vec 泛型实例化 |
| p164_ops.he | 运算符 | 运算符定义 |
| p218_str.he | 字符串扩展 | Str 扩展方法 |
| p218_types.he | 类型 | 类型定义 |
| p260_mymath.he | 自定义数学 | 数学函数 |
| p261_shape.he | 形状 | 几何形状 |
| p262_ringbuf.he | 环形缓冲 | RingBuf 定义 |

### 1.7 编译期内省 (Compile-Time Introspection) — 7 个文件

| 文件 | 功能点 | .ref.i | 描述 |
|------|--------|--------|------|
| p50_if_has_operator.ce | @has_operator编译期分支 | ❌ | `if(@has_operator(T, op))` 死枝剪除 |
| p219_v013_intrinsics.ce | v0.13内省原语 | ❌ | @has_method, @has_field, @is_arithmetic, @is_pointer, @type_name |
| p220_hashmap.ce | HashMap+编译期检查 | ❌ | @static_assert + @has_method/operator 门控 |
| p250_has_method.ce | @has_method+扩展方法 | ❌ | 检测扩展方法 |
| p251_has_field.ce | @has_field+嵌套结构体 | ❌ | 字段检测 |
| p252_is_arithmetic.ce | @is_arithmetic+typedef | ❌ | typedef'd 类型识别 |
| p253_static_assert.ce | 复杂@static_assert | ❌ | &&, \|\|, ! 组合，死枝剪除 |

### 1.8 扩展方法 (Extension Methods) — 9 个文件

| 文件 | 功能点 | .ref.i | 描述 |
|------|--------|--------|------|
| p223_str_extensions.ce | std::Str扩展 | ❌ | count_byte, contains on Str |
| p228_array_struct.ce | 数组在结构体中+方法 | ✅ | Matrix 4x4 with extension methods |
| p229_func_ptr.ce | 函数指针字段 | ✅ | Button with callback function pointer |
| p240_ext_method_call.ce | 方法调用另一方法 | ❌ | Stats.record_many calls record |
| p241_ext_method_samename.ce | 同名不同类分发 | ❌ | A.get_val vs B.get_val dispatch |
| p243_ext_escape.ce | 返回this指针 | ✅ | Counter* escape pattern |
| p245_composition_inherit.ce | 组合继承 | ✅ | Dog embeds Animal, delegates methods |
| p264_runtime_struct.ce | 运行时结构体布局 | ❌ | Vec3D field access, copy |
| p265_runtime_extmethod.ce | 运行时扩展链 | ❌ | Accumulator reset/add/get |

### 1.9 C 互操作 (C Interop) — 4 个文件

| 文件 | 功能点 | .ref.i | 描述 |
|------|--------|--------|------|
| p218_to_cstr.ce | Str→C字符串转换 | ❌ | @to_cstr 替换，Str slice |
| p221_std_include.ce | 直接std库include | ✅ | types.he, vec.he, str.he, hashmap.he |
| p227_voidptr_erasure.ce | void*类型擦除 | ❌ | AnyBox with void* fields |
| pc00220_c_struct_bare.ce | C头文件结构体 | ❌ | Bare struct name from C header |

### 1.10 控制流 (Control Flow) — 6 个文件

| 文件 | 功能点 | .ref.i | 描述 |
|------|--------|--------|------|
| p254_switch.ce | switch语句边界 | ❌ | 穿透、break、default、case中return |
| p255_do_while.ce | do-while循环 | ❌ | 基础、break、continue |
| p256_nested_loop.ce | 嵌套循环 | ❌ | break内层、continue、while in for |
| p258_for_edge.ce | for循环边界 | ❌ | 空init/cond/incr、无限循环、逗号 |
| p259_goto.ce | goto与标签 | ❌ | 前后goto、多标签 |
| p274_string_literal.ce | 字符串字面量模式 | ✅ | str_len, str_eq, null check |

### 1.11 代码生成与运行时验证 (Codegen & Runtime) — 8 个文件

| 文件 | 功能点 | .ref.i | 描述 |
|------|--------|--------|------|
| p269_codegen_typedef.ce | typedef解析链 | ❌ | int→MyInt→YourInt→OurInt |
| p272_codegen_enum.ce | 枚举代码生成 | ❌ | enum 含命名常量 |
| p273_struct_array.ce | 结构体数组 | ✅ | Vec2[4] 初始化 |
| p275_sizeof.ce | sizeof复杂类型 | ✅ | struct, pointer, array, typedef |
| p276_compound_literal.ce | 复合字面量/临时结构体 | ✅ | 函数返回结构体，链式调用 |
| p277_static_var.ce | 静态变量 | ✅ | 静态局部、全局变量 |

### 1.12 C 兼容性探针 (C Compatibility Probes) — 100+ 个文件

#### 早期探针 (p05–p49) — 基础C语法

| 文件 | 功能类别 | 描述 |
|------|----------|------|
| p05_compound_assign.c | 运算符 | 复合赋值运算符 |
| p06_struct_basic.c | 结构体 | 基础结构体 |
| p07_typedef.c | 类型别名 | 基础typedef |
| p08_enum.c | 枚举 | 枚举类型 |
| p09_ptr_param.c | 指针 | 指针参数 |
| p10_array_param.c | 数组 | 数组参数 |
| p11_string_lit.c | 字符串 | 字符串字面量 |
| p12_ternary.c | 控制流 | 三元运算符 |
| p13_while.c | 控制流 | while循环 |
| p14_if.c | 控制流 | if语句 |
| p15_func_proto.c | 函数 | 函数原型 |
| p16_for.c | 控制流 | for循环 |
| p17_const_local.c | const | 局部const变量 |
| p18_sizeof_int.c | sizeof | sizeof(int) |
| p19_ptr_arith.c | 指针 | 指针算术 |
| p20_for_postfix.c | 控制流 | for后置递增 |
| p21_kr_func.c | 函数 | K&R风格函数 |
| p22_union.c | 联合 | union类型 |
| p23_bitfield.c | 位域 | 结构体位域 |
| p24_float_double.c | 浮点 | float/double类型 |
| p25_neg_lit.c | 字面量 | 负数字面量 |
| p26_shift_op.c | 运算符 | 移位运算符 |
| p27_addr_arr.c | 指针 | 取地址数组 |
| p28_func_ptr.c | 函数指针 | 函数指针 |
| p29_init_list.c | 初始化 | 初始化列表 |
| p30_struct_init.c | 结构体 | 结构体初始化 |
| p31_string_arr.c | 字符串 | 字符串数组 |
| p32_for_no_decl.c | 控制流 | for无声明 |
| p33_do_while.c | 控制流 | do-while |
| p34_struct_ptr.c | 指针 | 结构体指针 |
| p35_void_ret.c | 函数 | void返回 |
| p36_inline.c | 函数 | inline函数 |
| p37_volatile.c | 限定符 | volatile限定符 |
| p38_register.c | 存储类 | register存储 |
| p39_compound_lit.c | 复合字面量 | C99复合字面量 |
| p40_designated.c | 初始化 | 指定初始化器 |
| p41_paren_declarator.c | 声明 | 括号声明器 |
| p42_anon_struct.c | 结构体 | 匿名结构体 |
| p43_attribute.c | GCC扩展 | GCC属性 |
| p44_union_typedef.c | 联合 | union typedef |
| p45_gcc_aliases.c | GCC扩展 | GCC函数别名 |
| p47_nested_anon_and_fnptr.c | 复合类型 | 嵌套匿名+函数指针 |
| p48_extension_strconcat_typedefcast.c | 扩展 | 字符串拼接+typedef转换 |
| p49_libc_idioms.c | C惯用法 | 常见C模式 |

#### 中后期探针 (p50–p132) — C扩展特性

| 文件 | 功能类别 | 描述 |
|------|----------|------|
| p50_if_has_operator.ce | 编译期内省 | @has_operator编译期分支 |
| p51_computed_goto.c | 控制流 | 计算goto (GCC扩展) |
| p52_anon_typedef_arr.c | 类型别名 | 匿名typedef数组 |
| p53_const_ptr_param.c | const | const指针参数 |
| p54_c_null_ident.c | 标识符 | C null标识符 |
| p55_thread_local.c | 存储类 | 线程局部存储 |
| p58_fn_type_typedef.c | 函数指针 | 函数类型typedef |
| p59_block_scope_enum.c | 作用域 | 块作用域枚举 |
| p60_do_while_single_stmt.c | 控制流 | 单语句do-while |
| p61_typedef_fnptr_field.c | 函数指针 | typedef函数指针字段 |
| p62_anon_bitfield.c | 位域 | 匿名位域 |
| p63_attribute_stmt.c | GCC扩展 | 语句属性 |
| p64_postfix_const.c | const | 后缀const |
| p66_abstract_fnptr_cast.c | 类型转换 | 抽象函数指针转换 |
| p67_tentative_def.c | 声明 | 试探性定义 |
| p68_fn_returning_fnptr.c | 函数指针 | 返回函数指针的函数 |
| p69_fnptr_array.c | 函数指针 | 函数指针数组 |
| p70_builtin_va_arg.c | 变参 | __builtin_va_arg |
| p71_stmt_expr.c | GCC扩展 | 语句表达式 |
| p72_atomic.c | 原子操作 | 原子类型 |
| p73_attribute_passthrough.c | GCC扩展 | 属性透传 |
| p74_alignof.c | 对齐 | alignof运算符 |
| p75_sizeof_array_type.c | sizeof | sizeof数组类型 |
| p76_asm_stmt.c | 内联汇编 | asm语句 |
| p77_sizeof_noparen.c | sizeof | 无括号sizeof |
| p78_anon_union.c | 联合 | 匿名联合 |
| p79_typedef_opaque_struct.c | 结构体 | 不透明结构体typedef |
| p80_sizeof_array_typedef.c | typedef | sizeof数组typedef |
| p81_ptr_to_array_typedef_ret.c | 函数返回 | 指向数组的指针typedef返回 |
| p82_nested_fnptr_decl.c | 函数指针 | 嵌套函数指针声明 |
| p83_operator_ident_name.c | 标识符 | 运算符标识符名 |
| p84_vararg_fnptr_type.c | 变参 | 变参函数指针类型 |
| p85_int128.c | 类型 | __int128类型 |
| p86_atomic_qualifier.c | 原子 | 原子限定符 |
| p87_typeof_type.c | typeof | typeof类型 |
| p88_auto_type.c | auto | C11 auto类型 |
| p89_multi_decl_bitfield.c | 声明 | 多声明位域 |
| p90_struct_tag_vs_typedef.c | 类型 | struct tag vs typedef |
| p91_union_forward_typedef.c | 联合 | 联合前向typedef |
| p92_vla_restrict.c | VLA | VLA + restrict |
| p93_enum_forward.c | 枚举 | 前向枚举 |
| p94_transparent_union.c | 联合 | 透明联合 (GCC) |
| p95_paren_field_name.c | 结构体 | 括号字段名 |
| p95_static_assert_toplevel.c | static_assert | 顶层static_assert |
| p97_volatile_const_cast.c | 类型转换 | volatile+const转换 |
| p98_typeof_expr.c | typeof | typeof表达式 |
| p99_param_attribute.c | 参数 | 参数属性 |
| p100_offsetof_unexpanded.c | offsetof | offsetof不展开 |

#### 结构体/函数/类型探针 (p120–p164)

| 文件 | 功能类别 | 描述 |
|------|----------|------|
| p120_fnptr_returning_array.c | 函数返回 | 返回数组的函数指针 |
| p121_noreturn.c | 属性 | noreturn属性 |
| p122_alignas.c | 对齐 | alignas对齐 |
| p123_typeof_stmt_expr.c | typeof | typeof+语句表达式 |
| p124_array_of_fnptr_typedef.c | 函数指针 | 函数指针数组typedef |
| p125_register_param.c | 寄存器 | register参数 |
| p126_wchar_string.c | 宽字符 | wchar字符串 |
| p127_multidim_array_param.c | 数组 | 多维数组参数 |
| p128_fnptr_array_param.c | 函数指针 | 函数指针数组参数 |
| p129_wide_string_literal.c | 字符串 | 宽字符串字面量 |
| p130_compound_lit_address.c | 复合字面量 | 复合字面量地址 |
| p131_inline_storage.c | 存储类 | inline存储 |
| p132_struct_leading_attribute.c | 结构体 | 前导属性结构体 |
| p161_ios_basic.c | iOS | 基础iOS测试 |

#### pc 系列 — C 基准测试 (pc00001–pc00220) — 220 个文件

这些文件按编号覆盖 C 语言的基础语法和特性，作为 Sharp 作为 C 超集的基准测试。按功能分组如下:

**基础类型与字面量** (pc00001–pc00020):

| 范围 | 功能 | 示例文件 |
|------|------|----------|
| pc00001–pc00005 | int/char/float基本类型 | pc00001.c |
| pc00006–pc00010 | 整数/浮点字面量 | pc00006.c |
| pc00011–pc00015 | 字符串/字符字面量 | pc00011.c |
| pc00016–pc00020 | 数组初始化 | pc00016.c |

**运算符与表达式** (pc00021–pc00050):

| 范围 | 功能 | 示例文件 |
|------|------|----------|
| pc00021–pc00030 | 算术运算符 | pc00021.c |
| pc00031–pc00040 | 逻辑/比较运算符 | pc00031.c |
| pc00041–pc00050 | 位运算符 | pc00041.c |

**控制流** (pc00051–pc00080):

| 范围 | 功能 | 示例文件 |
|------|------|----------|
| pc00051–pc00060 | if/else/switch | pc00051.c |
| pc00061–pc00070 | for/while/do-while | pc00061.c |
| pc00071–pc00080 | break/continue/return | pc00071.c |

**函数** (pc00081–pc00110):

| 范围 | 功能 | 示例文件 |
|------|------|----------|
| pc00081–pc00090 | 函数声明/调用 | pc00081.c |
| pc00091–pc00100 | 参数传递/返回值 | pc00091.c |
| pc00101–pc00110 | 递归/变参 | pc00101.c |

**指针与数组** (pc00111–pc00140):

| 范围 | 功能 | 示例文件 |
|------|------|----------|
| pc00111–pc00120 | 指针算术 | pc00111.c |
| pc00121–pc00130 | 多维数组 | pc00121.c |
| pc00131–pc00140 | 指针与数组互操作 | pc00131.c |

**结构体/联合/枚举** (pc00141–pc00170):

| 范围 | 功能 | 示例文件 |
|------|------|----------|
| pc00141–pc00150 | 结构体定义/初始化 | pc00141.c |
| pc00151–pc00160 | 联合/枚举 | pc00151.c |
| pc00161–pc00170 | 嵌套结构体/位域 | pc00161.c |

**类型系统** (pc00171–pc00200):

| 范围 | 功能 | 示例文件 |
|------|------|----------|
| pc00171–pc00180 | typedef/const/volatile | pc00171.c |
| pc00181–pc00190 | sizeof/alignof/typeof | pc00181.c |
| pc00191–pc00200 | 类型转换 | pc00191.c |

**预处理器** (pc00201–pc00220):

| 范围 | 功能 | 示例文件 |
|------|------|----------|
| pc00201–pc00210 | #include/#define | pc00201.c |
| pc00211–pc00220 | #if/#ifdef/#error | pc00211.c |

**特殊测试** (pc00044, pc00048–pc00049, pc00060, pc00075, pc00082, pc00084, pc00095, pc00098, pc00100, pc00104, pc00106, pc00109, pc00112, pc00114, pc00116, pc00118, pc00123, pc00125, pc00127, pc00130, pc00138, pc00139, pc00143, pc00153, pc00154, pc00156, pc00160, pc00162–pc00166, pc00175, pc00181–pc00183, pc00186, pc00192–pc00194, pc00200, pc00203, pc00214, pc00219–pc00220):

这些是特定边界情况和 C 方言兼容性测试。

### 1.13 其他独立测试文件

| 文件 | 功能点 | .ref.i | 描述 |
|------|--------|--------|------|
| p_brace_init.c | 大括号初始化 | ❌ | C 风格大括号初始化语法 |
| p50_if_has_operator.ce | @has_operator条件编译 | ❌ | 编译期运算符检测分支 |

---

## 二、bugs/ — Bug 回归测试 — 5 个文件

| 文件 | Bug类型 | 描述 |
|------|---------|------|
| bug001_stddef_preamble.ce | 编译基础 | 最小程序返回42，测试基本编译流程 |
| bug002_const_auto.ce | const+auto交互 | `const auto k = 42` const auto推导 |
| bug003_ptr_return_method.ce | 解析器bug | 方法返回指针被误认为字段声明 |
| bug004_typedef_return.ce | 类型解析bug | 函数返回typedef结构体类型 |
| bug005_local_func_decl.ce | 作用域bug | main()内前向函数声明 |

---

## 三、根目录独立测试文件 — 8 个文件

### 3.1 功能测试 (.ce)

| 文件 | 功能类别 | 描述 |
|------|----------|------|
| raw_string_test.ce | 字符串语法 | r#"..."# 原始字符串，含 r"", r#"..."#, r##"..."## 多hash语法 |
| cjson_test.ce | 库集成测试 | cJSON 1.7.19 完整测试(30+测试): 解析、打印、创建、比较、复制、压缩等 |
| webgui_counter.ce | WebGUI演示 | 简单计数器应用，wg_app, wg_button, 事件处理 (端口9002) |
| webgui_todo.ce | WebGUI动态子元素 | 待办列表，表单提交，动态添加/删除，userdata (端口9001) |
| webgui_settings.ce | WebGUI设置面板 | 复选框、选择框、输入框、可见性切换、事件绑定 (端口9003) |
| webgui_clock.ce | WebGUI定时器 | 时钟显示，启停定时器 wg_set_timer (端口9004) |

### 3.2 工具文件 (.c)

| 文件 | 功能 | 描述 |
|------|------|------|
| probe.c | 探针测试运行器 | Phase 0 审计探针，验证 Sharp 前端词法/语法层特性映射 |
| tokcmp.c | Token 比较工具 | Sharp/C 往返验证: 比较 gcc -E 和 sharpc→gcc -E 的 token 序列 |

---

## 四、cpp_probes/ — C 预处理器宏测试 — 20 个文件

| 文件 | 功能类别 | 描述 |
|------|----------|------|
| probe_simple_after_target.c | 宏展开 | 目标定义后的 SIMPLE_BARE token |
| probe_dispatch_type.c | 宏分发 | GET_MACRO_16 选择 F0(对象) 或 F1(函数) |
| probe_dispatch_alone.c | 独立分发 | DISPATCH_CALL(X) 按参数计数选择宏 |
| probe_dispatch_reverse.c | 反向分发 | DISPATCH_CALL 和 DISPATCH_BARE |
| probe_call_first.c | 宏分发 | 基础 DISPATCH_CALL + FOO/BAR 展开 |
| probe_seq_1_funcname.c | 序列解析 | F→M1 解析链 |
| probe_dispatch_rename.c | 重命名分发 | BAREBARE vs DISPATCH_CALL |
| probe_availability_h.c | 系统头展开 | strdup + Availability.h |
| probe_va_func_macro_dispatch.c | 变参宏分发 | GET_MACRO_16 变参到 TARGET |
| probe_va_macro_dispatch.c | 变参宏分发 | 按参数计数分发: 0, 1, 2 参数 |
| probe_seq_2_objname.c | 对象名解析 | 序列对象名解析模式 |
| probe_seq_3_simple_first.c | 简单优先解析 | 序列简单优先解析模式 |
| probe_foo_first.c | FOO优先展开 | FOO 优先展开模式 |
| probe_foo_simple.c | 简单FOO展开 | 简单 FOO 展开 |
| probe_getmacro_dispatch.c | GET_MACRO分发 | GET_MACRO 宏分发 |
| probe_two_simple.c | 双宏展开 | 两个简单宏 |
| probe_availability2.c | 可用性检查2 | 可用性宏测试 |
| probe_ifdef_1.c | 条件编译 | #ifdef 测试 |
| probe_simple_bare_first.c | 裸宏优先 | 简单裸宏优先展开 |
| test_runner.c | 测试运行器 | 运行 cpp 探针测试 |

---

## 五、regress/ — 回归测试 — 3 个文件

| 文件 | 功能 | 描述 |
|------|------|------|
| typedef_alias_ptr.c | typedef指针别名 | 测试指针类型的 typedef 别名正确性 |
| typedef_alias_qual.c | typedef限定符别名 | 测试 const/volatile 限定符的 typedef 别名 |
| typedef_alias_no_expand.c | typedef别名不展开 | 测试 typedef 别名不过度展开 |

---

## 六、special/ — 特殊测试 — 2 个文件

| 文件 | 功能 | 描述 |
|------|------|------|
| p160_cross_compile_linux.c | 交叉编译 | Linux 交叉编译测试 (--target x86_64-linux-gnu) |
| p160b_pthread_cross.c | pthread交叉编译 | pthread 交叉编译测试 |

---

## 七、测试基础设施

| 文件 | 功能 | 描述 |
|------|------|------|
| run_regression.py | 回归测试运行器 | 统一回归测试: 并行执行所有探针测试，支持 -v, -j, --json |

---

## 八、功能覆盖热力图

| 功能领域 | 测试文件数 | 覆盖率评估 | 建议 |
|----------|-----------|-----------|------|
| **泛型** | 29 | ★★★★★ 极高 | 存在重复(如p106与p238自引用泛型)，可合并 |
| **defer** | 5 | ★★★★ 高 | 覆盖全面，无明显重复 |
| **运算符重载** | 13 | ★★★★★ 极高 | p110/p113与p154有重叠，p239/p246有重叠 |
| **auto推导** | 3 | ★★★★ 高 | p111/p157有重叠 |
| **const/限定符** | 9 | ★★★★ 高 | p141/p147/p156部分重叠 |
| **模块/导入** | 11 | ★★★★★ 极高 | 多个main.sp入口点可能可合并 |
| **编译期内省** | 7 | ★★★★ 高 | p50/p219/p220/@has相关可整合 |
| **扩展方法** | 9 | ★★★★ 高 | p240/p241/p265有相似性 |
| **C互操作** | 4 | ★★★ 中 | 覆盖基本，可增加更多场景 |
| **控制流** | 6 | ★★★ 中 | 基本覆盖，缺异常处理 |
| **代码生成** | 8 | ★★★★ 高 | p269/p272/p273/p275/p276/p277各有侧重 |
| **C兼容性** | 220+pc+130p | ★★★★★ 极高 | pc系列存在大量冗余，需仔细去重 |
| **WebGUI演示** | 4 | ★★ 低 | 演示性质，非严格测试 |
| **cJSON集成** | 1 | ★★★★★ 高 | 单文件30+测试，完整性高 |
| **C预处理器宏** | 20 | ★★★ 中 | 宏分发测试有模式重复 |
| **Bug回归** | 5 | ★★★ 中 | 随bug修复增长 |

---

## 九、合并去重建议

### 9.1 高优先级合并 (显著重复)

| 合并组 | 文件 | 建议 |
|--------|------|------|
| 自引用泛型 | p106_generic_selfref.ce, p238_generic_selfref.ce | 合并为一个，保留更全面的版本 |
| 运算符链式 | p239_method_chain.ce, p246_operator_chain.ce | 合并为方法+运算符链式测试 |
| auto推导基础 | p111_auto_type.ce, p157_auto_deduction.ce | 合并边界情况到单一文件 |
| const方法基础 | p141_const_method.ce, p147_const_nonconst_mix.ce | 合并const方法基础与混合 |
| 模块入口点 | p159_main.ce, p162_main.ce, p163_main.ce, p164_main.ce | 合并为单一模块消费者测试 |
| 泛型实例化 | p268_runtime_generic.ce, p270_codegen_multi_inst.ce | 合并泛型运行时与多实例化 |
| 扩展方法调用 | p240_ext_method_call.ce, p265_runtime_extmethod.ce | 合并扩展方法调用测试 |

### 9.2 中优先级合并 (部分重叠)

| 合并组 | 文件 | 建议 |
|--------|------|------|
| defer多场景 | p108, p109, p155, p257, p266 | 可整合为2-3个文件(返回/循环/复杂) |
| 运算符全组合 | p110, p112, p113, p154 | p154已覆盖全部，p110/p112/p113可作为子场景保留 |
| 编译期内省 | p50, p219, p250, p251, p252, p253 | 按@intrinsic类型分组整合 |
| WebGUI演示 | 4个webgui*.ce | 保留为演示，但标记为非回归测试 |

### 9.3 pc系列去重策略

pc00001–pc00220 系列是最大冗余来源。建议:

1. **按功能分组**: 将220个文件归类到15-20个功能组
2. **每组保留代表性文件**: 每组保留3-5个最具代表性的文件
3. **边界情况单独保留**: 特殊GCC扩展、C99特性等保留
4. **预期缩减**: 220个 → 60-80个

### 9.4 cpp_probes合并

cpp_probes中宏分发测试有固定模式。建议:

1. 合并probe_seq_1/2/3为一个序列测试
2. 合并probe_dispatch_*为一个分发测试文件
3. 合并probe_va_*为一个变参测试
4. 预期缩减: 20个 → 8-10个

---

## 十、测试执行方式

```bash
# 运行所有回归测试
python3 run_regression.py

# 指定sharpc路径
python3 run_regression.py --sharpc /path/to/sharpc

# 并行执行(默认 CPU核心数, 最多16)
python3 run_regression.py -j 8

# 详细输出
python3 run_regression.py -v

# JSON输出
python3 run_regression.py --json
```

---

## 十一、文件命名规范

| 前缀 | 含义 |
|------|------|
| pXXX_ | Sharp 特性探针 (p05–p278) |
| pcXXXXX | C 兼容性探针 (pc00001–pc00220) |
| bugXXX_ | Bug 回归测试 |
| pXXX_*.he | 模块头文件 |
| pXXX_*.ref.i | 预期生成的 C 代码参考 |

---

*本文档为 sharp-pkg 项目 sharp-test 目录的完整测试分类记录，可用于后续测试合并、去重和优化工作。*
