/*
 * cjson_test.sp — cJSON test harness (uses original cJSON 1.7.19 source)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

#include "../std/cjson.sph"

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(cond, msg) do { \
    if (!(cond)) { \
        printf("  FAIL: %s\n", msg); \
        tests_failed++; \
    } else { \
        printf("  PASS: %s\n", msg); \
        tests_passed++; \
    } \
} while(0)

static void test_create_null(void) {
    printf("test_create_null\n");
    cJSON *n = cJSON_CreateNull();
    TEST_ASSERT(cJSON_IsNull(n), "create null");
    TEST_ASSERT(!cJSON_IsString(n), "null is not string");
    cJSON_Delete(n);
}

static void test_create_true_false(void) {
    printf("test_create_true_false\n");
    cJSON *t = cJSON_CreateTrue();
    cJSON *f = cJSON_CreateFalse();
    TEST_ASSERT(cJSON_IsTrue(t), "create true");
    TEST_ASSERT(cJSON_IsFalse(f), "create false");
    TEST_ASSERT(cJSON_IsBool(t), "true is bool");
    TEST_ASSERT(cJSON_IsBool(f), "false is bool");
    TEST_ASSERT(!cJSON_IsBool(cJSON_CreateNull()), "null is not bool");
    cJSON_Delete(t);
    cJSON_Delete(f);
}

static void test_create_number(void) {
    printf("test_create_number\n");
    cJSON *n = cJSON_CreateNumber(3.14);
    TEST_ASSERT(cJSON_IsNumber(n), "create number");
    TEST_ASSERT(cJSON_GetNumberValue(n) == 3.14, "number value correct");
    TEST_ASSERT(!cJSON_IsString(n), "number is not string");
    cJSON_Delete(n);
}

static void test_create_string(void) {
    printf("test_create_string\n");
    cJSON *s = cJSON_CreateString(str_from_lit("hello world"));
    TEST_ASSERT(cJSON_IsString(s), "create string");
    Str _s = cJSON_GetStringValue(s); TEST_ASSERT(_s == str_from_lit("hello world"), "string value correct");
    TEST_ASSERT(!cJSON_IsNumber(s), "string is not number");
    cJSON_Delete(s);
}

static void test_create_array(void) {
    printf("test_create_array\n");
    cJSON *arr = cJSON_CreateArray();
    TEST_ASSERT(cJSON_IsArray(arr), "create array");
    TEST_ASSERT(cJSON_GetArraySize(arr) == 0, "empty array size 0");

    cJSON_AddItemToArray(arr, cJSON_CreateNumber(1.0));
    cJSON_AddItemToArray(arr, cJSON_CreateNumber(2.0));
    cJSON_AddItemToArray(arr, cJSON_CreateString(str_from_lit("three")));

    TEST_ASSERT(cJSON_GetArraySize(arr) == 3, "array size 3");
    TEST_ASSERT(cJSON_GetNumberValue(cJSON_GetArrayItem(arr, 0)) == 1.0, "arr[0] == 1");
    TEST_ASSERT(cJSON_GetNumberValue(cJSON_GetArrayItem(arr, 1)) == 2.0, "arr[1] == 2");
    TEST_ASSERT(cJSON_IsString(cJSON_GetArrayItem(arr, 2)), "arr[2] is string");

    cJSON_Delete(arr);
}

static void test_create_object(void) {
    printf("test_create_object\n");
    cJSON *obj = cJSON_CreateObject();
    TEST_ASSERT(cJSON_IsObject(obj), "create object");

    cJSON_AddNumberToObject(obj, str_from_lit("x"), 10.0);
    cJSON_AddStringToObject(obj, str_from_lit("name"), str_from_lit("test"));
    cJSON_AddTrueToObject(obj, str_from_lit("flag"));
    cJSON_AddFalseToObject(obj, str_from_lit("disabled"));

    TEST_ASSERT(cJSON_GetObjectItem(obj, str_from_lit("x")) != 0, "obj has 'x'");
    TEST_ASSERT(cJSON_GetNumberValue(cJSON_GetObjectItem(obj, str_from_lit("x"))) == 10.0, "obj.x == 10");
    TEST_ASSERT(cJSON_GetObjectItem(obj, str_from_lit("NAME")) != 0, "case insensitive get");
    TEST_ASSERT(cJSON_IsString(cJSON_GetObjectItem(obj, str_from_lit("name"))), "obj.name is string");
    TEST_ASSERT(cJSON_IsTrue(cJSON_GetObjectItem(obj, str_from_lit("flag"))), "obj.flag is true");
    TEST_ASSERT(cJSON_IsFalse(cJSON_GetObjectItem(obj, str_from_lit("disabled"))), "obj.disabled is false");

    cJSON_Delete(obj);
}

static void test_parse_simple(void) {
    printf("test_parse_simple\n");
    cJSON *root;

    root = cJSON_Parse(str_from_lit("null"));
    TEST_ASSERT(cJSON_IsNull(root), "parse null");
    cJSON_Delete(root);

    root = cJSON_Parse(str_from_lit("true"));
    TEST_ASSERT(cJSON_IsTrue(root), "parse true");
    cJSON_Delete(root);

    root = cJSON_Parse(str_from_lit("false"));
    TEST_ASSERT(cJSON_IsFalse(root), "parse false");
    cJSON_Delete(root);

    root = cJSON_Parse(str_from_lit("3.14"));
    TEST_ASSERT(cJSON_IsNumber(root), "parse number");
    cJSON_Delete(root);

    root = cJSON_Parse(str_from_lit("\"hello\""));
    TEST_ASSERT(cJSON_IsString(root), "parse string");
    Str _s = cJSON_GetStringValue(root); TEST_ASSERT(_s == str_from_lit("hello"), "parsed string value");
    cJSON_Delete(root);
}

static void test_parse_array(void) {
    printf("test_parse_array\n");
    cJSON *root = cJSON_Parse(str_from_lit("[1,2,\"three\"]"));
    TEST_ASSERT(cJSON_IsArray(root), "parse array");
    TEST_ASSERT(cJSON_GetArraySize(root) == 3, "parsed array size 3");
    cJSON_Delete(root);

    root = cJSON_Parse(str_from_lit("[]"));
    TEST_ASSERT(cJSON_IsArray(root), "parse empty array");
    TEST_ASSERT(cJSON_GetArraySize(root) == 0, "parsed empty array size 0");
    cJSON_Delete(root);
}

static void test_parse_object(void) {
    printf("test_parse_object\n");
    cJSON *root = cJSON_Parse(str_from_lit("{\"name\":\"test\",\"value\":42}"));
    TEST_ASSERT(cJSON_IsObject(root), "parse object");
    TEST_ASSERT(cJSON_GetObjectItem(root, str_from_lit("name")) != 0, "has name");
    TEST_ASSERT(cJSON_GetObjectItem(root, str_from_lit("value")) != 0, "has value");
    TEST_ASSERT(cJSON_GetNumberValue(cJSON_GetObjectItem(root, str_from_lit("value"))) == 42.0, "value == 42");
    cJSON_Delete(root);

    root = cJSON_Parse(str_from_lit("{}"));
    TEST_ASSERT(cJSON_IsObject(root), "parse empty object");
    cJSON_Delete(root);
}

static void test_parse_nested(void) {
    printf("test_parse_nested\n");
    cJSON *root = cJSON_Parse(str_from_lit("{\"arr\":[1,2,3],\"obj\":{\"a\":1}}"));
    TEST_ASSERT(cJSON_IsObject(root), "parse nested object");
    TEST_ASSERT(cJSON_IsArray(cJSON_GetObjectItem(root, str_from_lit("arr"))), "arr is array");
    TEST_ASSERT(cJSON_GetArraySize(cJSON_GetObjectItem(root, str_from_lit("arr"))) == 3, "nested arr size 3");
    TEST_ASSERT(cJSON_IsObject(cJSON_GetObjectItem(root, str_from_lit("obj"))), "obj is object");
    TEST_ASSERT(cJSON_GetNumberValue(cJSON_GetObjectItem(cJSON_GetObjectItem(root, str_from_lit("obj")), str_from_lit("a"))) == 1.0, "obj.a == 1");
    cJSON_Delete(root);
}

static void test_print_unformatted(void) {
    printf("test_print_unformatted\n");
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, str_from_lit("x"), 1.0);
    cJSON_AddStringToObject(root, str_from_lit("y"), str_from_lit("hello"));

    char *printed = cJSON_PrintUnformatted(root);
    TEST_ASSERT(printed != 0, "print unformatted not null");
    TEST_ASSERT(strcmp(printed, "{\"x\":1,\"y\":\"hello\"}") == 0, "print unformatted correct");

    free(printed);
    cJSON_Delete(root);
}

static void test_print_formatted(void) {
    printf("test_print_formatted\n");
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, str_from_lit("x"), 1.0);

    char *printed = cJSON_Print(root);
    TEST_ASSERT(printed != 0, "print formatted not null");

    cJSON *parsed = cJSON_Parse(str_from_lit(printed));
    TEST_ASSERT(parsed != 0, "re-parse formatted");
    TEST_ASSERT(cJSON_IsObject(parsed), "re-parsed is object");
    TEST_ASSERT(cJSON_GetNumberValue(cJSON_GetObjectItem(parsed, str_from_lit("x"))) == 1.0, "re-parsed value correct");

    cJSON_Delete(parsed);
    free(printed);
    cJSON_Delete(root);
}

static void test_print_and_parse_roundtrip(void) {
    printf("test_print_and_parse_roundtrip\n");
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, str_from_lit("id"), 1.0);
    cJSON_AddStringToObject(root, str_from_lit("name"), str_from_lit("test"));
    cJSON_AddTrueToObject(root, str_from_lit("active"));
    cJSON_AddFalseToObject(root, str_from_lit("deleted"));

    char *printed = cJSON_PrintUnformatted(root);
    cJSON *parsed = cJSON_Parse(str_from_lit(printed));
    TEST_ASSERT(parsed != 0, "roundtrip parse");
    TEST_ASSERT(cJSON_Compare(root, parsed, 1), "roundtrip compare");

    free(printed);
    cJSON_Delete(parsed);
    cJSON_Delete(root);
}

static void test_array_operations(void) {
    printf("test_array_operations\n");
    cJSON *arr = cJSON_CreateArray();
    cJSON_AddItemToArray(arr, cJSON_CreateNumber(0.0));
    cJSON_AddItemToArray(arr, cJSON_CreateNumber(1.0));
    cJSON_AddItemToArray(arr, cJSON_CreateNumber(2.0));

    cJSON_InsertItemInArray(arr, 0, cJSON_CreateNumber(-1.0));
    TEST_ASSERT(cJSON_GetArraySize(arr) == 4, "size after insert");
    TEST_ASSERT(cJSON_GetNumberValue(cJSON_GetArrayItem(arr, 0)) == -1.0, "inserted at 0");

    cJSON_InsertItemInArray(arr, 2, cJSON_CreateNumber(0.5));
    TEST_ASSERT(cJSON_GetArraySize(arr) == 5, "size after second insert");
    TEST_ASSERT(cJSON_GetNumberValue(cJSON_GetArrayItem(arr, 2)) == 0.5, "inserted at 2");

    cJSON_InsertItemInArray(arr, 5, cJSON_CreateNumber(3.0));
    TEST_ASSERT(cJSON_GetArraySize(arr) == 6, "size after end insert");
    TEST_ASSERT(cJSON_GetNumberValue(cJSON_GetArrayItem(arr, 5)) == 3.0, "inserted at end");

    cJSON_DeleteItemFromArray(arr, 0);
    TEST_ASSERT(cJSON_GetArraySize(arr) == 5, "size after delete");
    TEST_ASSERT(cJSON_GetNumberValue(cJSON_GetArrayItem(arr, 0)) == 0.0, "after delete first");

    cJSON_ReplaceItemInArray(arr, 0, cJSON_CreateNumber(100.0));
    TEST_ASSERT(cJSON_GetNumberValue(cJSON_GetArrayItem(arr, 0)) == 100.0, "replaced item");

    cJSON_Delete(arr);
}

static void test_object_operations(void) {
    printf("test_object_operations\n");
    cJSON *obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, str_from_lit("a"), 1.0);
    cJSON_AddNumberToObject(obj, str_from_lit("b"), 2.0);

    cJSON_ReplaceItemInObject(obj, str_from_lit("a"), cJSON_CreateString(str_from_lit("replaced")));
    TEST_ASSERT(cJSON_IsString(cJSON_GetObjectItem(obj, str_from_lit("a"))), "replaced a to string");

    cJSON *detached = cJSON_DetachItemFromObject(obj, str_from_lit("b"));
    TEST_ASSERT(detached != 0, "detached b");
    TEST_ASSERT(cJSON_GetObjectItem(obj, str_from_lit("b")) == 0, "b no longer in obj");
    cJSON_Delete(detached);

    cJSON_Delete(obj);
}

static void test_compare(void) {
    printf("test_compare\n");
    cJSON *a = cJSON_Parse(str_from_lit("{\"x\":1,\"y\":2}"));
    cJSON *b = cJSON_Parse(str_from_lit("{\"y\":2,\"x\":1}"));
    TEST_ASSERT(cJSON_Compare(a, b, 0), "compare objects different order");
    cJSON_Delete(a);
    cJSON_Delete(b);

    a = cJSON_Parse(str_from_lit("[1,2,3]"));
    b = cJSON_Parse(str_from_lit("[1,2,4]"));
    TEST_ASSERT(!cJSON_Compare(a, b, 0), "compare different arrays");
    cJSON_Delete(a);
    cJSON_Delete(b);
}

static void test_version(void) {
    printf("test_version\n");
    Str ver = cJSON_Version();
    TEST_ASSERT(ver.ptr != 0, "version not null");
    TEST_ASSERT(ver == str_from_lit("1.7.19"), "version is 1.7.19");
}

static void test_duplicate(void) {
    printf("test_duplicate\n");
    cJSON *original = cJSON_Parse(str_from_lit("{\"a\":1,\"b\":\"hello\"}"));
    cJSON *dup = cJSON_Duplicate(original, 1);
    TEST_ASSERT(dup != 0, "duplicate not null");
    TEST_ASSERT(cJSON_Compare(original, dup, 1), "duplicate compare");
    TEST_ASSERT(dup != original, "duplicate different pointer");

    cJSON_Delete(original);
    cJSON_Delete(dup);
}

static void test_minify(void) {
    printf("test_minify\n");
    char json[] = "{\n  \"a\":  1,\n  \"b\":  \"hello\"\n}";
    cJSON_Minify(json);
    TEST_ASSERT(strcmp(json, "{\"a\":1,\"b\":\"hello\"}") == 0, "minify correct");
}

static void test_parse_errors(void) {
    printf("test_parse_errors\n");
    cJSON *result;

    result = cJSON_Parse(str_from_lit("{"));
    TEST_ASSERT(result == 0, "parse incomplete object returns null");
    cJSON_Delete(result);

    result = cJSON_Parse(str_from_lit("["));
    TEST_ASSERT(result == 0, "parse incomplete array returns null");
    cJSON_Delete(result);

    result = cJSON_Parse(str_from_lit("invalid"));
    TEST_ASSERT(result == 0, "parse invalid returns null");
    cJSON_Delete(result);
}

static void test_bulk_create(void) {
    printf("test_bulk_create\n");
    int ints[] = {1, 2, 3, 4, 5};
    cJSON *arr = cJSON_CreateIntArray(ints, 5);
    TEST_ASSERT(arr != 0, "create int array");
    TEST_ASSERT(cJSON_GetArraySize(arr) == 5, "int array size 5");
    TEST_ASSERT(cJSON_GetNumberValue(cJSON_GetArrayItem(arr, 2)) == 3.0, "int arr[2] == 3");
    cJSON_Delete(arr);

    const char *strs[] = {"a", "b", "c"};
    arr = cJSON_CreateStringArray(strs, 3);
    TEST_ASSERT(arr != 0, "create string array");
    TEST_ASSERT(cJSON_GetArraySize(arr) == 3, "string array size 3");
    Str _s = cJSON_GetStringValue(cJSON_GetArrayItem(arr, 1)); TEST_ASSERT(_s == str_from_lit("b"), "str arr[1] == b");
    cJSON_Delete(arr);

    double doubles[] = {1.1, 2.2};
    arr = cJSON_CreateDoubleArray(doubles, 2);
    TEST_ASSERT(arr != 0, "create double array");
    TEST_ASSERT(cJSON_GetArraySize(arr) == 2, "double array size 2");
    cJSON_Delete(arr);
}

static void test_parse_unicode(void) {
    printf("test_parse_unicode\n");
    cJSON *root = cJSON_Parse(str_from_lit("\"hello\\nworld\""));
    TEST_ASSERT(root != 0, "parse escape");
    Str _s = cJSON_GetStringValue(root); TEST_ASSERT(_s == str_from_lit("hello\nworld"), "escape newline");
    cJSON_Delete(root);

    root = cJSON_Parse(str_from_lit("\"tab\\there\""));
    TEST_ASSERT(root != 0, "parse tab escape");
    Str _s2 = cJSON_GetStringValue(root); TEST_ASSERT(_s2 == str_from_lit("tab\there"), "escape tab");
    cJSON_Delete(root);
}

static void test_parse_bool_in_context(void) {
    printf("test_parse_bool_in_context\n");
    cJSON *root = cJSON_Parse(str_from_lit("true"));
    TEST_ASSERT(cJSON_IsTrue(root), "top-level true");
    cJSON_Delete(root);

    root = cJSON_Parse(str_from_lit("false"));
    TEST_ASSERT(cJSON_IsFalse(root), "top-level false");
    cJSON_Delete(root);

    root = cJSON_Parse(str_from_lit("null"));
    TEST_ASSERT(cJSON_IsNull(root), "top-level null");
    cJSON_Delete(root);
}

static void test_set_value(void) {
    printf("test_set_value\n");
    cJSON *n = cJSON_CreateNumber(0.0);
    cJSON_SetNumberValue(n, 42.5);
    TEST_ASSERT(cJSON_GetNumberValue(n) == 42.5, "set number value");
    cJSON_Delete(n);

    cJSON *s = cJSON_CreateString(str_from_lit("old"));
    cJSON_SetValuestring(s, str_from_lit("new"));
    Str _s = cJSON_GetStringValue(s); TEST_ASSERT(_s == str_from_lit("new"), "set string value");
    cJSON_Delete(s);
}

static void test_error_ptr(void) {
    printf("test_error_ptr\n");
    cJSON *r = cJSON_Parse(str_from_lit("invalid"));
    TEST_ASSERT(r == 0, "bad parse returns null");
    Str err = cJSON_GetErrorPtr();
    TEST_ASSERT(err.ptr != 0, "error ptr not null after parse failure");
    TEST_ASSERT(err.len > 0, "error ptr has content");
}

static void test_parse_with_length(void) {
    printf("test_parse_with_length\n");
    cJSON *root = cJSON_ParseWithLength(str_from_lit("{\"a\":1}"), 7);
    TEST_ASSERT(root != 0, "parse with length");
    TEST_ASSERT(cJSON_IsObject(root), "parsed is object");
    TEST_ASSERT(cJSON_GetNumberValue(cJSON_GetObjectItem(root, str_from_lit("a"))) == 1.0, "value correct");
    cJSON_Delete(root);

    root = cJSON_ParseWithLength(str_from_lit("{\"a\":1,\"b\":2}"), 7);
    /* 7 chars = {"a":1, which is incomplete JSON -- parser correctly returns NULL */
    TEST_ASSERT(root == 0, "parse with length truncates returns null");
    /* no cJSON_Delete: root is null */
}

static void test_parse_with_opts_null_terminated(void) {
    printf("test_parse_with_opts_null_terminated\n");
    const char *end = NULL;
    cJSON *root = cJSON_ParseWithOpts(str_from_lit("123abc"), &end, 1);
    TEST_ASSERT(root == 0, "require null terminated fails");

    root = cJSON_ParseWithOpts(str_from_lit("123"), &end, 1);
    TEST_ASSERT(root != 0, "valid null terminated succeeds");
    TEST_ASSERT(cJSON_IsNumber(root), "parsed number");
    cJSON_Delete(root);
}

static void test_print_buffered(void) {
    printf("test_print_buffered\n");
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, str_from_lit("x"), 42.0);

    char *printed = cJSON_PrintBuffered(root, 64, 0);
    TEST_ASSERT(printed != 0, "print buffered not null");
    TEST_ASSERT(strcmp(printed, "{\"x\":42}") == 0, "print buffered unformatted correct");
    free(printed);

    printed = cJSON_PrintBuffered(root, 64, 1);
    TEST_ASSERT(printed != 0, "print buffered formatted not null");
    free(printed);

    cJSON_Delete(root);
}

static void test_print_preallocated(void) {
    printf("test_print_preallocated\n");
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, str_from_lit("x"), 1.0);

    char buffer[64];
    memset(buffer, 0, sizeof(buffer));
    cJSON_bool result = cJSON_PrintPreallocated(root, buffer, sizeof(buffer), 0);
    TEST_ASSERT(result == 1, "print preallocated success");
    TEST_ASSERT(strcmp(buffer, "{\"x\":1}") == 0, "preallocated content correct");

    result = cJSON_PrintPreallocated(root, buffer, 5, 0);
    TEST_ASSERT(result == 0, "print preallocated buffer too small fails");

    result = cJSON_PrintPreallocated(root, NULL, 64, 0);
    TEST_ASSERT(result == 0, "print preallocated null buffer fails");

    result = cJSON_PrintPreallocated(root, buffer, -1, 0);
    TEST_ASSERT(result == 0, "print preallocated negative length fails");

    cJSON_Delete(root);
}

static void test_create_raw(void) {
    printf("test_create_raw\n");
    cJSON *raw = cJSON_CreateRaw(str_from_lit("true"));
    TEST_ASSERT(cJSON_IsRaw(raw), "create raw");
    TEST_ASSERT(strcmp(raw->valuestring, "true") == 0, "raw value correct");

    char *printed = cJSON_PrintUnformatted(raw);
    TEST_ASSERT(strcmp(printed, "true") == 0, "print raw correct");
    free(printed);
    cJSON_Delete(raw);
}

static void test_create_references(void) {
    printf("test_create_references\n");
    cJSON *arr = cJSON_CreateArray();
    cJSON_AddItemToArray(arr, cJSON_CreateNumber(1.0));

    cJSON *ref = cJSON_CreateArrayReference(arr);
    TEST_ASSERT(ref != 0, "create array reference");
    TEST_ASSERT((ref->type & cJSON_IsReference) != 0, "reference flag set");
    TEST_ASSERT(ref->child == arr, "reference points to original");
    cJSON_Delete(ref);

    cJSON *obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, str_from_lit("a"), 1.0);
    ref = cJSON_CreateObjectReference(obj);
    TEST_ASSERT(ref != 0, "create object reference");
    TEST_ASSERT((ref->type & cJSON_IsReference) != 0, "reference flag set");
    cJSON_Delete(ref);

    cJSON *str_ref = cJSON_CreateStringReference(str_from_lit("hello"));
    TEST_ASSERT(str_ref != 0, "create string reference");
    TEST_ASSERT((str_ref->type & cJSON_IsReference) != 0, "reference flag set");
    TEST_ASSERT(strcmp(str_ref->valuestring, "hello") == 0, "string ref value correct");
    cJSON_Delete(str_ref);

    cJSON_Delete(arr);
    cJSON_Delete(obj);
}

static void test_add_convenience_funcs(void) {
    printf("test_add_convenience_funcs\n");
    cJSON *obj = cJSON_CreateObject();

    cJSON *null_item = cJSON_AddNullToObject(obj, str_from_lit("n"));
    TEST_ASSERT(null_item != 0, "add null to object");
    TEST_ASSERT(cJSON_IsNull(null_item), "added item is null");

    cJSON *bool_item = cJSON_AddBoolToObject(obj, str_from_lit("b"), 1);
    TEST_ASSERT(bool_item != 0, "add bool to object");
    TEST_ASSERT(cJSON_IsTrue(bool_item), "added bool is true");

    bool_item = cJSON_AddBoolToObject(obj, str_from_lit("f"), 0);
    TEST_ASSERT(bool_item != 0, "add false to object");
    TEST_ASSERT(cJSON_IsFalse(bool_item), "added bool is false");

    cJSON *raw_item = cJSON_AddRawToObject(obj, str_from_lit("r"), str_from_lit("42"));
    TEST_ASSERT(raw_item != 0, "add raw to object");
    TEST_ASSERT(cJSON_IsRaw(raw_item), "added item is raw");

    cJSON *nested_obj = cJSON_AddObjectToObject(obj, str_from_lit("nested"));
    TEST_ASSERT(nested_obj != 0, "add object to object");
    TEST_ASSERT(cJSON_IsObject(nested_obj), "added item is object");
    cJSON_AddNumberToObject(nested_obj, str_from_lit("x"), 1.0);

    cJSON *nested_arr = cJSON_AddArrayToObject(obj, str_from_lit("arr"));
    TEST_ASSERT(nested_arr != 0, "add array to object");
    TEST_ASSERT(cJSON_IsArray(nested_arr), "added item is array");
    cJSON_AddItemToArray(nested_arr, cJSON_CreateNumber(1.0));

    char *printed = cJSON_PrintUnformatted(obj);
    TEST_ASSERT(printed != 0, "print complex object");
    free(printed);

    cJSON_Delete(obj);
}

static void test_detach_operations(void) {
    printf("test_detach_operations\n");
    cJSON *arr = cJSON_CreateArray();
    cJSON_AddItemToArray(arr, cJSON_CreateNumber(1.0));
    cJSON_AddItemToArray(arr, cJSON_CreateNumber(2.0));
    cJSON_AddItemToArray(arr, cJSON_CreateNumber(3.0));

    cJSON *detached = cJSON_DetachItemFromArray(arr, 1);
    TEST_ASSERT(detached != 0, "detach from array");
    TEST_ASSERT(cJSON_GetArraySize(arr) == 2, "array size after detach");
    TEST_ASSERT(cJSON_GetNumberValue(detached) == 2.0, "detached value correct");
    cJSON_Delete(detached);

    detached = cJSON_DetachItemFromArray(arr, 0);
    TEST_ASSERT(detached != 0, "detach first from array");
    TEST_ASSERT(cJSON_GetNumberValue(detached) == 1.0, "detached first value correct");
    cJSON_Delete(detached);
    cJSON_Delete(arr);

    cJSON *obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, str_from_lit("a"), 1.0);
    cJSON_AddNumberToObject(obj, str_from_lit("b"), 2.0);

    detached = cJSON_DetachItemFromObject(obj, str_from_lit("a"));
    TEST_ASSERT(detached != 0, "detach from object");
    TEST_ASSERT(cJSON_GetObjectItem(obj, str_from_lit("a")) == 0, "a no longer in object");
    cJSON_Delete(detached);

    detached = cJSON_DetachItemFromObjectCaseSensitive(obj, str_from_lit("B"));
    TEST_ASSERT(detached == 0, "case sensitive detach fails");

    detached = cJSON_DetachItemFromObjectCaseSensitive(obj, str_from_lit("b"));
    TEST_ASSERT(detached != 0, "case sensitive detach succeeds");
    cJSON_Delete(detached);
    cJSON_Delete(obj);
}

static void test_delete_items(void) {
    printf("test_delete_items\n");
    cJSON *arr = cJSON_CreateArray();
    cJSON_AddItemToArray(arr, cJSON_CreateNumber(1.0));
    cJSON_AddItemToArray(arr, cJSON_CreateNumber(2.0));
    cJSON_AddItemToArray(arr, cJSON_CreateNumber(3.0));

    cJSON_DeleteItemFromArray(arr, 1);
    TEST_ASSERT(cJSON_GetArraySize(arr) == 2, "delete item from array");
    TEST_ASSERT(cJSON_GetNumberValue(cJSON_GetArrayItem(arr, 1)) == 3.0, "remaining item correct");
    cJSON_Delete(arr);

    cJSON *obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, str_from_lit("a"), 1.0);
    cJSON_AddNumberToObject(obj, str_from_lit("b"), 2.0);

    cJSON_DeleteItemFromObject(obj, str_from_lit("a"));
    TEST_ASSERT(cJSON_GetObjectItem(obj, str_from_lit("a")) == 0, "a deleted from object");
    TEST_ASSERT(cJSON_GetObjectItem(obj, str_from_lit("b")) != 0, "b still in object");

    cJSON_DeleteItemFromObjectCaseSensitive(obj, str_from_lit("b"));
    TEST_ASSERT(cJSON_GetObjectItem(obj, str_from_lit("b")) == 0, "b deleted case sensitive");
    cJSON_Delete(obj);
}

static void test_replace_operations(void) {
    printf("test_replace_operations\n");
    cJSON *arr = cJSON_CreateArray();
    cJSON_AddItemToArray(arr, cJSON_CreateNumber(1.0));
    cJSON_AddItemToArray(arr, cJSON_CreateNumber(2.0));

    cJSON_bool result = cJSON_ReplaceItemInArray(arr, 0, cJSON_CreateNumber(100.0));
    TEST_ASSERT(result == 1, "replace in array success");
    TEST_ASSERT(cJSON_GetNumberValue(cJSON_GetArrayItem(arr, 0)) == 100.0, "replaced value correct");

    result = cJSON_ReplaceItemInArray(arr, 10, cJSON_CreateNumber(999.0));
    TEST_ASSERT(result == 0, "replace out of bounds fails");
    cJSON_Delete(arr);

    cJSON *obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, str_from_lit("x"), 1.0);

    result = cJSON_ReplaceItemInObject(obj, str_from_lit("x"), cJSON_CreateString(str_from_lit("replaced")));
    TEST_ASSERT(result == 1, "replace in object success");
    TEST_ASSERT(cJSON_IsString(cJSON_GetObjectItem(obj, str_from_lit("x"))), "replaced type correct");

    result = cJSON_ReplaceItemInObjectCaseSensitive(obj, str_from_lit("X"), cJSON_CreateNumber(999.0));
    TEST_ASSERT(result == 0, "case sensitive replace fails for different case");

    result = cJSON_ReplaceItemInObjectCaseSensitive(obj, str_from_lit("x"), cJSON_CreateNumber(42.0));
    TEST_ASSERT(result == 1, "case sensitive replace succeeds");
    TEST_ASSERT(cJSON_GetNumberValue(cJSON_GetObjectItem(obj, str_from_lit("x"))) == 42.0, "replaced value correct");
    cJSON_Delete(obj);
}

static void test_replace_via_pointer(void) {
    printf("test_replace_via_pointer\n");
    cJSON *arr = cJSON_CreateArray();
    cJSON_AddItemToArray(arr, cJSON_CreateNumber(1.0));
    cJSON_AddItemToArray(arr, cJSON_CreateNumber(2.0));

    cJSON *old_item = cJSON_GetArrayItem(arr, 0);
    cJSON_bool result = cJSON_ReplaceItemViaPointer(arr, old_item, cJSON_CreateNumber(100.0));
    TEST_ASSERT(result == 1, "replace via pointer success");
    TEST_ASSERT(cJSON_GetNumberValue(cJSON_GetArrayItem(arr, 0)) == 100.0, "replaced value correct");

    result = cJSON_ReplaceItemViaPointer(NULL, old_item, cJSON_CreateNumber(1.0));
    TEST_ASSERT(result == 0, "replace via pointer null parent fails");

    cJSON_Delete(arr);
}

static void test_detach_via_pointer(void) {
    printf("test_detach_via_pointer\n");
    cJSON *arr = cJSON_CreateArray();
    cJSON_AddItemToArray(arr, cJSON_CreateNumber(1.0));
    cJSON_AddItemToArray(arr, cJSON_CreateNumber(2.0));
    cJSON_AddItemToArray(arr, cJSON_CreateNumber(3.0));

    cJSON *middle = cJSON_GetArrayItem(arr, 1);
    cJSON *detached = cJSON_DetachItemViaPointer(arr, middle);
    TEST_ASSERT(detached != 0, "detach middle via pointer");
    TEST_ASSERT(cJSON_GetArraySize(arr) == 2, "size after detach middle");
    TEST_ASSERT(cJSON_GetNumberValue(detached) == 2.0, "detached value correct");
    cJSON_Delete(detached);

    cJSON *first = cJSON_GetArrayItem(arr, 0);
    detached = cJSON_DetachItemViaPointer(arr, first);
    TEST_ASSERT(detached != 0, "detach first via pointer");
    TEST_ASSERT(cJSON_GetArraySize(arr) == 1, "size after detach first");
    cJSON_Delete(detached);

    cJSON_Delete(arr);
}

static void test_has_object_item(void) {
    printf("test_has_object_item\n");
    cJSON *obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, str_from_lit("key"), 1.0);

    TEST_ASSERT(cJSON_HasObjectItem(obj, str_from_lit("key")) == 1, "has key");
    TEST_ASSERT(cJSON_HasObjectItem(obj, str_from_lit("KEY")) == 1, "has key case insensitive");
    TEST_ASSERT(cJSON_HasObjectItem(obj, str_from_lit("missing")) == 0, "does not have missing key");

    cJSON_Delete(obj);
}

static void test_get_number_value_nan_inf(void) {
    printf("test_get_number_value_nan_inf\n");
    cJSON *n = cJSON_CreateNumber(0.0);
    cJSON_SetNumberValue(n, 123.456);
    TEST_ASSERT(cJSON_GetNumberValue(n) == 123.456, "get number value after set");
    cJSON_Delete(n);
}

static void test_malloc_free(void) {
    printf("test_malloc_free\n");
    void *ptr = cJSON_malloc(100);
    TEST_ASSERT(ptr != 0, "cJSON_malloc returns pointer");
    cJSON_free(ptr);
}

static void test_array_foreach(void) {
    printf("test_array_foreach\n");
    cJSON *arr = cJSON_CreateArray();
    cJSON_AddItemToArray(arr, cJSON_CreateNumber(10.0));
    cJSON_AddItemToArray(arr, cJSON_CreateNumber(20.0));
    cJSON_AddItemToArray(arr, cJSON_CreateNumber(30.0));

    int sum = 0;
    int count = 0;
    cJSON *elem = NULL;
    cJSON_ArrayForEach(elem, arr) {
        sum += (int)cJSON_GetNumberValue(elem);
        count++;
    }
    TEST_ASSERT(sum == 60, "array foreach sum correct");
    TEST_ASSERT(count == 3, "array foreach count correct");
    cJSON_Delete(arr);
}

static void test_nested_print_parse(void) {
    printf("test_nested_print_parse\n");
    cJSON *root = cJSON_CreateObject();

    cJSON *inner_obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(inner_obj, str_from_lit("inner_num"), 42.0);
    cJSON_AddStringToObject(inner_obj, str_from_lit("inner_str"), str_from_lit("hello"));
    cJSON_AddItemToObject(root, str_from_lit("inner"), inner_obj);

    cJSON *inner_arr = cJSON_CreateArray();
    cJSON_AddItemToArray(inner_arr, cJSON_CreateNumber(1.0));
    cJSON_AddItemToArray(inner_arr, cJSON_CreateNumber(2.0));
    cJSON_AddItemToObject(root, str_from_lit("arr"), inner_arr);

    char *printed = cJSON_Print(root);
    cJSON *parsed = cJSON_Parse(str_from_lit(printed));
    TEST_ASSERT(parsed != 0, "nested roundtrip parse");
    TEST_ASSERT(cJSON_Compare(root, parsed, 0), "nested roundtrip compare");

    cJSON_Delete(parsed);
    free(printed);
    cJSON_Delete(root);
}

static void test_add_item_to_object_cs(void) {
    printf("test_add_item_to_object_cs\n");
    cJSON *obj = cJSON_CreateObject();
    cJSON_bool result = cJSON_AddItemToObjectCS(obj, str_from_lit("key"), cJSON_CreateNumber(1.0));
    TEST_ASSERT(result == 1, "add item object cs success");
    TEST_ASSERT(cJSON_GetObjectItemCaseSensitive(obj, str_from_lit("key")) != 0, "key exists case sensitive");
    TEST_ASSERT(cJSON_GetObjectItem(obj, str_from_lit("KEY")) != 0, "key exists case insensitive");
    cJSON_Delete(obj);
}

static void test_add_reference(void) {
    printf("test_add_reference\n");
    cJSON *arr1 = cJSON_CreateArray();
    cJSON *arr2 = cJSON_CreateArray();
    cJSON *num = cJSON_CreateNumber(42.0);

    cJSON_bool result = cJSON_AddItemReferenceToArray(arr1, num);
    TEST_ASSERT(result == 1, "add reference to array");
    TEST_ASSERT(cJSON_GetArraySize(arr1) == 1, "array size after reference");

    cJSON *obj = cJSON_CreateObject();
    result = cJSON_AddItemReferenceToObject(obj, str_from_lit("ref"), num);
    TEST_ASSERT(result == 1, "add reference to object");
    TEST_ASSERT(cJSON_GetObjectItem(obj, str_from_lit("ref")) != 0, "reference exists");

    cJSON_Delete(arr1);
    cJSON_Delete(arr2);
    cJSON_Delete(num);
    cJSON_Delete(obj);
}

static void test_edge_cases(void) {
    printf("test_edge_cases\n");
    cJSON *r;

    r = cJSON_Parse(str_from_lit(""));
    TEST_ASSERT(r == 0, "parse empty string returns null");
    cJSON_Delete(r);

    r = cJSON_Parse(str_from_parts(0, 0));
    TEST_ASSERT(r == 0, "parse null returns null");
    cJSON_Delete(r);

    r = cJSON_ParseWithLength(str_from_parts(0, 0), 10);
    TEST_ASSERT(r == 0, "parse with length null returns null");
    cJSON_Delete(r);

    r = cJSON_ParseWithLength(str_from_lit(""), 0);
    TEST_ASSERT(r == 0, "parse with length zero returns null");
    cJSON_Delete(r);

    r = cJSON_Parse(str_from_lit("   "));
    TEST_ASSERT(r == 0, "parse whitespace only returns null");
    cJSON_Delete(r);
}

static void test_deeply_nested(void) {
    printf("test_deeply_nested\n");
    char json[3000];
    char *p = json;
    for (int i = 0; i < 100; i++) {
        *p++ = '[';
    }
    *p++ = '1';
    for (int i = 0; i < 100; i++) {
        *p++ = ']';
    }
    *p = '\0';

    cJSON *root = cJSON_Parse(str_from_lit(json));
    TEST_ASSERT(root != 0, "parse 100 levels deep");
    TEST_ASSERT(cJSON_IsArray(root), "parsed is array");
    cJSON_Delete(root);
}

static void test_number_saturation(void) {
    printf("test_number_saturation\n");
    cJSON *n = cJSON_CreateNumber(1e20);
    TEST_ASSERT(n->valueint == INT_MAX, "large number saturates to INT_MAX");
    cJSON_Delete(n);

    n = cJSON_CreateNumber(-1e20);
    TEST_ASSERT(n->valueint == INT_MIN, "small number saturates to INT_MIN");
    cJSON_Delete(n);
}

static void test_string_escapes(void) {
    printf("test_string_escapes\n");
    cJSON *root = cJSON_Parse(str_from_lit("\"line1\\nline2\""));
    TEST_ASSERT(root != 0, "parse newline escape");
    Str _s = cJSON_GetStringValue(root); TEST_ASSERT(_s == str_from_lit("line1\nline2"), "newline escape correct");
    cJSON_Delete(root);

    root = cJSON_Parse(str_from_lit("\"tab\\there\""));
    Str _s2 = cJSON_GetStringValue(root); TEST_ASSERT(_s2 == str_from_lit("tab\there"), "tab escape correct");
    cJSON_Delete(root);

    root = cJSON_Parse(str_from_lit("\"quote\\\"here\""));
    Str _s5 = cJSON_GetStringValue(root); TEST_ASSERT(_s5 == str_from_lit("quote\"here"), "quote escape correct");
    cJSON_Delete(root);

    root = cJSON_Parse(str_from_lit("\"backslash\\\\here\""));
    Str _s3 = cJSON_GetStringValue(root); TEST_ASSERT(_s3 == str_from_lit("backslash\\here"), "backslash escape correct");
    cJSON_Delete(root);

    root = cJSON_Parse(str_from_lit("\"slash\\/here\""));
    Str _s4 = cJSON_GetStringValue(root); TEST_ASSERT(_s4 == str_from_lit("slash/here"), "slash escape correct");
    cJSON_Delete(root);
}

static void test_invalid_type_checks(void) {
    printf("test_invalid_type_checks\n");
    cJSON *item = cJSON_CreateNull();
    TEST_ASSERT(!cJSON_IsInvalid(item), "null is not invalid");
    TEST_ASSERT(!cJSON_IsNumber(item), "null is not number");
    TEST_ASSERT(!cJSON_IsString(item), "null is not string");
    TEST_ASSERT(!cJSON_IsArray(item), "null is not array");
    TEST_ASSERT(!cJSON_IsObject(item), "null is not object");
    TEST_ASSERT(!cJSON_IsRaw(item), "null is not raw");
    TEST_ASSERT(!cJSON_IsTrue(item), "null is not true");
    TEST_ASSERT(!cJSON_IsFalse(item), "null is not false");
    cJSON_Delete(item);

    TEST_ASSERT(cJSON_IsInvalid(NULL) == 0, "null pointer not invalid type");
    TEST_ASSERT(cJSON_IsNumber(NULL) == 0, "null pointer not number");
    TEST_ASSERT(cJSON_IsString(NULL) == 0, "null pointer not string");
}

static void test_compare_edge_cases(void) {
    printf("test_compare_edge_cases\n");
    TEST_ASSERT(cJSON_Compare(NULL, NULL, 0) == 0, "compare nulls returns false");

    cJSON *a = cJSON_CreateNull();
    cJSON *b = cJSON_CreateNull();
    TEST_ASSERT(cJSON_Compare(a, b, 0) == 1, "compare nulls equal");
    cJSON_Delete(a);
    cJSON_Delete(b);

    a = cJSON_CreateTrue();
    b = cJSON_CreateTrue();
    TEST_ASSERT(cJSON_Compare(a, b, 0) == 1, "compare trues equal");
    cJSON_Delete(a);
    cJSON_Delete(b);

    a = cJSON_Parse(str_from_lit("\"hello\""));
    b = cJSON_Parse(str_from_lit("\"world\""));
    TEST_ASSERT(cJSON_Compare(a, b, 0) == 0, "compare different strings");
    cJSON_Delete(a);
    cJSON_Delete(b);
}

static void test_duplicate_non_recursive(void) {
    printf("test_duplicate_non_recursive\n");
    cJSON *obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, str_from_lit("a"), 1.0);
    cJSON_AddStringToObject(obj, str_from_lit("b"), str_from_lit("hello"));

    cJSON *dup = cJSON_Duplicate(obj, 0);
    TEST_ASSERT(dup != 0, "non-recursive duplicate not null");
    TEST_ASSERT(cJSON_IsObject(dup), "duplicate is object");
    /* recurse=0: copies the item itself but NOT its children (array/object items).
     * Both "a" and "b" are children of obj, so neither should appear in dup. */
    TEST_ASSERT(cJSON_GetObjectItem(dup, str_from_lit("a")) == 0, "non-recursive does not copy first child");
    TEST_ASSERT(cJSON_GetObjectItem(dup, str_from_lit("b")) == 0, "non-recursive does not copy children");
    cJSON_Delete(dup);
    cJSON_Delete(obj);
}

static void test_minify_edge_cases(void) {
    printf("test_minify_edge_cases\n");
    cJSON_Minify(NULL);

    char empty[] = "";
    cJSON_Minify(empty);
    TEST_ASSERT(strcmp(empty, "") == 0, "minify empty string");

    char spaces[] = "   \t\n\r  ";
    cJSON_Minify(spaces);
    TEST_ASSERT(strcmp(spaces, "") == 0, "minify whitespace only");

    char with_comments[] = "/* comment */ {\"a\":1} // line comment";
    cJSON_Minify(with_comments);
    TEST_ASSERT(strcmp(with_comments, "{\"a\":1}") == 0, "minify with comments");
}

static void test_float_array(void) {
    printf("test_float_array\n");
    float floats[] = {1.5f, 2.5f, 3.5f};
    cJSON *arr = cJSON_CreateFloatArray(floats, 3);
    TEST_ASSERT(arr != 0, "create float array");
    TEST_ASSERT(cJSON_GetArraySize(arr) == 3, "float array size 3");
    TEST_ASSERT(cJSON_GetNumberValue(cJSON_GetArrayItem(arr, 0)) == 1.5, "float arr[0] correct");
    TEST_ASSERT(cJSON_GetNumberValue(cJSON_GetArrayItem(arr, 1)) == 2.5, "float arr[1] correct");
    cJSON_Delete(arr);

    arr = cJSON_CreateFloatArray(NULL, 3);
    TEST_ASSERT(arr == 0, "create float array with null data");

    arr = cJSON_CreateFloatArray(floats, -1);
    TEST_ASSERT(arr == 0, "create float array with negative count");
}

static void test_parse_with_length_opts(void) {
    printf("test_parse_with_length_opts\n");
    const char *end = NULL;
    cJSON *root = cJSON_ParseWithLengthOpts(str_from_lit("{\"a\":1}"), 7, &end, 0);
    TEST_ASSERT(root != 0, "parse with length opts");
    TEST_ASSERT(cJSON_GetObjectItem(root, str_from_lit("a")) != 0, "has a");
    cJSON_Delete(root);

    root = cJSON_ParseWithLengthOpts(str_from_lit("123abc"), 6, &end, 1);
    TEST_ASSERT(root == 0, "require null terminated with length fails");

    /* buffer_length must include the null terminator for require_null_terminated to
     * work correctly (same as cJSON_ParseWithOpts which does strlen+1 internally). */
    root = cJSON_ParseWithLengthOpts(str_from_lit("123"), 4, &end, 1);
    TEST_ASSERT(root != 0, "require null terminated with length succeeds");
    cJSON_Delete(root);
}

static void test_get_string_and_number_value(void) {
    printf("test_get_string_and_number_value\n");
    cJSON *s = cJSON_CreateString(str_from_lit("test"));
    Str _s = cJSON_GetStringValue(s); TEST_ASSERT(_s == str_from_lit("test"), "get string value");
    TEST_ASSERT(cJSON_GetStringValue(cJSON_CreateNumber(1.0)).ptr == 0, "get string value from number returns null");
    cJSON_Delete(s);

    cJSON *n = cJSON_CreateNumber(42.0);
    TEST_ASSERT(cJSON_GetNumberValue(n) == 42.0, "get number value");
    TEST_ASSERT(cJSON_GetNumberValue(cJSON_CreateString(str_from_lit("test"))) != cJSON_GetNumberValue(cJSON_CreateString(str_from_lit("test"))), "get number value from string returns NaN");
    cJSON_Delete(n);
}

static void test_complex_json(void) {
    printf("test_complex_json\n");
    const char *json = 
        "{"
            "\"name\":\"John\","
            "\"age\":30,"
            "\"active\":true,"
            "\"scores\":[95,87,92],"
            "\"address\":{"
                "\"street\":\"123 Main St\","
                "\"city\":\"Anytown\""
            "}"
        "}";

    cJSON *root = cJSON_Parse(str_from_lit(json));
    TEST_ASSERT(root != 0, "parse complex json");
    TEST_ASSERT(cJSON_IsObject(root), "is object");

    Str _s = cJSON_GetStringValue(cJSON_GetObjectItem(root, str_from_lit("name"))); TEST_ASSERT(_s == str_from_lit("John"), "name correct");
    TEST_ASSERT(cJSON_GetNumberValue(cJSON_GetObjectItem(root, str_from_lit("age"))) == 30.0, "age correct");
    TEST_ASSERT(cJSON_IsTrue(cJSON_GetObjectItem(root, str_from_lit("active"))), "active correct");

    cJSON *scores = cJSON_GetObjectItem(root, str_from_lit("scores"));
    TEST_ASSERT(cJSON_IsArray(scores), "scores is array");
    TEST_ASSERT(cJSON_GetArraySize(scores) == 3, "scores size 3");
    TEST_ASSERT(cJSON_GetNumberValue(cJSON_GetArrayItem(scores, 0)) == 95.0, "score[0] correct");

    cJSON *address = cJSON_GetObjectItem(root, str_from_lit("address"));
    TEST_ASSERT(cJSON_IsObject(address), "address is object");
    Str _s2 = cJSON_GetStringValue(cJSON_GetObjectItem(address, str_from_lit("street"))); TEST_ASSERT(_s2 == str_from_lit("123 Main St"), "street correct");

    cJSON_Delete(root);
}

int main(void) {
    printf("=== cJSON Test Suite (original cJSON 1.7.19) ===\n\n");

    test_version();
    test_create_null();
    test_create_true_false();
    test_create_number();
    test_create_string();
    test_create_array();
    test_create_object();
    test_create_raw();
    test_create_references();
    test_parse_simple();
    test_parse_array();
    test_parse_object();
    test_parse_nested();
    test_parse_bool_in_context();
    test_parse_unicode();
    test_parse_errors();
    test_parse_with_length();
    test_parse_with_opts_null_terminated();
    test_parse_with_length_opts();
    test_print_unformatted();
    test_print_formatted();
    test_print_buffered();
    test_print_preallocated();
    test_print_and_parse_roundtrip();
    test_array_operations();
    test_object_operations();
    test_detach_operations();
    test_delete_items();
    test_replace_operations();
    test_replace_via_pointer();
    test_detach_via_pointer();
    test_compare();
    test_compare_edge_cases();
    test_duplicate();
    test_duplicate_non_recursive();
    test_minify();
    test_minify_edge_cases();
    test_bulk_create();
    test_float_array();
    test_set_value();
    test_error_ptr();
    test_add_convenience_funcs();
    test_add_item_to_object_cs();
    test_add_reference();
    test_has_object_item();
    test_get_number_value_nan_inf();
    test_get_string_and_number_value();
    test_malloc_free();
    test_array_foreach();
    test_nested_print_parse();
    test_edge_cases();
    test_deeply_nested();
    test_number_saturation();
    test_string_escapes();
    test_invalid_type_checks();
    test_complex_json();

    printf("\n=========================\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);

    if (tests_failed > 0) {
        printf("\nRESULT: SOME TESTS FAILED\n");
        return 1;
    }

    printf("\nRESULT: ALL TESTS PASSED\n");
    return 0;
}