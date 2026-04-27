extern int puts(const char* s);
int main() {
    const char* msg = "hello const";
    puts(msg);
    return 0;
}