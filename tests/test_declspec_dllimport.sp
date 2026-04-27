/* compiler: msvc */
/* TDD-3.2: __declspec(dllimport) extern variable declaration */
extern __declspec(dllimport) int external_value;

int main() {
    return 0;
}