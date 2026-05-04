# C-superset probes

41 minimal C programs, each isolating ONE C language feature.  Each is
designed so the correct answer = exit code 0.

The script `../c_superset_probe.sh` runs every probe through:
1. sharpc (sharp-cpp + sharp-fe)
2. cc compile
3. execute

A probe passes iff all three succeed.

| File                  | Feature exercised               | Sub-stage |
|-----------------------|---------------------------------|-----------|
| p01_void_param.c      | `int main(void)`                | done (kept) |
| p02_static.c          | top-level `static`              | S1 |
| p03_unsigned.c        | `unsigned int`                  | works |
| p04_postfix_inc.c     | `i++`                           | works |
| p05_compound_assign.c | `s += 5`                        | works |
| p06_struct_basic.c    | basic struct                    | works |
| p07_typedef.c         | typedef alias as type           | S3a |
| p08_enum.c            | `enum`                          | S2a |
| p09_ptr_param.c       | `int*` parameter                | works |
| p10_array_param.c     | `int a[]` parameter             | done (kept) |
| p11_string_lit.c      | `s[0] - 'h'`                    | S5 (sema char-literal) |
| p12_ternary.c         | `?:`                            | works |
| p13_while.c           | `while`                         | works |
| p14_switch.c          | `switch / case / default`       | S2c |
| p15_func_proto.c      | prototype + same-name def       | S3b |
| p16_extern.c          | top-level `extern`              | S1 |
| p17_const_local.c     | `const int x = 5`               | S5 (sema const-arith) |
| p18_sizeof_int.c      | `sizeof(int)`                   | works |
| p19_ptr_arith.c       | `*(p+1)`                        | works |
| p20_for_postfix.c     | `for (… ; … ; i++)`             | works |
| p21_kr_func.c         | K&R style function              | S5 (optional) |
| p22_union.c           | `union`                         | S2b |
| p23_bitfield.c        | `int a:3` bitfield              | S5 |
| p24_float_double.c    | float / double                  | works |
| p25_neg_lit.c         | `-5`                            | works |
| p26_shift_op.c        | `1 << 3`                        | works |
| p27_addr_arr.c        | `&a[0]`                         | works |
| p28_func_ptr.c        | `int (*f)(int)`                 | S1 |
| p29_init_list.c       | `int a[3] = {1,2,3}`            | S4 |
| p30_struct_init.c     | `struct P p = {3, 4}`           | S4 |
| p31_string_arr.c      | `char s[] = "abc"`              | S4 |
| p32_for_no_decl.c     | for-init w/o decl               | works |
| p33_do_while.c        | `do/while`                      | works |
| p34_struct_ptr.c      | `pp->x`                         | works |
| p35_void_ret.c        | `void f(void)` returns nothing  | done (kept) |
| p36_inline.c          | `static inline`                 | S1 |
| p37_volatile.c        | local `volatile`                | S1 |
| p38_register.c        | local `register`                | S1 |
| p39_compound_lit.c    | `(struct P){.x=3}`              | S4 |
| p40_designated.c      | `{[0]=1, [4]=5}`                | S4 |
| p_brace_init.c        | `const int a[3] = {1,2,3};`     | S4 |

Adding new probes: name them `pNN_short_label.c`, design them so the
correct exit code is 0, keep them minimal (one feature per probe).
