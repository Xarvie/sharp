#define INNER "ll"
#define OUTER INNER
#define DEEPER OUTER

void f() {
  callfunc(INNER);
  callfunc(OUTER);
  callfunc(DEEPER);
}
