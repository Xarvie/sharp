# Sharp 编译器运行说明

## 项目简介

Sharp 是一个 C 超集编译器，支持 C11、Rust 风格 `impl`、C++ 模板、RAII 和内置 C 预处理器。

## 构建

```powershell
mkdir build-debug && cd build-debug
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Debug
```

输出：`build-debug/sharpc.exe`

## 命令行

```
sharpc input.sp          # 生成 input.c
sharpc input.sp -E       # 预处理，生成 input.c
sharpc input.sp -        # 输出到 stdout
sharpc input.sp -dump-hir      # 打印 HIR
```

| 参数 | 说明 |
|------|------|
| `-o <file>` | 指定输出文件 |
| `-E` | 仅预处理，生成 .c |
| `-` | 输出到 stdout |
| `-dump-hir` | 打印 HIR |
| `--target <triple>` | 目标三元组 |
| `-I <dir>` | 头文件路径 |
| `-D<NAME>[=V]` | 定义宏 |
| `-U<NAME>` | 取消定义宏 |

## 编译 Lua 5.4.6

`test/lua-5.4.6/` 是 vendored 源码，不要修改。

### 1. 预处理

```powershell
mkdir -Force test\lua-5.4.6\output
$files = @("lapi.c","lcode.c","lctype.c","ldebug.c","ldo.c","ldump.c",
    "lfunc.c","lgc.c","llex.c","lmem.c","lobject.c","lopcodes.c",
    "lparser.c","lstate.c","lstring.c","ltable.c","ltm.c",
    "lundump.c","lvm.c","lzio.c","lauxlib.c","lbaselib.c",
    "lcorolib.c","ldblib.c","liolib.c","lmathlib.c","loadlib.c",
    "loslib.c","lstrlib.c","ltablib.c","lutf8lib.c","linit.c","lua.c")
foreach ($f in $files) {
    sharpc test\lua-5.4.6\src\$f -E
}
```

### 2. 编译链接

```powershell
cd test\lua-5.4.6\output
$env:INCLUDE = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.43.34808\include;C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\ucrt;C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\shared;C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\um"
$env:Path = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.43.34808\bin\Hostx64\x64;" + $env:Path
foreach ($f in Get-ChildItem *.c -Name) { cl /O2 /c /nologo /I. $f }
link /nologo /OUT:lua.exe *.obj
.\lua.exe -v
```

### 一键脚本

```powershell
.\build_lua.ps1
```

## 常见问题

- **找不到头文件**：确保已安装 Visual Studio，sharpc 会自动发现路径
- **栈溢出**：在 CMakeLists.txt 中增加 `/STACK:16777216`
