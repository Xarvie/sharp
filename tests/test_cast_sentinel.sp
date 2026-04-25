/* §19 强转指针哨兵值 */

i32 main() {
    void* p = (void*)(-1);
    i32* ip = (i32*)(0xFFFFFFFF);
    return 0;
}
