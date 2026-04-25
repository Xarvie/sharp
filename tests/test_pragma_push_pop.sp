/* compiler: any */
/* §9 #pragma push_macro / pop_macro */
#pragma push_macro("NDEBUG")
#undef NDEBUG
#pragma pop_macro("NDEBUG")

i32 main() {
    return 0;
}
