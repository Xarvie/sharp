Write-Host "=== 剩余 22 个失败分类 ===" -ForegroundColor Cyan

Write-Host "`n1. clang 编译失败 (8个) - MSVC 专属语法，clang 也不支持:" -ForegroundColor Yellow
Write-Host "   conio.h, excpt.h, malloc.h, process.h, stdlib.h,"
Write-Host "   wingdi.h, winreg.h, winver.h"
Write-Host "   + sec_api/ 下的 conio_s.h, stdlib_s.h"

Write-Host "`n2. sharpc 解析失败 (7个):" -ForegroundColor Red
Write-Host "   sec_api/crtdbg_s.h"
Write-Host "   sec_api/mbstring_s.h"
Write-Host "   sec_api/search_s.h"
Write-Host "   sec_api/stralign_s.h"
Write-Host "   varargs.h"
Write-Host "   winapi/winbase.h"
Write-Host "   winapi/windef.h"
Write-Host "   winapi/windows.h"
Write-Host "   winapi/winnt.h"
Write-Host "   winapi/winuser.h"

Write-Host "`n3. clang 编译失败 (sys/stat.h) - 结构体重复定义" -ForegroundColor Red

Write-Host "`n4. varargs.h - tcc 故意报错废弃此头文件" -ForegroundColor Gray
