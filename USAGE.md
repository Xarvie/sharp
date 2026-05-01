# Sharp 编译器使用指南

## 1. 项目简介

Sharp 是一个 C 超集编译器，支持：
- C11 标准
- Rust 风格的 `impl` 语法
- C++ 风格的模板
- RAII 资源管理
- 内置 C 预处理器（支持完整宏展开）

## 2. 构建 Sharp 编译器

### 2.1 使用 CMake（推荐）

```powershell
# 创建并进入构建目录
mkdir build-debug
cd build-debug

# 配置项目
cmake .. -G "Visual Studio 17 2022" -A x64

# 构建
cmake --build . --config Debug
```

### 2.2 使用 Makefile（MinGW/GCC/Clang）

```bash
make
```

构建成功后，可执行文件位于 `build-debug/sharpc.exe`（Windows）或 `sharpc`（Linux/macOS）。

## 3. sharpc 命令行用法

### 3.1 基本用法

```bash
# 编译 .sp 文件，生成 .c 和 .exe
sharpc input.sp

# 指定输出文件
sharpc input.sp -o output.exe

# 仅生成 C 代码（不编译为可执行文件）
sharpc input.sp -o output.c
sharpc input.sp -no-link

# 预处理器输出到标准输出
sharpc input.sp -E

# 预处理器输出到文件
sharpc input.sp -E -o output.pp.c

# 生成 C 代码到标准输出
sharpc input.sp -

# 转储 HIR（高层中间表示）
sharpc input.sp -dump-hir
```

### 3.2 命令行参数

| 参数 | 说明 |
|------|------|
| `<file.sp>` | 输入源文件（`.sp` 或 `.c`） |
| `-o <file>` | 指定输出文件 |
| `-E` | 仅预处理，输出到 stdout 或 `-o` 指定文件 |
| `-` | 输出到标准输出 |
| `-dump-hir` | 打印 HIR 到 stderr |
| `-no-link` | 跳过编译为可执行文件 |
| `--target <triple>` | 指定目标三元组 |
| `-I <dir>` | 添加头文件搜索路径 |
| `-D<NAME>[=VALUE]` | 定义宏 |
| `-U<NAME>` | 取消定义宏 |

### 3.3 目标三元组（Target Triple）

格式：`arch-os-abi`

**支持的架构：**
- `x86_64` / `amd64` — x64
- `x86` / `i386` / `i686` — x86
- `aarch64` / `arm64` — ARM64
- `arm*` — ARM
- `wasm32` — WebAssembly
- `riscv64` — RISC-V 64-bit

**支持的操作系统：**
- `windows` / `win32`
- `linux`
- `macos` / `darwin`
- `none` / `freestanding` — 无操作系统（裸机）
- `wasi` — WebAssembly System Interface

**支持的 ABI：**
- `msvc` — Microsoft Visual C++
- `gnu` — GNU
- `gnux32` — GNU x32

**默认行为：** 如果不指定 `--target`，编译器会根据当前构建主机自动推断。

**示例：**
```bash
sharpc input.sp --target x86_64-windows-msvc
sharpc input.sp --target aarch64-linux-gnu
sharpc input.sp --target wasm32-wasi
sharpc input.sp --target x86_64-macos
sharpc input.sp --target riscv64-none
```

### 3.4 头文件与宏定义

```bash
# 添加头文件搜索路径
sharpc input.sp -I "C:\my\headers"

# 定义宏
sharpc input.sp -DDEBUG -DVERSION=1

# 取消定义宏
sharpc input.sp -U_WIN32
```

## 4. 编译器后端配置

Sharp 本身生成 C 代码，然后调用系统 C 编译器完成最终编译。

### 4.1 Windows 编译器搜索顺序

Sharp 在 Windows 上按以下顺序尝试使用 C 编译器：
1. `clang-cl` — LLVM/Clang MSVC 兼容模式（推荐）
2. `cl` — MSVC 传统编译器

如果都找不到，会报错提示安装 LLVM 或 MSVC Build Tools。

## 5. UCRT / VS / clang-cl 目录配置

### 5.1 目录结构说明

Sharp 的预处理器需要以下头文件路径才能正确处理 Windows C 代码：

```
Visual Studio 安装:
C:\Program Files\Microsoft Visual Studio\2022\<Edition>\VC\Tools\MSVC\<version>\include\

Windows SDK (UCRT):
C:\Program Files (x86)\Windows Kits\10\Include\<sdk_version>\ucrt\
C:\Program Files (x86)\Windows Kits\10\Include\<sdk_version>\shared\
C:\Program Files (x86)\Windows Kits\10\Include\<sdk_version>\um\
```

其中 `<Edition>` 可以是：
- `Community` — 免费社区版
- `Professional` — 专业版
- `Enterprise` — 企业版
- `BuildTools` — 独立构建工具

### 5.2 方式一：使用 vcvarsall.bat（推荐）

最简单的方式是在 Developer Command Prompt for VS 2022 中运行，或者手动调用 vcvarsall：

```powershell
# 初始化 VS 环境
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

# 之后使用 sharpc，它会自动从 %INCLUDE% 环境变量读取头文件路径
sharpc input.sp
```

`vcvarsall.bat` 会设置以下环境变量，Sharp 会自动识别：
- `INCLUDE` — 包含 VC、SDK、UCRT 的所有头文件路径
- `VCToolsInstallDir` — VC 工具安装目录

### 5.3 方式二：手动配置（无需 vcvarsall）

如果没有运行 vcvarsall，Sharp 会自动尝试发现头文件路径：

1. 首先检查 `VCToolsInstallDir` 环境变量
2. 如果没有，则枚举 VS 2022 已知安装路径：
   - `C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\`
   - `C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Tools\MSVC\`
   - `C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Tools\MSVC\`
   - `C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\`

3. 然后自动查找最新的 Windows SDK 版本，添加 `ucrt`、`shared`、`um` 子目录

### 5.4 clang-cl 配置

**安装 LLVM：**
从 https://llvm.org/ 下载并安装 LLVM for Windows。安装时勾选 "Add LLVM to the system PATH"。

**验证安装：**
```powershell
where clang-cl
# 应该输出类似：C:\Program Files\LLVM\bin\clang-cl.exe
```

**clang-cl 常用参数：**

```powershell
# MSVC 兼容模式编译
clang-cl --target=x86_64-pc-windows-msvc -O2 -o output.exe input.c

# 仅生成目标文件
clang-cl --target=x86_64-pc-windows-msvc -c -O2 input.c

# 使用 UCRT 头文件
clang-cl --target=x86_64-pc-windows-msvc ^
    /I "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.40.33807\include" ^
    /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\ucrt" ^
    /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\shared" ^
    /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\um" ^
    -O2 -c input.c
```

**常用 clang-cl 标志：**
| 标志 | 说明 |
|------|------|
| `--target=x86_64-pc-windows-msvc` | 指定 MSVC 兼容目标 |
| `-O2` | 优化级别 2 |
| `-Od` | 禁用优化（调试） |
| `-c` | 仅编译，不链接 |
| `/I <dir>` | 添加头文件路径（MSVC 风格） |
| `/Fe<file>` | 指定输出文件名 |
| `/Fo<file>` | 指定目标文件名 |
| `/Zi` | 生成调试信息 |
| `/MD` | 使用多线程 DLL CRT |
| `/MT` | 使用多线程静态 CRT |
| `/W3` | 警告级别 3 |
| `/nologo` | 隐藏版权信息 |

### 5.5 UCRT 运行时库配置

UCRT（Universal C Runtime）是 Windows 10+ 的标准 C 运行时库。

**链接时需要的库：**
- `ucrt.lib` — Universal C Runtime
- `vcruntime.lib` — Visual C++ Runtime
- `msvcrt.lib` — MSVCRT 导入库
- `oldnames.lib` — POSIX 兼容性名称
- `legacy_stdio_definitions.lib` — 遗留 stdio 函数

**链接器示例：**
```powershell
link /nologo /OUT:program.exe ^
    kernel32.lib advapi32.lib ^
    msvcrt.lib vcruntime.lib oldnames.lib ^
    program.obj
```

### 5.6 典型工作流

```powershell
# 1. 初始化 VS 环境
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

# 2. 用 sharpc 编译 Sharp 源文件
sharpc myapp.sp -o myapp.exe

# 或者仅生成 C 代码
sharpc myapp.sp -no-link -o myapp.c

# 3. 用 clang-cl 编译生成的 C 代码
clang-cl --target=x86_64-pc-windows-msvc -O2 /Fe:myapp.exe myapp.c
```

## 6. 测试

```bash
# 运行正例测试
cmake --build . --target test_positive

# 运行 CMake 测试
ctest --output-on-failure
```

测试文件位于 `tests/` 目录。

## 7. 常见问题

### Q: 找不到头文件 `<stdio.h>` 等
确保已安装 Visual Studio 并运行了 `vcvarsall.bat`，或安装了 Windows SDK。

### Q: clang-cl 找不到
从 https://llvm.org/ 安装 LLVM，并确保已添加到 PATH。

### Q: 链接错误：找不到 CRT 函数
确保链接时包含了 `ucrt.lib`、`vcruntime.lib`、`msvcrt.lib` 等运行时库。

### Q: 栈溢出
Sharp 在 Windows 上默认设置栈大小为 8MB。如果处理大型头文件仍不够，可以增加：
```powershell
# 在 CMakeLists.txt 中修改
set_target_properties(sharpc PROPERTIES LINK_FLAGS "/STACK:16777216")
```
