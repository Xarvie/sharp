/* compiler: msvc */
/* C Feature: Complex extern function declarations with calling convention + variadic */
/* Expected: Parser should handle complex extern declarations */

extern i32 __cdecl printf(const char* format, ...);
extern void __stdcall ExitProcess(u32 exitCode);
extern i32 __fastcall add(i32 a, i32 b);

i32 main() {
    return 0;
}
