/* Bug-003: struct 体内声明返回指针类型的方法，
 * 解析器将其误判为指针字段声明 */
struct Vec {
    int x, y;
    int* get_x() {
        return &this->x;
    }
};

int main(void) {
    struct Vec v;
    v.x = 10;
    return v.get_x()[0];
}
