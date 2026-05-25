class Vec<T> {
    T* data;
    long len;
    long cap;
}

long Vec<T>.size(this) const {
    return this->len;
}

void Vec<T>.push(this, T item) {
    if (this->len >= this->cap) return;
    this->data[this->len] = item;
    this->len = this->len + 1;
}

T Vec<T>.at(this, long i) const {
    return this->data[i];
}

int main() {
    return 0;
}