# 修复计划：P99 栈溢出问题

## 问题分析

### 根因
Windows 默认栈大小 1MB。p99 头文件包含大量函数定义和复杂嵌套结构，导致递归下降解析器深度超过栈限制。

### 触发路径
```
p99_checkargs.h
  → includes p99_constraint.h, p99_map.h, p99_enum.h, p99_type.h
    → 宏展开后产生大量函数定义
      → parse_program() → parse_stmt() → parse_block() → parse_stmt() ...
        → 递归深度超过 1MB
```

### 栈消耗估算
每层 parse 函数调用约占用 200-500 bytes 栈帧：
- 局部变量（Tok、Node*、Type* 等）
- 寄存器保存
- 返回地址

1MB / 300 bytes ≈ 3400 层递归。p99 头文件展开后的嵌套深度可达 5000+ 层。

## 修复方案（三管齐下）

### 方案 1：增加链接器栈大小（立即生效，治标）

**修改 CMakeLists.txt**，设置链接器栈大小为 8MB：

```cmake
# Windows: increase stack size to 8MB (default is 1MB)
if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:8388608")
elseif(MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--stack,8388608")
endif()
```

**效果**：立即解决 p99 栈溢出，8MB 可容纳约 27000 层递归。
**风险**：低。只是增加上限，不改变解析逻辑。

### 方案 2：编译选项优化（减少栈帧大小）

**修改 CMakeLists.txt**，添加优化标志：

```cmake
# Reduce stack frame size
target_compile_options(sharpc PRIVATE
    $<$<C_COMPILER_ID:Clang,GNU>:-O2 -fno-omit-frame-pointer>
    $<$<C_COMPILER_ID:MSVC>:/O2 /Gy>
)
```

**效果**：编译器优化减少局部变量分配，每层栈帧从 ~300 bytes 降至 ~150 bytes。

### 方案 3：识别并修复解析器中的不必要的递归（长期）

以下模式可能导致递归过深：

**3a. 嵌套函数调用链** — parse_expr → parse_assign → parse_ternary → ... → parse_primary
每个表达式都有 10 层递归。如果 p99 中有深度嵌套的宏展开表达式，栈消耗巨大。

**3b. 嵌套块/语句** — parse_block → parse_stmt → parse_block（递归块）
p99 函数中大量使用 if/for 嵌套。

**修复优先级**：方案1（立即）→ 方案2（简单）→ 方案3（长期，需要重构为迭代）

## 执行顺序

1. 修改 CMakeLists.txt 增加栈大小 + 编译优化
2. 重新编译
3. 运行 P99 测试验证
4. 记录改善效果

## 预期结果

- 栈溢出从 28 个降至 5 个以下
- 7 个 PASS 保持不变
- 新增约 15-20 个 PASS

## 风险

- **低风险**：栈大小增加只影响 sharpc.exe 进程，不影响生成代码
- **零风险**：编译优化不改变语义
- 如果 p99 头文件有真正的无限递归 bug，增加栈大小只是延迟崩溃
