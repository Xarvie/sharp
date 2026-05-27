// 来源: p112_operator_index.sp
// operator[] 重载测试

class Arr {
    int data[4];
}
int Arr.operator[](this, int i) { return this->data[i]; }

int main(void) {
    Arr a;
    a.data[0] = 10; a.data[1] = 20; a.data[2] = 30; a.data[3] = 40;
    if (a[0] != 10) return 1;
    if (a[2] != 30) return 2;
    return 0;
}