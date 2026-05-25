class Vec {
    int* data;
    long len;
    long cap;
}

long Vec.size(this) const {
    return this->len;
}

void Vec.push(this, int item) {
    this->data[this->len] = item;
    this->len++;
}

int main(void) {
    int arr[10];
    Vec v = { arr, 0, 10 };
    v.push(42);
    v.push(99);
    long s = v.size();
    return (int)(s - 2);  /* expect 0 */
}