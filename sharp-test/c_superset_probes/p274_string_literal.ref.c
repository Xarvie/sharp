

#line 6 "c_superset_probes/p274_string_literal.sp"
int str_len(const char * s) {

#line 7 "c_superset_probes/p274_string_literal.sp"
    int n = 0;

#line 8 "c_superset_probes/p274_string_literal.sp"
    while (s[n] != 0) 
        n = n + 1;

#line 9 "c_superset_probes/p274_string_literal.sp"
    return n;
}

#line 12 "c_superset_probes/p274_string_literal.sp"
int str_eq(const char * a, const char * b) {

#line 13 "c_superset_probes/p274_string_literal.sp"
    int i = 0;

#line 14 "c_superset_probes/p274_string_literal.sp"
    while (a[i] != 0 && b[i] != 0) {

#line 15 "c_superset_probes/p274_string_literal.sp"
        if (a[i] != b[i]) 
            return 0;

#line 16 "c_superset_probes/p274_string_literal.sp"
        i = i + 1;
    }

#line 18 "c_superset_probes/p274_string_literal.sp"
    return a[i] == b[i];
}

#line 21 "c_superset_probes/p274_string_literal.sp"
int main() {

#line 23 "c_superset_probes/p274_string_literal.sp"
    if (str_len("hello") != 5) 
        return 1;

#line 24 "c_superset_probes/p274_string_literal.sp"
    if (str_len("") != 0) 
        return 2;

#line 27 "c_superset_probes/p274_string_literal.sp"
    if (!str_eq("abc", "abc")) 
        return 3;

#line 28 "c_superset_probes/p274_string_literal.sp"
    if (str_eq("abc", "abd")) 
        return 4;

#line 29 "c_superset_probes/p274_string_literal.sp"
    if (str_eq("ab", "abc")) 
        return 5;

#line 32 "c_superset_probes/p274_string_literal.sp"
    const char * null_str = (const char *)0;

#line 33 "c_superset_probes/p274_string_literal.sp"
    if (null_str != (const char *)0) 
        return 6;

#line 35 "c_superset_probes/p274_string_literal.sp"
    return 0;
}
