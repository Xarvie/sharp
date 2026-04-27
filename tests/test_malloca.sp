/* compiler: msvc */
/* §29 _malloca / _freea 智能栈堆分配 */
extern void* _malloca(long size);
extern void _freea(void* p);

int main() {
    return 0;
}