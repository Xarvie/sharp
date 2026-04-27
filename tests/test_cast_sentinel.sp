/* compiler: any */
/* §19 强转指针哨兵值 */

int main() {
    void* p = (void*)(-1);
    int* ip = (int*)(0xFFFFFFFF);
    return 0;
}