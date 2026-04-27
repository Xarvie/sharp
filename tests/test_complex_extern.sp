/* compiler: msvc */
/* C Feature: Complex extern function declarations with calling convention + variadic */
/* Expected: Parser should handle complex extern declarations */

extern int __cdecl printf(const char* format, ...);
extern void __stdcall ExitProcess(int exitCode);
extern int __fastcall add(int a, int b);

int main() {
    return 0;
}