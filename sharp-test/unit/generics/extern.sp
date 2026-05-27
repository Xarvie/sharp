// Merged generic extern, template, and nested type tests
// Sources: p150_extern_func_inst.sp, p151_vec_template.sp, p120_gfunc_nested_gtgt.sp

/* Generic swap with extern declaration (p150) */
void swap<T>(T* a, T* b) {
    T tmp = *a;
    *a = *b;
    *b = tmp;
}

/* extern prevents code generation for these instantiations */
extern swap<int>;
extern swap<float>;

/* Vec template (p151) */
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

/* Nested generic types (p120) */
typedef struct Vec2<T> {
    T* data;
    int len;
} Vec2;

Vec2<Vec2<int>> flatten(Vec2<Vec2<int>> v) {
    return v;
}

int main() {
    /* Test extern prevents code gen (p150) */
    float fa = 1.0f, fb = 2.0f;
    swap<float>(&fa, &fb);
    if ((int)(fa + fb - 3.0f) != 0) return 1;

    /* Test Vec template (p151) */
    int arr[10];
    Vec<int> v = { arr, 0, 10 };
    v.push(100);
    v.push(200);
    if (v.size() != 2) return 2;
    if (v.at(0) != 100) return 3;
    if (v.at(1) != 200) return 4;

    /* Test nested generic types (p120) */
    Vec2<Vec2<int>> x;
    flatten(x);

    return 0;
}
