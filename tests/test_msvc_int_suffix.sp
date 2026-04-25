// TDD-§5: MSVC 整数字面量后缀 i8/i16/i32/i64

i32 main() {
    i32 a = 42i32;
    i64 b = 42i64;
    i16 c = 42i16;
    i8  d = 42i8;
    u32 e = 42u32;
    u64 f = 42u64;
    u16 g = 42u16;
    u8  h = 42u8;
    i64 i = 42ui64;
    i32 j = 42ui32;
    return a;
}
