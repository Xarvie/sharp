# Sharp 编译器运行说明

## 1. 项目简介

Sharp 是一个 C 超集编译器，支持：
- C11 标准
- Rust 风格的 `impl` 语法
- C++ 风格的模板
- RAII 资源管理
- 内置 C 预处理器（支持完整宏展开）

## 2. 构建 Sharp 编译器

```powershell
# 创建并进入构建目录
mkdir build-debug
cd build-debug

# 配置项目
cmake .. -G "Visual Studio 17 2022" -A x64

# 构建
cmake --build . --config Debug
```

构建成功后，可执行文件位于 `build-debug/sharpc.exe`。

## 3. sharpc 命令行用法

```bash
sharpc input.sp -o output.c      # 生成 C 代码
sharpc input.sp -E -o output.pp  # 仅预处理
sharpc input.sp -E               # 预处理输出到 stdout
sharpc input.sp -                # C 代码输出到 stdout
sharpc input.sp -dump-hir        # 打印 HIR 到 stderr
```

### 命令行参数

| 参数 | 说明 |
|------|------|
| `<file>` | 输入源文件（`.sp` 或 `.c`） |
| `-o <file>` | 指定输出文件 |
| `-E` | 仅预处理，输出到 stdout 或 `-o` 指定文件 |
| `-` | 输出到标准输出 |
| `-dump-hir` | 打印 HIR 到 stderr |
| `--target <triple>` | 指定目标三元组 |
| `-I <dir>` | 添加头文件搜索路径 |
| `-D<NAME>[=VALUE]` | 定义宏 |
| `-U<NAME>` | 取消定义宏 |

## 4. 编译并运行 Lua 5.4.6

`test/lua-5.4.6/` 是 vendored 的 Lua 源码，不要修改。

### 4.1 预处理阶段

使用 sharpc 对 Lua 源文件进行预处理（展开宏、包含头文件）：

```powershell
# 创建输出目录
mkdir -Force test\lua-5.4.6\output

# 预处理所有 Lua 源文件（除去 luac.c 编译器部分）
$files = @(
    "lapi.c","lcode.c","lctype.c","ldebug.c","ldo.c","ldump.c",
    "lfunc.c","lgc.c","llex.c","lmem.c","lobject.c","lopcodes.c",
    "lparser.c","lstate.c","lstring.c","ltable.c","ltm.c",
    "lundump.c","lvm.c","lzio.c","lauxlib.c","lbaselib.c",
    "lcorolib.c","ldblib.c","liolib.c","lmathlib.c","loadlib.c",
    "loslib.c","lstrlib.c","ltablib.c","lutf8lib.c","linit.c",
    "lua.c"
)
foreach ($f in $files) {
    sharpc test\lua-5.4.6\src\$f -E -o test\lua-5.4.6\output\$f
}
```

### 4.2 编译并链接

预处理后的文件已包含标准 `#line` 指令，可直接用 MSVC 编译：

```powershell
cd test\lua-5.4.6\output
```

```powershell
# 设置 MSVC 环境
$env:INCLUDE = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.43.34808\include;C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\ucrt;C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\shared;C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\um"
$env:Path = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.43.34808\bin\Hostx64\x64;" + $env:Path

# 编译所有 .c 文件
foreach ($f in Get-ChildItem *.c -Name) {
    cl /O2 /c /nologo /I. $f
}

# 链接为 lua.exe
link /nologo /OUT:lua.exe *.obj

# 运行
.\lua.exe -v
```

### 4.4 一键脚本

也可以使用提供的 `build_lua.ps1` 脚本快速构建：

```powershell
.\build_lua.ps1
```

## 5. 常见问题

### Q: 找不到头文件 `<stdio.h>` 等
确保已安装 Visual Studio，sharpc 会自动发现 VS 2022 安装路径。

### Q: 预处理后 linemarker 导致编译失败
用正则替换 `#\s+\d+\s+"[^"]*"` 为空行即可。

### Q: 栈溢出
sharpc 默认栈大小为 8MB，处理大型头文件如不够可在 CMakeLists.txt 中增加：
```cmake
set_target_properties(sharpc PROPERTIES LINK_FLAGS "/STACK:16777216")
```
