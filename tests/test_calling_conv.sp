// TDD-2.1~2.4: calling convention keywords passthrough

/* Function declaration with __cdecl */
extern int __cdecl cdecl_fn(int a, int b);

/* Function declaration with __stdcall */
extern int __stdcall stdcall_fn(int x);

/* Function declaration with __fastcall */
extern void __fastcall fast_fn(int a, int b);

/* Variable with __unaligned (TDD-2.4) */
i32 main() {
    __unaligned int* ptr;
    return 0;
}
