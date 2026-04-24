/*
 * stdio.sp — Sharp 标准库 stdio 包装
 *
 * Sharp 标准库内部文件。使用 #include <sharp/platform.h> 获取跨平台宏常量。
 * 用户只需 #include <stdio.sp>，不需要感知内部宏。
 */

/* ========== 内部宏引入 ========== */
/*
 * _SH_EOF, _SH_SEEK_SET, _SH_SEEK_CUR, _SH_SEEK_END,
 * _SH_BUFSIZ, _SH_FILENAME_MAX, _SH_FOPEN_MAX,
 * _SH_TMP_MAX, _SH_IOFBF, _SH_IOLBF, _SH_IONBF
 */
#include <sharp/platform.h>

/* ========== 不透明类型 ========== */
/*
 * [说明] FILE 在 C 中是完整结构体定义，Sharp 无法翻译其内部字段
 * （包含缓冲区、标志位等），只能作为 typedef 指针使用。
 * 这也意味着无法在 Sharp 中创建 FILE 实例，只能通过 fopen/fdopen 等获取。
 */
typedef FILE* File;

/*
 * [说明] va_list 需要 stdarg.h 支持，以下函数跳过：
 *   vprintf, vfprintf, vsprintf, vsnprintf, vscanf, vfscanf, vsscanf
 *   共 7 个函数，需要 Sharp 支持 stdarg.h 宏或原生 va_list 类型。
 */

/* ========== 格式化 I/O ========== */
/*
 * [说明] printf/fprintf/sprintf/snprintf/scanf/fscanf/sscanf
 *   是 C 风格可变参数函数。Sharp extern 不支持 ... 语法。
 *   此处声明为最简签名，实际调用时可传入额外参数（C 层面工作但 Sharp 不检查）。
 */
extern i32 printf(const u8* fmt, ...);
extern i32 fprintf(File stream, const u8* fmt, ...);
extern i32 sprintf(u8* dest, const u8* fmt, ...);
extern i32 snprintf(u8* s, usize n, const u8* fmt, ...);

extern i32 scanf(const u8* fmt, ...);
extern i32 fscanf(File stream, const u8* fmt, ...);
extern i32 sscanf(const u8* src, const u8* fmt, ...);

/* ========== 文件操作 ========== */
extern File fopen(const u8* filename, const u8* mode);
extern File freopen(const u8* filename, const u8* mode, File stream);
extern i32 fclose(File stream);
extern i32 remove(const u8* filename);
extern i32 rename(const u8* oldname, const u8* newname);

/*
 * [说明] stdin/stdout/stderr 是 extern FILE* const 全局变量。
 *   在 Sharp 支持 extern 变量之前，通过以下宏常量替代：
 *   （这些宏由平台常量头文件提供，内部使用）
 */

/* ========== 字符 I/O ========== */
extern i32 fgetc(File stream);
extern i32 fputc(i32 ch, File stream);
extern i32 getc(File stream);
extern i32 putc(i32 ch, File stream);
extern i32 ungetc(i32 ch, File stream);

/*
 * [说明] getchar()/putchar() 在 C 中通常是函数式宏:
 *   #define getchar()  getc(stdin)
 *   #define putchar(c) putc((c), stdout)
 *   也可用内联函数实现，此处保留 extern 声明。
 */
extern i32 getchar();
extern i32 putchar(i32 ch);

/* ========== 字符串/行 I/O ========== */
extern u8* fgets(u8* s, i32 n, File stream);
extern i32 fputs(const u8* s, File stream);
extern i32 puts(const u8* s);

/* ========== 二进制 I/O ========== */
extern usize fread(void* ptr, usize size, usize count, File stream);
extern usize fwrite(const void* ptr, usize size, usize count, File stream);

/* ========== 定位 ========== */
extern i32 fseek(File stream, i64 offset, i32 whence);
extern i64 ftell(File stream);
extern void rewind(File stream);
extern i32 fgetpos(File stream, void* pos);
extern i32 fsetpos(File stream, const void* pos);

/* ========== 缓冲与状态 ========== */
extern i32 fflush(File stream);
extern i32 feof(File stream);
extern i32 ferror(File stream);
extern void clearerr(File stream);
extern void perror(const u8* s);

extern void setbuf(File stream, u8* buf);
extern i32 setvbuf(File stream, u8* buf, i32 mode, usize size);

/* ========== 临时文件 ========== */
extern File tmpfile();
extern u8* tmpnam(u8* s);

/*
 * [说明] 以下 C 宏常量已由 platform.h 提供为 _SH_ 前缀版本：
 *   EOF         → _SH_EOF          (-1)
 *   SEEK_SET    → _SH_SEEK_SET     (0)
 *   SEEK_CUR    → _SH_SEEK_CUR     (1)
 *   SEEK_END    → _SH_SEEK_END     (2)
 *   BUFSIZ      → _SH_BUFSIZ       (512/8192/1024, 平台相关)
 *   FILENAME_MAX → _SH_FILENAME_MAX (260/4096/1024, 平台相关)
 *   FOPEN_MAX   → _SH_FOPEN_MAX    (20/16, 平台相关)
 *   TMP_MAX     → _SH_TMP_MAX      (32767)
 *   _IOFBF      → _SH_IOFBF        (0)
 *   _IOLBF      → _SH_IOLBF        (0x0040)
 *   _IONBF      → _SH_IONBF        (0x0004)
 *
 * 函数式宏（Sharp 不支持）:
 *   #define getchar()       getc(stdin)
 *   #define putchar(c)      putc((c), stdout)
 *   #define clearerr(s)     (void)((s)->_flag &= ~(_IOERR|_IOEOF))
 *   #define fileno(s)       ((s)->_file)
 */

/* ========== 宽字符 I/O ========== */
/*
 * [跳过] wchar_t 类型不支持。
 *   fgetws, fputws, getwchar, putwchar, fwide, ungetwc,
 *   fwprintf, fwscanf, swprintf, swscanf, vfwprintf, vfwscanf,
 *   vswprintf, vswscanf, vwprintf, vwscanf, wprintf, wscanf
 */

/* ========== POSIX 扩展 ========== */
/*
 * [跳过] 以下 POSIX 接口需要额外类型支持：
 *   fdopen/fileno - 需要文件描述符 (int → i32 可近似)
 *   getline/getdelim - 需要 ssize_t* 输出参数
 *   stat/fstat/lstat - 需要 struct stat 结构体
 */
