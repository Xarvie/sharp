#define F(x) G(x)
#define G(x) x ## 1

int A1 = 1;

int main() {
    return F(A);
}
