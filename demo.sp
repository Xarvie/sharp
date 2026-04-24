/* demo.sp - Comprehensive stdio.h API coverage test */
#include <stdio.h>

/* Helper: check if pointer is non-null */
i32 is_not_null(void* p) {
    if (p != NULL) { return 1; } else { return 0; }
}

/* --- Test Functions --- */

/* File operations */
i32 test_file_ops() {
    FILE* f;
    f = fopen("test.txt", "r");
    if (is_not_null(f)) {
        fclose(f);
    }
    f = freopen("output.txt", "w", stdout);
    remove("temp.txt");
    rename("old.txt", "new.txt");
    f = tmpfile();
    if (is_not_null(f)) fclose(f);
    return 0;
}

/* Formatted output: printf family */
i32 test_printf() {
    FILE* f = stdout;
    printf("Hello %s, number %d\n", "world", 42);
    fprintf(f, "File output: %f\n", 3.14);
    return 0;
}

/* String formatted output: sprintf family */
i32 test_sprintf() {
    c8 buf[256];
    sprintf(buf, "formatted: %s", "test");
    snprintf(buf, 256, "safe: %d", 123);
    return 0;
}

/* Formatted input: scanf family */
i32 test_scanf() {
    i32 x;
    c8 name[64];
    FILE* f = stdin;
    scanf("%d", x);
    fscanf(f, "%s", name);
    sscanf("42", "%d", x);
    return 0;
}

/* Character I/O */
i32 test_char_io() {
    FILE* f = stdout;
    i32 c;
    c = fgetc(f);
    fputc('A', f);
    c = getc(f);
    putc('B', f);
    c = getchar();
    putchar(c);
    ungetc(c, f);
    return 0;
}

/* String I/O */
i32 test_string_io() {
    FILE* f = stdin;
    c8 buf[128];
    fgets(buf, 128, f);
    fputs("hello", stdout);
    puts("line output");
    return 0;
}

/* Block I/O */
i32 test_block_io() {
    FILE* f;
    c8 buf[64];
    usize n_read, n_written;
    f = fopen("data.bin", "rb");
    if (is_not_null(f)) {
        n_read = fread(buf, 1, 64, f);
        n_written = fwrite(buf, 1, n_read, stdout);
        fclose(f);
    }
    return 0;
}

/* File positioning */
i32 test_positioning() {
    FILE* f;
    i64 pos;
    fpos_t fpos;
    f = fopen("seek.txt", "r");
    if (is_not_null(f)) {
        pos = ftell(f);
        fseek(f, 0, SEEK_SET);
        fseek(f, 10, SEEK_CUR);
        fseek(f, -5, SEEK_END);
        fgetpos(f, fpos);
        fsetpos(f, fpos);
        rewind(f);
        fclose(f);
    }
    return 0;
}

/* Buffer control */
i32 test_buffering() {
    FILE* f;
    c8 buf[512];
    f = fopen("buf.txt", "w");
    if (is_not_null(f)) {
        setbuf(f, buf);
        setvbuf(f, buf, _IOFBF, 512);
        fflush(f);
        fclose(f);
    }
    return 0;
}

/* Error/status */
i32 test_error_status() {
    FILE* f;
    f = fopen("check.txt", "r");
    if (is_not_null(f)) {
        feof(f);
        ferror(f);
        clearerr(f);
        fclose(f);
    }
    return 0;
}

/* Wide character I/O */
i32 test_wide_io() {
    FILE* f = stdout;
    wchar_t wc;
    wchar_t wbuf[128];
    wc = fgetwc(f);
    fputwc(wc, f);
    wc = getwc(f);
    putwc(wc, f);
    fgetws(wbuf, 128, f);
    fputws(wbuf, f);
    return 0;
}

/* Utility / extensions */
i32 test_utility() {
    FILE* f;
    i32 fd;
    c8* path;
    fd = fileno(stdin);
    f = _fdopen(fd, "r");
    f = _fsopen("locked.txt", "r", 1);
    path = tmpnam(NULL);
    if (is_not_null(f)) fclose(f);
    return 0;
}

/* Pipe I/O */
i32 test_pipe_io() {
    FILE* pipe;
    pipe = _popen("dir", "r");
    if (is_not_null(pipe)) {
        _pclose(pipe);
    }
    return 0;
}

/* Error reporting */
i32 test_error_report() {
    perror("Error occurred");
    return 0;
}

/* --- Main: call all test categories --- */
i32 main() {
    puts("=== stdio.h API Coverage Test ===");

    test_file_ops();
    test_printf();
    test_sprintf();
    test_scanf();
    test_char_io();
    test_string_io();
    test_block_io();
    test_positioning();
    test_buffering();
    test_error_status();
    test_wide_io();
    test_utility();
    test_pipe_io();
    test_error_report();

    puts("=== All stdio.h API categories tested ===");
    return 0;
}
