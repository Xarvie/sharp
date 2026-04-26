# C 标准库头文件适配检查清单

## 核心 C 标准库 (C89/C99)

| # | 头文件 | 解析状态 | 编译状态 | 备注 |
|---|--------|---------|---------|------|
| 1 | [assert.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/assert.h) | ⬜ 待测 | ⬜ 待测 | |
| 2 | [ctype.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/ctype.h) | ⬜ 待测 | ⬜ 待测 | |
| 3 | [errno.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/errno.h) | ⬜ 待测 | ⬜ 待测 | |
| 4 | [float.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/float.h) | ⬜ 待测 | ⬜ 待测 | |
| 5 | [limits.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/limits.h) | ⬜ 待测 | ⬜ 待测 | |
| 6 | [locale.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/locale.h) | ⬜ 待测 | ⬜ 待测 | |
| 7 | [math.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/math.h) | ⬜ 待测 | ⬜ 待测 | |
| 8 | [setjmp.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/setjmp.h) | ⬜ 待测 | ⬜ 待测 | |
| 9 | [signal.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/signal.h) | ⬜ 待测 | ⬜ 待测 | |
| 10 | [stdarg.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/stdarg.h) | ⬜ 待测 | ⬜ 待测 | |
| 11 | [stdbool.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/stdbool.h) | ⬜ 待测 | ⬜ 待测 | |
| 12 | [stddef.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/stddef.h) | ⬜ 待测 | ⬜ 待测 | |
| 13 | [stdint.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/stdint.h) | ⬜ 待测 | ⬜ 待测 | |
| 14 | [stdio.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/stdio.h) | ✅ 通过 | ✅ 通过 | 已完成 |
| 15 | [stdlib.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/stdlib.h) | ✅ 通过 | ✅ 通过 | 函数指针返回类型、inline body skip、static inline skip、parse_c_type lookahead 修复 |
| 16 | [string.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/string.h) | ⬜ 待测 | ⬜ 待测 | |
| 17 | [time.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/time.h) | ⬜ 待测 | ⬜ 待测 | |

## C99 扩展标准库

| # | 头文件 | 解析状态 | 编译状态 | 备注 |
|---|--------|---------|---------|------|
| 18 | [complex.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/) | ⬜ 跳过 | ⬜ 跳过 | 可能不存在 |
| 19 | [fenv.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/fenv.h) | ⬜ 待测 | ⬜ 待测 | |
| 20 | [inttypes.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/inttypes.h) | ⬜ 待测 | ⬜ 待测 | |
| 21 | [iso646.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/) | ⬜ 跳过 | ⬜ 跳过 | 可能不存在 |
| 22 | [tgmath.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/) | ⬜ 跳过 | ⬜ 跳过 | 可能不存在 |
| 23 | [wchar.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/wchar.h) | ⬜ 待测 | ⬜ 待测 | |
| 24 | [wctype.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/wctype.h) | ⬜ 待测 | ⬜ 待测 | |

## MinGW/MSVC 扩展头文件

| # | 头文件 | 解析状态 | 编译状态 | 备注 |
|---|--------|---------|---------|------|
| 25 | [malloc.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/malloc.h) | ⬜ 待测 | ⬜ 待测 | |
| 26 | [io.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/io.h) | ⬜ 待测 | ⬜ 待测 | |
| 27 | [process.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/process.h) | ⬜ 待测 | ⬜ 待测 | |
| 28 | [share.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/share.h) | ⬜ 待测 | ⬜ 待测 | |
| 29 | [direct.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/direct.h) | ⬜ 待测 | ⬜ 待测 | |
| 30 | [conio.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/conio.h) | ⬜ 待测 | ⬜ 待测 | |
| 31 | [tchar.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/tchar.h) | ⬜ 待测 | ⬜ 待测 | |
| 32 | [varargs.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/varargs.h) | ⬜ 待测 | ⬜ 待测 | |
| 33 | [values.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/values.h) | ⬜ 待测 | ⬜ 待测 | |
| 34 | [mem.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/mem.h) | ⬜ 待测 | ⬜ 待测 | |
| 35 | [memory.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/memory.h) | ⬜ 待测 | ⬜ 待测 | |
| 36 | [excpt.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/excpt.h) | ⬜ 待测 | ⬜ 待测 | |
| 37 | [vadefs.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/vadefs.h) | ⬜ 待测 | ⬜ 待测 | |
| 38 | [dir.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/dir.h) | ⬜ 待测 | ⬜ 待测 | |
| 39 | [dos.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/dos.h) | ⬜ 待测 | ⬜ 待测 | |
| 40 | [fcntl.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/fcntl.h) | ⬜ 待测 | ⬜ 待测 | |
| 41 | [search.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/) | ⬜ 待测 | ⬜ 待测 | 可能在 sec_api |

## sys/ 子目录头文件

| # | 头文件 | 解析状态 | 编译状态 | 备注 |
|---|--------|---------|---------|------|
| 42 | [sys/types.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/sys/types.h) | ⬜ 待测 | ⬜ 待测 | |
| 43 | [sys/time.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/sys/time.h) | ⬜ 待测 | ⬜ 待测 | |
| 44 | [sys/stat.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/sys/stat.h) | ⬜ 待测 | ⬜ 待测 | |
| 45 | [sys/fcntl.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/sys/fcntl.h) | ⬜ 待测 | ⬜ 待测 | |
| 46 | [sys/file.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/sys/file.h) | ⬜ 待测 | ⬜ 待测 | |
| 47 | [sys/locking.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/sys/locking.h) | ⬜ 待测 | ⬜ 待测 | |
| 48 | [sys/timeb.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/sys/timeb.h) | ⬜ 待测 | ⬜ 待测 | |
| 49 | [sys/utime.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/sys/utime.h) | ⬜ 待测 | ⬜ 待测 | |
| 50 | [sys/unistd.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/sys/unistd.h) | ⬜ 待测 | ⬜ 待测 | |

## Windows API 头文件 (可选适配)

| # | 头文件 | 解析状态 | 编译状态 | 备注 |
|---|--------|---------|---------|------|
| 51 | [windef.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/winapi/windef.h) | ⬜ 待测 | ⬜ 待测 | Windows 基础类型 |
| 52 | [winnt.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/winapi/winnt.h) | ⬜ 待测 | ⬜ 待测 | Windows NT 类型 |
| 53 | [windows.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/winapi/windows.h) | ⬜ 待测 | ⬜ 待测 | 主 Windows 头 |
| 54 | [winerror.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/winapi/winerror.h) | ⬜ 待测 | ⬜ 待测 | 错误码 |
| 55 | [winbase.h](file:///c:/Users/ftp/Desktop/sharp/third_party/tcc/include/winapi/winbase.h) | ⬜ 待测 | ⬜ 待测 | 基础 API |

---

## 测试方法

每个头文件的测试步骤：
1. `sharpc.exe <header.h> -o test.c -no-link` — 检查解析是否成功
2. `cl.exe /c test.c` — 检查生成的 C 代码能否编译
3. 检查错误类型：
   - 解析错误 = parser.c 需要扩展
   - 编译错误 = cgen.c 需要修复
   - 链接错误 = 暂不关心（只是声明文件）

## 统计

- 总计: 55 个头文件
- 已完成: 1
- 待测试: 54
- 完成率: 1.8%
