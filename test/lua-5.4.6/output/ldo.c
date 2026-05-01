#line 1 "src/ldo.c"



#line 1 "src/lprefix.h"
















#line 11 "src/ldo.c"


#line 1 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/setjmp.h"








#line 1 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/vcruntime.h"































#line 1 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/sal.h"





















































































































































































































































































#line 706 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/sal.h"
#pragma region Input Buffer SAL 1 compatibility macros

































































































































































































































































#line 1471 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/sal.h"
#pragma endregion Input Buffer SAL 1 compatibility macros
























































































































#line 1 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/concurrencysal.h"























#line 2975 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/sal.h"
#line 58 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/vcruntime.h"
#line 1 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/vadefs.h"











#line 15 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/vadefs.h"
#pragma pack ( push , 8 )











#line 46 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/vadefs.h"
#pragma warning ( push )
#line 48 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/vadefs.h"
#pragma warning ( disable : 4514 4820 )



#line 61 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/vadefs.h"
 typedef unsigned __int64 uintptr_t;

#line 72 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/vadefs.h"
 typedef char* va_list;




#line 154 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/vadefs.h"
 void __cdecl __va_start(va_list* , ...);







#line 206 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/vadefs.h"
#pragma warning ( pop ) 
#line 208 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/vadefs.h"
#pragma pack ( pop )
#line 59 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/vcruntime.h"

#line 59 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/vcruntime.h"
#pragma warning ( push )
#line 61 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/vcruntime.h"
#pragma warning ( disable : 4514 4820 )

































#line 188 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/vcruntime.h"
 typedef unsigned __int64 size_t;
 typedef __int64 ptrdiff_t;
 typedef __int64 intptr_t;

#line 204 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/vcruntime.h"
 typedef _Bool __vcrt_bool;






#line 223 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/vcruntime.h"
 typedef unsigned short wchar_t;


















#line 332 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/vcruntime.h"
#pragma push_macro ( "msvc" )
#line 334 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/vcruntime.h"
#pragma push_macro ( "constexpr" )




#line 347 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/vcruntime.h"
#pragma pop_macro ( "constexpr" )
#line 349 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/vcruntime.h"
#pragma pop_macro ( "msvc" )








#line 390 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/vcruntime.h"
 void __cdecl __security_init_cookie(void);

#line 399 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/vcruntime.h"
 void __cdecl __security_check_cookie( uintptr_t _StackCookie);
 __declspec(noreturn) void __cdecl __report_gsfailure( uintptr_t _StackCookie);

#line 403 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/vcruntime.h"
extern uintptr_t __security_cookie;




#line 411 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/vcruntime.h"
#pragma warning ( pop ) 

#line 12 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/setjmp.h"


#line 20 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/setjmp.h"
#pragma warning ( push )
#line 22 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/setjmp.h"
#pragma warning ( disable : 4514 4820 )







#line 50 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/setjmp.h"
 typedef struct __declspec(align(16)) _SETJMP_FLOAT128
 {
 unsigned __int64 Part[2];
 } SETJMP_FLOAT128;

#line 57 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/setjmp.h"
 typedef SETJMP_FLOAT128 _JBTYPE;

 typedef struct _JUMP_BUFFER
 {
 unsigned __int64 Frame;
 unsigned __int64 Rbx;
 unsigned __int64 Rsp;
 unsigned __int64 Rbp;
 unsigned __int64 Rsi;
 unsigned __int64 Rdi;
 unsigned __int64 R12;
 unsigned __int64 R13;
 unsigned __int64 R14;
 unsigned __int64 R15;
 unsigned __int64 Rip;
 unsigned long MxCsr;
 unsigned short FpCsr;
 unsigned short Spare;

 SETJMP_FLOAT128 Xmm6;
 SETJMP_FLOAT128 Xmm7;
 SETJMP_FLOAT128 Xmm8;
 SETJMP_FLOAT128 Xmm9;
 SETJMP_FLOAT128 Xmm10;
 SETJMP_FLOAT128 Xmm11;
 SETJMP_FLOAT128 Xmm12;
 SETJMP_FLOAT128 Xmm13;
 SETJMP_FLOAT128 Xmm14;
 SETJMP_FLOAT128 Xmm15;
 } _JUMP_BUFFER;





#line 148 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/setjmp.h"
 typedef _JBTYPE jmp_buf[ 16];







int __cdecl _setjmp(
 jmp_buf _Buf
 );

#line 170 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/setjmp.h"
 __declspec(noreturn) void __cdecl longjmp(
 jmp_buf _Buf,
 int _Value
 );




#line 175 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/setjmp.h"
#pragma warning ( pop ) 
#line 14 "src/ldo.c"
#line 1 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"








#line 1 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt.h"













































#line 120 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt.h"
#pragma warning ( push )
#line 122 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt.h"
#pragma warning ( disable : 4324 4514 4574 4710 4793 4820 4995 4996 28719 28726 28727 )








































#line 274 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt.h"
 typedef _Bool __crt_bool;

























#line 370 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt.h"
 void __cdecl _invalid_parameter_noinfo(void);
 __declspec(noreturn) void __cdecl _invalid_parameter_noinfo_noreturn(void);

__declspec(noreturn)
 void __cdecl _invoke_watson(
 wchar_t const* _Expression,
 wchar_t const* _FunctionName,
 wchar_t const* _FileName,
 unsigned int _LineNo,
 uintptr_t _Reserved);



































































#line 596 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt.h"
typedef int errno_t;
typedef unsigned short wint_t;
typedef unsigned short wctype_t;
typedef long __time32_t;
typedef __int64 __time64_t;

typedef struct __crt_locale_data_public
{
 unsigned short const* _locale_pctype;
 int _locale_mb_cur_max;
 unsigned int _locale_lc_codepage;
} __crt_locale_data_public;

typedef struct __crt_locale_pointers
{
 struct __crt_locale_data* locinfo;
 struct __crt_multibyte_data* mbcinfo;
} __crt_locale_pointers;

typedef __crt_locale_pointers* _locale_t;

typedef struct _Mbstatet
{
 unsigned long _Wchar;
 unsigned short _Byte, _State;
} _Mbstatet;

typedef _Mbstatet mbstate_t;



#line 645 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt.h"
 typedef __time64_t time_t;



#line 655 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt.h"
 typedef size_t rsize_t;





























































































#line 2071 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt.h"
#pragma warning ( pop ) 
#line 13 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
#line 1 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_malloc.h"










#line 12 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_malloc.h"
#pragma warning ( push )
#line 14 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_malloc.h"
#pragma warning ( disable : 4324 4514 4574 4710 4793 4820 4995 4996 28719 28726 28727 )








#line 56 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_malloc.h"
 __declspec(allocator) __declspec(restrict)
void* __cdecl _calloc_base(
 size_t _Count,
 size_t _Size
 );


 __declspec(allocator) __declspec(restrict)
void* __cdecl calloc(
 size_t _Count,
 size_t _Size
 );


 int __cdecl _callnewh(
 size_t _Size
 );


 __declspec(allocator)
void* __cdecl _expand(
 void* _Block,
 size_t _Size
 );


void __cdecl _free_base(
 void* _Block
 );


void __cdecl free(
 void* _Block
 );


 __declspec(allocator) __declspec(restrict)
void* __cdecl _malloc_base(
 size_t _Size
 );


 __declspec(allocator) __declspec(restrict)
void* __cdecl malloc(
 size_t _Size
 );



size_t __cdecl _msize_base(
 void* _Block
 );



size_t __cdecl _msize(
 void* _Block
 );


 __declspec(allocator) __declspec(restrict)
void* __cdecl _realloc_base(
 void* _Block,
 size_t _Size
 );


 __declspec(allocator) __declspec(restrict)
void* __cdecl realloc(
 void* _Block,
 size_t _Size
 );


 __declspec(allocator) __declspec(restrict)
void* __cdecl _recalloc_base(
 void* _Block,
 size_t _Count,
 size_t _Size
 );


 __declspec(allocator) __declspec(restrict)
void* __cdecl _recalloc(
 void* _Block,
 size_t _Count,
 size_t _Size
 );


void __cdecl _aligned_free(
 void* _Block
 );


 __declspec(allocator) __declspec(restrict)
void* __cdecl _aligned_malloc(
 size_t _Size,
 size_t _Alignment
 );


 __declspec(allocator) __declspec(restrict)
void* __cdecl _aligned_offset_malloc(
 size_t _Size,
 size_t _Alignment,
 size_t _Offset
 );



size_t __cdecl _aligned_msize(
 void* _Block,
 size_t _Alignment,
 size_t _Offset
 );


 __declspec(allocator) __declspec(restrict)
void* __cdecl _aligned_offset_realloc(
 void* _Block,
 size_t _Size,
 size_t _Alignment,
 size_t _Offset
 );


 __declspec(allocator) __declspec(restrict)
void* __cdecl _aligned_offset_recalloc(
 void* _Block,
 size_t _Count,
 size_t _Size,
 size_t _Alignment,
 size_t _Offset
 );


 __declspec(allocator) __declspec(restrict)
void* __cdecl _aligned_realloc(
 void* _Block,
 size_t _Size,
 size_t _Alignment
 );


 __declspec(allocator) __declspec(restrict)
void* __cdecl _aligned_recalloc(
 void* _Block,
 size_t _Count,
 size_t _Size,
 size_t _Alignment
 );






#line 229 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_malloc.h"
#pragma warning ( pop ) 
#line 14 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
#line 1 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_search.h"











#line 1 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stddef.h"









#line 13 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stddef.h"
#pragma warning ( push )
#line 15 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stddef.h"
#pragma warning ( disable : 4324 4514 4574 4710 4793 4820 4995 4996 28719 28726 28727 )










#line 34 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stddef.h"
 int* __cdecl _errno(void);

#line 37 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stddef.h"
 errno_t __cdecl _set_errno( int _Value);
 errno_t __cdecl _get_errno( int* _Value);





#line 54 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stddef.h"
 extern unsigned long __cdecl __threadid(void);
#line 57 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stddef.h"
 extern uintptr_t __cdecl __threadhandle(void);





#pragma warning ( pop ) 
#line 15 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_search.h"

#line 15 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_search.h"
#pragma warning ( push )
#line 17 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_search.h"
#pragma warning ( disable : 4324 4514 4574 4710 4793 4820 4995 4996 28719 28726 28727 )





#line 18 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_search.h"
 typedef int (__cdecl* _CoreCrtSecureSearchSortCompareFunction)(void*, void const*, void const*);
 typedef int (__cdecl* _CoreCrtNonSecureSearchSortCompareFunction)(void const*, void const*);




#line 28 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_search.h"
 void* __cdecl bsearch_s(
 void const* _Key,
 void const* _Base,
 rsize_t _NumOfElements,
 rsize_t _SizeOfElements,
 _CoreCrtSecureSearchSortCompareFunction _CompareFunction,
 void* _Context
 );

 void __cdecl qsort_s(
 void* _Base,
 rsize_t _NumOfElements,
 rsize_t _SizeOfElements,
 _CoreCrtSecureSearchSortCompareFunction _CompareFunction,
 void* _Context
 );





#line 48 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_search.h"
 void* __cdecl bsearch(
 void const* _Key,
 void const* _Base,
 size_t _NumOfElements,
 size_t _SizeOfElements,
 _CoreCrtNonSecureSearchSortCompareFunction _CompareFunction
 );

 void __cdecl qsort(
 void* _Base,
 size_t _NumOfElements,
 size_t _SizeOfElements,
 _CoreCrtNonSecureSearchSortCompareFunction _CompareFunction
 );


 void* __cdecl _lfind_s(
 void const* _Key,
 void const* _Base,
 unsigned int* _NumOfElements,
 size_t _SizeOfElements,
 _CoreCrtSecureSearchSortCompareFunction _CompareFunction,
 void* _Context
 );


 void* __cdecl _lfind(
 void const* _Key,
 void const* _Base,
 unsigned int* _NumOfElements,
 unsigned int _SizeOfElements,
 _CoreCrtNonSecureSearchSortCompareFunction _CompareFunction
 );


 void* __cdecl _lsearch_s(
 void const* _Key,
 void* _Base,
 unsigned int* _NumOfElements,
 size_t _SizeOfElements,
 _CoreCrtSecureSearchSortCompareFunction _CompareFunction,
 void* _Context
 );


 void* __cdecl _lsearch(
 void const* _Key,
 void* _Base,
 unsigned int* _NumOfElements,
 unsigned int _SizeOfElements,
 _CoreCrtNonSecureSearchSortCompareFunction _CompareFunction
 );












#line 213 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_search.h"
#pragma warning ( pop ) 
#line 15 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
#line 1 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_wstdlib.h"










#line 12 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_wstdlib.h"
#pragma warning ( push )
#line 14 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_wstdlib.h"
#pragma warning ( disable : 4324 4514 4574 4710 4793 4820 4995 4996 28719 28726 28727 )


















#line 51 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_wstdlib.h"
 errno_t __cdecl _itow_s(
 int _Value,
 wchar_t* _Buffer,
 size_t _BufferCount,
 int _Radix
 );



 
 wchar_t* __cdecl _itow( 
 int _Value, 
 wchar_t * _Buffer, 
 int _Radix 
);



 errno_t __cdecl _ltow_s(
 long _Value,
 wchar_t* _Buffer,
 size_t _BufferCount,
 int _Radix
 );



 
 wchar_t* __cdecl _ltow( 
 long _Value, 
 wchar_t * _Buffer, 
 int _Radix 
);


 errno_t __cdecl _ultow_s(
 unsigned long _Value,
 wchar_t* _Buffer,
 size_t _BufferCount,
 int _Radix
 );



 
 wchar_t* __cdecl _ultow( 
 unsigned long _Value, 
 wchar_t * _Buffer, 
 int _Radix 
);


 double __cdecl wcstod(
 wchar_t const* _String,
 wchar_t** _EndPtr
 );


 double __cdecl _wcstod_l(
 wchar_t const* _String,
 wchar_t** _EndPtr,
 _locale_t _Locale
 );


 long __cdecl wcstol(
 wchar_t const* _String,
 wchar_t** _EndPtr,
 int _Radix
 );


 long __cdecl _wcstol_l(
 wchar_t const* _String,
 wchar_t** _EndPtr,
 int _Radix,
 _locale_t _Locale
 );


 long long __cdecl wcstoll(
 wchar_t const* _String,
 wchar_t** _EndPtr,
 int _Radix
 );


 long long __cdecl _wcstoll_l(
 wchar_t const* _String,
 wchar_t** _EndPtr,
 int _Radix,
 _locale_t _Locale
 );


 unsigned long __cdecl wcstoul(
 wchar_t const* _String,
 wchar_t** _EndPtr,
 int _Radix
 );


 unsigned long __cdecl _wcstoul_l(
 wchar_t const* _String,
 wchar_t** _EndPtr,
 int _Radix,
 _locale_t _Locale
 );


 unsigned long long __cdecl wcstoull(
 wchar_t const* _String,
 wchar_t** _EndPtr,
 int _Radix
 );


 unsigned long long __cdecl _wcstoull_l(
 wchar_t const* _String,
 wchar_t** _EndPtr,
 int _Radix,
 _locale_t _Locale
 );


 long double __cdecl wcstold(
 wchar_t const* _String,
 wchar_t** _EndPtr
 );


 long double __cdecl _wcstold_l(
 wchar_t const* _String,
 wchar_t** _EndPtr,
 _locale_t _Locale
 );


 float __cdecl wcstof(
 wchar_t const* _String,
 wchar_t** _EndPtr
 );


 float __cdecl _wcstof_l(
 wchar_t const* _String,
 wchar_t** _EndPtr,
 _locale_t _Locale
 );


 double __cdecl _wtof(
 wchar_t const* _String
 );


 double __cdecl _wtof_l(
 wchar_t const* _String,
 _locale_t _Locale
 );


 int __cdecl _wtoi(
 wchar_t const* _String
 );


 int __cdecl _wtoi_l(
 wchar_t const* _String,
 _locale_t _Locale
 );


 long __cdecl _wtol(
 wchar_t const* _String
 );


 long __cdecl _wtol_l(
 wchar_t const* _String,
 _locale_t _Locale
 );


 long long __cdecl _wtoll(
 wchar_t const* _String
 );


 long long __cdecl _wtoll_l(
 wchar_t const* _String,
 _locale_t _Locale
 );


 errno_t __cdecl _i64tow_s(
 __int64 _Value,
 wchar_t* _Buffer,
 size_t _BufferCount,
 int _Radix
 );


 wchar_t* __cdecl _i64tow(
 __int64 _Value,
 wchar_t* _Buffer,
 int _Radix
 );


 errno_t __cdecl _ui64tow_s(
 unsigned __int64 _Value,
 wchar_t* _Buffer,
 size_t _BufferCount,
 int _Radix
 );


 wchar_t* __cdecl _ui64tow(
 unsigned __int64 _Value,
 wchar_t* _Buffer,
 int _Radix
 );


 __int64 __cdecl _wtoi64(
 wchar_t const* _String
 );


 __int64 __cdecl _wtoi64_l(
 wchar_t const* _String,
 _locale_t _Locale
 );


 __int64 __cdecl _wcstoi64(
 wchar_t const* _String,
 wchar_t** _EndPtr,
 int _Radix
 );


 __int64 __cdecl _wcstoi64_l(
 wchar_t const* _String,
 wchar_t** _EndPtr,
 int _Radix,
 _locale_t _Locale
 );


 unsigned __int64 __cdecl _wcstoui64(
 wchar_t const* _String,
 wchar_t** _EndPtr,
 int _Radix
 );


 unsigned __int64 __cdecl _wcstoui64_l(
 wchar_t const* _String,
 wchar_t** _EndPtr,
 int _Radix,
 _locale_t _Locale
 );

#pragma push_macro ( "_wfullpath" )



#line 336 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_wstdlib.h"
 __declspec(allocator) wchar_t* __cdecl _wfullpath(
 wchar_t* _Buffer,
 wchar_t const* _Path,
 size_t _BufferCount
 );

#pragma pop_macro ( "_wfullpath" )


#line 346 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_wstdlib.h"
 errno_t __cdecl _wmakepath_s(
 wchar_t* _Buffer,
 size_t _BufferCount,
 wchar_t const* _Drive,
 wchar_t const* _Dir,
 wchar_t const* _Filename,
 wchar_t const* _Ext
 );



 
 void __cdecl _wmakepath( 
 wchar_t * _Buffer, 
 wchar_t const* _Drive, 
 wchar_t const* _Dir, 
 wchar_t const* _Filename, 
 wchar_t const* _Ext 
);

 void __cdecl _wperror(
 wchar_t const* _ErrorMessage
 );


 void __cdecl _wsplitpath(
 wchar_t const* _FullPath,
 wchar_t* _Drive,
 wchar_t* _Dir,
 wchar_t* _Filename,
 wchar_t* _Ext
 );

 errno_t __cdecl _wsplitpath_s(
 wchar_t const* _FullPath,
 wchar_t* _Drive,
 size_t _DriveCount,
 wchar_t* _Dir,
 size_t _DirCount,
 wchar_t* _Filename,
 size_t _FilenameCount,
 wchar_t* _Ext,
 size_t _ExtCount
 );



#pragma push_macro ( "_wdupenv_s" )


#line 407 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_wstdlib.h"
 errno_t __cdecl _wdupenv_s(
 wchar_t** _Buffer,
 size_t* _BufferCount,
 wchar_t const* _VarName
 );

#pragma pop_macro ( "_wdupenv_s" )


 wchar_t* __cdecl _wgetenv(
 wchar_t const* _VarName
 );



 errno_t __cdecl _wgetenv_s(
 size_t* _RequiredCount,
 wchar_t* _Buffer,
 size_t _BufferCount,
 wchar_t const* _VarName
 );




 int __cdecl _wputenv(
 wchar_t const* _EnvString
 );


 errno_t __cdecl _wputenv_s(
 wchar_t const* _Name,
 wchar_t const* _Value
 );

 errno_t __cdecl _wsearchenv_s(
 wchar_t const* _Filename,
 wchar_t const* _VarName,
 wchar_t* _Buffer,
 size_t _BufferCount
 );



 
 void __cdecl _wsearchenv( 
 wchar_t const* _Filename, 
 wchar_t const* _VarName, 
 wchar_t * _ResultPath 
);

 int __cdecl _wsystem(
 wchar_t const* _Command
 );






#line 476 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_wstdlib.h"
#pragma warning ( pop ) 
#line 16 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
#line 1 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/limits.h"









#line 12 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/limits.h"
#pragma warning ( push )
#line 14 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/limits.h"
#pragma warning ( disable : 4514 4820 )















#line 73 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/limits.h"
#pragma warning ( pop ) 
#line 17 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"

#line 17 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
#pragma warning ( push )
#line 19 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
#pragma warning ( disable : 4324 4514 4574 4710 4793 4820 4995 4996 28719 28726 28727 )













#line 35 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
 void __cdecl _swab(
 char* _Buf1,
 char* _Buf2,
 int _SizeInBytes
 );










#line 56 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
 __declspec(noreturn) void __cdecl exit( int _Code);
 __declspec(noreturn) void __cdecl _exit( int _Code);
 __declspec(noreturn) void __cdecl _Exit( int _Code);
 __declspec(noreturn) void __cdecl quick_exit( int _Code);
 __declspec(noreturn) void __cdecl abort(void);



#line 66 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
 unsigned int __cdecl _set_abort_behavior(
 unsigned int _Flags,
 unsigned int _Mask
 );




#line 76 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
 typedef int ( __cdecl* _onexit_t)(void);







#line 144 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
 int __cdecl atexit(void (__cdecl*)(void));
 _onexit_t __cdecl _onexit( _onexit_t _Func);

int __cdecl at_quick_exit(void (__cdecl*)(void));









#line 158 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
 typedef void (__cdecl* _purecall_handler)(void);


 typedef void (__cdecl* _invalid_parameter_handler)(
 wchar_t const*,
 wchar_t const*,
 wchar_t const*,
 unsigned int,
 uintptr_t
 );


 _purecall_handler __cdecl _set_purecall_handler(
 _purecall_handler _Handler
 );

 _purecall_handler __cdecl _get_purecall_handler(void);


 _invalid_parameter_handler __cdecl _set_invalid_parameter_handler(
 _invalid_parameter_handler _Handler
 );

 _invalid_parameter_handler __cdecl _get_invalid_parameter_handler(void);

 _invalid_parameter_handler __cdecl _set_thread_local_invalid_parameter_handler(
 _invalid_parameter_handler _Handler
 );

 _invalid_parameter_handler __cdecl _get_thread_local_invalid_parameter_handler(void);







#line 211 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
 int __cdecl _set_error_mode( int _Mode);



#line 217 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
 int* __cdecl _errno(void);

 errno_t __cdecl _set_errno( int _Value);
 errno_t __cdecl _get_errno( int* _Value);

 unsigned long* __cdecl __doserrno(void);

#line 225 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
 errno_t __cdecl _set_doserrno( unsigned long _Value);
 errno_t __cdecl _get_doserrno( unsigned long * _Value);


 char** __cdecl __sys_errlist(void);

#line 232 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
 int * __cdecl __sys_nerr(void);

#line 235 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
 void __cdecl perror( char const* _ErrMsg);




#line 238 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
 char** __cdecl __p__pgmptr (void);
 wchar_t** __cdecl __p__wpgmptr(void);
 int* __cdecl __p__fmode (void);



#line 257 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
 errno_t __cdecl _get_pgmptr ( char** _Value);


 errno_t __cdecl _get_wpgmptr( wchar_t** _Value);

 errno_t __cdecl _set_fmode ( int _Mode );

 errno_t __cdecl _get_fmode ( int* _PMode);








typedef struct _div_t
{
 int quot;
 int rem;
} div_t;

typedef struct _ldiv_t
{
 long quot;
 long rem;
} ldiv_t;

typedef struct _lldiv_t
{
 long long quot;
 long long rem;
} lldiv_t;

 int __cdecl abs ( int _Number);
 long __cdecl labs ( long _Number);
 long long __cdecl llabs ( long long _Number);
 __int64 __cdecl _abs64( __int64 _Number);

 unsigned short __cdecl _byteswap_ushort( unsigned short _Number);
 unsigned long __cdecl _byteswap_ulong ( unsigned long _Number);
 unsigned __int64 __cdecl _byteswap_uint64( unsigned __int64 _Number);

 div_t __cdecl div ( int _Numerator, int _Denominator);
 ldiv_t __cdecl ldiv ( long _Numerator, long _Denominator);
 lldiv_t __cdecl lldiv( long long _Numerator, long long _Denominator);



#pragma warning ( push )
#line 309 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
#pragma warning ( disable : 6540 )

#line 310 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
unsigned int __cdecl _rotl(
 unsigned int _Value,
 int _Shift
 );


unsigned long __cdecl _lrotl(
 unsigned long _Value,
 int _Shift
 );

unsigned __int64 __cdecl _rotl64(
 unsigned __int64 _Value,
 int _Shift
 );

unsigned int __cdecl _rotr(
 unsigned int _Value,
 int _Shift
 );


unsigned long __cdecl _lrotr(
 unsigned long _Value,
 int _Shift
 );

unsigned __int64 __cdecl _rotr64(
 unsigned __int64 _Value,
 int _Shift
 );

#pragma warning ( pop )





#line 349 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
 void __cdecl srand( unsigned int _Seed);

 int __cdecl rand(void);












#line 393 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
#pragma pack ( push , 4 )
#line 395 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
 typedef struct
 {
 unsigned char ld[10];
 } _LDOUBLE;
#pragma pack ( pop )



#line 414 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
typedef struct
{
 double x;
} _CRT_DOUBLE;

typedef struct
{
 float f;
} _CRT_FLOAT;


#pragma push_macro ( "long" )

#line 428 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
typedef struct
{
 long double x;
} _LONGDOUBLE;

#pragma pop_macro ( "long" )

#pragma pack ( push , 4 )
#line 437 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
typedef struct
{
 unsigned char ld12[12];
} _LDBL12;
#pragma pack ( pop )








#line 442 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
 double __cdecl atof ( char const* _String);
 int __cdecl atoi ( char const* _String);
 long __cdecl atol ( char const* _String);
 long long __cdecl atoll ( char const* _String);
 __int64 __cdecl _atoi64( char const* _String);

 double __cdecl _atof_l ( char const* _String, _locale_t _Locale);
 int __cdecl _atoi_l ( char const* _String, _locale_t _Locale);
 long __cdecl _atol_l ( char const* _String, _locale_t _Locale);
 long long __cdecl _atoll_l ( char const* _String, _locale_t _Locale);
 __int64 __cdecl _atoi64_l( char const* _String, _locale_t _Locale);

 int __cdecl _atoflt ( _CRT_FLOAT* _Result, char const* _String);
 int __cdecl _atodbl ( _CRT_DOUBLE* _Result, char* _String);
 int __cdecl _atoldbl( _LDOUBLE* _Result, char* _String);


 int __cdecl _atoflt_l(
 _CRT_FLOAT* _Result,
 char const* _String,
 _locale_t _Locale
 );


 int __cdecl _atodbl_l(
 _CRT_DOUBLE* _Result,
 char* _String,
 _locale_t _Locale
 );



 int __cdecl _atoldbl_l(
 _LDOUBLE* _Result,
 char* _String,
 _locale_t _Locale
 );


 float __cdecl strtof(
 char const* _String,
 char** _EndPtr
 );


 float __cdecl _strtof_l(
 char const* _String,
 char** _EndPtr,
 _locale_t _Locale
 );


 double __cdecl strtod(
 char const* _String,
 char** _EndPtr
 );


 double __cdecl _strtod_l(
 char const* _String,
 char** _EndPtr,
 _locale_t _Locale
 );


 long double __cdecl strtold(
 char const* _String,
 char** _EndPtr
 );


 long double __cdecl _strtold_l(
 char const* _String,
 char** _EndPtr,
 _locale_t _Locale
 );


 long __cdecl strtol(
 char const* _String,
 char** _EndPtr,
 int _Radix
 );


 long __cdecl _strtol_l(
 char const* _String,
 char** _EndPtr,
 int _Radix,
 _locale_t _Locale
 );


 long long __cdecl strtoll(
 char const* _String,
 char** _EndPtr,
 int _Radix
 );


 long long __cdecl _strtoll_l(
 char const* _String,
 char** _EndPtr,
 int _Radix,
 _locale_t _Locale
 );


 unsigned long __cdecl strtoul(
 char const* _String,
 char** _EndPtr,
 int _Radix
 );


 unsigned long __cdecl _strtoul_l(
 char const* _String,
 char** _EndPtr,
 int _Radix,
 _locale_t _Locale
 );


 unsigned long long __cdecl strtoull(
 char const* _String,
 char** _EndPtr,
 int _Radix
 );


 unsigned long long __cdecl _strtoull_l(
 char const* _String,
 char** _EndPtr,
 int _Radix,
 _locale_t _Locale
 );


 __int64 __cdecl _strtoi64(
 char const* _String,
 char** _EndPtr,
 int _Radix
 );


 __int64 __cdecl _strtoi64_l(
 char const* _String,
 char** _EndPtr,
 int _Radix,
 _locale_t _Locale
 );


 unsigned __int64 __cdecl _strtoui64(
 char const* _String,
 char** _EndPtr,
 int _Radix
 );


 unsigned __int64 __cdecl _strtoui64_l(
 char const* _String,
 char** _EndPtr,
 int _Radix,
 _locale_t _Locale
 );










 errno_t __cdecl _itoa_s(
 int _Value,
 char* _Buffer,
 size_t _BufferCount,
 int _Radix
 );



 
 char* __cdecl _itoa( 
 int _Value, 
 char * _Buffer, 
 int _Radix 
);



 errno_t __cdecl _ltoa_s(
 long _Value,
 char* _Buffer,
 size_t _BufferCount,
 int _Radix
 );



 
 char* __cdecl _ltoa( 
 long _Value, 
 char * _Buffer, 
 int _Radix 
);



 errno_t __cdecl _ultoa_s(
 unsigned long _Value,
 char* _Buffer,
 size_t _BufferCount,
 int _Radix
 );



 
 char* __cdecl _ultoa( 
 unsigned long _Value, 
 char * _Buffer, 
 int _Radix 
);



 errno_t __cdecl _i64toa_s(
 __int64 _Value,
 char* _Buffer,
 size_t _BufferCount,
 int _Radix
 );



 char* __cdecl _i64toa(
 __int64 _Value,
 char* _Buffer,
 int _Radix
 );



 errno_t __cdecl _ui64toa_s(
 unsigned __int64 _Value,
 char* _Buffer,
 size_t _BufferCount,
 int _Radix
 );


 char* __cdecl _ui64toa(
 unsigned __int64 _Value,
 char* _Buffer,
 int _Radix
 );














#line 738 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
 errno_t __cdecl _ecvt_s(
 char* _Buffer,
 size_t _BufferCount,
 double _Value,
 int _DigitCount,
 int* _PtDec,
 int* _PtSign
 );




 char* __cdecl _ecvt(
 double _Value,
 int _DigitCount,
 int* _PtDec,
 int* _PtSign
 );



 errno_t __cdecl _fcvt_s(
 char* _Buffer,
 size_t _BufferCount,
 double _Value,
 int _FractionalDigitCount,
 int* _PtDec,
 int* _PtSign
 );





 char* __cdecl _fcvt(
 double _Value,
 int _FractionalDigitCount,
 int* _PtDec,
 int* _PtSign
 );


 errno_t __cdecl _gcvt_s(
 char* _Buffer,
 size_t _BufferCount,
 double _Value,
 int _DigitCount
 );




 char* __cdecl _gcvt(
 double _Value,
 int _DigitCount,
 char* _Buffer
 );













#line 841 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
 int __cdecl ___mb_cur_max_func(void);


 int __cdecl ___mb_cur_max_l_func(_locale_t _Locale);




#line 848 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
 int __cdecl mblen(
 char const* _Ch,
 size_t _MaxCount
 );


 int __cdecl _mblen_l(
 char const* _Ch,
 size_t _MaxCount,
 _locale_t _Locale
 );



 size_t __cdecl _mbstrlen(
 char const* _String
 );



 size_t __cdecl _mbstrlen_l(
 char const* _String,
 _locale_t _Locale
 );



 size_t __cdecl _mbstrnlen(
 char const* _String,
 size_t _MaxCount
 );



 size_t __cdecl _mbstrnlen_l(
 char const* _String,
 size_t _MaxCount,
 _locale_t _Locale
 );


 int __cdecl mbtowc(
 wchar_t* _DstCh,
 char const* _SrcCh,
 size_t _SrcSizeInBytes
 );


 int __cdecl _mbtowc_l(
 wchar_t* _DstCh,
 char const* _SrcCh,
 size_t _SrcSizeInBytes,
 _locale_t _Locale
 );


 errno_t __cdecl mbstowcs_s(
 size_t* _PtNumOfCharConverted,
 wchar_t* _DstBuf,
 size_t _SizeInWords,
 char const* _SrcBuf,
 size_t _MaxCount
 );



 
 size_t __cdecl mbstowcs( 
 wchar_t * _Dest, 
 char const* _Source, 
 size_t _MaxCount 
);


 errno_t __cdecl _mbstowcs_s_l(
 size_t* _PtNumOfCharConverted,
 wchar_t* _DstBuf,
 size_t _SizeInWords,
 char const* _SrcBuf,
 size_t _MaxCount,
 _locale_t _Locale
 );



 
 size_t __cdecl _mbstowcs_l( 
 wchar_t * _Dest, 
 char const* _Source, 
 size_t _MaxCount, 
 _locale_t _Locale 
);





 int __cdecl wctomb(
 char* _MbCh,
 wchar_t _WCh
 );


 int __cdecl _wctomb_l(
 char* _MbCh,
 wchar_t _WCh,
 _locale_t _Locale
 );



#line 976 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
 errno_t __cdecl wctomb_s(
 int* _SizeConverted,
 char* _MbCh,
 rsize_t _SizeInBytes,
 wchar_t _WCh
 );



#line 986 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
 errno_t __cdecl _wctomb_s_l(
 int* _SizeConverted,
 char* _MbCh,
 size_t _SizeInBytes,
 wchar_t _WCh,
 _locale_t _Locale);


 errno_t __cdecl wcstombs_s(
 size_t* _PtNumOfCharConverted,
 char* _Dst,
 size_t _DstSizeInBytes,
 wchar_t const* _Src,
 size_t _MaxCountInBytes
 );



 
 size_t __cdecl wcstombs( 
 char * _Dest, 
 wchar_t const* _Source, 
 size_t _MaxCount 
);


 errno_t __cdecl _wcstombs_s_l(
 size_t* _PtNumOfCharConverted,
 char* _Dst,
 size_t _DstSizeInBytes,
 wchar_t const* _Src,
 size_t _MaxCountInBytes,
 _locale_t _Locale
 );



 
 size_t __cdecl _wcstombs_l( 
 char * _Dest, 
 wchar_t const* _Source, 
 size_t _MaxCount, 
 _locale_t _Locale 
);












#line 1061 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
#pragma push_macro ( "_fullpath" )



 __declspec(allocator) char* __cdecl _fullpath(
 char* _Buffer,
 char const* _Path,
 size_t _BufferCount
 );

#pragma pop_macro ( "_fullpath" )


#line 1075 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
 errno_t __cdecl _makepath_s(
 char* _Buffer,
 size_t _BufferCount,
 char const* _Drive,
 char const* _Dir,
 char const* _Filename,
 char const* _Ext
 );



 
 void __cdecl _makepath( 
 char * _Buffer, 
 char const* _Drive, 
 char const* _Dir, 
 char const* _Filename, 
 char const* _Ext 
);


 void __cdecl _splitpath(
 char const* _FullPath,
 char* _Drive,
 char* _Dir,
 char* _Filename,
 char* _Ext
 );


 errno_t __cdecl _splitpath_s(
 char const* _FullPath,
 char* _Drive,
 size_t _DriveCount,
 char* _Dir,
 size_t _DirCount,
 char* _Filename,
 size_t _FilenameCount,
 char* _Ext,
 size_t _ExtCount
 );






#line 1129 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
 errno_t __cdecl getenv_s(
 size_t* _RequiredCount,
 char* _Buffer,
 rsize_t _BufferCount,
 char const* _VarName
 );





 int* __cdecl __p___argc (void);
 char*** __cdecl __p___argv (void);
 wchar_t*** __cdecl __p___wargv(void);


#line 1157 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
 char*** __cdecl __p__environ (void);
 wchar_t*** __cdecl __p__wenviron(void);










#line 1182 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
 char* __cdecl getenv(
 char const* _VarName
 );





#line 1199 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
 errno_t __cdecl _dupenv_s(
 char** _Buffer,
 size_t* _BufferCount,
 char const* _VarName
 );


#line 1210 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
 int __cdecl system(
 char const* _Command
 );



#pragma warning ( push )
#line 1218 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
#pragma warning ( disable : 6540 )


#line 1219 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
 int __cdecl _putenv(
 char const* _EnvString
 );


 errno_t __cdecl _putenv_s(
 char const* _Name,
 char const* _Value
 );

#pragma warning ( pop )

#line 1232 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
 errno_t __cdecl _searchenv_s(
 char const* _Filename,
 char const* _VarName,
 char* _Buffer,
 size_t _BufferCount
 );



 
 void __cdecl _searchenv( 
 char const* _Filename, 
 char const* _VarName, 
 char * _Buffer 
);


 __declspec(deprecated( "This function or variable has been superceded by newer library or operating system functionality. Consider using SetErrorMode instead. See online help for details."))
 void __cdecl _seterrormode(
 int _Mode
 );

 __declspec(deprecated( "This function or variable has been superceded by newer library or operating system functionality. Consider using Beep instead. See online help for details."))
 void __cdecl _beep(
 unsigned _Frequency,
 unsigned _Duration
 );

 __declspec(deprecated( "This function or variable has been superceded by newer library or operating system functionality. Consider using Sleep instead. See online help for details."))
 void __cdecl _sleep(
 unsigned long _Duration
 );













#line 1356 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/stdlib.h"
#pragma warning ( pop ) 
#line 15 "src/ldo.c"
#line 1 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/string.h"








#line 1 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_memory.h"











#line 1 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_memcpy_s.h"








#line 1 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/errno.h"









#line 13 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/errno.h"
#pragma warning ( push )
#line 15 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/errno.h"
#pragma warning ( disable : 4324 4514 4574 4710 4793 4820 4995 4996 28719 28726 28727 )






#line 23 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/errno.h"
 int* __cdecl _errno(void);

 errno_t __cdecl _set_errno( int _Value);
 errno_t __cdecl _get_errno( int* _Value);

 unsigned long* __cdecl __doserrno(void);

#line 31 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/errno.h"
 errno_t __cdecl _set_doserrno( unsigned long _Value);
 errno_t __cdecl _get_doserrno( unsigned long * _Value);















#line 131 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/errno.h"
#pragma warning ( pop ) 
#line 12 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_memcpy_s.h"
#line 1 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/vcruntime_string.h"









#line 11 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/vcruntime_string.h"
#pragma warning ( push )
#line 13 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/vcruntime_string.h"
#pragma warning ( disable : 4514 4820 )







#line 16 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/vcruntime_string.h"
 void* __cdecl memchr(
 void const* _Buf,
 int _Val,
 size_t _MaxCount
 );


int __cdecl memcmp(
 void const* _Buf1,
 void const* _Buf2,
 size_t _Size
 );




void* __cdecl memcpy(
 void* _Dst,
 void const* _Src,
 size_t _Size
 );


 void* __cdecl memmove(
 void* _Dst,
 void const* _Src,
 size_t _Size
 );



void* __cdecl memset(
 void* _Dst,
 int _Val,
 size_t _Size
 );


 char* __cdecl strchr(
 char const* _Str,
 int _Val
 );


 char* __cdecl strrchr(
 char const* _Str,
 int _Ch
 );


 char* __cdecl strstr(
 char const* _Str,
 char const* _SubStr
 );



 wchar_t* __cdecl wcschr(
 wchar_t const* _Str,
 wchar_t _Ch
 );


 wchar_t* __cdecl wcsrchr(
 wchar_t const* _Str,
 wchar_t _Ch
 );



 wchar_t* __cdecl wcsstr(
 wchar_t const* _Str,
 wchar_t const* _SubStr
 );






#line 112 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/vcruntime_string.h"
#pragma warning ( pop ) 
#line 13 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_memcpy_s.h"

#line 13 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_memcpy_s.h"
#pragma warning ( push )
#line 15 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_memcpy_s.h"
#pragma warning ( disable : 4324 4514 4574 4710 4793 4820 4995 4996 28719 28726 28727 )









#line 36 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_memcpy_s.h"
 static __inline errno_t __cdecl memcpy_s(
 void* const _Destination,
 rsize_t const _DestinationSize,
 void const* const _Source,
 rsize_t const _SourceSize
 )
 {
 if (_SourceSize == 0)
 {
 return 0;
 }

 { int _Expr_val=!!(_Destination != ((void *)0)); if (!(_Expr_val)) { (*_errno()) = 22; _invalid_parameter_noinfo(); return 22; } };
 if (_Source == ((void *)0) || _DestinationSize < _SourceSize)
 {
 memset(_Destination, 0, _DestinationSize);

 { int _Expr_val=!!(_Source != ((void *)0)); if (!(_Expr_val)) { (*_errno()) = 22; _invalid_parameter_noinfo(); return 22; } };
 { int _Expr_val=!!(_DestinationSize >= _SourceSize); if (!(_Expr_val)) { (*_errno()) = 34; _invalid_parameter_noinfo(); return 34; } };


 return 22;
 }
 memcpy(_Destination, _Source, _SourceSize);
 return 0;
 }


 static __inline errno_t __cdecl memmove_s(
 void* const _Destination,
 rsize_t const _DestinationSize,
 void const* const _Source,
 rsize_t const _SourceSize
 )
 {
 if (_SourceSize == 0)
 {
 return 0;
 }

 { int _Expr_val=!!(_Destination != ((void *)0)); if (!(_Expr_val)) { (*_errno()) = 22; _invalid_parameter_noinfo(); return 22; } };
 { int _Expr_val=!!(_Source != ((void *)0)); if (!(_Expr_val)) { (*_errno()) = 22; _invalid_parameter_noinfo(); return 22; } };
 { int _Expr_val=!!(_DestinationSize >= _SourceSize); if (!(_Expr_val)) { (*_errno()) = 34; _invalid_parameter_noinfo(); return 34; } };

 memmove(_Destination, _Source, _SourceSize);
 return 0;
 }




#line 90 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_memcpy_s.h"
#pragma warning ( pop ) 

#line 15 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_memory.h"

#pragma warning ( push )
#line 18 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_memory.h"
#pragma warning ( disable : 4324 4514 4574 4710 4793 4820 4995 4996 28719 28726 28727 )








#line 22 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_memory.h"
 int __cdecl _memicmp(
 void const* _Buf1,
 void const* _Buf2,
 size_t _Size
 );


 int __cdecl _memicmp_l(
 void const* _Buf1,
 void const* _Buf2,
 size_t _Size,
 _locale_t _Locale
 );


















#line 121 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_memory.h"
#pragma warning ( pop ) 
#line 14 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/string.h"
#line 1 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_wstring.h"










#line 13 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_wstring.h"
#pragma warning ( push )
#line 15 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_wstring.h"
#pragma warning ( disable : 4324 4514 4574 4710 4793 4820 4995 4996 28719 28726 28727 )














 errno_t __cdecl wcscat_s(
 wchar_t* _Destination,
 rsize_t _SizeInWords,
 wchar_t const* _Source
 );


 errno_t __cdecl wcscpy_s(
 wchar_t* _Destination,
 rsize_t _SizeInWords,
 wchar_t const* _Source
 );


 errno_t __cdecl wcsncat_s(
 wchar_t* _Destination,
 rsize_t _SizeInWords,
 wchar_t const* _Source,
 rsize_t _MaxCount
 );


 errno_t __cdecl wcsncpy_s(
 wchar_t* _Destination,
 rsize_t _SizeInWords,
 wchar_t const* _Source,
 rsize_t _MaxCount
 );


 wchar_t* __cdecl wcstok_s(
 wchar_t* _String,
 wchar_t const* _Delimiter,
 wchar_t** _Context
 );











#line 81 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_wstring.h"
 __declspec(allocator) wchar_t* __cdecl _wcsdup(
 wchar_t const* _String
 );






#line 100 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_wstring.h"
 
 wchar_t* __cdecl wcscat( 
 wchar_t * _Destination, 
 wchar_t const* _Source 
);


#line 106 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_wstring.h"
 int __cdecl wcscmp(
 wchar_t const* _String1,
 wchar_t const* _String2
 );



 
 wchar_t* __cdecl wcscpy( 
 wchar_t * _Destination, 
 wchar_t const* _Source 
);


 size_t __cdecl wcscspn(
 wchar_t const* _String,
 wchar_t const* _Control
 );


 size_t __cdecl wcslen(
 wchar_t const* _String
 );




 size_t __cdecl wcsnlen(
 wchar_t const* _Source,
 size_t _MaxCount
 );





#line 151 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_wstring.h"
 static __inline size_t __cdecl wcsnlen_s(
 wchar_t const* _Source,
 size_t _MaxCount
 )
 {
 return (_Source == 0) ? 0 : wcsnlen(_Source, _MaxCount);
 }




#line 170 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_wstring.h"
 
 wchar_t* __cdecl wcsncat( 
 wchar_t * _Destination, 
 wchar_t const* _Source, 
 size_t _Count 
);


 int __cdecl wcsncmp(
 wchar_t const* _String1,
 wchar_t const* _String2,
 size_t _MaxCount
 );



 
 wchar_t* __cdecl wcsncpy( 
 wchar_t * _Destination, 
 wchar_t const* _Source, 
 size_t _Count 
);


 wchar_t* __cdecl wcspbrk(
 wchar_t const* _String,
 wchar_t const* _Control
 );


 size_t __cdecl wcsspn(
 wchar_t const* _String,
 wchar_t const* _Control
 );


 wchar_t* __cdecl wcstok(
 wchar_t* _String,
 wchar_t const* _Delimiter,
 wchar_t** _Context
 );




#line 237 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_wstring.h"
 static __inline wchar_t* __cdecl _wcstok(
 wchar_t* const _String,
 wchar_t const* const _Delimiter
 )
 {
 return wcstok(_String, _Delimiter, 0);
 }








#line 263 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_wstring.h"
 wchar_t* __cdecl _wcserror(
 int _ErrorNumber
 );


 errno_t __cdecl _wcserror_s(
 wchar_t* _Buffer,
 size_t _SizeInWords,
 int _ErrorNumber
 );






 wchar_t* __cdecl __wcserror(
 wchar_t const* _String
 );

 errno_t __cdecl __wcserror_s(
 wchar_t* _Buffer,
 size_t _SizeInWords,
 wchar_t const* _ErrorMessage
 );



 int __cdecl _wcsicmp(
 wchar_t const* _String1,
 wchar_t const* _String2
 );

 int __cdecl _wcsicmp_l(
 wchar_t const* _String1,
 wchar_t const* _String2,
 _locale_t _Locale
 );

 int __cdecl _wcsnicmp(
 wchar_t const* _String1,
 wchar_t const* _String2,
 size_t _MaxCount
 );

 int __cdecl _wcsnicmp_l(
 wchar_t const* _String1,
 wchar_t const* _String2,
 size_t _MaxCount,
 _locale_t _Locale
 );

 errno_t __cdecl _wcsnset_s(
 wchar_t* _Destination,
 size_t _SizeInWords,
 wchar_t _Value,
 size_t _MaxCount
 );



 
 wchar_t* __cdecl _wcsnset( 
 wchar_t * _String, 
 wchar_t _Value, 
 size_t _MaxCount 
);

 wchar_t* __cdecl _wcsrev(
 wchar_t* _String
 );

 errno_t __cdecl _wcsset_s(
 wchar_t* _Destination,
 size_t _SizeInWords,
 wchar_t _Value
 );



 
 wchar_t* __cdecl _wcsset( 
 wchar_t * _String, 
 wchar_t _Value 
);

 errno_t __cdecl _wcslwr_s(
 wchar_t* _String,
 size_t _SizeInWords
 );



 
 wchar_t* __cdecl _wcslwr( 
 wchar_t * _String 
);


 errno_t __cdecl _wcslwr_s_l(
 wchar_t* _String,
 size_t _SizeInWords,
 _locale_t _Locale
 );



 
 wchar_t* __cdecl _wcslwr_l( 
 wchar_t * _String, 
 _locale_t _Locale 
);


 errno_t __cdecl _wcsupr_s(
 wchar_t* _String,
 size_t _Size
 );



 
 wchar_t* __cdecl _wcsupr( 
 wchar_t * _String 
);


 errno_t __cdecl _wcsupr_s_l(
 wchar_t* _String,
 size_t _Size,
 _locale_t _Locale
 );



 
 wchar_t* __cdecl _wcsupr_l( 
 wchar_t * _String, 
 _locale_t _Locale 
);



 size_t __cdecl wcsxfrm(
 wchar_t* _Destination,
 wchar_t const* _Source,
 size_t _MaxCount
 );



 size_t __cdecl _wcsxfrm_l(
 wchar_t* _Destination,
 wchar_t const* _Source,
 size_t _MaxCount,
 _locale_t _Locale
 );


 int __cdecl wcscoll(
 wchar_t const* _String1,
 wchar_t const* _String2
 );


 int __cdecl _wcscoll_l(
 wchar_t const* _String1,
 wchar_t const* _String2,
 _locale_t _Locale
 );


 int __cdecl _wcsicoll(
 wchar_t const* _String1,
 wchar_t const* _String2
 );


 int __cdecl _wcsicoll_l(
 wchar_t const* _String1,
 wchar_t const* _String2,
 _locale_t _Locale
 );


 int __cdecl _wcsncoll(
 wchar_t const* _String1,
 wchar_t const* _String2,
 size_t _MaxCount
 );


 int __cdecl _wcsncoll_l(
 wchar_t const* _String1,
 wchar_t const* _String2,
 size_t _MaxCount,
 _locale_t _Locale
 );


 int __cdecl _wcsnicoll(
 wchar_t const* _String1,
 wchar_t const* _String2,
 size_t _MaxCount
 );


 int __cdecl _wcsnicoll_l(
 wchar_t const* _String1,
 wchar_t const* _String2,
 size_t _MaxCount,
 _locale_t _Locale
 );






















#line 640 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/corecrt_wstring.h"
#pragma warning ( pop ) 
#line 15 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/string.h"


#line 18 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/string.h"
#pragma warning ( push )
#line 20 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/string.h"
#pragma warning ( disable : 4324 4514 4574 4710 4793 4820 4995 4996 28719 28726 28727 )









 errno_t __cdecl strcpy_s(
 char* _Destination,
 rsize_t _SizeInBytes,
 char const* _Source
 );


 errno_t __cdecl strcat_s(
 char* _Destination,
 rsize_t _SizeInBytes,
 char const* _Source
 );


 errno_t __cdecl strerror_s(
 char* _Buffer,
 size_t _SizeInBytes,
 int _ErrorNumber);


 errno_t __cdecl strncat_s(
 char* _Destination,
 rsize_t _SizeInBytes,
 char const* _Source,
 rsize_t _MaxCount
 );


 errno_t __cdecl strncpy_s(
 char* _Destination,
 rsize_t _SizeInBytes,
 char const* _Source,
 rsize_t _MaxCount
 );


 char* __cdecl strtok_s(
 char* _String,
 char const* _Delimiter,
 char** _Context
 );


#line 75 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/string.h"
 void* __cdecl _memccpy(
 void* _Dst,
 void const* _Src,
 int _Val,
 size_t _MaxCount
 );




#line 90 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/string.h"
 
 char* __cdecl strcat( 
 char * _Destination, 
 char const* _Source 
);



#line 98 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/string.h"
int __cdecl strcmp(
 char const* _Str1,
 char const* _Str2
 );


 int __cdecl _strcmpi(
 char const* _String1,
 char const* _String2
 );


 int __cdecl strcoll(
 char const* _String1,
 char const* _String2
 );


 int __cdecl _strcoll_l(
 char const* _String1,
 char const* _String2,
 _locale_t _Locale
 );



 
 char* __cdecl strcpy( 
 char * _Destination, 
 char const* _Source 
);


 size_t __cdecl strcspn(
 char const* _Str,
 char const* _Control
 );



#line 146 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/string.h"
 __declspec(allocator) char* __cdecl _strdup(
 char const* _Source
 );





#line 155 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/string.h"
 char* __cdecl _strerror(
 char const* _ErrorMessage
 );


 errno_t __cdecl _strerror_s(
 char* _Buffer,
 size_t _SizeInBytes,
 char const* _ErrorMessage
 );





 char* __cdecl strerror(
 int _ErrorMessage
 );




 int __cdecl _stricmp(
 char const* _String1,
 char const* _String2
 );


 int __cdecl _stricoll(
 char const* _String1,
 char const* _String2
 );


 int __cdecl _stricoll_l(
 char const* _String1,
 char const* _String2,
 _locale_t _Locale
 );


 int __cdecl _stricmp_l(
 char const* _String1,
 char const* _String2,
 _locale_t _Locale
 );


size_t __cdecl strlen(
 char const* _Str
 );


 errno_t __cdecl _strlwr_s(
 char* _String,
 size_t _Size
 );



 
 char* __cdecl _strlwr( 
 char * _String 
);


 errno_t __cdecl _strlwr_s_l(
 char* _String,
 size_t _Size,
 _locale_t _Locale
 );



 
 char* __cdecl _strlwr_l( 
 char * _String, 
 _locale_t _Locale 
);



 
 char* __cdecl strncat( 
 char * _Destination, 
 char const* _Source, 
 size_t _Count 
);


 int __cdecl strncmp(
 char const* _Str1,
 char const* _Str2,
 size_t _MaxCount
 );


 int __cdecl _strnicmp(
 char const* _String1,
 char const* _String2,
 size_t _MaxCount
 );


 int __cdecl _strnicmp_l(
 char const* _String1,
 char const* _String2,
 size_t _MaxCount,
 _locale_t _Locale
 );


 int __cdecl _strnicoll(
 char const* _String1,
 char const* _String2,
 size_t _MaxCount
 );


 int __cdecl _strnicoll_l(
 char const* _String1,
 char const* _String2,
 size_t _MaxCount,
 _locale_t _Locale
 );


 int __cdecl _strncoll(
 char const* _String1,
 char const* _String2,
 size_t _MaxCount
 );


 int __cdecl _strncoll_l(
 char const* _String1,
 char const* _String2,
 size_t _MaxCount,
 _locale_t _Locale
 );

 size_t __cdecl __strncnt(
 char const* _String,
 size_t _Count
 );



 
 char* __cdecl strncpy( 
 char * _Destination, 
 char const* _Source, 
 size_t _Count 
);




 size_t __cdecl strnlen(
 char const* _String,
 size_t _MaxCount
 );





#line 357 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/string.h"
 static __inline size_t __cdecl strnlen_s(
 char const* _String,
 size_t _MaxCount
 )
 {
 return _String == 0 ? 0 : strnlen(_String, _MaxCount);
 }



#line 376 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/string.h"
 errno_t __cdecl _strnset_s(
 char* _String,
 size_t _SizeInBytes,
 int _Value,
 size_t _MaxCount
 );



 
 char* __cdecl _strnset( 
 char * _Destination, 
 int _Value, 
 size_t _Count 
);


 char* __cdecl strpbrk(
 char const* _Str,
 char const* _Control
 );

 char* __cdecl _strrev(
 char* _Str
 );


 errno_t __cdecl _strset_s(
 char* _Destination,
 size_t _DestinationSize,
 int _Value
 );



 
 char* __cdecl _strset( 
 char * _Destination, 
 int _Value 
);


 size_t __cdecl strspn(
 char const* _Str,
 char const* _Control
 );


 char* __cdecl strtok(
 char* _String,
 char const* _Delimiter
 );


 errno_t __cdecl _strupr_s(
 char* _String,
 size_t _Size
 );



 
 char* __cdecl _strupr( 
 char * _String 
);


 errno_t __cdecl _strupr_s_l(
 char* _String,
 size_t _Size,
 _locale_t _Locale
 );



 
 char* __cdecl _strupr_l( 
 char * _String, 
 _locale_t _Locale 
);



 size_t __cdecl strxfrm(
 char* _Destination,
 char const* _Source,
 size_t _MaxCount
 );



 size_t __cdecl _strxfrm_l(
 char* _Destination,
 char const* _Source,
 size_t _MaxCount,
 _locale_t _Locale
 );











#line 585 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/string.h"
#pragma warning ( pop ) 
#line 16 "src/ldo.c"

#line 1 "src/lua.h"




#line 1 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/stdarg.h"









#line 12 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/stdarg.h"
#pragma warning ( push )
#line 14 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/stdarg.h"
#pragma warning ( disable : 4514 4820 )










#line 24 "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/include/stdarg.h"
#pragma warning ( pop ) 
#line 13 "src/lua.h"


#line 1 "src/luaconf.h"







































































































































































































































#line 17 "src/lua.h"

















#line 55 "src/lua.h"
typedef struct lua_State lua_State;















#line 87 "src/lua.h"
typedef double lua_Number;



typedef long long lua_Integer;


typedef unsigned long long lua_Unsigned;


typedef ptrdiff_t lua_KContext;



typedef int (*lua_CFunction) (lua_State *L);


typedef int (*lua_KFunction) (lua_State *L, int status, lua_KContext ctx);



typedef const char * (*lua_Reader) (lua_State *L, void *ud, size_t *sz);

typedef int (*lua_Writer) (lua_State *L, const void *p, size_t sz, void *ud);



typedef void * (*lua_Alloc) (void *ud, void *ptr, size_t osize, size_t nsize);



typedef void (*lua_WarnFunction) (void *ud, const char *msg, int tocont);



typedef struct lua_Debug lua_Debug;



typedef void (*lua_Hook) (lua_State *L, lua_Debug *ar);






#line 152 "src/lua.h"
extern const char lua_ident[];



 extern lua_State *(lua_newstate) (lua_Alloc f, void *ud);
 extern void (lua_close) (lua_State *L);
 extern lua_State *(lua_newthread) (lua_State *L);
 extern int (lua_closethread) (lua_State *L, lua_State *from);
 extern int (lua_resetthread) (lua_State *L);

 extern lua_CFunction (lua_atpanic) (lua_State *L, lua_CFunction panicf);


 extern lua_Number (lua_version) (lua_State *L);



 extern int (lua_absindex) (lua_State *L, int idx);
 extern int (lua_gettop) (lua_State *L);
 extern void (lua_settop) (lua_State *L, int idx);
 extern void (lua_pushvalue) (lua_State *L, int idx);
 extern void (lua_rotate) (lua_State *L, int idx, int n);
 extern void (lua_copy) (lua_State *L, int fromidx, int toidx);
 extern int (lua_checkstack) (lua_State *L, int n);

 extern void (lua_xmove) (lua_State *from, lua_State *to, int n);




 extern int (lua_isnumber) (lua_State *L, int idx);
 extern int (lua_isstring) (lua_State *L, int idx);
 extern int (lua_iscfunction) (lua_State *L, int idx);
 extern int (lua_isinteger) (lua_State *L, int idx);
 extern int (lua_isuserdata) (lua_State *L, int idx);
 extern int (lua_type) (lua_State *L, int idx);
 extern const char *(lua_typename) (lua_State *L, int tp);

 extern lua_Number (lua_tonumberx) (lua_State *L, int idx, int *isnum);
 extern lua_Integer (lua_tointegerx) (lua_State *L, int idx, int *isnum);
 extern int (lua_toboolean) (lua_State *L, int idx);
 extern const char *(lua_tolstring) (lua_State *L, int idx, size_t *len);
 extern lua_Unsigned (lua_rawlen) (lua_State *L, int idx);
 extern lua_CFunction (lua_tocfunction) (lua_State *L, int idx);
 extern void *(lua_touserdata) (lua_State *L, int idx);
 extern lua_State *(lua_tothread) (lua_State *L, int idx);
 extern const void *(lua_topointer) (lua_State *L, int idx);





#line 230 "src/lua.h"
 extern void (lua_arith) (lua_State *L, int op);


#line 236 "src/lua.h"
 extern int (lua_rawequal) (lua_State *L, int idx1, int idx2);
 extern int (lua_compare) (lua_State *L, int idx1, int idx2, int op);



 extern void (lua_pushnil) (lua_State *L);
 extern void (lua_pushnumber) (lua_State *L, lua_Number n);
 extern void (lua_pushinteger) (lua_State *L, lua_Integer n);
 extern const char *(lua_pushlstring) (lua_State *L, const char *s, size_t len);
 extern const char *(lua_pushstring) (lua_State *L, const char *s);
 extern const char *(lua_pushvfstring) (lua_State *L, const char *fmt,
 va_list argp);
 extern const char *(lua_pushfstring) (lua_State *L, const char *fmt, ...);
 extern void (lua_pushcclosure) (lua_State *L, lua_CFunction fn, int n);
 extern void (lua_pushboolean) (lua_State *L, int b);
 extern void (lua_pushlightuserdata) (lua_State *L, void *p);
 extern int (lua_pushthread) (lua_State *L);



 extern int (lua_getglobal) (lua_State *L, const char *name);
 extern int (lua_gettable) (lua_State *L, int idx);
 extern int (lua_getfield) (lua_State *L, int idx, const char *k);
 extern int (lua_geti) (lua_State *L, int idx, lua_Integer n);
 extern int (lua_rawget) (lua_State *L, int idx);
 extern int (lua_rawgeti) (lua_State *L, int idx, lua_Integer n);
 extern int (lua_rawgetp) (lua_State *L, int idx, const void *p);

 extern void (lua_createtable) (lua_State *L, int narr, int nrec);
 extern void *(lua_newuserdatauv) (lua_State *L, size_t sz, int nuvalue);
 extern int (lua_getmetatable) (lua_State *L, int objindex);
 extern int (lua_getiuservalue) (lua_State *L, int idx, int n);



 extern void (lua_setglobal) (lua_State *L, const char *name);
 extern void (lua_settable) (lua_State *L, int idx);
 extern void (lua_setfield) (lua_State *L, int idx, const char *k);
 extern void (lua_seti) (lua_State *L, int idx, lua_Integer n);
 extern void (lua_rawset) (lua_State *L, int idx);
 extern void (lua_rawseti) (lua_State *L, int idx, lua_Integer n);
 extern void (lua_rawsetp) (lua_State *L, int idx, const void *p);
 extern int (lua_setmetatable) (lua_State *L, int objindex);
 extern int (lua_setiuservalue) (lua_State *L, int idx, int n);



 extern void (lua_callk) (lua_State *L, int nargs, int nresults,
 lua_KContext ctx, lua_KFunction k);

#line 295 "src/lua.h"
 extern int (lua_pcallk) (lua_State *L, int nargs, int nresults, int errfunc,
 lua_KContext ctx, lua_KFunction k);

#line 299 "src/lua.h"
 extern int (lua_load) (lua_State *L, lua_Reader reader, void *dt,
 const char *chunkname, const char *mode);

 extern int (lua_dump) (lua_State *L, lua_Writer writer, void *data, int strip);



 extern int (lua_yieldk) (lua_State *L, int nresults, lua_KContext ctx,
 lua_KFunction k);
 extern int (lua_resume) (lua_State *L, lua_State *from, int narg,
 int *nres);
 extern int (lua_status) (lua_State *L);
 extern int (lua_isyieldable) (lua_State *L);




#line 317 "src/lua.h"
 extern void (lua_setwarnf) (lua_State *L, lua_WarnFunction f, void *ud);
 extern void (lua_warning) (lua_State *L, const char *msg, int tocont);





#line 341 "src/lua.h"
 extern int (lua_gc) (lua_State *L, int what, ...);




 extern int (lua_error) (lua_State *L);

 extern int (lua_next) (lua_State *L, int idx);

 extern void (lua_concat) (lua_State *L, int n);
 extern void (lua_len) (lua_State *L, int idx);

 extern size_t (lua_stringtonumber) (lua_State *L, const char *s);

 extern lua_Alloc (lua_getallocf) (lua_State *L, void **ud);
 extern void (lua_setallocf) (lua_State *L, lua_Alloc f, void *ud);

 extern void (lua_toclose) (lua_State *L, int idx);
 extern void (lua_closeslot) (lua_State *L, int idx);




































#line 455 "src/lua.h"
 extern int (lua_getstack) (lua_State *L, int level, lua_Debug *ar);
 extern int (lua_getinfo) (lua_State *L, const char *what, lua_Debug *ar);
 extern const char *(lua_getlocal) (lua_State *L, const lua_Debug *ar, int n);
 extern const char *(lua_setlocal) (lua_State *L, const lua_Debug *ar, int n);
 extern const char *(lua_getupvalue) (lua_State *L, int funcindex, int n);
 extern const char *(lua_setupvalue) (lua_State *L, int funcindex, int n);

 extern void *(lua_upvalueid) (lua_State *L, int fidx, int n);
 extern void (lua_upvaluejoin) (lua_State *L, int fidx1, int n1,
 int fidx2, int n2);

 extern void (lua_sethook) (lua_State *L, lua_Hook func, int mask, int count);
 extern lua_Hook (lua_gethook) (lua_State *L);
 extern int (lua_gethookmask) (lua_State *L);
 extern int (lua_gethookcount) (lua_State *L);

 extern int (lua_setcstacklimit) (lua_State *L, unsigned int limit);

struct lua_Debug {
 int event;
 const char *name;
 const char *namewhat;
 const char *what;
 const char *source;
 size_t srclen;
 int currentline;
 int linedefined;
 int lastlinedefined;
 unsigned char nups;
 unsigned char nparams;
 char isvararg;
 char istailcall;
 unsigned short ftransfer;
 unsigned short ntransfer;
 char short_src[ 60];

 struct CallInfo *i_ci;
};







#line 18 "src/ldo.c"

#line 1 "src/lapi.h"




#line 1 "src/llimits.h"






#line 1 "src/lua.h"



#line 16 "src/llimits.h"



#line 27 "src/llimits.h"
typedef size_t lu_mem;
typedef ptrdiff_t l_mem;



#line 33 "src/llimits.h"
typedef unsigned char lu_byte;
typedef signed char ls_byte;



























#line 92 "src/llimits.h"
typedef double l_uacNumber;
typedef long long l_uacInt;































#line 200 "src/llimits.h"
typedef unsigned int l_uint32;

#line 204 "src/llimits.h"
typedef l_uint32 Instruction;



















































#line 12 "src/lapi.h"
#line 1 "src/lstate.h"



#line 1 "src/lua.h"



#line 11 "src/lstate.h"



#line 11 "src/lstate.h"
typedef struct CallInfo CallInfo;


#line 1 "src/lobject.h"







#line 1 "src/llimits.h"


#line 16 "src/lobject.h"
#line 1 "src/lua.h"



#line 17 "src/lobject.h"
















#line 43 "src/lobject.h"
typedef union Value {
 struct GCObject *gc;
 void *p;
 lua_CFunction f;
 lua_Integer i;
 lua_Number n;

 lu_byte ub;
} Value;





#line 66 "src/lobject.h"
typedef struct TValue {
 Value value_; lu_byte tt_;
} TValue;








































#line 138 "src/lobject.h"
typedef union StackValue {
 TValue val;
 struct {
 Value value_; lu_byte tt_;
 unsigned short delta;
 } tbclist;
} StackValue;



typedef StackValue *StkId;



typedef union {
 StkId p;
 ptrdiff_t offset;
} StkIdRel;





































































#line 289 "src/lobject.h"
typedef struct GCObject {
 struct GCObject *next; lu_byte tt; lu_byte marked;
} GCObject;









































#line 381 "src/lobject.h"
typedef struct TString {
 struct GCObject *next; lu_byte tt; lu_byte marked;
 lu_byte extra;
 lu_byte shrlen;
 unsigned int hash;
 union {
 size_t lnglen;
 struct TString *hnext;
 } u;
 char contents[1];
} TString;




























#line 448 "src/lobject.h"
typedef union UValue {
 TValue uv;
 lua_Number n; double u; void *s; lua_Integer i; long l;
} UValue;



typedef struct Udata {
 struct GCObject *next; lu_byte tt; lu_byte marked;
 unsigned short nuvalue;
 size_t len;
 struct Table *metatable;
 GCObject *gclist;
 UValue uv[1];
} Udata;



typedef struct Udata0 {
 struct GCObject *next; lu_byte tt; lu_byte marked;
 unsigned short nuvalue;
 size_t len;
 struct Table *metatable;
 union { lua_Number n; double u; void *s; lua_Integer i; long l;} bindata;
} Udata0;
















#line 510 "src/lobject.h"
typedef struct Upvaldesc {
 TString *name;
 lu_byte instack;
 lu_byte idx;
 lu_byte kind;
} Upvaldesc;



typedef struct LocVar {
 TString *varname;
 int startpc;
 int endpc;
} LocVar;



typedef struct AbsLineInfo {
 int pc;
 int line;
} AbsLineInfo;


typedef struct Proto {
 struct GCObject *next; lu_byte tt; lu_byte marked;
 lu_byte numparams;
 lu_byte is_vararg;
 lu_byte maxstacksize;
 int sizeupvalues;
 int sizek;
 int sizecode;
 int sizelineinfo;
 int sizep;
 int sizelocvars;
 int sizeabslineinfo;
 int linedefined;
 int lastlinedefined;
 TValue *k;
 Instruction *code;
 struct Proto **p;
 Upvaldesc *upvalues;
 ls_byte *lineinfo;
 AbsLineInfo *abslineinfo;
 LocVar *locvars;
 TString *source;
 GCObject *gclist;
} Proto;





















#line 624 "src/lobject.h"
typedef struct UpVal {
 struct GCObject *next; lu_byte tt; lu_byte marked;
 union {
 TValue *p;
 ptrdiff_t offset;
 } v;
 union {
 struct {
 struct UpVal *next;
 struct UpVal **previous;
 } open;
 TValue value;
 } u;
} UpVal;




#line 648 "src/lobject.h"
typedef struct CClosure {
 struct GCObject *next; lu_byte tt; lu_byte marked; lu_byte nupvalues; GCObject *gclist;
 lua_CFunction f;
 TValue upvalue[1];
} CClosure;


typedef struct LClosure {
 struct GCObject *next; lu_byte tt; lu_byte marked; lu_byte nupvalues; GCObject *gclist;
 struct Proto *p;
 UpVal *upvals[1];
} LClosure;


typedef union Closure {
 CClosure c;
 LClosure l;
} Closure;















#line 692 "src/lobject.h"
typedef union Node {
 struct NodeKey {
 Value value_; lu_byte tt_;
 lu_byte key_tt;
 int next;
 Value key_val;
 } u;
 TValue i_val;
} Node;












#line 737 "src/lobject.h"
typedef struct Table {
 struct GCObject *next; lu_byte tt; lu_byte marked;
 lu_byte flags;
 lu_byte lsizenode;
 unsigned int alimit;
 TValue *array;
 Node *node;
 Node *lastfree;
 struct Table *metatable;
 GCObject *gclist;
} Table;






















#line 798 "src/lobject.h"
 extern int luaO_utf8esc (char *buff, unsigned long x);
 extern int luaO_ceillog2 (unsigned int x);
 extern int luaO_rawarith (lua_State *L, int op, const TValue *p1,
 const TValue *p2, TValue *res);
 extern void luaO_arith (lua_State *L, int op, const TValue *p1,
 const TValue *p2, StkId res);
 extern size_t luaO_str2num (const char *s, TValue *o);
 extern int luaO_hexavalue (int c);
 extern void luaO_tostring (lua_State *L, TValue *obj);
 extern const char *luaO_pushvfstring (lua_State *L, const char *fmt,
 va_list argp);
 extern const char *luaO_pushfstring (lua_State *L, const char *fmt, ...);
 extern void luaO_chunkid (char *out, const char *source, size_t srclen);



#line 18 "src/lstate.h"
#line 1 "src/ltm.h"




#line 1 "src/lobject.h"




#line 12 "src/ltm.h"
#line 1 "src/lstate.h"



#line 13 "src/ltm.h"



#line 13 "src/ltm.h"
typedef enum {
 TM_INDEX,
 TM_NEWINDEX,
 TM_GC,
 TM_MODE,
 TM_LEN,
 TM_EQ,
 TM_ADD,
 TM_SUB,
 TM_MUL,
 TM_MOD,
 TM_POW,
 TM_DIV,
 TM_IDIV,
 TM_BAND,
 TM_BOR,
 TM_BXOR,
 TM_SHL,
 TM_SHR,
 TM_UNM,
 TM_BNOT,
 TM_LT,
 TM_LE,
 TM_CONCAT,
 TM_CALL,
 TM_CLOSE,
 TM_N
} TMS;











#line 71 "src/ltm.h"
 extern const char *const luaT_typenames_[ ( ( 9+1)   + 2)];


 extern const char *luaT_objtypename (lua_State *L, const TValue *o);

 extern const TValue *luaT_gettm (Table *events, TMS event, TString *ename);
 extern const TValue *luaT_gettmbyobj (lua_State *L, const TValue *o,
 TMS event);
 extern void luaT_init (lua_State *L);

 extern void luaT_callTM (lua_State *L, const TValue *f, const TValue *p1,
 const TValue *p2, const TValue *p3);
 extern void luaT_callTMres (lua_State *L, const TValue *f,
 const TValue *p1, const TValue *p2, StkId p3);
 extern void luaT_trybinTM (lua_State *L, const TValue *p1, const TValue *p2,
 StkId res, TMS event);
 extern void luaT_tryconcatTM (lua_State *L);
 extern void luaT_trybinassocTM (lua_State *L, const TValue *p1,
 const TValue *p2, int inv, StkId res, TMS event);
 extern void luaT_trybiniTM (lua_State *L, const TValue *p1, lua_Integer i2,
 int inv, StkId res, TMS event);
 extern int luaT_callorderTM (lua_State *L, const TValue *p1,
 const TValue *p2, TMS event);
 extern int luaT_callorderiTM (lua_State *L, const TValue *p1, int v2,
 int inv, int isfloat, TMS event);

 extern void luaT_adjustvarargs (lua_State *L, int nfixparams,
 CallInfo *ci, const Proto *p);
 extern void luaT_getvarargs (lua_State *L, CallInfo *ci,
 StkId where, int wanted);


#line 19 "src/lstate.h"
#line 1 "src/lzio.h"




#line 1 "src/lua.h"



#line 12 "src/lzio.h"

#line 1 "src/lmem.h"





#line 1 "src/llimits.h"


#line 14 "src/lmem.h"
#line 1 "src/lua.h"



#line 15 "src/lmem.h"



















#line 76 "src/lmem.h"
 extern void __declspec(noreturn) luaM_toobig (lua_State *L);


 extern void *luaM_realloc_ (lua_State *L, void *block, size_t oldsize,
 size_t size);
 extern void *luaM_saferealloc_ (lua_State *L, void *block, size_t oldsize,
 size_t size);
 extern void luaM_free_ (lua_State *L, void *block, size_t osize);
 extern void *luaM_growaux_ (lua_State *L, void *block, int nelems,
 int *size, int size_elem, int limit,
 const char *what);
 extern void *luaM_shrinkvector_ (lua_State *L, void *block, int *nelem,
 int final_n, int size_elem);
 extern void *luaM_malloc_ (lua_State *L, size_t size, int tag);


#line 14 "src/lzio.h"



typedef struct Zio ZIO;



typedef struct Mbuffer {
 char *buffer;
 size_t n;
 size_t buffsize;
} Mbuffer;








#line 45 "src/lzio.h"
 extern void luaZ_init (lua_State *L, ZIO *z, lua_Reader reader,
 void *data);
 extern size_t luaZ_read (ZIO* z, void *b, size_t n);





struct Zio {
 size_t n;
 const char *p;
 lua_Reader reader;
 void *data;
 lua_State *L;
};


 extern int luaZ_fill (ZIO *z);

#line 20 "src/lstate.h"

























#line 118 "src/lstate.h"
struct lua_longjmp;



#line 1 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/signal.h"









#line 13 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/signal.h"
#pragma warning ( push )
#line 15 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/signal.h"
#pragma warning ( disable : 4324 4514 4574 4710 4793 4820 4995 4996 28719 28726 28727 )






#line 16 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/signal.h"
typedef int sig_atomic_t;

typedef void ( __cdecl* _crt_signal_t)(int);













#line 53 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/signal.h"
 void** __cdecl __pxcptinfoptrs(void);


#line 62 "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt/signal.h"
 _crt_signal_t __cdecl signal( int _Signal, _crt_signal_t _Function);

 int __cdecl raise( int _Signal);





#pragma warning ( pop ) 
#line 131 "src/lstate.h"











#line 153 "src/lstate.h"
typedef struct stringtable {
 TString **hash;
 int nuse;
 int size;
} stringtable;



struct CallInfo {
 StkIdRel func;
 StkIdRel top;
 struct CallInfo *previous, *next;
 union {
 struct {
 const Instruction *savedpc;
 volatile sig_atomic_t trap;
 int nextraargs;
 } l;
 struct {
 lua_KFunction k;
 ptrdiff_t old_errfunc;
 lua_KContext ctx;
 } c;
 } u;
 union {
 int funcidx;
 int nyield;
 int nres;
 struct {
 unsigned short ftransfer;
 unsigned short ntransfer;
 } transferinfo;
 } u2;
 short nresults;
 unsigned short callstatus;
};

















#line 249 "src/lstate.h"
typedef struct global_State {
 lua_Alloc frealloc;
 void *ud;
 l_mem totalbytes;
 l_mem GCdebt;
 lu_mem GCestimate;
 lu_mem lastatomic;
 stringtable strt;
 TValue l_registry;
 TValue nilvalue;
 unsigned int seed;
 lu_byte currentwhite;
 lu_byte gcstate;
 lu_byte gckind;
 lu_byte gcstopem;
 lu_byte genminormul;
 lu_byte genmajormul;
 lu_byte gcstp;
 lu_byte gcemergency;
 lu_byte gcpause;
 lu_byte gcstepmul;
 lu_byte gcstepsize;
 GCObject *allgc;
 GCObject **sweepgc;
 GCObject *finobj;
 GCObject *gray;
 GCObject *grayagain;
 GCObject *weak;
 GCObject *ephemeron;
 GCObject *allweak;
 GCObject *tobefnz;
 GCObject *fixedgc;

 GCObject *survival;
 GCObject *old1;
 GCObject *reallyold;
 GCObject *firstold1;
 GCObject *finobjsur;
 GCObject *finobjold1;
 GCObject *finobjrold;
 struct lua_State *twups;
 lua_CFunction panic;
 struct lua_State *mainthread;
 TString *memerrmsg;
 TString *tmname[TM_N];
 struct Table *mt[ 9];
 TString *strcache[ 53][ 2];
 lua_WarnFunction warnf;
 void *ud_warn;
} global_State;



struct lua_State {
 struct GCObject *next; lu_byte tt; lu_byte marked;
 lu_byte status;
 lu_byte allowhook;
 unsigned short nci;
 StkIdRel top;
 global_State *l_G;
 CallInfo *ci;
 StkIdRel stack_last;
 StkIdRel stack;
 UpVal *openupval;
 StkIdRel tbclist;
 GCObject *gclist;
 struct lua_State *twups;
 struct lua_longjmp *errorJmp;
 CallInfo base_ci;
 volatile lua_Hook hook;
 ptrdiff_t errfunc;
 l_uint32 nCcalls;
 int oldpc;
 int basehookcount;
 int hookcount;
 volatile sig_atomic_t hookmask;
};







#line 342 "src/lstate.h"
union GCUnion {
 GCObject gc;
 struct TString ts;
 struct Udata u;
 union Closure cl;
 struct Table h;
 struct Proto p;
 struct lua_State th;
 struct UpVal upv;
};












#line 395 "src/lstate.h"
 extern void luaE_setdebt (global_State *g, l_mem debt);
 extern void luaE_freethread (lua_State *L, lua_State *L1);
 extern CallInfo *luaE_extendCI (lua_State *L);
 extern void luaE_freeCI (lua_State *L);
 extern void luaE_shrinkCI (lua_State *L);
 extern void luaE_checkcstack (lua_State *L);
 extern void luaE_incCstack (lua_State *L);
 extern void luaE_warning (lua_State *L, const char *msg, int tocont);
 extern void luaE_warnerror (lua_State *L, const char *where);
 extern int luaE_resetthread (lua_State *L, int status);



#line 13 "src/lapi.h"
















#line 20 "src/ldo.c"
#line 1 "src/ldebug.h"




#line 1 "src/lstate.h"



#line 12 "src/ldebug.h"














#line 37 "src/ldebug.h"
 extern int luaG_getfuncline (const Proto *f, int pc);
 extern const char *luaG_findlocal (lua_State *L, CallInfo *ci, int n,
 StkId *pos);
 extern void __declspec(noreturn) luaG_typeerror (lua_State *L, const TValue *o,
 const char *opname);
 extern void __declspec(noreturn) luaG_callerror (lua_State *L, const TValue *o);
 extern void __declspec(noreturn) luaG_forerror (lua_State *L, const TValue *o,
 const char *what);
 extern void __declspec(noreturn) luaG_concaterror (lua_State *L, const TValue *p1,
 const TValue *p2);
 extern void __declspec(noreturn) luaG_opinterror (lua_State *L, const TValue *p1,
 const TValue *p2,
 const char *msg);
 extern void __declspec(noreturn) luaG_tointerror (lua_State *L, const TValue *p1,
 const TValue *p2);
 extern void __declspec(noreturn) luaG_ordererror (lua_State *L, const TValue *p1,
 const TValue *p2);
 extern void __declspec(noreturn) luaG_runerror (lua_State *L, const char *fmt, ...);
 extern const char *luaG_addinfo (lua_State *L, const char *msg,
 TString *src, int line);
 extern void __declspec(noreturn) luaG_errormsg (lua_State *L);
 extern int luaG_traceexec (lua_State *L, const Instruction *pc);


#line 21 "src/ldo.c"
#line 1 "src/ldo.h"




#line 1 "src/llimits.h"


#line 12 "src/ldo.h"
#line 1 "src/lobject.h"




#line 13 "src/ldo.h"
#line 1 "src/lstate.h"



#line 14 "src/ldo.h"
#line 1 "src/lzio.h"



#line 15 "src/ldo.h"




















#line 58 "src/ldo.h"
typedef void (*Pfunc) (lua_State *L, void *ud);

 extern void luaD_seterrorobj (lua_State *L, int errcode, StkId oldtop);
 extern int luaD_protectedparser (lua_State *L, ZIO *z, const char *name,
 const char *mode);
 extern void luaD_hook (lua_State *L, int event, int line,
 int fTransfer, int nTransfer);
 extern void luaD_hookcall (lua_State *L, CallInfo *ci);
 extern int luaD_pretailcall (lua_State *L, CallInfo *ci, StkId func,
 int narg1, int delta);
 extern CallInfo *luaD_precall (lua_State *L, StkId func, int nResults);
 extern void luaD_call (lua_State *L, StkId func, int nResults);
 extern void luaD_callnoyield (lua_State *L, StkId func, int nResults);
 extern StkId luaD_tryfuncTM (lua_State *L, StkId func);
 extern int luaD_closeprotected (lua_State *L, ptrdiff_t level, int status);
 extern int luaD_pcall (lua_State *L, Pfunc func, void *u,
 ptrdiff_t oldtop, ptrdiff_t ef);
 extern void luaD_poscall (lua_State *L, CallInfo *ci, int nres);
 extern int luaD_reallocstack (lua_State *L, int newsize, int raiseerror);
 extern int luaD_growstack (lua_State *L, int n, int raiseerror);
 extern void luaD_shrinkstack (lua_State *L);
 extern void luaD_inctop (lua_State *L);

 extern void __declspec(noreturn) luaD_throw (lua_State *L, int errcode);
 extern int luaD_rawrunprotected (lua_State *L, Pfunc f, void *ud);


#line 22 "src/ldo.c"
#line 1 "src/lfunc.h"




#line 1 "src/lobject.h"




#line 12 "src/lfunc.h"






















#line 48 "src/lfunc.h"
 extern Proto *luaF_newproto (lua_State *L);
 extern CClosure *luaF_newCclosure (lua_State *L, int nupvals);
 extern LClosure *luaF_newLclosure (lua_State *L, int nupvals);
 extern void luaF_initupvals (lua_State *L, LClosure *cl);
 extern UpVal *luaF_findupval (lua_State *L, StkId level);
 extern void luaF_newtbcupval (lua_State *L, StkId level);
 extern void luaF_closeupval (lua_State *L, StkId level);
 extern StkId luaF_close (lua_State *L, StkId level, int status, int yy);
 extern void luaF_unlinkupval (UpVal *uv);
 extern void luaF_freeproto (lua_State *L, Proto *f);
 extern const char *luaF_getlocalname (const Proto *func, int local_number,
 int pc);


#line 23 "src/ldo.c"
#line 1 "src/lgc.h"




#line 1 "src/lobject.h"




#line 12 "src/lgc.h"
#line 1 "src/lstate.h"



#line 13 "src/lgc.h"




























































#line 187 "src/lgc.h"
 extern void luaC_fix (lua_State *L, GCObject *o);
 extern void luaC_freeallobjects (lua_State *L);
 extern void luaC_step (lua_State *L);
 extern void luaC_runtilstate (lua_State *L, int statesmask);
 extern void luaC_fullgc (lua_State *L, int isemergency);
 extern GCObject *luaC_newobj (lua_State *L, int tt, size_t sz);
 extern GCObject *luaC_newobjdt (lua_State *L, int tt, size_t sz,
 size_t offset);
 extern void luaC_barrier_ (lua_State *L, GCObject *o, GCObject *v);
 extern void luaC_barrierback_ (lua_State *L, GCObject *o);
 extern void luaC_checkfinalizer (lua_State *L, GCObject *o, Table *mt);
 extern void luaC_changemode (lua_State *L, int newmode);


#line 24 "src/ldo.c"
#line 1 "src/lmem.h"



#line 25 "src/ldo.c"
#line 1 "src/lobject.h"




#line 26 "src/ldo.c"
#line 1 "src/lopcodes.h"



#line 1 "src/llimits.h"


#line 11 "src/lopcodes.h"





#line 11 "src/lopcodes.h"
enum OpMode {iABC, iABx, iAsBx, iAx, isJ};




























































#line 183 "src/lopcodes.h"
typedef enum {

OP_MOVE,
OP_LOADI,
OP_LOADF,
OP_LOADK,
OP_LOADKX,
OP_LOADFALSE,
OP_LFALSESKIP,
OP_LOADTRUE,
OP_LOADNIL,
OP_GETUPVAL,
OP_SETUPVAL,

OP_GETTABUP,
OP_GETTABLE,
OP_GETI,
OP_GETFIELD,

OP_SETTABUP,
OP_SETTABLE,
OP_SETI,
OP_SETFIELD,

OP_NEWTABLE,

OP_SELF,

OP_ADDI,

OP_ADDK,
OP_SUBK,
OP_MULK,
OP_MODK,
OP_POWK,
OP_DIVK,
OP_IDIVK,

OP_BANDK,
OP_BORK,
OP_BXORK,

OP_SHRI,
OP_SHLI,

OP_ADD,
OP_SUB,
OP_MUL,
OP_MOD,
OP_POW,
OP_DIV,
OP_IDIV,

OP_BAND,
OP_BOR,
OP_BXOR,
OP_SHL,
OP_SHR,

OP_MMBIN,
OP_MMBINI,
OP_MMBINK,

OP_UNM,
OP_BNOT,
OP_NOT,
OP_LEN,

OP_CONCAT,

OP_CLOSE,
OP_TBC,
OP_JMP,
OP_EQ,
OP_LT,
OP_LE,

OP_EQK,
OP_EQI,
OP_LTI,
OP_LEI,
OP_GTI,
OP_GEI,

OP_TEST,
OP_TESTSET,

OP_CALL,
OP_TAILCALL,

OP_RETURN,
OP_RETURN0,
OP_RETURN1,

OP_FORLOOP,
OP_FORPREP,

OP_TFORPREP,
OP_TFORCALL,
OP_TFORLOOP,

OP_SETLIST,

OP_CLOSURE,

OP_VARARG,

OP_VARARGPREP,

OP_EXTRAARG
} OpCode;










#line 314 "src/lopcodes.h"
 extern const lu_byte luaP_opmodes[ ((int)(OP_EXTRAARG) + 1)];










#line 27 "src/ldo.c"
#line 1 "src/lparser.h"



#line 1 "src/llimits.h"


#line 11 "src/lparser.h"
#line 1 "src/lobject.h"




#line 12 "src/lparser.h"
#line 1 "src/lzio.h"



#line 13 "src/lparser.h"





#line 13 "src/lparser.h"
typedef enum {
 VVOID,
 VNIL,
 VTRUE,
 VFALSE,
 VK,
 VKFLT,
 VKINT,
 VKSTR,
 VNONRELOC,
 VLOCAL,
 VUPVAL,
 VCONST,
 VINDEXED,
 VINDEXUP,
 VINDEXI,
 VINDEXSTR,
 VJMP,
 VRELOC,
 VCALL,
 VVARARG
} expkind;




#line 66 "src/lparser.h"
typedef struct expdesc {
 expkind k;
 union {
 lua_Integer ival;
 lua_Number nval;
 TString *strval;
 int info;
 struct {
 short idx;
 lu_byte t;
 } ind;
 struct {
 lu_byte ridx;
 unsigned short vidx;
 } var;
 } u;
 int t;
 int f;
} expdesc;





#line 94 "src/lparser.h"
typedef union Vardesc {
 struct {
 Value value_; lu_byte tt_;
 lu_byte kind;
 lu_byte ridx;
 short pidx;
 TString *name;
 } vd;
 TValue k;
} Vardesc;




typedef struct Labeldesc {
 TString *name;
 int pc;
 int line;
 lu_byte nactvar;
 lu_byte close;
} Labeldesc;



typedef struct Labellist {
 Labeldesc *arr;
 int n;
 int size;
} Labellist;



typedef struct Dyndata {
 struct {
 Vardesc *arr;
 int n;
 int size;
 } actvar;
 Labellist gt;
 Labellist label;
} Dyndata;



struct BlockCnt;



typedef struct FuncState {
 Proto *f;
 struct FuncState *prev;
 struct LexState *ls;
 struct BlockCnt *bl;
 int pc;
 int lasttarget;
 int previousline;
 int nk;
 int np;
 int nabslineinfo;
 int firstlocal;
 int firstlabel;
 short ndebugvars;
 lu_byte nactvar;
 lu_byte nups;
 lu_byte freereg;
 lu_byte iwthabs;
 lu_byte needclose;
} FuncState;


 extern int luaY_nvarstack (FuncState *fs);
 extern LClosure *luaY_parser (lua_State *L, ZIO *z, Mbuffer *buff,
 Dyndata *dyd, const char *name, int firstchar);


#line 28 "src/ldo.c"
#line 1 "src/lstate.h"



#line 29 "src/ldo.c"
#line 1 "src/lstring.h"



#line 1 "src/lgc.h"


#line 11 "src/lstring.h"
#line 1 "src/lobject.h"




#line 12 "src/lstring.h"
#line 1 "src/lstate.h"



#line 13 "src/lstring.h"















#line 42 "src/lstring.h"
 extern unsigned int luaS_hash (const char *str, size_t l, unsigned int seed);
 extern unsigned int luaS_hashlongstr (TString *ts);
 extern int luaS_eqlngstr (TString *a, TString *b);
 extern void luaS_resize (lua_State *L, int newsize);
 extern void luaS_clearcache (global_State *g);
 extern void luaS_init (lua_State *L);
 extern void luaS_remove (lua_State *L, TString *ts);
 extern Udata *luaS_newudata (lua_State *L, size_t s, int nuvalue);
 extern TString *luaS_newlstr (lua_State *L, const char *str, size_t l);
 extern TString *luaS_new (lua_State *L, const char *str);
 extern TString *luaS_createlngstrobj (lua_State *L, size_t l);


#line 30 "src/ldo.c"
#line 1 "src/ltable.h"



#line 1 "src/lobject.h"




#line 11 "src/ltable.h"
















#line 36 "src/ltable.h"
 extern const TValue *luaH_getint (Table *t, lua_Integer key);
 extern void luaH_setint (lua_State *L, Table *t, lua_Integer key,
 TValue *value);
 extern const TValue *luaH_getshortstr (Table *t, TString *key);
 extern const TValue *luaH_getstr (Table *t, TString *key);
 extern const TValue *luaH_get (Table *t, const TValue *key);
 extern void luaH_newkey (lua_State *L, Table *t, const TValue *key,
 TValue *value);
 extern void luaH_set (lua_State *L, Table *t, const TValue *key,
 TValue *value);
 extern void luaH_finishset (lua_State *L, Table *t, const TValue *key,
 const TValue *slot, TValue *value);
 extern Table *luaH_new (lua_State *L);
 extern void luaH_resize (lua_State *L, Table *t, unsigned int nasize,
 unsigned int nhsize);
 extern void luaH_resizearray (lua_State *L, Table *t, unsigned int nasize);
 extern void luaH_free (lua_State *L, Table *t);
 extern int luaH_next (lua_State *L, Table *t, StkId key);
 extern lua_Unsigned luaH_getn (Table *t);
 extern unsigned int luaH_realasize (const Table *t);




#line 31 "src/ldo.c"
#line 1 "src/ltm.h"


#line 32 "src/ldo.c"
#line 1 "src/lundump.h"



#line 1 "src/llimits.h"


#line 11 "src/lundump.h"
#line 1 "src/lobject.h"




#line 12 "src/lundump.h"
#line 1 "src/lzio.h"



#line 13 "src/lundump.h"









#line 28 "src/lundump.h"
 extern LClosure* luaU_undump (lua_State* L, ZIO* Z, const char* name);


 extern int luaU_dump (lua_State* L, const Proto* f, lua_Writer w,
 void* data, int strip);

#line 33 "src/ldo.c"
#line 1 "src/lvm.h"




#line 1 "src/ldo.h"



#line 12 "src/lvm.h"
#line 1 "src/lobject.h"




#line 13 "src/lvm.h"
#line 1 "src/ltm.h"


#line 14 "src/lvm.h"










#line 38 "src/lvm.h"
typedef enum {
 F2Ieq,
 F2Ifloor,
 F2Iceil
} F2Imod;






























#line 117 "src/lvm.h"
 extern int luaV_equalobj (lua_State *L, const TValue *t1, const TValue *t2);
 extern int luaV_lessthan (lua_State *L, const TValue *l, const TValue *r);
 extern int luaV_lessequal (lua_State *L, const TValue *l, const TValue *r);
 extern int luaV_tonumber_ (const TValue *obj, lua_Number *n);
 extern int luaV_tointeger (const TValue *obj, lua_Integer *p, F2Imod mode);
 extern int luaV_tointegerns (const TValue *obj, lua_Integer *p,
 F2Imod mode);
 extern int luaV_flttointeger (lua_Number n, lua_Integer *p, F2Imod mode);
 extern void luaV_finishget (lua_State *L, const TValue *t, TValue *key,
 StkId val, const TValue *slot);
 extern void luaV_finishset (lua_State *L, const TValue *t, TValue *key,
 TValue *val, const TValue *slot);
 extern void luaV_finishOp (lua_State *L);
 extern void luaV_execute (lua_State *L, CallInfo *ci);
 extern void luaV_concat (lua_State *L, int total);
 extern lua_Integer luaV_idiv (lua_State *L, lua_Integer x, lua_Integer y);
 extern lua_Integer luaV_mod (lua_State *L, lua_Integer x, lua_Integer y);
 extern lua_Number luaV_modf (lua_State *L, lua_Number x, lua_Number y);
 extern lua_Integer luaV_shiftl (lua_Integer x, lua_Integer y);
 extern void luaV_objlen (lua_State *L, StkId ra, const TValue *rb);

#line 34 "src/ldo.c"
#line 1 "src/lzio.h"



#line 35 "src/ldo.c"

















#line 80 "src/ldo.c"
struct lua_longjmp {
 struct lua_longjmp *previous;
 jmp_buf b;
 volatile int status;
};


void luaD_seterrorobj (lua_State *L, int errcode, StkId oldtop) {
 switch (errcode) {
 case 4: {
 { TValue *io = ( (&( oldtop)->val)); TString *x_ = ( (L->l_G)->memerrmsg); ((io)->value_).gc = ( &( ((union GCUnion *)( (x_)))->gc)); ((io)->tt_=( ((x_->tt) | (1 << 6)))); ((void)L, ((void)0)); };
 break;
 }
 case 5: {
 { TValue *io = ( (&( oldtop)->val)); TString *x_ = ( (luaS_newlstr(L, "error in error handling", (sizeof( "error in error handling")/sizeof(char))-1))); ((io)->value_).gc = ( &( ((union GCUnion *)( (x_)))->gc)); ((io)->tt_=( ((x_->tt) | (1 << 6)))); ((void)L, ((void)0)); };
 break;
 }
 case 0: {
 (( (&(oldtop)->val))->tt_=( (( 0) | (( 0) << 4))));
 break;
 }
 default: {
 ((void)0);
 { TValue *io1=( (&( oldtop)->val)); const TValue *io2=( (&( L->top.p - 1)->val)); io1->value_ = io2->value_; ((io1)->tt_=( io2->tt_)); ((void)L, ((void)0)); ((void)0); };
 break;
 }
 }
 L->top.p = oldtop + 1;
}


 void __declspec(noreturn) luaD_throw (lua_State *L, int errcode) {
 if (L->errorJmp) {
 L->errorJmp->status = errcode;
 longjmp(( L->errorJmp)->b, 1);
 }
 else {
 global_State *g = (L->l_G);
 errcode = luaE_resetthread(L, errcode);
 if (g->mainthread->errorJmp) {
 { TValue *io1=( (&( g->mainthread->top.p++)->val)); const TValue *io2=( (&( L->top.p - 1)->val)); io1->value_ = io2->value_; ((io1)->tt_=( io2->tt_)); ((void)L, ((void)0)); ((void)0); };
 luaD_throw(g->mainthread, errcode);
 }
 else {
 if (g->panic) {
 ((void) 0);
 g->panic(L);
 }
 abort();
 }
 }
}


int luaD_rawrunprotected (lua_State *L, Pfunc f, void *ud) {
 l_uint32 oldnCcalls = L->nCcalls;
 struct lua_longjmp lj;
 lj.status = 0;
 lj.previous = L->errorJmp;
 L->errorJmp = &lj;
 if ( _setjmp(( &lj)->b) == 0) { 
 (*f)(L, ud);
 };
 L->errorJmp = lj.previous;
 L->nCcalls = oldnCcalls;
 return lj.status;
}








static void relstack (lua_State *L) {
 CallInfo *ci;
 UpVal *up;
 L->top.offset = ( ((char *)( ( L->top.p))) - ((char *)( (L->stack.p))));
 L->tbclist.offset = ( ((char *)( ( L->tbclist.p))) - ((char *)( (L->stack.p))));
 for (up = L->openupval; up != ((void *)0); up = up->u.open.next)
 up->v.offset = ( ((char *)( ( ( ((StkId)( (up)->v.p)))))) - ((char *)( (L->stack.p))));
 for (ci = L->ci; ci != ((void *)0); ci = ci->previous) {
 ci->top.offset = ( ((char *)( ( ci->top.p))) - ((char *)( (L->stack.p))));
 ci->func.offset = ( ((char *)( ( ci->func.p))) - ((char *)( (L->stack.p))));
 }
}



static void correctstack (lua_State *L) {
 CallInfo *ci;
 UpVal *up;
 L->top.p = ((StkId)( ((char *)( (L->stack.p))) + ( L->top.offset)));
 L->tbclist.p = ((StkId)( ((char *)( (L->stack.p))) + ( L->tbclist.offset)));
 for (up = L->openupval; up != ((void *)0); up = up->u.open.next)
 up->v.p = (&( ((StkId)( ((char *)( (L->stack.p))) + ( up->v.offset))))->val);
 for (ci = L->ci; ci != ((void *)0); ci = ci->previous) {
 ci->top.p = ((StkId)( ((char *)( (L->stack.p))) + ( ci->top.offset)));
 ci->func.p = ((StkId)( ((char *)( (L->stack.p))) + ( ci->func.offset)));
 if ( (!((ci)->callstatus & (1<<1)  )))
 ci->u.l.trap = 1;
 }
}





#line 200 "src/ldo.c"
int luaD_reallocstack (lua_State *L, int newsize, int raiseerror) {
 int oldsize = ((int)( ((L)->stack_last.p - (L)->stack.p)));
 int i;
 StkId newstack;
 int oldgcstop = (L->l_G)->gcstopem;
 ((void)0);
 relstack(L);
 (L->l_G)->gcstopem = 1;
 newstack = ( (( StackValue *)( luaM_realloc_(L, L->stack.p, ((size_t)( ( oldsize + 5))) * sizeof( StackValue), ((size_t)( (
 newsize + 5))) * sizeof( StackValue)))));
 (L->l_G)->gcstopem = oldgcstop;
 if ( (newstack == ((void *)0))) {
 correctstack(L);
 if (raiseerror)
 luaD_throw(L, 4);
 else return 0;
 }
 L->stack.p = newstack;
 correctstack(L);
 L->stack_last.p = L->stack.p + newsize;
 for (i = oldsize + 5; i < newsize + 5; i++)
 (( (&(newstack + i)->val))->tt_=( (( 0) | (( 0) << 4))));
 return 1;
}



int luaD_growstack (lua_State *L, int n, int raiseerror) {
 int size = ((int)( ((L)->stack_last.p - (L)->stack.p)));
 if ( (size > 1000000)) {

 ((void)0);
 if (raiseerror)
 luaD_throw(L, 5);
 return 0;
 }
 else if (n < 1000000) {
 int newsize = 2 * size;
 int needed = ((int)( (L->top.p - L->stack.p))) + n;
 if (newsize > 1000000)
 newsize = 1000000;
 if (newsize < needed)
 newsize = needed;
 if ( (newsize <= 1000000))
 return luaD_reallocstack(L, newsize, raiseerror);
 }


 luaD_reallocstack(L, ( 1000000 + 200), raiseerror);
 if (raiseerror)
 luaG_runerror(L, "stack overflow");
 return 0;
}



static int stackinuse (lua_State *L) {
 CallInfo *ci;
 int res;
 StkId lim = L->top.p;
 for (ci = L->ci; ci != ((void *)0); ci = ci->previous) {
 if (lim < ci->top.p) lim = ci->top.p;
 }
 ((void)0);
 res = ((int)( (lim - L->stack.p))) + 1;
 if (res < 20)
 res = 20;
 return res;
}



void luaD_shrinkstack (lua_State *L) {
 int inuse = stackinuse(L);
 int max = (inuse > 1000000 / 3) ? 1000000 : inuse * 3;

 if (inuse <= 1000000 && ((int)( ((L)->stack_last.p - (L)->stack.p))) > max) {
 int nsize = (inuse > 1000000 / 2) ? 1000000 : inuse * 2;
 luaD_reallocstack(L, nsize, 0);
 }
 else
 ((void)0);
 luaE_shrinkCI(L);
}


void luaD_inctop (lua_State *L) {
 if ( (L->stack_last.p - L->top.p <= ( 1))) { (void)0; luaD_growstack(L, 1, 1); (void)0; } else { ((void)0); };
 L->top.p++;
}





void luaD_hook (lua_State *L, int event, int line,
 int ftransfer, int ntransfer) {
 lua_Hook hook = L->hook;
 if (hook && L->allowhook) {
 int mask = (1<<3)  ;
 CallInfo *ci = L->ci;
 ptrdiff_t top = ( ((char *)( ( L->top.p))) - ((char *)( (L->stack.p))));
 ptrdiff_t ci_top = ( ((char *)( ( ci->top.p))) - ((char *)( (L->stack.p))));
 lua_Debug ar;
 ar.event = event;
 ar.currentline = line;
 ar.i_ci = ci;
 if (ntransfer != 0) {
 mask |= (1<<8)  ;
 ci->u2.transferinfo.ftransfer = ftransfer;
 ci->u2.transferinfo.ntransfer = ntransfer;
 }
 if ( (!((ci)->callstatus & (1<<1)  )) && L->top.p < ci->top.p)
 L->top.p = ci->top.p;
 if ( (L->stack_last.p - L->top.p <= ( 20))) { (void)0; luaD_growstack(L, 20, 1); (void)0; } else { ((void)0); };
 if (ci->top.p < L->top.p + 20)
 ci->top.p = L->top.p + 20;
 L->allowhook = 0;
 ci->callstatus |= mask;
 ((void) 0);
 (*hook)(L, &ar);
 ((void) 0);
 ((void)0);
 L->allowhook = 1;
 ci->top.p = ((StkId)( ((char *)( (L->stack.p))) + ( ci_top)));
 L->top.p = ((StkId)( ((char *)( (L->stack.p))) + ( top)));
 ci->callstatus &= ~mask;
 }
}



void luaD_hookcall (lua_State *L, CallInfo *ci) {
 L->oldpc = 0;
 if (L->hookmask & (1 << 0)) {
 int event = (ci->callstatus & (1<<5)  ) ? 4
 : 0;
 Proto *p = ( ( ( &(( ((union GCUnion *)( ( (( (&((ci)->func.p)->val))->value_).gc))))->cl.l))))->p;
 ci->u.l.savedpc++;
 luaD_hook(L, event, -1, 1, p->numparams);
 ci->u.l.savedpc--;
 }
}



static void rethook (lua_State *L, CallInfo *ci, int nres) {
 if (L->hookmask & (1 << 1)) {
 StkId firstres = L->top.p - nres;
 int delta = 0;
 int ftransfer;
 if ( (!((ci)->callstatus & (1<<1)  ))) {
 Proto *p = ( ( ( &(( ((union GCUnion *)( ( (( (&((ci)->func.p)->val))->value_).gc))))->cl.l))))->p;
 if (p->is_vararg)
 delta = ci->u.l.nextraargs + p->numparams + 1;
 }
 ci->func.p += delta;
 ftransfer = ((unsigned short)( firstres - ci->func.p));
 luaD_hook(L, 1, -1, ftransfer, nres);
 ci->func.p -= delta;
 }
 if ( (!((ci = ci->previous)->callstatus & (1<<1)  )))
 L->oldpc = ( ((int)( ((ci->u.l.savedpc) - ( ( ( ( &(( ((union GCUnion *)( ( (( (&((ci)->func.p)->val))->value_).gc))))->cl.l))))->p)->code))) - 1);
}



StkId luaD_tryfuncTM (lua_State *L, StkId func) {
 const TValue *tm;
 StkId p;
 if ( (L->stack_last.p - L->top.p <= ( 1))) { ptrdiff_t t__ = ( ((char *)( ( func))) - ((char *)( (L->stack.p))));   { if ( (L->l_G)->GCdebt > 0) { (void)0; luaC_step(L); (void)0;}; ((void)0); }; luaD_growstack(L, 1, 1);   func = ((StkId)( ((char *)( (L->stack.p))) + ( t__))); } else { ((void)0); }  ;
 tm = luaT_gettmbyobj(L, (&(func)->val), TM_CALL);
 if ( ( ( ( (( (((tm))->tt_)) & 0x0F)) == ( 0))))
 luaG_callerror(L, (&(func)->val));
 for (p = L->top.p; p > func; p--)
 { TValue *io1=( (&( p)->val)); const TValue *io2=( (&( p-1)->val)); io1->value_ = io2->value_; ((io1)->tt_=( io2->tt_)); ((void)L, ((void)0)); ((void)0); };
 L->top.p++;
 { TValue *io1=( (&( func)->val)); const TValue *io2=( tm); io1->value_ = io2->value_; ((io1)->tt_=( io2->tt_)); ((void)L, ((void)0)); ((void)0); };
 return func;
}



 static   void moveresults (lua_State *L, StkId res, int nres, int wanted) {
 StkId firstresult;
 int i;
 switch (wanted) {
 case 0:
 L->top.p = res;
 return;
 case 1:
 if (nres == 0)
 (( (&(res)->val))->tt_=( (( 0) | (( 0) << 4))));
 else
 { TValue *io1=( (&( res)->val)); const TValue *io2=( (&( L->top.p - nres)->val)); io1->value_ = io2->value_; ((io1)->tt_=( io2->tt_)); ((void)L, ((void)0)); ((void)0); };
 L->top.p = res + 1;
 return;
 case (-1):
 wanted = nres;
 break;
 default:
 if ( ((wanted) < (-1))) {
 L->ci->callstatus |= (1<<9)  ;
 L->ci->u2.nres = nres;
 res = luaF_close(L, res, (-1), 1);
 L->ci->callstatus &= ~ (1<<9)  ;
 if (L->hookmask) {
 ptrdiff_t savedres = ( ((char *)( ( res))) - ((char *)( (L->stack.p))));
 rethook(L, L->ci, nres);
 res = ((StkId)( ((char *)( (L->stack.p))) + ( savedres)));
 }
 wanted = (-(wanted) - 3);
 if (wanted == (-1))
 wanted = nres;
 }
 break;
 }

 firstresult = L->top.p - nres;
 if (nres > wanted)
 nres = wanted;
 for (i = 0; i < nres; i++)
 { TValue *io1=( (&( res + i)->val)); const TValue *io2=( (&( firstresult + i)->val)); io1->value_ = io2->value_; ((io1)->tt_=( io2->tt_)); ((void)L, ((void)0)); ((void)0); };
 for (; i < wanted; i++)
 (( (&(res + i)->val))->tt_=( (( 0) | (( 0) << 4))));
 L->top.p = res + wanted;
}



void luaD_poscall (lua_State *L, CallInfo *ci, int nres) {
 int wanted = ci->nresults;
 if ( (L->hookmask && ! ((wanted) < (-1))))
 rethook(L, ci, nres);

 moveresults(L, ci->func.p, nres, wanted);

 ((void)0);
 L->ci = ci->previous;
}





#line 500 "src/ldo.c"
 static   CallInfo *prepCallInfo (lua_State *L, StkId func, int nret,
 int mask, StkId top) {
 CallInfo *ci = L->ci = (L->ci->next ? L->ci->next : luaE_extendCI(L));
 ci->func.p = func;
 ci->nresults = nret;
 ci->callstatus = mask;
 ci->top.p = top;
 return ci;
}



 static   int precallC (lua_State *L, StkId func, int nresults,
 lua_CFunction f) {
 int n;
 CallInfo *ci;
 if ( (L->stack_last.p - L->top.p <= ( 20))) { ptrdiff_t t__ = ( ((char *)( ( func))) - ((char *)( (L->stack.p))));   { if ( (L->l_G)->GCdebt > 0) { (void)0; luaC_step(L); (void)0;}; ((void)0); }; luaD_growstack(L, 20, 1);   func = ((StkId)( ((char *)( (L->stack.p))) + ( t__))); } else { ((void)0); }  ;
 L->ci = ci = prepCallInfo(L, func, nresults, (1<<1)  ,
 L->top.p + 20);
 ((void)0);
 if ( (L->hookmask & (1 << 0))) {
 int narg = ((int)( (L->top.p - func))) - 1;
 luaD_hook(L, 0, -1, 1, narg);
 }
 ((void) 0);
 n = (*f)(L);
 ((void) 0);
 ((void)L, ((void)0));
 luaD_poscall(L, ci, n);
 return n;
}



int luaD_pretailcall (lua_State *L, CallInfo *ci, StkId func,
 int narg1, int delta) {
 retry:
 switch ( (( (( (&(func)->val))->tt_)) & 0x3F)) {
 case (( 6) | (( 2) << 4))  :
 return precallC(L, func, (-1), ( ( &(( ((union GCUnion *)( ( (( (&(func)->val))->value_).gc))))->cl.c)))->f);
 case (( 6) | (( 1) << 4))  :
 return precallC(L, func, (-1), ( (( (&(func)->val))->value_).f));
 case (( 6) | (( 0) << 4))  : {
 Proto *p = ( ( &(( ((union GCUnion *)( ( (( (&(func)->val))->value_).gc))))->cl.l)))->p;
 int fsize = p->maxstacksize;
 int nfixparams = p->numparams;
 int i;
 if ( (L->stack_last.p - L->top.p <= ( fsize - delta))) { ptrdiff_t t__ = ( ((char *)( ( func))) - ((char *)( (L->stack.p))));   { if ( (L->l_G)->GCdebt > 0) { (void)0; luaC_step(L); (void)0;}; ((void)0); }; luaD_growstack(L, fsize - delta, 1);   func = ((StkId)( ((char *)( (L->stack.p))) + ( t__))); } else { ((void)0); }  ;
 ci->func.p -= delta;
 for (i = 0; i < narg1; i++)
 { TValue *io1=( (&( ci->func.p + i)->val)); const TValue *io2=( (&( func + i)->val)); io1->value_ = io2->value_; ((io1)->tt_=( io2->tt_)); ((void)L, ((void)0)); ((void)0); };
 func = ci->func.p;
 for (; narg1 <= nfixparams; narg1++)
 (( (&(func + narg1)->val))->tt_=( (( 0) | (( 0) << 4))));
 ci->top.p = func + 1 + fsize;
 ((void)0);
 ci->u.l.savedpc = p->code;
 ci->callstatus |= (1<<5)  ;
 L->top.p = func + narg1;
 return -1;
 }
 default: {
 func = luaD_tryfuncTM(L, func);

 narg1++;
 goto retry;
 }
 }
}



CallInfo *luaD_precall (lua_State *L, StkId func, int nresults) {
 retry:
 switch ( (( (( (&(func)->val))->tt_)) & 0x3F)) {
 case (( 6) | (( 2) << 4))  :
 precallC(L, func, nresults, ( ( &(( ((union GCUnion *)( ( (( (&(func)->val))->value_).gc))))->cl.c)))->f);
 return ((void *)0);
 case (( 6) | (( 1) << 4))  :
 precallC(L, func, nresults, ( (( (&(func)->val))->value_).f));
 return ((void *)0);
 case (( 6) | (( 0) << 4))  : {
 CallInfo *ci;
 Proto *p = ( ( &(( ((union GCUnion *)( ( (( (&(func)->val))->value_).gc))))->cl.l)))->p;
 int narg = ((int)( (L->top.p - func))) - 1;
 int nfixparams = p->numparams;
 int fsize = p->maxstacksize;
 if ( (L->stack_last.p - L->top.p <= ( fsize))) { ptrdiff_t t__ = ( ((char *)( ( func))) - ((char *)( (L->stack.p))));   { if ( (L->l_G)->GCdebt > 0) { (void)0; luaC_step(L); (void)0;}; ((void)0); }; luaD_growstack(L, fsize, 1);   func = ((StkId)( ((char *)( (L->stack.p))) + ( t__))); } else { ((void)0); }  ;
 L->ci = ci = prepCallInfo(L, func, nresults, 0, func + 1 + fsize);
 ci->u.l.savedpc = p->code;
 for (; narg < nfixparams; narg++)
 (( (&(L->top.p++)->val))->tt_=( (( 0) | (( 0) << 4))));
 ((void)0);
 return ci;
 }
 default: {
 func = luaD_tryfuncTM(L, func);

 goto retry;
 }
 }
}



 static   void ccall (lua_State *L, StkId func, int nResults, l_uint32 inc) {
 CallInfo *ci;
 L->nCcalls += inc;
 if ( ( ((L)->nCcalls & 0xffff) >= 200)) {
 if ( (L->stack_last.p - L->top.p <= ( 0))) { ptrdiff_t t__ = ( ((char *)( ( func))) - ((char *)( (L->stack.p)))); luaD_growstack(L, 0, 1);   func = ((StkId)( ((char *)( (L->stack.p))) + ( t__))); } else { ((void)0); }  ;
 luaE_checkcstack(L);
 }
 if ((ci = luaD_precall(L, func, nResults)) != ((void *)0)) {
 ci->callstatus = (1<<2)  ;
 luaV_execute(L, ci);
 }
 L->nCcalls -= inc;
}



void luaD_call (lua_State *L, StkId func, int nResults) {
 ccall(L, func, nResults, 1);
}



void luaD_callnoyield (lua_State *L, StkId func, int nResults) {
 ccall(L, func, nResults, (0x10000 | 1));
}



static int finishpcallk (lua_State *L, CallInfo *ci) {
 int status = (((ci)->callstatus >> 10) & 7);
 if ( (status == 0))
 status = 1;
 else {
 StkId func = ((StkId)( ((char *)( (L->stack.p))) + ( ci->u2.funcidx)));
 L->allowhook = ((ci->callstatus) & (1<<0)  );
 func = luaF_close(L, func, status, 1);
 luaD_seterrorobj(L, status, func);
 luaD_shrinkstack(L);
 (   ((ci)->callstatus = ((ci)->callstatus & ~(7 << 10)) | (( 0) << 10)));
 }
 ci->callstatus &= ~ (1<<4)  ;
 L->errfunc = ci->u.c.old_errfunc;

 return status;
}



static void finishCcall (lua_State *L, CallInfo *ci) {
 int n;
 if (ci->callstatus & (1<<9)  ) {
 ((void)0);
 n = ci->u2.nres;

 }
 else {
 int status = 1;

 ((void)0);
 if (ci->callstatus & (1<<4)  )
 status = finishpcallk(L, ci);
 { if (( (-1)) <= (-1) && L->ci->top.p < L->top.p) L->ci->top.p = L->top.p; };
 ((void) 0);
 n = (*ci->u.c.k)(L, status, ci->u.c.ctx);
 ((void) 0);
 ((void)L, ((void)0));
 }
 luaD_poscall(L, ci, n);
}



static void unroll (lua_State *L, void *ud) {
 CallInfo *ci;
 ((void)(ud));
 while ((ci = L->ci) != &L->base_ci) {
 if (! (!((ci)->callstatus & (1<<1)  )))
 finishCcall(L, ci);
 else {
 luaV_finishOp(L);
 luaV_execute(L, ci);
 }
 }
}



static CallInfo *findpcall (lua_State *L) {
 CallInfo *ci;
 for (ci = L->ci; ci != ((void *)0); ci = ci->previous) {
 if (ci->callstatus & (1<<4)  )
 return ci;
 }
 return ((void *)0);
}



static int resume_error (lua_State *L, const char *msg, int narg) {
 L->top.p -= narg;
 { TValue *io = ( (&( L->top.p)->val)); TString *x_ = ( luaS_new(L, msg)); ((io)->value_).gc = ( &( ((union GCUnion *)( (x_)))->gc)); ((io)->tt_=( ((x_->tt) | (1 << 6)))); ((void)L, ((void)0)); };
 {L->top.p++; ((void)L, ((void)0));};
 ((void) 0);
 return 2;
}



static void resume (lua_State *L, void *ud) {
 int n = *( ((int*)( ud)));
 StkId firstArg = L->top.p - n;
 CallInfo *ci = L->ci;
 if (L->status == 0)
 ccall(L, firstArg - 1, (-1), 0);
 else {
 ((void)0);
 L->status = 0;
 if ( (!((ci)->callstatus & (1<<1)  ))) {
 L->top.p = firstArg;
 luaV_execute(L, ci);
 }
 else {
 if (ci->u.c.k != ((void *)0)) {
 ((void) 0);
 n = (*ci->u.c.k)(L, 1, ci->u.c.ctx);
 ((void) 0);
 ((void)L, ((void)0));
 }
 luaD_poscall(L, ci, n);
 }
 unroll(L, ((void *)0));
 }
}



static int precover (lua_State *L, int status) {
 CallInfo *ci;
 while ( ((status) > 1) && (ci = findpcall(L)) != ((void *)0)) {
 L->ci = ci;
 (   ((ci)->callstatus = ((ci)->callstatus & ~(7 << 10)) | (( status) << 10)));
 status = luaD_rawrunprotected(L, unroll, ((void *)0));
 }
 return status;
}


 extern int lua_resume (lua_State *L, lua_State *from, int nargs,
 int *nresults) {
 int status;
 ((void) 0);
 if (L->status == 0) {
 if (L->ci != &L->base_ci)
 return resume_error(L, "cannot resume non-suspended coroutine", nargs);
 else if (L->top.p - (L->ci->func.p + 1) == nargs)
 return resume_error(L, "cannot resume dead coroutine", nargs);
 }
 else if (L->status != 1)
 return resume_error(L, "cannot resume dead coroutine", nargs);
 L->nCcalls = (from) ? ((from)->nCcalls & 0xffff) : 0;
 if ( ((L)->nCcalls & 0xffff) >= 200)
 return resume_error(L, "C stack overflow", nargs);
 L->nCcalls++;
 ((void)L);
 ((void)L, ((void)0));
 status = luaD_rawrunprotected(L, resume, &nargs);

 status = precover(L, status);
 if ( (! ((status) > 1)))
 ((void)0);
 else {
 L->status = ((lu_byte)( (status)));
 luaD_seterrorobj(L, status, L->top.p);
 L->ci->top.p = L->top.p;
 }
 *nresults = (status == 1) ? L->ci->u2.nyield
 : ((int)( (L->top.p - (L->ci->func.p + 1))));
 ((void) 0);
 return status;
}


 extern int lua_isyieldable (lua_State *L) {
 return (((L)->nCcalls & 0xffff0000) == 0);
}


 extern int lua_yieldk (lua_State *L, int nresults, lua_KContext ctx,
 lua_KFunction k) {
 CallInfo *ci;
 ((void)L);
 ((void) 0);
 ci = L->ci;
 ((void)L, ((void)0));
 if ( (! (((L)->nCcalls & 0xffff0000) == 0))) {
 if (L != (L->l_G)->mainthread)
 luaG_runerror(L, "attempt to yield across a C-call boundary");
 else
 luaG_runerror(L, "attempt to yield from outside a coroutine");
 }
 L->status = 1;
 ci->u2.nyield = nresults;
 if ( (!((ci)->callstatus & (1<<1)  ))) {
 ((void)0);
 ((void)L, ((void)0));
 ((void)L, ((void)0));
 }
 else {
 if ((ci->u.c.k = k) != ((void *)0))
 ci->u.c.ctx = ctx;
 luaD_throw(L, 1);
 }
 ((void)0);
 ((void) 0);
 return 0;
}



struct CloseP {
 StkId level;
 int status;
};



static void closepaux (lua_State *L, void *ud) {
 struct CloseP *pcl = ((struct CloseP *)( ud));
 luaF_close(L, pcl->level, pcl->status, 0);
}



int luaD_closeprotected (lua_State *L, ptrdiff_t level, int status) {
 CallInfo *old_ci = L->ci;
 lu_byte old_allowhooks = L->allowhook;
 for (;;) {
 struct CloseP pcl;
 pcl.level = ((StkId)( ((char *)( (L->stack.p))) + ( level))); pcl.status = status;
 status = luaD_rawrunprotected(L, &closepaux, &pcl);
 if ( (status == 0))
 return pcl.status;
 else {
 L->ci = old_ci;
 L->allowhook = old_allowhooks;
 }
 }
}



int luaD_pcall (lua_State *L, Pfunc func, void *u,
 ptrdiff_t old_top, ptrdiff_t ef) {
 int status;
 CallInfo *old_ci = L->ci;
 lu_byte old_allowhooks = L->allowhook;
 ptrdiff_t old_errfunc = L->errfunc;
 L->errfunc = ef;
 status = luaD_rawrunprotected(L, func, u);
 if ( (status != 0)) {
 L->ci = old_ci;
 L->allowhook = old_allowhooks;
 status = luaD_closeprotected(L, old_top, status);
 luaD_seterrorobj(L, status, ((StkId)( ((char *)( (L->stack.p))) + ( old_top))));
 luaD_shrinkstack(L);
 }
 L->errfunc = old_errfunc;
 return status;
}




struct SParser {
 ZIO *z;
 Mbuffer buff;
 Dyndata dyd;
 const char *mode;
 const char *name;
};


static void checkmode (lua_State *L, const char *mode, const char *x) {
 if (mode && strchr(mode, x[0]) == ((void *)0)) {
 luaO_pushfstring(L,
 "attempt to load a %s chunk (mode is '%s')", x, mode);
 luaD_throw(L, 3);
