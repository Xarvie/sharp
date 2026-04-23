/*
 * stdio.sp — Sharp 包装 C 标准库 stdio.h
 *
 * 缺失的 C11 特性导致以下接口无法包装:
 *   [MISSING: const 限定符]     Sharp 不支持函数参数的 const 修饰符
 *   [MISSING: restrict 限定符]  Sharp 不支持 restrict 别名语义
 *   [MISSING: va_list]          Sharp 没有 va_list/stdarg.h 支持
 *   [MISSING: FILE 结构体]      FILE 是内部结构体，只能作为不透明指针使用
 *   [MISSING: const 常量]       Sharp 没有 const 变量声明 (const FILE* stdin)
 */

/* ========== 不透明类型 ========== */
typedef FILE* File;

/*
 * [MISSING: va_list] vprintf, vfprintf, vsprintf, vsnprintf,
 *   vscanf, vfscanf, vsscanf 需要 va_list 参数类型，Sharp 不支持
 *  跳过 7 个函数:
 *   extern i32 vprintf(u8* fmt, va_list args);
 *   extern i32 vfprintf(File file, u8* fmt, va_list args);
 *   ...
 */

/*
 * [MISSING: 可变参数] printf/fprintf/sprintf/snprintf/scanf/fscanf/sscanf
 *   是 C 风格可变参数函数。Sharp extern 不支持 ... 语法。
 *   此处声明为最简签名用于基本调用。
 *   注意: 这些是可变参函数，传入额外参数在 C 层面工作但 Sharp 不检查。
 */
extern i32 printf(u8* fmt);
extern i32 fprintf(File file, u8* fmt);
extern i32 sprintf(u8* dest, u8* fmt);
extern i32 snprintf(u8* s, usize n, u8* fmt);

extern i32 scanf(u8* fmt);
extern i32 fscanf(File file, u8* fmt);
extern i32 sscanf(u8* src, u8* fmt);

/* ========== 文件操作 ========== */
extern File fopen(u8* filename, u8* mode);
extern File freopen(u8* filename, u8* mode, File file);
extern i32 fclose(File file);

/*
 * [MISSING: const FILE*] stdin/stdout/stderr
 *   C 中是 extern FILE* const，Sharp 没有 const 变量
 *   也无法表达宏: #define stdin (&__iob_func()[0])
 */

/* ========== 字符 I/O ========== */
extern i32 fgetc(File file);
extern i32 fputc(i32 ch, File file);
extern i32 getchar();
extern i32 putchar(i32 ch);
extern i32 getc(File file);
extern i32 putc(i32 ch, File file);
extern i32 ungetc(i32 ch, File file);

/* ========== 字符串/行 I/O ========== */
extern u8* fgets(u8* buf, i32 maxcount, File file);
extern i32 fputs(u8* str, File file);
extern i32 puts(u8* str);

/* ========== 二进制 I/O ========== */
extern usize fread(void* dst, usize size, usize count, File file);
extern usize fwrite(void* src, usize size, usize count, File file);

/* ========== 定位 ========== */
extern i32 fseek(File file, i64 offset, i32 origin);
extern i64 ftell(File file);
extern void rewind(File file);
extern i32 fgetpos(File file, void* pos);
extern i32 fsetpos(File file, void* pos);

/* ========== 缓冲与状态 ========== */
extern i32 fflush(File file);
extern i32 feof(File file);
extern i32 ferror(File file);
extern void clearerr(File file);
extern void perror(u8* msg);

extern void setbuf(File file, u8* buf);
extern i32 setvbuf(File file, u8* buf, i32 mode, usize size);

/* ========== 文件管理 ========== */
extern i32 remove(u8* filename);
extern i32 rename(u8* oldname, u8* newname);

/* ========== 临时文件 ========== */
extern File tmpfile();
extern u8* tmpnam(u8* buf);

/* ========== 宏常量 ========== */
/*
 * [MISSING: const 常量] Sharp 没有 const 变量声明语法
 *   [MISSING: enum] Sharp 不支持 enum
 *   当前无法在 Sharp 中定义 EOF/SEEK_SET 等常量。
 *   用户需硬编码值，或在生成的 C 代码中通过 #define 提供。
 */
/* #define EOF (-1)          */
/* #define SEEK_SET 0        */
/* #define SEEK_CUR 1        */
/* #define SEEK_END 2        */
/* #define BUFSIZ 512        */
/* #define FILENAME_MAX 260  */
/* #define FOPEN_MAX 20      */
/* #define TMP_MAX 32767     */
/* #define _IOFBF 0x0000     */
/* #define _IOLBF 0x0040     */
/* #define _IONBF 0x0004     */
