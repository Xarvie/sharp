class Counter {
    long count;
};

/* non-const method: T* this, can mutate */
void Counter.inc(this) { this->count = this->count + 1; }

/* const method: const T* this, read-only */
long Counter.get(this) const { return this->count; }

int main(void) {
    Counter c = { 0 };
    c.inc();
    c.inc();
    long v = c.get();  /* 2 */
    return v - 2;
}