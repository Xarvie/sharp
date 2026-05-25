class Vec<T> {
    T* data;
    long len;
    long cap;
}

long Vec<T>.size(this) const {
    return this->len;
}

T Vec<T>.at(this, long i) const {
    return this->data[i];
}

void Vec<T>.push(this, T item) {
    this->data[this->len] = item;
    this->len++;
}

int main(void) {
    int arr[5];
    Vec<int> v = { arr, 0, 5 };
    v.push(10);
    v.push(20);
    v.push(30);
    long s = v.size();
    int a = v.at(1);
    return a + (int)s - 23;  /* 20 + 3 - 23 = 0 */
}