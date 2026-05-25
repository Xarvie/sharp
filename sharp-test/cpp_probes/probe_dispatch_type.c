#define GET_MACRO_16(_0,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,NAME,...) NAME

#define F0  object_ok
#define F1(x)  func_##x

#define DISPATCH_OBJ(...)  GET_MACRO_16(__VA_ARGS__, V15, V14, V13, V12, V11, V10, V9, V8, V7, V6, V5, V4, V3, V2, V1, F0, SENTINEL)
#define DISPATCH_FUNC(...) GET_MACRO_16(__VA_ARGS__, V15, V14, V13, V12, V11, V10, V9, V8, V7, V6, V5, V4, V3, V2, V1, F1, SENTINEL)

DISPATCH_OBJ(X)
DISPATCH_FUNC(X)