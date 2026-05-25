// ============================================================
//  The Apple Availability.h pattern:
//    #define __API_AVAILABLE(...)
//      GET_MACRO_16(__VA_ARGS__, ..., API0, 0)(__VA_ARGS__)
//
//  After GET_MACRO selects the Nth variant (e.g. __API_AVAILABLE0),
//  the trailing (__VA_ARGS__) should become its argument list.
// ============================================================
#define GET_MACRO_16(_0,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,NAME,...) NAME

// Function-like macro that GET_MACRO will select
#define TARGET(x)  target_##x

// Without trailing `(__VA_ARGS__)`  — TARGET is bare, should pass through
#define DISPATCH_BARE(...) GET_MACRO_16(__VA_ARGS__, V15,V14,V13,V12,V11,V10,V9,V8,V7,V6,V5,V4,V3,V2,V1, TARGET, S)

// WITH trailing `(__VA_ARGS__)`  — TARGET should combine with (X)
#define DISPATCH_CALL(...) GET_MACRO_16(__VA_ARGS__, V15,V14,V13,V12,V11,V10,V9,V8,V7,V6,V5,V4,V3,V2,V1, TARGET, S)(__VA_ARGS__)

DISPATCH_BARE(X)
DISPATCH_CALL(X)