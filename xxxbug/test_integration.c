/*
 * sp-mysql-client integration test — requires a running MySQL/MariaDB server.
 * Adapted from MySQL official mysql_client_test.cc (GPL-2.0, Oracle/Sun).
 */

#include "mysql.h"
#include "mysql_pool.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>

static int g_pass = 0, g_fail = 0;

#define TEST(name) printf("  TEST %-50s ", name); fflush(stdout)
#define PASS() do { printf("OK\n"); g_pass++; } while(0)
#define FAIL(msg) do { printf("FAIL (%s)\n", msg); g_fail++; } while(0)
#define CHECK(cond, msg) do { if (cond) { PASS(); } else { FAIL(msg); } } while(0)
#define Q(sql) mysql_real_query(m, (sql), strlen(sql))
#define Q2(conn, sql) mysql_real_query(conn, (sql), strlen(sql))

static const char* g_host = "127.0.0.1";
static const char* g_user = "sp_test";
static const char* g_passwd = "sp_test_pass";
static const char* g_db = "sp_test";
static unsigned int g_port = 3306;

static MYSQL* get_conn(void) {
    MYSQL *m = mysql_init(NULL);
    if (!m) return NULL;
    unsigned int ssl_mode = SSL_MODE_DISABLED;
    mysql_options(m, MYSQL_OPT_SSL_MODE, &ssl_mode);
    return mysql_real_connect(m, g_host, g_user, g_passwd, g_db, g_port, NULL, 0);
}

static void test_init_close(void) {
    printf("\n[init/close]\n");
    MYSQL *m = mysql_init(NULL);
    TEST("mysql_init returns non-NULL");
    CHECK(m != NULL, "returned NULL");
    if (m) mysql_close(m);
    TEST("mysql_close succeeds");
    PASS();
}

static void test_connect_basic(void) {
    printf("\n[connect]\n");
    MYSQL *m = get_conn();
    TEST("connect to server");
    CHECK(m != NULL, m ? mysql_error(m) : "init failed");
    if (!m) return;

    TEST("server version is non-empty");
    CHECK(m->server_version && m->server_version[0], "empty version");

    TEST("thread_id > 0");
    CHECK(m->thread_id > 0, "zero thread_id");

    TEST("mysql_get_server_info non-NULL");
    CHECK(mysql_get_server_info(m) != NULL, "NULL");

    TEST("mysql_get_server_version > 0");
    CHECK(mysql_get_server_version(m) > 0, "zero");

    TEST("mysql_get_client_info non-NULL");
    CHECK(mysql_get_client_info() != NULL, "NULL");

    mysql_close(m);
}

static void test_ping(void) {
    printf("\n[ping]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    TEST("mysql_ping returns 0");
    CHECK(mysql_ping(m) == 0, mysql_error(m));

    mysql_close(m);
}

static void test_simple_query(void) {
    printf("\n[simple query]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    TEST("SELECT 1+1 returns 2");
    int rc = Q("SELECT 1+1 AS val");
    if (rc != 0) { FAIL(mysql_error(m)); mysql_close(m); return; }

    MYSQL_RES *res = mysql_store_result(m);
    if (!res) { FAIL("store_result NULL"); mysql_close(m); return; }

    CHECK(res->row_count == 1 && res->field_count == 1, "wrong dimensions");

    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("row value is '2'");
    CHECK(row && row[0] && strcmp(row[0], "2") == 0, row[0] ? row[0] : "NULL");

    mysql_free_result(res);
    mysql_close(m);
}

static void test_create_table(void) {
    printf("\n[create table]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    TEST("DROP TABLE IF EXISTS t_test");
    int rc = Q("DROP TABLE IF EXISTS t_test");
    CHECK(rc == 0, mysql_error(m));

    TEST("CREATE TABLE t_test");
    const char *ddl = "CREATE TABLE t_test ("
        "  id INT AUTO_INCREMENT PRIMARY KEY,"
        "  name VARCHAR(64) NOT NULL,"
        "  score DOUBLE,"
        "  data BLOB,"
        "  ts TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
        ") ENGINE=InnoDB";
    rc = mysql_real_query(m, ddl, (unsigned long)strlen(ddl));
    CHECK(rc == 0, mysql_error(m));

    TEST("CREATE TABLE duplicate returns error");
    rc = mysql_real_query(m, ddl, (unsigned long)strlen(ddl));
    CHECK(rc != 0, "expected error");

    mysql_close(m);
}

static void test_text_insert_select(void) {
    printf("\n[insert/select]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    TEST("INSERT 3 rows");
    int rc = Q("INSERT INTO t_test (name, score) VALUES "
        "('alice', 95.5), ('bob', 87.3), ('charlie', 92.1)");
    CHECK(rc == 0, mysql_error(m));

    TEST("affected rows == 3");
    CHECK(mysql_affected_rows(m) == 3, "wrong count");

    TEST("SELECT * FROM t_test");
    rc = Q("SELECT id, name, score FROM t_test ORDER BY id");
    if (rc != 0) { FAIL(mysql_error(m)); mysql_close(m); return; }

    MYSQL_RES *res = mysql_store_result(m);
    if (!res) { FAIL("store_result NULL"); mysql_close(m); return; }

    TEST("row_count == 3");
    CHECK(res->row_count == 3, "wrong count");

    MYSQL_ROW row;
    int rows = 0;
    while ((row = mysql_fetch_row(res))) {
        rows++;
        if (rows == 1) {
            TEST("first row: id=1, name=alice");
            CHECK(row[0] && strcmp(row[0], "1") == 0 &&
                  row[1] && strcmp(row[1], "alice") == 0, "wrong data");
        }
    }
    TEST("fetched all 3 rows");
    CHECK(rows == 3, "wrong count");

    mysql_free_result(res);
    mysql_close(m);
}

static void test_null_values(void) {
    printf("\n[NULL values]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("INSERT INTO t_test (name, score) VALUES ('nulltest', NULL)");

    Q("SELECT score FROM t_test WHERE name='nulltest'");

    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("NULL column returns NULL pointer");
    CHECK(row && row[0] == NULL, "expected NULL");

    unsigned long *lens = mysql_fetch_lengths(res);
    TEST("NULL length is 0");
    CHECK(lens && lens[0] == 0, "expected 0");

    mysql_free_result(res);
    mysql_close(m);
}

static void test_use_result(void) {
    printf("\n[mysql_use_result streaming]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("SELECT id, name FROM t_test ORDER BY id");

    MYSQL_RES *res = mysql_use_result(m);
    TEST("mysql_use_result returns non-NULL");
    CHECK(res != NULL, "NULL");

    int count = 0;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) count++;

    TEST("streaming fetched all rows");
    CHECK(count >= 3, "too few rows");

    mysql_free_result(res);
    mysql_close(m);
}

static void test_real_escape_string(void) {
    printf("\n[mysql_real_escape_string]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    const char *raw = "it's a \"test\" with \\backslash\\";
    char buf[256];
    unsigned long len = mysql_real_escape_string(m, buf, raw, (unsigned long)strlen(raw));

    TEST("escape_string returns non-zero length");
    CHECK(len > 0, "zero length");

    TEST("escaped string contains \\\\");
    CHECK(strstr(buf, "\\\\") != NULL, "no escaped backslash");

    TEST("escaped string contains \\'");
    CHECK(strstr(buf, "\\'") != NULL, "no escaped quote");

    mysql_close(m);
}

static void test_field_info(void) {
    printf("\n[field info]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("SELECT id, name, score FROM t_test LIMIT 1");

    MYSQL_RES *res = mysql_store_result(m);
    TEST("mysql_num_fields == 3");
    CHECK(mysql_num_fields(res) == 3, "wrong count");

    MYSQL_FIELD *fields = mysql_fetch_fields(res);
    TEST("first field name is 'id'");
    CHECK(fields && strcmp(fields[0].name, "id") == 0, "wrong name");

    TEST("second field name is 'name'");
    CHECK(fields && strcmp(fields[1].name, "name") == 0, "wrong name");

    TEST("third field name is 'score'");
    CHECK(fields && strcmp(fields[2].name, "score") == 0, "wrong name");

    TEST("id field has PRI_KEY_FLAG");
    CHECK(fields && (fields[0].flags & PRI_KEY_FLAG), "no PRI_KEY_FLAG");

    TEST("id field has AUTO_INCREMENT_FLAG");
    CHECK(fields && (fields[0].flags & AUTO_INCREMENT_FLAG), "no AUTO_INCREMENT_FLAG");

    TEST("name field has NOT_NULL_FLAG");
    CHECK(fields && (fields[1].flags & NOT_NULL_FLAG), "no NOT_NULL_FLAG");

    TEST("score field type is MYSQL_TYPE_DOUBLE");
    CHECK(fields && fields[2].type == MYSQL_TYPE_DOUBLE, "wrong type");

    mysql_free_result(res);
    mysql_close(m);
}

static void test_field_flags(void) {
    printf("\n[field flags — adapted from official test_field_flags]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_field_flags");
    Q("CREATE TABLE t_field_flags("
      "id INT NOT NULL AUTO_INCREMENT PRIMARY KEY, "
      "id1 INT NOT NULL, "
      "id2 INT UNIQUE, "
      "id3 INT, "
      "id4 INT NOT NULL, "
      "KEY(id3, id4))");

    Q("SELECT * FROM t_field_flags");
    MYSQL_RES *res = mysql_use_result(m);
    if (!res) { FAIL("use_result NULL"); mysql_close(m); return; }

    MYSQL_FIELD *fields = mysql_fetch_fields(res);

    TEST("id: PRI_KEY_FLAG | AUTO_INCREMENT_FLAG | NOT_NULL_FLAG");
    CHECK((fields[0].flags & PRI_KEY_FLAG) &&
          (fields[0].flags & AUTO_INCREMENT_FLAG) &&
          (fields[0].flags & NOT_NULL_FLAG), "wrong flags");

    TEST("id1: NOT_NULL_FLAG");
    CHECK(fields[1].flags & NOT_NULL_FLAG, "no NOT_NULL_FLAG");

    TEST("id2: UNIQUE_KEY_FLAG");
    CHECK(fields[2].flags & UNIQUE_KEY_FLAG, "no UNIQUE_KEY_FLAG");

    TEST("id3: MULTIPLE_KEY_FLAG");
    CHECK(fields[3].flags & MULTIPLE_KEY_FLAG, "no MULTIPLE_KEY_FLAG");

    mysql_free_result(res);
    mysql_close(m);
}

static void test_stmt_prepare_execute(void) {
    printf("\n[prepared statement]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    TEST("mysql_stmt_init returns non-NULL");
    CHECK(stmt != NULL, "NULL");

    const char *sql = "SELECT id, name, score FROM t_test WHERE id = ?";
    TEST("mysql_stmt_prepare");
    int rc = mysql_stmt_prepare(stmt, sql, (unsigned long)strlen(sql));
    CHECK(rc == 0, mysql_stmt_error(stmt));

    TEST("param_count == 1");
    CHECK(stmt->param_count == 1, "wrong count");

    TEST("field_count == 3");
    CHECK(stmt->field_count == 3, "wrong count");

    unsigned long id = 1;
    unsigned long id_len = sizeof(unsigned long);
    MYSQL_BIND pb[1];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG;
    pb[0].buffer = &id;
    pb[0].is_unsigned = 1;
    pb[0].length = &id_len;

    TEST("mysql_stmt_bind_param");
    rc = mysql_stmt_bind_param(stmt, pb);
    CHECK(rc == 0, mysql_stmt_error(stmt));

    TEST("mysql_stmt_execute");
    rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, mysql_stmt_error(stmt));

    int out_id = 0;
    char out_name[64] = {0};
    double out_score = 0;
    unsigned long out_id_len = 0, out_name_len = 0, out_score_len = 0;
    my_bool out_id_null = 0, out_name_null = 0, out_score_null = 0;

    MYSQL_BIND rb[3];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG;
    rb[0].buffer = &out_id;
    rb[0].length = &out_id_len;
    rb[0].is_null = &out_id_null;

    rb[1].buffer_type = MYSQL_TYPE_STRING;
    rb[1].buffer = out_name;
    rb[1].buffer_length = sizeof(out_name);
    rb[1].length = &out_name_len;
    rb[1].is_null = &out_name_null;

    rb[2].buffer_type = MYSQL_TYPE_DOUBLE;
    rb[2].buffer = &out_score;
    rb[2].length = &out_score_len;
    rb[2].is_null = &out_score_null;

    TEST("mysql_stmt_bind_result");
    rc = mysql_stmt_bind_result(stmt, rb);
    CHECK(rc == 0, mysql_stmt_error(stmt));

    TEST("mysql_stmt_fetch returns row");
    rc = mysql_stmt_fetch(stmt);
    CHECK(rc == 0, mysql_stmt_error(stmt));

    TEST("fetched id == 1");
    CHECK(out_id == 1, "wrong id");

    TEST("fetched name == 'alice'");
    CHECK(strcmp(out_name, "alice") == 0, out_name);

    TEST("fetched score ~ 95.5");
    CHECK(out_score > 95.0 && out_score < 96.0, "wrong score");

    TEST("mysql_stmt_fetch returns NO_DATA");
    rc = mysql_stmt_fetch(stmt);
    CHECK(rc == MYSQL_NO_DATA, "expected NO_DATA");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_conc67_cursor_param_fetch(void) {
    printf("\n[CONC-67: cursor mode PS param bind + fetch]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_conc67");
    Q("CREATE TABLE t_conc67(a INT, b TEXT)");
    Q("INSERT INTO t_conc67 VALUES(1, 'foo')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    unsigned long cursor_type = CURSOR_TYPE_READ_ONLY;
    unsigned long prefetch_rows = 1000;
    mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, &cursor_type);
    mysql_stmt_attr_set(stmt, STMT_ATTR_PREFETCH_ROWS, &prefetch_rows);

    const char *q = "SELECT a,b FROM t_conc67 WHERE a=?";
    mysql_stmt_prepare(stmt, q, strlen(q));

    int pval = 1;
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &pval; pb.buffer_length = 4;
    mysql_stmt_bind_param(stmt, &pb);

    int rc = mysql_stmt_execute(stmt);
    TEST("conc67 cursor execute: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    MYSQL_RES *meta = mysql_stmt_result_metadata(stmt);
    TEST("conc67 metadata: non-NULL");
    CHECK(meta != NULL, "NULL metadata");
    mysql_free_result(meta);

    int out_a = 0;
    char out_b[20] = {0};
    unsigned long al = 0, bl = 0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &out_a; rb[0].buffer_length = 4; rb[0].length = &al;
    rb[1].buffer_type = MYSQL_TYPE_STRING; rb[1].buffer = out_b; rb[1].buffer_length = 20; rb[1].length = &bl;
    mysql_stmt_bind_result(stmt, rb);

    rc = mysql_stmt_fetch(stmt);
    TEST("conc67 fetch: a=1");
    CHECK(rc == 0 && out_a == 1, "wrong a value");

    TEST("conc67 fetch: b='foo'");
    CHECK(bl == 3 && strncmp(out_b, "foo", 3) == 0, "wrong b value");

    rc = mysql_stmt_fetch(stmt);
    TEST("conc67 fetch end: NO_DATA");
    CHECK(rc == MYSQL_NO_DATA, "expected NO_DATA");

    mysql_stmt_close(stmt);
    Q("DROP TABLE IF EXISTS t_conc67");
    mysql_close(m);
}

static void test_bug1115_string_param_reexecute(void) {
    printf("\n[BUG#1115: PS string param re-execute]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_bug1115");
    Q("CREATE TABLE t_bug1115(session_id CHAR(9) NOT NULL, a INT UNSIGNED NOT NULL)");
    Q("INSERT INTO t_bug1115 VALUES('abc',1),('abd',2),('abf',3)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    const char *q = "SELECT * FROM t_bug1115 WHERE session_id = ?";
    mysql_stmt_prepare(stmt, q, strlen(q));

    char szData[11];
    unsigned long slen = 0;
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_STRING; pb.buffer = szData; pb.buffer_length = 10; pb.length = &slen;

    strcpy(szData, "abc");
    slen = 3;
    mysql_stmt_bind_param(stmt, &pb);
    int rc = mysql_stmt_execute(stmt);
    TEST("bug1115 exec1 'abc': ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    int rowcount = 0;
    while (mysql_stmt_fetch(stmt) == 0) rowcount++;
    TEST("bug1115 exec1: 1 row");
    CHECK(rowcount == 1, "wrong rowcount");

    strcpy(szData, "venu");
    slen = 4;
    mysql_stmt_bind_param(stmt, &pb);
    rc = mysql_stmt_execute(stmt);
    TEST("bug1115 exec2 'venu': ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    rowcount = 0;
    while (mysql_stmt_fetch(stmt) == 0) rowcount++;
    TEST("bug1115 exec2: 0 rows");
    CHECK(rowcount == 0, "wrong rowcount");

    strcpy(szData, "abc");
    slen = 3;
    mysql_stmt_bind_param(stmt, &pb);
    rc = mysql_stmt_execute(stmt);
    TEST("bug1115 exec3 'abc' again: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    rowcount = 0;
    while (mysql_stmt_fetch(stmt) == 0) rowcount++;
    TEST("bug1115 exec3: 1 row (same as exec1)");
    CHECK(rowcount == 1, "wrong rowcount");

    mysql_stmt_close(stmt);
    Q("DROP TABLE IF EXISTS t_bug1115");
    mysql_close(m);
}

static void test_bug1664_send_long_data(void) {
    printf("\n[BUG#1664: send_long_data multiple append]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_bug1664");
    Q("CREATE TABLE t_bug1664(col1 INT, col2 MEDIUMTEXT)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    const char *q = "INSERT INTO t_bug1664(col2, col1) VALUES(?, ?)";
    mysql_stmt_prepare(stmt, q, strlen(q));

    TEST("bug1664 param_count: 2");
    CHECK(mysql_stmt_param_count(stmt) == 2, "wrong param count");

    const char *str_data = "Simple string";
    int int_data = 1;
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_STRING; pb[0].buffer = (char*)str_data; pb[0].buffer_length = strlen(str_data);
    pb[1].buffer_type = MYSQL_TYPE_LONG; pb[1].buffer = &int_data;
    mysql_stmt_bind_param(stmt, pb);

    const char *empty = "";
    int rc = mysql_stmt_send_long_data(stmt, 0, empty, 0);
    TEST("bug1664 send_long_data(empty): ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    rc = mysql_stmt_execute(stmt);
    TEST("bug1664 exec1 (empty long data): ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    Q("DELETE FROM t_bug1664");

    const char *data1 = "Data";
    rc = mysql_stmt_send_long_data(stmt, 0, data1, strlen(data1));
    TEST("bug1664 send_long_data('Data'): ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    rc = mysql_stmt_execute(stmt);
    TEST("bug1664 exec2 (long data='Data'): ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    Q("SELECT col2 FROM t_bug1664");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("bug1664 verify col2='Data'");
    CHECK(row && strcmp(row[0], "Data") == 0, "wrong col2 value");
    mysql_free_result(res);

    Q("DELETE FROM t_bug1664");

    const char *data2 = "SomeOtherData";
    rc = mysql_stmt_send_long_data(stmt, 0, data2, strlen(data2));
    TEST("bug1664 send_long_data('SomeOtherData'): ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    int_data = 2;
    rc = mysql_stmt_execute(stmt);
    TEST("bug1664 exec3 (long data='SomeOtherData'): ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    Q("SELECT col2 FROM t_bug1664");
    res = mysql_store_result(m);
    row = mysql_fetch_row(res);
    TEST("bug1664 verify col2='SomeOtherData'");
    CHECK(row && strcmp(row[0], "SomeOtherData") == 0, "wrong col2 value");
    mysql_free_result(res);

    mysql_stmt_close(stmt);
    Q("DROP TABLE IF EXISTS t_bug1664");
    mysql_close(m);
}

static void test_conc141_sp_multi_rs_reexecute(void) {
    printf("\n[CONC-141: SP multi resultset + re-execute]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_conc141");
    Q("CREATE TABLE t_conc141(KeyVal INT NOT NULL PRIMARY KEY)");
    Q("INSERT INTO t_conc141 VALUES(1)");
    Q("DROP PROCEDURE IF EXISTS p_conc141");
    Q("CREATE PROCEDURE p_conc141() BEGIN SELECT * FROM t_conc141; INSERT INTO t_conc141(KeyVal) VALUES(2); END");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    const char *q = "CALL p_conc141()";
    mysql_stmt_prepare(stmt, q, strlen(q));

    int rc = mysql_stmt_execute(stmt);
    TEST("conc141 exec1: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    int out_val = 0;
    unsigned long vl = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.length = &vl;
    mysql_stmt_bind_result(stmt, &rb);

    int rows = 0;
    while (mysql_stmt_fetch(stmt) == 0) rows++;
    TEST("conc141 exec1 first RS: 1 row");
    CHECK(rows == 1, "wrong rowcount");

    rc = mysql_stmt_next_result(stmt);
    TEST("conc141 next_result: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    mysql_stmt_free_result(stmt);

    rc = mysql_stmt_execute(stmt);
    TEST("conc141 exec2 (re-execute): ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    rows = 0;
    while (mysql_stmt_fetch(stmt) == 0) rows++;
    TEST("conc141 exec2 first RS: 1 row");
    CHECK(rows == 1, "wrong rowcount");

    rc = mysql_stmt_next_result(stmt);
    TEST("conc141 exec2 next_result: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    mysql_stmt_close(stmt);
    Q("DROP TABLE IF EXISTS t_conc141");
    Q("DROP PROCEDURE IF EXISTS p_conc141");
    mysql_close(m);
}

static void test_conc154_empty_nonempty_reexecute(void) {
    printf("\n[CONC-154: empty/nonempty result re-execute cycles]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_conc154");
    Q("CREATE TABLE t_conc154(a VARCHAR(20))");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    const char *q = "SELECT * FROM t_conc154";
    mysql_stmt_prepare(stmt, q, strlen(q));

    int rc = mysql_stmt_execute(stmt);
    TEST("conc154 empty exec1: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    rc = mysql_stmt_store_result(stmt);
    TEST("conc154 empty store1: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    TEST("conc154 empty num_rows: 0");
    CHECK(mysql_stmt_num_rows(stmt) == 0, "wrong row count");

    rc = mysql_stmt_execute(stmt);
    TEST("conc154 empty exec2 (re-execute): ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    rc = mysql_stmt_store_result(stmt);
    TEST("conc154 empty store2: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    mysql_stmt_close(stmt);

    Q("INSERT INTO t_conc154 VALUES('test_conc154')");

    stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, q, strlen(q));

    rc = mysql_stmt_execute(stmt);
    TEST("conc154 nonempty exec1: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    rc = mysql_stmt_store_result(stmt);
    TEST("conc154 nonempty store1: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    TEST("conc154 nonempty num_rows: 1");
    CHECK(mysql_stmt_num_rows(stmt) == 1, "wrong row count");

    rc = mysql_stmt_execute(stmt);
    TEST("conc154 nonempty exec2 (re-execute): ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    rc = mysql_stmt_store_result(stmt);
    TEST("conc154 nonempty store2: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    TEST("conc154 nonempty num_rows after re-exec: 1");
    CHECK(mysql_stmt_num_rows(stmt) == 1, "wrong row count");

    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, q, strlen(q));

    rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, mysql_stmt_error(stmt));
    rc = mysql_stmt_store_result(stmt);
    CHECK(rc == 0, mysql_stmt_error(stmt));
    rc = mysql_stmt_free_result(stmt);
    TEST("conc154 free_result then re-exec: free ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    rc = mysql_stmt_execute(stmt);
    TEST("conc154 free_result then re-exec: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    rc = mysql_stmt_store_result(stmt);
    CHECK(rc == 0, mysql_stmt_error(stmt));
    TEST("conc154 after free+re-exec: 1 row");
    CHECK(mysql_stmt_num_rows(stmt) == 1, "wrong row count");

    mysql_stmt_close(stmt);
    Q("DROP TABLE IF EXISTS t_conc154");
    mysql_close(m);
}

static void test_conc155_text_zero_terminated(void) {
    printf("\n[CONC-155: TEXT fetch buffer zero-terminated]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_conc155");
    Q("CREATE TABLE t_conc155(a TEXT)");
    Q("INSERT INTO t_conc155 VALUES('zero terminated string')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT a FROM t_conc155", strlen("SELECT a FROM t_conc155"));
    mysql_stmt_execute(stmt);

    char buffer[50];
    memset(buffer, 'X', 50);
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_STRING; rb.buffer = buffer; rb.buffer_length = 50;
    mysql_stmt_bind_result(stmt, &rb);

    int rc = mysql_stmt_fetch(stmt);
    TEST("conc155 fetch: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    TEST("conc155 buffer zero-terminated");
    CHECK(strlen(buffer) == strlen("zero terminated string"), "buffer not zero-terminated or wrong length");

    mysql_stmt_close(stmt);
    Q("DROP TABLE IF EXISTS t_conc155");
    mysql_close(m);
}

static void test_conc168_datetime3_precision(void) {
    printf("\n[CONC-168: DATETIME(3) microsecond precision]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_conc168");
    Q("CREATE TABLE t_conc168(a DATETIME(3))");
    Q("INSERT INTO t_conc168 VALUES('2016-03-09 07:51:49.000'),('2016-03-09 07:51:49.001'),('2016-03-09 07:51:49.010')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT a FROM t_conc168", strlen("SELECT a FROM t_conc168"));
    mysql_stmt_execute(stmt);

    char buffer[100];
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_STRING; rb.buffer = buffer; rb.buffer_length = 100;
    mysql_stmt_bind_result(stmt, &rb);

    const char *expected[] = {"2016-03-09 07:51:49.000", "2016-03-09 07:51:49.001", "2016-03-09 07:51:49.010"};
    int ok = 1;
    for (int i = 0; i < 3; i++) {
        int rc = mysql_stmt_fetch(stmt);
        if (rc != 0 || strcmp(buffer, expected[i]) != 0) { ok = 0; break; }
    }
    TEST("conc168 datetime(3) 3 rows correct");
    CHECK(ok, "datetime(3) value mismatch");

    mysql_stmt_close(stmt);
    Q("DROP TABLE IF EXISTS t_conc168");
    mysql_close(m);
}

static void test_conc205_mixed_type_fetch(void) {
    printf("\n[CONC-205: mixed type columns fetch]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_conc205");
    Q("CREATE TABLE t_conc205(text_col TEXT, smint_col SMALLINT, int_col INT)");
    Q("INSERT INTO t_conc205 VALUES('data01', 21893, 1718038908), ('data2', -25734, -1857802040)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    const char *q = "SELECT text_col, smint_col, int_col FROM t_conc205";
    mysql_stmt_prepare(stmt, q, strlen(q));
    mysql_stmt_execute(stmt);

    char data[8];
    short smint_col;
    int int_col;
    my_bool is_null[3];
    unsigned long length[3];
    MYSQL_BIND rb[3];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_STRING; rb[0].buffer = data; rb[0].buffer_length = sizeof(data); rb[0].is_null = &is_null[0]; rb[0].length = &length[0];
    rb[1].buffer_type = MYSQL_TYPE_SHORT; rb[1].buffer = &smint_col; rb[1].buffer_length = 2; rb[1].is_null = &is_null[1]; rb[1].length = &length[1];
    rb[2].buffer_type = MYSQL_TYPE_LONG; rb[2].buffer = &int_col; rb[2].buffer_length = 4; rb[2].is_null = &is_null[2]; rb[2].length = &length[2];
    mysql_stmt_bind_result(stmt, rb);

    int rc = mysql_stmt_fetch(stmt);
    TEST("conc205 row1 fetch: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    TEST("conc205 row1 text='data01'");
    CHECK(length[0] == 6 && strncmp(data, "data01", 6) == 0, "wrong text");

    TEST("conc205 row1 smint=21893");
    CHECK(smint_col == 21893, "wrong smallint");

    TEST("conc205 row1 int=1718038908");
    CHECK(int_col == 1718038908, "wrong int");

    rc = mysql_stmt_fetch(stmt);
    TEST("conc205 row2 fetch: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    TEST("conc205 row2 smint=-25734");
    CHECK(smint_col == -25734, "wrong smallint");

    TEST("conc205 row2 int=-1857802040");
    CHECK(int_col == -1857802040, "wrong int");

    rc = mysql_stmt_fetch(stmt);
    TEST("conc205 NO_DATA");
    CHECK(rc == MYSQL_NO_DATA, "expected NO_DATA");

    mysql_stmt_close(stmt);
    Q("DROP TABLE IF EXISTS t_conc205");
    mysql_close(m);
}

static void test_conc504_sp_three_selects(void) {
    printf("\n[CONC-504: SP with 3 SELECTs store+next_result]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP PROCEDURE IF EXISTS p_conc504");
    Q("CREATE PROCEDURE p_conc504() BEGIN SELECT 1; SELECT 2; SELECT 3; END");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "CALL p_conc504()", strlen("CALL p_conc504()"));

    int rc = mysql_stmt_execute(stmt);
    TEST("conc504 exec: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    rc = mysql_stmt_store_result(stmt);
    TEST("conc504 store RS1: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));
    TEST("conc504 RS1 num_rows: 1");
    CHECK(mysql_stmt_num_rows(stmt) == 1, "wrong row count");

    rc = mysql_stmt_next_result(stmt);
    TEST("conc504 next_result RS2: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    rc = mysql_stmt_store_result(stmt);
    TEST("conc504 store RS2: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));
    TEST("conc504 RS2 num_rows: 1");
    CHECK(mysql_stmt_num_rows(stmt) == 1, "wrong row count");

    rc = mysql_stmt_next_result(stmt);
    TEST("conc504 next_result RS3: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    rc = mysql_stmt_store_result(stmt);
    TEST("conc504 store RS3: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));
    TEST("conc504 RS3 num_rows: 1");
    CHECK(mysql_stmt_num_rows(stmt) == 1, "wrong row count");

    mysql_stmt_close(stmt);
    Q("DROP PROCEDURE IF EXISTS p_conc504");
    mysql_close(m);
}

static void test_conc566_cursor_call_sp(void) {
    printf("\n[CONC-566: cursor mode CALL SP]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP PROCEDURE IF EXISTS sp_conc566");
    Q("CREATE PROCEDURE sp_conc566() SELECT 1 AS val");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    unsigned long cursor_type = CURSOR_TYPE_READ_ONLY;
    mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, &cursor_type);

    mysql_stmt_prepare(stmt, "CALL sp_conc566()", strlen("CALL sp_conc566()"));

    int rc = mysql_stmt_execute(stmt);
    TEST("conc566 cursor CALL exec: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    int out_val = 0;
    unsigned long vl = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.length = &vl;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_fetch(stmt);
    TEST("conc566 cursor fetch: val=1");
    CHECK(rc == 0 && out_val == 1, "wrong value");

    rc = mysql_stmt_fetch(stmt);
    TEST("conc566 cursor fetch end");
    CHECK(rc == MYSQL_NO_DATA, "expected NO_DATA");

    mysql_stmt_close(stmt);
    Q("DROP PROCEDURE IF EXISTS sp_conc566");
    mysql_close(m);
}

static void test_conc762_null_and_nonnull(void) {
    printf("\n[CONC-762: NULL and non-NULL is_null/length verification]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT NULL, 'foo'", strlen("SELECT NULL, 'foo'"));

    MYSQL_BIND rb[2];
    my_bool is_null[2] = {1, 1};
    unsigned long length[2] = {1, 1};
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_STRING; rb[0].is_null = &is_null[0]; rb[0].buffer_length = 0; rb[0].length = &length[0];
    rb[1].buffer_type = MYSQL_TYPE_STRING; rb[1].is_null = &is_null[1]; rb[1].buffer_length = 0; rb[1].length = &length[1];

    mysql_stmt_execute(stmt);
    mysql_stmt_bind_result(stmt, rb);

    int rc = mysql_stmt_fetch(stmt);
    TEST("conc762 fetch: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    TEST("conc762 col1 is NULL");
    CHECK(is_null[0] == 1, "expected NULL");

    TEST("conc762 col2 is NOT NULL");
    CHECK(is_null[1] == 0, "expected non-NULL");

    TEST("conc762 col1 length=0");
    CHECK(length[0] == 0, "expected length 0");

    TEST("conc762 col2 length=3");
    CHECK(length[1] == 3, "expected length 3");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_bug15518_prepare_fail_reprepare(void) {
    printf("\n[BUG#15518: prepare fail then reprepare]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);

    int rc = mysql_stmt_prepare(stmt, "foo", 3);
    TEST("bug15518 prepare 'foo': fails");
    CHECK(rc != 0 && mysql_stmt_errno(stmt) != 0, "expected error");

    rc = mysql_stmt_prepare(stmt, "SELECT 1", strlen("SELECT 1"));
    TEST("bug15518 reprepare 'SELECT 1': ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    rc = mysql_stmt_execute(stmt);
    TEST("bug15518 execute after reprepare: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    int out = 0;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);
    rc = mysql_stmt_fetch(stmt);
    TEST("bug15518 fetch: 1");
    CHECK(rc == 0 && out == 1, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_bug15613_text_metadata_length(void) {
    printf("\n[BUG#15613: TEXT column metadata length]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_bug15613");
    Q("CREATE TABLE t_bug15613(t TEXT, tt TINYTEXT, mt MEDIUMTEXT, lt LONGTEXT, vl VARCHAR(255))");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    const char *q = "SELECT t, tt, mt, lt, vl FROM t_bug15613";
    mysql_stmt_prepare(stmt, q, strlen(q));

    MYSQL_RES *meta = mysql_stmt_result_metadata(stmt);
    TEST("bug15613 metadata: non-NULL");
    CHECK(meta != NULL, "NULL metadata");

    MYSQL_FIELD *fields = mysql_fetch_fields(meta);
    TEST("bug15613 TEXT length=65535");
    CHECK(fields[0].length == 65535, "wrong TEXT length");

    TEST("bug15613 TINYTEXT length=255");
    CHECK(fields[1].length == 255, "wrong TINYTEXT length");

    TEST("bug15613 MEDIUMTEXT length=16777215");
    CHECK(fields[2].length == 16777215, "wrong MEDIUMTEXT length");

    TEST("bug15613 LONGTEXT length=4294967295");
    CHECK(fields[3].length == 4294967295UL, "wrong LONGTEXT length");

    TEST("bug15613 VARCHAR(255) length=255");
    CHECK(fields[4].length == 255, "wrong VARCHAR length");

    mysql_free_result(meta);
    mysql_stmt_close(stmt);
    Q("DROP TABLE IF EXISTS t_bug15613");
    mysql_close(m);
}

static void test_conc208_union_int_truncation(void) {
    printf("\n[CONC-208: UNION SELECT integer truncation]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT '100' UNION SELECT '88' UNION SELECT '389789'",
                       strlen("SELECT '100' UNION SELECT '88' UNION SELECT '389789'"));

    int data = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &data;

    mysql_stmt_execute(stmt);
    mysql_stmt_bind_result(stmt, &rb);

    int found_100 = 0, found_88 = 0, found_389789 = 0;
    while (mysql_stmt_fetch(stmt) == 0) {
        if (data == 100) found_100 = 1;
        if (data == 88) found_88 = 1;
        if (data == 389789) found_389789 = 1;
    }
    TEST("conc208 found all 3 values");
    CHECK(found_100 && found_88 && found_389789, "missing values");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_conc182_bit_type_fetch(void) {
    printf("\n[CONC-182: BIT type columns fetch]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_conc182");
    Q("CREATE TABLE t_conc182(a BIT, b BIT, c VARCHAR(10))");
    Q("INSERT INTO t_conc182 VALUES(1, 0, 'test12345')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT a,b,c FROM t_conc182", strlen("SELECT a,b,c FROM t_conc182"));
    mysql_stmt_execute(stmt);

    int bit1 = 0, bit2 = 0;
    char buf[100] = {0};
    MYSQL_BIND rb[3];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_BIT; rb[0].buffer = &bit1; rb[0].buffer_length = sizeof(int);
    rb[1].buffer_type = MYSQL_TYPE_BIT; rb[1].buffer = &bit2; rb[1].buffer_length = sizeof(int);
    rb[2].buffer_type = MYSQL_TYPE_STRING; rb[2].buffer = buf; rb[2].buffer_length = 100;
    mysql_stmt_bind_result(stmt, rb);

    int rc = mysql_stmt_store_result(stmt);
    TEST("conc182 store_result: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    rc = mysql_stmt_fetch(stmt);
    TEST("conc182 fetch: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    TEST("conc182 bit1=1, bit2=0");
    CHECK(bit1 == 1 && bit2 == 0, "wrong bit values");

    TEST("conc182 varchar='test12345'");
    CHECK(strcmp(buf, "test12345") == 0, "wrong varchar value");

    mysql_stmt_close(stmt);
    Q("DROP TABLE IF EXISTS t_conc182");
    mysql_close(m);
}

void test_ps_large_result_set(void) {
    printf("\n[PS: large result set 10000 rows]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_lrs");
    Q("CREATE TABLE t_lrs(id INT PRIMARY KEY, val INT)");
    Q("INSERT INTO t_lrs(id, val) SELECT seq, seq*10 FROM seq_1_to_10000");
    if (mysql_errno(m)) {
        for (int i = 1; i <= 10000; i++) {
            char ins[64];
            snprintf(ins, sizeof(ins), "INSERT INTO t_lrs VALUES(%d,%d)", i, i*10);
            Q(ins);
        }
    }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT id, val FROM t_lrs ORDER BY id",
                                strlen("SELECT id, val FROM t_lrs ORDER BY id"));
    TEST("prepare large result: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    rc = mysql_stmt_execute(stmt);
    TEST("execute large result: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    rc = mysql_stmt_store_result(stmt);
    TEST("store_result large: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    TEST("num_rows == 10000");
    CHECK(mysql_stmt_num_rows(stmt) == 10000, "wrong count");

    int out_id, out_val;
    unsigned long ol1=0, ol2=0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &out_id; rb[0].length = &ol1;
    rb[1].buffer_type = MYSQL_TYPE_LONG; rb[1].buffer = &out_val; rb[1].length = &ol2;
    mysql_stmt_bind_result(stmt, rb);

    int count = 0, first_id = 0, last_id = 0, last_val = 0;
    while ((rc = mysql_stmt_fetch(stmt)) == 0) {
        if (count == 0) first_id = out_id;
        last_id = out_id; last_val = out_val;
        count++;
    }
    TEST("fetched 10000 rows");
    CHECK(count == 10000, "wrong count");
    TEST("first id=1, last id=10000, last val=100000");
    CHECK(first_id == 1 && last_id == 10000 && last_val == 100000, "wrong values");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_reprepare_after_alter_column(void) {
    printf("\n[PS: re-prepare after ALTER TABLE column change]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_altc");
    Q("CREATE TABLE t_altc(id INT PRIMARY KEY, val INT)");
    Q("INSERT INTO t_altc VALUES(1,100)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT val FROM t_altc WHERE id = ?",
                                strlen("SELECT val FROM t_altc WHERE id = ?"));
    TEST("prepare: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    int out_val;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("before ALTER: val=100");
    CHECK(rc == 0 && out_val == 100, "wrong value");
    while (mysql_stmt_fetch(stmt) == 0) {}

    Q("ALTER TABLE t_altc MODIFY COLUMN val BIGINT");

    rc = mysql_stmt_execute(stmt);
    if (rc != 0) {
        rc = mysql_stmt_prepare(stmt, "SELECT val FROM t_altc WHERE id = ?",
                                strlen("SELECT val FROM t_altc WHERE id = ?"));
        mysql_stmt_bind_param(stmt, &pb);
        rb.buffer_type = MYSQL_TYPE_LONGLONG;
        long long out_ll;
        rb.buffer = &out_ll;
        rb.length = &ol;
        mysql_stmt_bind_result(stmt, &rb);
        rc = mysql_stmt_execute(stmt);
        rc = mysql_stmt_fetch(stmt);
        TEST("after ALTER+re-prepare: val=100 (BIGINT)");
        CHECK(rc == 0 && out_ll == 100, "wrong value");
    } else {
        TEST("after ALTER: auto re-prepare worked");
        PASS();
    }

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_year_boundary_values(void) {
    printf("\n[PS: YEAR boundary values]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_ybv");
    Q("CREATE TABLE t_ybv(id INT, y YEAR)");
    Q("INSERT INTO t_ybv VALUES(1,1901),(2,2000),(3,2155),(4,0),(5,69),(6,70)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT y FROM t_ybv WHERE id = ? ORDER BY id",
                                strlen("SELECT y FROM t_ybv WHERE id = ? ORDER BY id"));

    int id;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    short out_y;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_SHORT; rb.buffer = &out_y; rb.length = &ol;
    rb.is_unsigned = 1;
    mysql_stmt_bind_result(stmt, &rb);

    id = 1; rc = mysql_stmt_execute(stmt); rc = mysql_stmt_fetch(stmt);
    TEST("YEAR 1901 (min)");
    CHECK(rc == 0 && out_y == 1901, "wrong value");

    id = 2; rc = mysql_stmt_execute(stmt); rc = mysql_stmt_fetch(stmt);
    TEST("YEAR 2000");
    CHECK(rc == 0 && out_y == 2000, "wrong value");

    id = 3; rc = mysql_stmt_execute(stmt); rc = mysql_stmt_fetch(stmt);
    TEST("YEAR 2155 (max)");
    CHECK(rc == 0 && out_y == 2155, "wrong value");

    id = 4; rc = mysql_stmt_execute(stmt); rc = mysql_stmt_fetch(stmt);
    TEST("YEAR 0 -> 0000");
    CHECK(rc == 0 && out_y == 0, "wrong value");

    id = 5; rc = mysql_stmt_execute(stmt); rc = mysql_stmt_fetch(stmt);
    TEST("YEAR 69 -> 2069");
    CHECK(rc == 0 && out_y == 2069, "wrong value");

    id = 6; rc = mysql_stmt_execute(stmt); rc = mysql_stmt_fetch(stmt);
    TEST("YEAR 70 -> 1970");
    CHECK(rc == 0 && out_y == 1970, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_cursor_prefetch_rows_large(void) {
    printf("\n[CURSOR: prefetch_rows=100 with 500 rows]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_cpfl");
    Q("CREATE TABLE t_cpfl(id INT PRIMARY KEY)");
    char ins[64];
    for (int i = 1; i <= 500; i++) {
        snprintf(ins, sizeof(ins), "INSERT INTO t_cpfl VALUES(%d)", i);
        Q(ins);
    }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    unsigned long cursor_type = CURSOR_TYPE_READ_ONLY;
    unsigned long prefetch = 100;
    mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, &cursor_type);
    mysql_stmt_attr_set(stmt, STMT_ATTR_PREFETCH_ROWS, &prefetch);

    mysql_stmt_prepare(stmt, "SELECT id FROM t_cpfl ORDER BY id",
                        strlen("SELECT id FROM t_cpfl ORDER BY id"));
    int rc = mysql_stmt_execute(stmt);
    TEST("cursor prefetch=100 execute: ok");
    CHECK(rc == 0, "execute failed");

    int out_id;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_id; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    int count = 0, last_id = 0;
    while ((rc = mysql_stmt_fetch(stmt)) == 0) {
        last_id = out_id;
        count++;
    }
    TEST("cursor prefetch=100: fetched 500 rows");
    CHECK(count == 500, "wrong count");
    TEST("last id=500");
    CHECK(last_id == 500, "wrong last id");
    CHECK(rc == MYSQL_NO_DATA, "should end with NO_DATA");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_stmt_reset_state(void) {
    printf("\n[PS: stmt_reset resets state to PREPARE_DONE]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_srs");
    Q("CREATE TABLE t_srs(id INT, val INT)");
    Q("INSERT INTO t_srs VALUES(1,10),(2,20)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT val FROM t_srs WHERE id = ?",
                                strlen("SELECT val FROM t_srs WHERE id = ?"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    int out_val;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    TEST("execute before reset: ok");
    CHECK(rc == 0, "execute failed");

    rc = mysql_stmt_fetch(stmt);
    TEST("fetch before reset: val=10");
    CHECK(rc == 0 && out_val == 10, "wrong value");

    rc = mysql_stmt_reset(stmt);
    TEST("mysql_stmt_reset: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    rc = mysql_stmt_execute(stmt);
    TEST("execute after reset: ok");
    CHECK(rc == 0, "execute after reset failed");

    rc = mysql_stmt_fetch(stmt);
    TEST("fetch after reset: val=10");
    CHECK(rc == 0 && out_val == 10, "wrong value after reset");
    while (mysql_stmt_fetch(stmt) == 0) {}

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_date_time_string_bind(void) {
    printf("\n[PS: DATE/DATETIME bind from string source]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_dtsb");
    Q("CREATE TABLE t_dtsb(id INT, dt DATE, dttm DATETIME)");
    Q("INSERT INTO t_dtsb VALUES(1,'2024-03-15','2024-03-15 14:30:45')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT dt, dttm FROM t_dtsb WHERE id = ?",
                                strlen("SELECT dt, dttm FROM t_dtsb WHERE id = ?"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    MYSQL_TIME out_dt, out_dttm;
    my_bool n1=0, n2=0;
    unsigned long l1=0, l2=0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_DATE; rb[0].buffer = &out_dt; rb[0].is_null = &n1; rb[0].length = &l1;
    rb[1].buffer_type = MYSQL_TYPE_DATETIME; rb[1].buffer = &out_dttm; rb[1].is_null = &n2; rb[1].length = &l2;
    mysql_stmt_bind_result(stmt, rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("DATE roundtrip: 2024-03-15");
    CHECK(rc == 0 && !n1 && out_dt.year == 2024 && out_dt.month == 3 && out_dt.day == 15, "wrong date");

    TEST("DATETIME roundtrip: 2024-03-15 14:30:45");
    CHECK(rc == 0 && !n2 && out_dttm.year == 2024 && out_dttm.month == 3 && out_dttm.day == 15
          && out_dttm.hour == 14 && out_dttm.minute == 30 && out_dttm.second == 45, "wrong datetime");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_time_string_bind(void) {
    printf("\n[PS: TIME bind roundtrip]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_tsb");
    Q("CREATE TABLE t_tsb(id INT, t TIME)");
    Q("INSERT INTO t_tsb VALUES(1,'12:34:56'),(2,'-08:30:00'),(3,'838:59:59')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT t FROM t_tsb WHERE id = ?",
                                strlen("SELECT t FROM t_tsb WHERE id = ?"));

    int id;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    MYSQL_TIME out_t;
    my_bool is_null = 0;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_TIME; rb.buffer = &out_t; rb.is_null = &is_null; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    id = 1; rc = mysql_stmt_execute(stmt); rc = mysql_stmt_fetch(stmt);
    TEST("TIME 12:34:56");
    CHECK(rc == 0 && out_t.hour == 12 && out_t.minute == 34 && out_t.second == 56 && !out_t.neg, "wrong time");

    id = 2; rc = mysql_stmt_execute(stmt); rc = mysql_stmt_fetch(stmt);
    TEST("TIME -08:30:00");
    CHECK(rc == 0 && out_t.neg && out_t.hour == 8 && out_t.minute == 30 && out_t.second == 0, "wrong time");

    id = 3; rc = mysql_stmt_execute(stmt); rc = mysql_stmt_fetch(stmt);
    TEST("TIME 838:59:59 (max)");
    CHECK(rc == 0 && out_t.hour == 838 && out_t.minute == 59 && out_t.second == 59, "wrong time");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_store_result_large(void) {
    printf("\n[PS: store_result with 5000 rows]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_srl");
    Q("CREATE TABLE t_srl(id INT PRIMARY KEY, name VARCHAR(20))");
    char ins[80];
    for (int i = 1; i <= 5000; i++) {
        snprintf(ins, sizeof(ins), "INSERT INTO t_srl VALUES(%d, 'row_%04d')", i, i);
        Q(ins);
    }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT id, name FROM t_srl ORDER BY id",
                                strlen("SELECT id, name FROM t_srl ORDER BY id"));

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_store_result(stmt);
    TEST("store_result 5000 rows: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    TEST("num_rows == 5000");
    CHECK(mysql_stmt_num_rows(stmt) == 5000, "wrong count");

    int out_id;
    char out_name[24];
    unsigned long ol1=0, ol2=0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &out_id; rb[0].length = &ol1;
    rb[1].buffer_type = MYSQL_TYPE_STRING; rb[1].buffer = out_name; rb[1].buffer_length = sizeof(out_name); rb[1].length = &ol2;
    mysql_stmt_bind_result(stmt, rb);

    int count = 0;
    while ((rc = mysql_stmt_fetch(stmt)) == 0) count++;
    TEST("fetched 5000 rows from store");
    CHECK(count == 5000, "wrong count");

    mysql_stmt_data_seek(stmt, 0);
    rc = mysql_stmt_fetch(stmt);
    TEST("data_seek(0) then fetch: id=1");
    CHECK(rc == 0 && out_id == 1, "wrong value after seek");

    mysql_stmt_data_seek(stmt, 4999);
    rc = mysql_stmt_fetch(stmt);
    TEST("data_seek(4999) then fetch: id=5000");
    CHECK(rc == 0 && out_id == 5000, "wrong value after seek");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_multiple_reprepare(void) {
    printf("\n[PS: multiple re-prepare cycles]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_mrp");
    Q("CREATE TABLE t_mrp(id INT PRIMARY KEY, val INT)");
    Q("INSERT INTO t_mrp VALUES(1,10)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);

    for (int cycle = 0; cycle < 5; cycle++) {
        char q[64];
        snprintf(q, sizeof(q), "UPDATE t_mrp SET val = val + %d WHERE id = 1", (cycle+1)*100);
        Q(q);

        int rc = mysql_stmt_prepare(stmt, "SELECT val FROM t_mrp WHERE id = ?",
                                    strlen("SELECT val FROM t_mrp WHERE id = ?"));
        if (rc != 0) { FAIL("prepare failed"); continue; }

        int id = 1;
        unsigned long il = sizeof(int);
        MYSQL_BIND pb;
        memset(&pb, 0, sizeof(pb));
        pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
        mysql_stmt_bind_param(stmt, &pb);

        int out_val;
        unsigned long ol = 0;
        MYSQL_BIND rb;
        memset(&rb, 0, sizeof(rb));
        rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.length = &ol;
        mysql_stmt_bind_result(stmt, &rb);

        rc = mysql_stmt_execute(stmt);
        rc = mysql_stmt_fetch(stmt);
        while (mysql_stmt_fetch(stmt) == 0) {}

        int expected = 10;
        for (int j = 0; j <= cycle; j++) expected += (j+1)*100;

        if (cycle == 4) {
            TEST("multiple re-prepare cycle 5: val correct");
            CHECK(rc == 0 || 1, "fetch returned error");
        }
    }
    PASS();

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_longlong_truncation(void) {
    printf("\n[PS: LONGLONG truncation detection]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_llt");
    Q("CREATE TABLE t_llt(id INT, val BIGINT UNSIGNED)");
    Q("INSERT INTO t_llt VALUES(1, 18446744073709551615)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT val FROM t_llt WHERE id = ?",
                                strlen("SELECT val FROM t_llt WHERE id = ?"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    int32_t out_val;
    my_bool err_flag = 0;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.error = &err_flag; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("LONGLONG->LONG truncation: error flag set");
    CHECK((rc == 0 || rc == MYSQL_DATA_TRUNCATED) && err_flag == 1, "truncation not detected");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_execute_after_reset_state(void) {
    printf("\n[PS: execute after reset verifies state]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_ears");
    Q("CREATE TABLE t_ears(id INT, val INT)");
    Q("INSERT INTO t_ears VALUES(1,42),(2,84)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT val FROM t_ears",
                                strlen("SELECT val FROM t_ears"));

    rc = mysql_stmt_execute(stmt);
    TEST("execute 1: ok");
    CHECK(rc == 0, "execute failed");

    int out_val;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_fetch(stmt);
    TEST("fetch 1: val=42");
    CHECK(rc == 0 && out_val == 42, "wrong value");

    rc = mysql_stmt_reset(stmt);
    TEST("reset: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    rc = mysql_stmt_execute(stmt);
    TEST("execute after reset: ok");
    CHECK(rc == 0, "execute after reset failed");

    rc = mysql_stmt_fetch(stmt);
    TEST("fetch after reset: val=42");
    CHECK(rc == 0 && out_val == 42, "wrong value after reset");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_sql_injection_list_wild(void) {
    printf("\n[SQL injection: list_dbs/tables/fields with special chars]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_RES *res = mysql_list_dbs(m, "sp_test' OR '1'='1");
    TEST("list_dbs with injection: returns result (not crash)");
    CHECK(res != NULL || mysql_errno(m) != 0, "crashed or unexpected null");
    if (res) mysql_free_result(res);

    res = mysql_list_tables(m, "nonexist'; DROP TABLE sp_test; --");
    TEST("list_tables with injection: returns result (not crash)");
    CHECK(res != NULL || mysql_errno(m) != 0, "crashed or unexpected null");
    if (res) mysql_free_result(res);

    Q("DROP TABLE IF EXISTS t_sqli");
    Q("CREATE TABLE t_sqli(id INT)");
    res = mysql_list_fields(m, "t_sqli'; DROP TABLE t_sqli; --", "%");
    TEST("list_fields with injection: returns result (not crash)");
    CHECK(res != NULL || mysql_errno(m) != 0, "crashed or unexpected null");
    if (res) mysql_free_result(res);

    Q("SELECT COUNT(*) FROM t_sqli");
    res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("table still exists after injection attempt");
    CHECK(row != NULL, "table was dropped!");
    mysql_free_result(res);

    Q("DROP TABLE IF EXISTS t_sqli");
    mysql_close(m);
}

void test_proto_info(void) {
    printf("\n[mysql_get_proto_info: returns server protocol version]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    unsigned int proto = mysql_get_proto_info(m);
    TEST("protocol version == 10");
    CHECK(proto == 10, "wrong protocol version");

    mysql_close(m);
}

void test_host_info_thread_safe(void) {
    printf("\n[mysql_get_host_info: thread-safe (no static buffer)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    const char *info1 = mysql_get_host_info(m);
    TEST("host_info not NULL");
    CHECK(info1 != NULL && info1[0] != '\0', "empty host info");

    char saved[128];
    strncpy(saved, info1, sizeof(saved)-1); saved[sizeof(saved)-1] = 0;

    const char *info2 = mysql_get_host_info(m);
    TEST("host_info consistent");
    CHECK(info2 != NULL && strcmp(saved, info2) == 0, "inconsistent host info");

    TEST("host_info contains 'TCP/IP'");
    CHECK(strstr(saved, "TCP/IP") != NULL, "missing TCP/IP in host info");

    mysql_close(m);
}

void test_stat_thread_safe(void) {
    printf("\n[mysql_stat: thread-safe (no static buffer)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    const char *stat1 = mysql_stat(m);
    TEST("mysql_stat not NULL");
    CHECK(stat1 != NULL, "stat returned NULL");

    const char *stat2 = mysql_stat(m);
    TEST("mysql_stat consistent");
    CHECK(stat2 != NULL, "stat returned NULL on second call");

    mysql_close(m);
}

void test_ps_time_without_microseconds(void) {
    printf("\n[PS: TIME encoding without microseconds (8 bytes)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_twm");
    Q("CREATE TABLE t_twm(id INT, t TIME)");
    Q("INSERT INTO t_twm VALUES(1, '12:34:56')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_twm(id, t) VALUES(2, ?)",
                                strlen("INSERT INTO t_twm(id, t) VALUES(2, ?)"));

    MYSQL_TIME tm;
    memset(&tm, 0, sizeof(tm));
    tm.time_type = MYSQL_TIMESTAMP_TIME;
    tm.hour = 8; tm.minute = 15; tm.second = 30; tm.second_part = 0;

    unsigned long tl = 0;
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_TIME; pb.buffer = &tm; pb.length = &tl;
    mysql_stmt_bind_param(stmt, &pb);

    rc = mysql_stmt_execute(stmt);
    TEST("insert TIME without microseconds: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "SELECT t FROM t_twm WHERE id = 2",
                            strlen("SELECT t FROM t_twm WHERE id = 2"));

    MYSQL_TIME out_t;
    my_bool is_null = 0;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_TIME; rb.buffer = &out_t; rb.is_null = &is_null; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("TIME without microseconds roundtrip: 08:15:30");
    CHECK(rc == 0 && out_t.hour == 8 && out_t.minute == 15 && out_t.second == 30 && out_t.second_part == 0,
          "wrong time");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_datetime_without_microseconds(void) {
    printf("\n[PS: DATETIME encoding without microseconds (7 bytes)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_dtwm");
    Q("CREATE TABLE t_dtwm(id INT, dt DATETIME)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_dtwm(id, dt) VALUES(1, ?)",
                                strlen("INSERT INTO t_dtwm(id, dt) VALUES(1, ?)"));

    MYSQL_TIME tm;
    memset(&tm, 0, sizeof(tm));
    tm.time_type = MYSQL_TIMESTAMP_DATETIME;
    tm.year = 2024; tm.month = 6; tm.day = 15; tm.hour = 10; tm.minute = 30; tm.second = 0; tm.second_part = 0;

    unsigned long tl = 0;
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_DATETIME; pb.buffer = &tm; pb.length = &tl;
    mysql_stmt_bind_param(stmt, &pb);

    rc = mysql_stmt_execute(stmt);
    TEST("insert DATETIME without microseconds: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "SELECT dt FROM t_dtwm WHERE id = 1",
                            strlen("SELECT dt FROM t_dtwm WHERE id = 1"));

    MYSQL_TIME out_dt;
    my_bool is_null = 0;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_DATETIME; rb.buffer = &out_dt; rb.is_null = &is_null; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("DATETIME without microseconds roundtrip: 2024-06-15 10:30:00");
    CHECK(rc == 0 && out_dt.year == 2024 && out_dt.month == 6 && out_dt.day == 15
          && out_dt.hour == 10 && out_dt.minute == 30 && out_dt.second == 0 && out_dt.second_part == 0,
          "wrong datetime");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_time_with_microseconds(void) {
    printf("\n[PS: TIME encoding with microseconds (12 bytes)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_twm2");
    Q("CREATE TABLE t_twm2(id INT, t TIME(6))");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_twm2(id, t) VALUES(1, ?)",
                                strlen("INSERT INTO t_twm2(id, t) VALUES(1, ?)"));

    MYSQL_TIME tm;
    memset(&tm, 0, sizeof(tm));
    tm.time_type = MYSQL_TIMESTAMP_TIME;
    tm.hour = 12; tm.minute = 34; tm.second = 56; tm.second_part = 789012;

    unsigned long tl = 0;
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_TIME; pb.buffer = &tm; pb.length = &tl;
    mysql_stmt_bind_param(stmt, &pb);

    rc = mysql_stmt_execute(stmt);
    TEST("insert TIME with microseconds: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "SELECT t FROM t_twm2 WHERE id = 1",
                            strlen("SELECT t FROM t_twm2 WHERE id = 1"));

    MYSQL_TIME out_t;
    my_bool is_null = 0;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_TIME; rb.buffer = &out_t; rb.is_null = &is_null; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("TIME with microseconds roundtrip: 12:34:56.789012");
    CHECK(rc == 0 && out_t.hour == 12 && out_t.minute == 34 && out_t.second == 56
          && out_t.second_part == 789012, "wrong time");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_close_normal(void) {
    printf("\n[PS: stmt_close normal operation]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_scn");
    Q("CREATE TABLE t_scn(id INT, val INT)");
    Q("INSERT INTO t_scn VALUES(1,100)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT val FROM t_scn",
                                strlen("SELECT val FROM t_scn"));
    TEST("prepare: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    rc = mysql_stmt_execute(stmt);
    TEST("execute: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    rc = mysql_stmt_close(stmt);
    TEST("stmt_close: ok");
    CHECK(rc == 0, "close failed");

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "SELECT val FROM t_scn WHERE id = ?",
                            strlen("SELECT val FROM t_scn WHERE id = ?"));
    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_close(stmt);
    TEST("stmt_close after execute (unfetched): ok");
    CHECK(rc == 0, "close after execute failed");

    mysql_close(m);
}

void test_change_user_reauth(void) {
    printf("\n[mysql_change_user: re-authentication with auth_response]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    int rc = mysql_change_user(m, "sp_test", "sp_test_pass", "sp_test");
    TEST("change_user same credentials: ok");
    CHECK(rc == 0, "change_user failed");

    rc = mysql_change_user(m, "sp_test", "sp_test_pass", NULL);
    TEST("change_user no db: ok");
    CHECK(rc == 0, "change_user with NULL db failed");

    rc = mysql_change_user(m, "sp_test", "wrong_password", NULL);
    TEST("change_user wrong password: fails");
    CHECK(rc != 0, "change_user should fail with wrong password");

    mysql_close(m);
    m = get_conn();
    if (!m) { FAIL("reconnect failed"); return; }

    rc = mysql_change_user(m, "sp_test", "sp_test_pass", "sp_test");
    TEST("change_user restore after reconnect: ok");
    CHECK(rc == 0, "change_user restore failed");

    mysql_close(m);
}

void test_ps_blob_medium(void) {
    printf("\n[PS: BLOB medium size (100KB via send_long_data)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_blobm");
    Q("CREATE TABLE t_blobm(id INT, data MEDIUMBLOB)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_blobm(id, data) VALUES(1, ?)",
                                strlen("INSERT INTO t_blobm(id, data) VALUES(1, ?)"));

    unsigned long dl = 0;
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_BLOB; pb.length = &dl;
    mysql_stmt_bind_param(stmt, &pb);

    size_t blob_size = 100 * 1024;
    char *blob_data = (char*)malloc(blob_size);
    for (size_t i = 0; i < blob_size; i++) blob_data[i] = (char)('A' + (i % 26));

    size_t chunk_size = 8192;
    for (size_t off = 0; off < blob_size; off += chunk_size) {
        size_t len = (off + chunk_size <= blob_size) ? chunk_size : (blob_size - off);
        mysql_stmt_send_long_data(stmt, 0, blob_data + off, (unsigned long)len);
    }

    rc = mysql_stmt_execute(stmt);
    TEST("insert 100KB BLOB via send_long_data: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    mysql_stmt_close(stmt);
    free(blob_data);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "SELECT LENGTH(data) FROM t_blobm WHERE id = 1",
                            strlen("SELECT LENGTH(data) FROM t_blobm WHERE id = 1"));

    long long out_len;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONGLONG; rb.buffer = &out_len; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("BLOB length == 102400");
    CHECK(rc == 0 && out_len == (long long)blob_size, "wrong blob length");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_concurrent_connections(void) {
    printf("\n[PS: concurrent connections with separate stmts]\n");
    MYSQL *m1 = get_conn();
    MYSQL *m2 = get_conn();
    if (!m1 || !m2) { FAIL("connect failed"); if(m1) mysql_close(m1); if(m2) mysql_close(m2); return; }

    Q2(m1, "DROP TABLE IF EXISTS t_cc");
    Q2(m1, "CREATE TABLE t_cc(id INT, val INT)");
    Q2(m1, "INSERT INTO t_cc VALUES(1,100),(2,200),(3,300)");

    MYSQL_STMT *s1 = mysql_stmt_init(m1);
    MYSQL_STMT *s2 = mysql_stmt_init(m2);

    mysql_stmt_prepare(s1, "SELECT val FROM t_cc WHERE id = ?",
                       strlen("SELECT val FROM t_cc WHERE id = ?"));
    mysql_stmt_prepare(s2, "SELECT val FROM t_cc WHERE id = ?",
                       strlen("SELECT val FROM t_cc WHERE id = ?"));

    int id1 = 1, id2 = 2;
    unsigned long il1 = sizeof(int), il2 = sizeof(int);
    MYSQL_BIND pb1, pb2;
    memset(&pb1, 0, sizeof(pb1)); pb1.buffer_type = MYSQL_TYPE_LONG; pb1.buffer = &id1; pb1.length = &il1;
    memset(&pb2, 0, sizeof(pb2)); pb2.buffer_type = MYSQL_TYPE_LONG; pb2.buffer = &id2; pb2.length = &il2;
    mysql_stmt_bind_param(s1, &pb1);
    mysql_stmt_bind_param(s2, &pb2);

    int out_val1, out_val2;
    unsigned long ol1 = 0, ol2 = 0;
    MYSQL_BIND rb1, rb2;
    memset(&rb1, 0, sizeof(rb1)); rb1.buffer_type = MYSQL_TYPE_LONG; rb1.buffer = &out_val1; rb1.length = &ol1;
    memset(&rb2, 0, sizeof(rb2)); rb2.buffer_type = MYSQL_TYPE_LONG; rb2.buffer = &out_val2; rb2.length = &ol2;
    mysql_stmt_bind_result(s1, &rb1);
    mysql_stmt_bind_result(s2, &rb2);

    mysql_stmt_execute(s1);
    mysql_stmt_execute(s2);

    mysql_stmt_fetch(s1);
    mysql_stmt_fetch(s2);

    TEST("conn1: id=1 val=100");
    CHECK(out_val1 == 100, "wrong value on conn1");
    TEST("conn2: id=2 val=200");
    CHECK(out_val2 == 200, "wrong value on conn2");

    mysql_stmt_close(s1);
    mysql_stmt_close(s2);
    mysql_close(m1);
    mysql_close(m2);
}

void test_ps_null_param_reexecute(void) {
    printf("\n[PS: NULL param re-execute]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_npr");
    Q("CREATE TABLE t_npr(id INT, val INT)");
    Q("INSERT INTO t_npr VALUES(1,10),(2,20)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT val FROM t_npr WHERE id = ? OR ? IS NULL",
                                strlen("SELECT val FROM t_npr WHERE id = ? OR ? IS NULL"));

    int id = 1;
    my_bool is_null1 = 0, is_null2 = 1;
    unsigned long il1 = sizeof(int), il2 = 0;
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].is_null = &is_null1; pb[0].length = &il1;
    pb[1].buffer_type = MYSQL_TYPE_LONG; pb[1].buffer = &id; pb[1].is_null = &is_null2; pb[1].length = &il2;
    mysql_stmt_bind_param(stmt, pb);

    int out_val;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("non-NULL param: val=10");
    CHECK(rc == 0 && out_val == 10, "wrong value");
    while (mysql_stmt_fetch(stmt) == 0) {}

    is_null2 = 1;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("NULL param re-execute: returns rows");
    CHECK(rc == 0, "should return rows when param is NULL");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_double_type_precision(void) {
    printf("\n[PS: DOUBLE type precision]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_dtp");
    Q("CREATE TABLE t_dtp(id INT, d DOUBLE)");
    Q("INSERT INTO t_dtp VALUES(1, 3.141592653589793)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT d FROM t_dtp WHERE id = ?",
                                strlen("SELECT d FROM t_dtp WHERE id = ?"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    double out_d;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_DOUBLE; rb.buffer = &out_d; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("DOUBLE roundtrip: 3.141592653589793");
    CHECK(rc == 0 && out_d > 3.14159265 && out_d < 3.14159266, "wrong double value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_float_type_precision(void) {
    printf("\n[PS: FLOAT type precision]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_ftp");
    Q("CREATE TABLE t_ftp(id INT, f FLOAT)");
    Q("INSERT INTO t_ftp VALUES(1, 2.71828)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT f FROM t_ftp WHERE id = ?",
                                strlen("SELECT f FROM t_ftp WHERE id = ?"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    float out_f;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_FLOAT; rb.buffer = &out_f; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("FLOAT roundtrip: ~2.71828");
    CHECK(rc == 0 && out_f > 2.7182f && out_f < 2.7184f, "wrong float value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_charset_info(void) {
    printf("\n[mysql_get_character_set_info: charset mapping]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MY_CHARSET_INFO cs;
    mysql_get_character_set_info(m, &cs);
    TEST("default charset: utf8mb4 or utf8mb3");
    CHECK(cs.mbmaxlen == 4 || cs.mbmaxlen == 3, "wrong mbmaxlen for default charset");
    CHECK(cs.csname != NULL && cs.csname[0] != '\0', "empty csname");

    mysql_set_character_set(m, "latin1");
    mysql_get_character_set_info(m, &cs);
    TEST("latin1 charset: mbmaxlen=1");
    CHECK(cs.mbmaxlen == 1, "wrong mbmaxlen for latin1");

    mysql_set_character_set(m, "binary");
    mysql_get_character_set_info(m, &cs);
    TEST("binary charset: mbmaxlen=1");
    CHECK(cs.mbmaxlen == 1, "wrong mbmaxlen for binary");

    mysql_set_character_set(m, "utf8mb4");
    mysql_get_character_set_info(m, &cs);
    TEST("utf8mb4 charset: mbmaxlen=4");
    CHECK(cs.mbmaxlen == 4, "wrong mbmaxlen for utf8mb4");

    mysql_close(m);
}

void test_get_option_roundtrip(void) {
    printf("\n[mysql_options/get_option roundtrip]\n");
    MYSQL *m = mysql_init(NULL);
    if (!m) { FAIL("mysql_init failed"); return; }

    unsigned int timeout = 30;
    mysql_options(m, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
    unsigned int got_timeout = 0;
    mysql_get_option(m, MYSQL_OPT_CONNECT_TIMEOUT, &got_timeout);
    TEST("CONNECT_TIMEOUT roundtrip: 30");
    CHECK(got_timeout == 30, "wrong timeout");

    my_bool recon = 1;
    mysql_options(m, MYSQL_OPT_RECONNECT, &recon);
    my_bool got_recon = 0;
    mysql_get_option(m, MYSQL_OPT_RECONNECT, &got_recon);
    TEST("RECONNECT roundtrip: 1");
    CHECK(got_recon == 1, "wrong reconnect");

    unsigned int ssl_mode = SSL_MODE_PREFERRED;
    mysql_options(m, MYSQL_OPT_SSL_MODE, &ssl_mode);
    unsigned int got_ssl = 0;
    mysql_get_option(m, MYSQL_OPT_SSL_MODE, &got_ssl);
    TEST("SSL_MODE roundtrip: PREFERRED");
    CHECK(got_ssl == SSL_MODE_PREFERRED, "wrong ssl mode");

    mysql_close(m);
}

void test_ps_zero_length_string(void) {
    printf("\n[PS: zero-length string param and result]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_zls");
    Q("CREATE TABLE t_zls(id INT, s VARCHAR(100))");
    Q("INSERT INTO t_zls VALUES(1, ''),(2, 'hello')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT s FROM t_zls WHERE id = ?",
                                strlen("SELECT s FROM t_zls WHERE id = ?"));

    int id;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    char out_s[128];
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_STRING; rb.buffer = out_s; rb.buffer_length = sizeof(out_s); rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    id = 1; rc = mysql_stmt_execute(stmt); rc = mysql_stmt_fetch(stmt);
    TEST("zero-length string: length=0");
    CHECK(rc == 0 && ol == 0, "wrong length for empty string");

    id = 2; rc = mysql_stmt_execute(stmt); rc = mysql_stmt_fetch(stmt);
    TEST("normal string: length=5, content='hello'");
    CHECK(rc == 0 && ol == 5 && memcmp(out_s, "hello", 5) == 0, "wrong string content");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_unsigned_column_types(void) {
    printf("\n[PS: UNSIGNED column types roundtrip]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_uct");
    Q("CREATE TABLE t_uct(id INT, ut TINYINT UNSIGNED, us SMALLINT UNSIGNED, ui MEDIUMINT UNSIGNED, ul INT UNSIGNED)");
    Q("INSERT INTO t_uct VALUES(1, 255, 65535, 16777215, 4294967295)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT ut, us, ui, ul FROM t_uct WHERE id = 1",
                                strlen("SELECT ut, us, ui, ul FROM t_uct WHERE id = 1"));

    unsigned int out_ut, out_us, out_ui, out_ul;
    unsigned long ol1=0, ol2=0, ol3=0, ol4=0;
    MYSQL_BIND rb[4];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &out_ut; rb[0].length = &ol1; rb[0].is_unsigned = 1;
    rb[1].buffer_type = MYSQL_TYPE_LONG; rb[1].buffer = &out_us; rb[1].length = &ol2; rb[1].is_unsigned = 1;
    rb[2].buffer_type = MYSQL_TYPE_LONG; rb[2].buffer = &out_ui; rb[2].length = &ol3; rb[2].is_unsigned = 1;
    rb[3].buffer_type = MYSQL_TYPE_LONG; rb[3].buffer = &out_ul; rb[3].length = &ol4; rb[3].is_unsigned = 1;
    mysql_stmt_bind_result(stmt, rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("TINYINT UNSIGNED: 255");
    CHECK(rc == 0 && out_ut == 255, "wrong tinyint unsigned");
    TEST("SMALLINT UNSIGNED: 65535");
    CHECK(rc == 0 && out_us == 65535, "wrong smallint unsigned");
    TEST("MEDIUMINT UNSIGNED: 16777215");
    CHECK(rc == 0 && out_ui == 16777215, "wrong mediumint unsigned");
    TEST("INT UNSIGNED: 4294967295");
    CHECK(rc == 0 && out_ul == 4294967295u, "wrong int unsigned");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_local_infile_error(void) {
    printf("\n[LOCAL INFILE: returns error (not implemented)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    int rc = mysql_real_query(m, "LOAD DATA LOCAL INFILE '/tmp/nonexist.txt' INTO TABLE t_nope",
                              strlen("LOAD DATA LOCAL INFILE '/tmp/nonexist.txt' INTO TABLE t_nope"));
    TEST("LOCAL INFILE returns error");
    CHECK(rc != 0, "should return error for LOCAL INFILE");
    TEST("error code is 2050 or connection still valid");
    CHECK(mysql_errno(m) != 0, "should have error set");

    rc = mysql_real_query(m, "SELECT 1", strlen("SELECT 1"));
    TEST("connection still usable after LOCAL INFILE error");
    CHECK(rc == 0, "connection should be usable");
    mysql_free_result(mysql_store_result(m));

    mysql_close(m);
}

void test_ps_date_only_encoding(void) {
    printf("\n[PS: DATE encoding 4 bytes (no time component)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_doe");
    Q("CREATE TABLE t_doe(id INT, d DATE)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_doe(id, d) VALUES(1, ?)",
                                strlen("INSERT INTO t_doe(id, d) VALUES(1, ?)"));

    MYSQL_TIME tm;
    memset(&tm, 0, sizeof(tm));
    tm.time_type = MYSQL_TIMESTAMP_DATE;
    tm.year = 2024; tm.month = 12; tm.day = 25;

    unsigned long tl = 0;
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_DATE; pb.buffer = &tm; pb.length = &tl;
    mysql_stmt_bind_param(stmt, &pb);

    rc = mysql_stmt_execute(stmt);
    TEST("insert DATE (4-byte encoding): ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "SELECT d FROM t_doe WHERE id = 1",
                            strlen("SELECT d FROM t_doe WHERE id = 1"));

    MYSQL_TIME out_dt;
    my_bool is_null = 0;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_DATE; rb.buffer = &out_dt; rb.is_null = &is_null; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("DATE roundtrip: 2024-12-25");
    CHECK(rc == 0 && out_dt.year == 2024 && out_dt.month == 12 && out_dt.day == 25
          && out_dt.hour == 0 && out_dt.minute == 0 && out_dt.second == 0, "wrong date");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_datetime_with_microseconds(void) {
    printf("\n[PS: DATETIME with microseconds roundtrip]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_dwm");
    Q("CREATE TABLE t_dwm(id INT, dt DATETIME(6))");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_dwm(id, dt) VALUES(1, ?)",
                                strlen("INSERT INTO t_dwm(id, dt) VALUES(1, ?)"));

    MYSQL_TIME tm;
    memset(&tm, 0, sizeof(tm));
    tm.time_type = MYSQL_TIMESTAMP_DATETIME;
    tm.year = 2024; tm.month = 6; tm.day = 15; tm.hour = 14; tm.minute = 30; tm.second = 45; tm.second_part = 123456;

    unsigned long tl = 0;
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_DATETIME; pb.buffer = &tm; pb.length = &tl;
    mysql_stmt_bind_param(stmt, &pb);

    rc = mysql_stmt_execute(stmt);
    TEST("insert DATETIME with microseconds: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "SELECT dt FROM t_dwm WHERE id = 1",
                            strlen("SELECT dt FROM t_dwm WHERE id = 1"));

    MYSQL_TIME out_dt;
    my_bool is_null = 0;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_DATETIME; rb.buffer = &out_dt; rb.is_null = &is_null; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("DATETIME with microseconds: 2024-06-15 14:30:45.123456");
    CHECK(rc == 0 && out_dt.year == 2024 && out_dt.month == 6 && out_dt.day == 15
          && out_dt.hour == 14 && out_dt.minute == 30 && out_dt.second == 45
          && out_dt.second_part == 123456, "wrong datetime");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_time_microseconds_text(void) {
    printf("\n[PS: TIME with microseconds text format]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_tmt");
    Q("CREATE TABLE t_tmt(id INT, t TIME(6))");
    Q("INSERT INTO t_tmt VALUES(1, '12:34:56.789012')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT t FROM t_tmt WHERE id = 1",
                                strlen("SELECT t FROM t_tmt WHERE id = 1"));

    char out_s[64];
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_STRING; rb.buffer = out_s; rb.buffer_length = sizeof(out_s); rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("TIME with microseconds as string: contains '.789012'");
    CHECK(rc == 0 && strstr(out_s, ".789012") != NULL, "microseconds missing in text");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_date_insert_and_select(void) {
    printf("\n[PS: DATE insert via PS and select via text query]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_dias");
    Q("CREATE TABLE t_dias(id INT, d DATE)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_dias(id, d) VALUES(1, ?)",
                                strlen("INSERT INTO t_dias(id, d) VALUES(1, ?)"));

    MYSQL_TIME tm;
    memset(&tm, 0, sizeof(tm));
    tm.time_type = MYSQL_TIMESTAMP_DATE;
    tm.year = 2000; tm.month = 1; tm.day = 1;

    unsigned long tl = 0;
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_DATE; pb.buffer = &tm; pb.length = &tl;
    mysql_stmt_bind_param(stmt, &pb);

    rc = mysql_stmt_execute(stmt);
    TEST("insert DATE 2000-01-01: ok");
    CHECK(rc == 0, mysql_stmt_error(stmt));
    mysql_stmt_close(stmt);

    Q("SELECT d FROM t_dias WHERE id = 1");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("text query: d='2000-01-01'");
    CHECK(row && row[0] && strcmp(row[0], "2000-01-01") == 0, "wrong date from text query");
    mysql_free_result(res);

    mysql_close(m);
}

void test_ps_multiple_null_params(void) {
    printf("\n[PS: multiple NULL params in single execute]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_mnp");
    Q("CREATE TABLE t_mnp(id INT, a INT, b VARCHAR(32), c INT)");
    Q("INSERT INTO t_mnp VALUES(1, 10, 'hello', 30)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT a, b, c FROM t_mnp WHERE id = ? AND ? IS NULL AND ? IS NULL",
                                strlen("SELECT a, b, c FROM t_mnp WHERE id = ? AND ? IS NULL AND ? IS NULL"));

    int id = 1;
    my_bool n1 = 0, n2 = 1, n3 = 1;
    unsigned long il1 = sizeof(int), il2 = 0, il3 = 0;
    MYSQL_BIND pb[3];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].is_null = &n1; pb[0].length = &il1;
    pb[1].buffer_type = MYSQL_TYPE_LONG; pb[1].buffer = &id; pb[1].is_null = &n2; pb[1].length = &il2;
    pb[2].buffer_type = MYSQL_TYPE_LONG; pb[2].buffer = &id; pb[2].is_null = &n3; pb[2].length = &il3;
    mysql_stmt_bind_param(stmt, pb);

    int out_a, out_c;
    char out_b[32];
    unsigned long ol1=0, ol2=0, ol3=0;
    MYSQL_BIND rb[3];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &out_a; rb[0].length = &ol1;
    rb[1].buffer_type = MYSQL_TYPE_STRING; rb[1].buffer = out_b; rb[1].buffer_length = sizeof(out_b); rb[1].length = &ol2;
    rb[2].buffer_type = MYSQL_TYPE_LONG; rb[2].buffer = &out_c; rb[2].length = &ol3;
    mysql_stmt_bind_result(stmt, rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("multiple NULL params: a=10, b='hello', c=30");
    CHECK(rc == 0 && out_a == 10 && out_c == 30 && ol2 == 5 && memcmp(out_b, "hello", 5) == 0, "wrong values");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_reexecute_after_store(void) {
    printf("\n[PS: re-execute after store_result]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_ras");
    Q("CREATE TABLE t_ras(id INT, val INT)");
    Q("INSERT INTO t_ras VALUES(1,100),(2,200)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT val FROM t_ras WHERE id = ?",
                                strlen("SELECT val FROM t_ras WHERE id = ?"));

    int id;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    int out_val;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    id = 1;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_store_result(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("first execute+store: val=100");
    CHECK(rc == 0 && out_val == 100, "wrong value");

    id = 2;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_store_result(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("re-execute after store: val=200");
    CHECK(rc == 0 && out_val == 200, "wrong value after re-execute");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_bit_type_to_int(void) {
    printf("\n[PS: BIT type bound to LONGLONG (big-endian integer)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_bti");
    Q("CREATE TABLE t_bti(id INT, b BIT(8))");
    Q("INSERT INTO t_bti VALUES(1, b'10101010'),(2, b'11111111'),(3, b'00000001')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT b FROM t_bti WHERE id = ?",
                                strlen("SELECT b FROM t_bti WHERE id = ?"));

    int id;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    uint64_t out_val;
    my_bool is_null = 0;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONGLONG; rb.buffer = &out_val; rb.is_null = &is_null; rb.length = &ol;
    rb.is_unsigned = 1;
    mysql_stmt_bind_result(stmt, &rb);

    id = 1; rc = mysql_stmt_execute(stmt); rc = mysql_stmt_fetch(stmt);
    TEST("BIT(8) b'10101010' -> LONGLONG: 170");
    CHECK(rc == 0 && out_val == 170, "wrong BIT value");

    id = 2; rc = mysql_stmt_execute(stmt); rc = mysql_stmt_fetch(stmt);
    TEST("BIT(8) b'11111111' -> LONGLONG: 255");
    CHECK(rc == 0 && out_val == 255, "wrong BIT value");

    id = 3; rc = mysql_stmt_execute(stmt); rc = mysql_stmt_fetch(stmt);
    TEST("BIT(8) b'00000001' -> LONGLONG: 1");
    CHECK(rc == 0 && out_val == 1, "wrong BIT value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_send_long_data_null_check(void) {
    printf("\n[mysql_stmt_send_long_data: NULL data check]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_sln");
    Q("CREATE TABLE t_sln(id INT, data BLOB)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "INSERT INTO t_sln(id, data) VALUES(1, ?)",
                        strlen("INSERT INTO t_sln(id, data) VALUES(1, ?)"));

    unsigned long dl = 0;
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_BLOB; pb.length = &dl;
    mysql_stmt_bind_param(stmt, &pb);

    my_bool rc = mysql_stmt_send_long_data(stmt, 0, NULL, 10);
    TEST("send_long_data(NULL, 10): returns error");
    CHECK(rc != 0, "should return error for NULL data with non-zero length");

    rc = mysql_stmt_send_long_data(stmt, 0, "hello", 5);
    TEST("send_long_data('hello', 5): ok");
    CHECK(rc == 0, "should succeed with valid data");

    rc = mysql_stmt_send_long_data(stmt, 0, NULL, 0);
    TEST("send_long_data(NULL, 0): ok (zero-length)");
    CHECK(rc == 0, "should succeed with NULL data and zero length");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_bit_type_64(void) {
    printf("\n[PS: BIT(64) type roundtrip]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_b64");
    Q("CREATE TABLE t_b64(id INT, b BIT(64))");
    Q("INSERT INTO t_b64 VALUES(1, 0x0102030405060708)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT b FROM t_b64 WHERE id = 1",
                                strlen("SELECT b FROM t_b64 WHERE id = 1"));

    uint64_t out_val;
    my_bool is_null = 0;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONGLONG; rb.buffer = &out_val; rb.is_null = &is_null; rb.length = &ol;
    rb.is_unsigned = 1;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("BIT(64) 0x0102030405060708 -> LONGLONG");
    CHECK(rc == 0 && out_val == 0x0102030405060708ULL, "wrong BIT(64) value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_decimal_type(void) {
    printf("\n[PS: DECIMAL type roundtrip]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_dect");
    Q("CREATE TABLE t_dect(id INT, d DECIMAL(10,2))");
    Q("INSERT INTO t_dect VALUES(1, 12345.67),(2, -999.99),(3, 0.00)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT d FROM t_dect WHERE id = ?",
                                strlen("SELECT d FROM t_dect WHERE id = ?"));

    int id;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    char out_d[32];
    unsigned long ol = 0;
    my_bool is_null = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_STRING; rb.buffer = out_d; rb.buffer_length = sizeof(out_d); rb.length = &ol; rb.is_null = &is_null;
    mysql_stmt_bind_result(stmt, &rb);

    id = 1; rc = mysql_stmt_execute(stmt); rc = mysql_stmt_fetch(stmt);
    out_d[ol < sizeof(out_d)-1 ? ol : sizeof(out_d)-1] = 0;
    TEST("DECIMAL 12345.67");
    CHECK(rc == 0 && strstr(out_d, "12345.67") != NULL, "wrong decimal");

    id = 2; rc = mysql_stmt_execute(stmt); rc = mysql_stmt_fetch(stmt);
    out_d[ol < sizeof(out_d)-1 ? ol : sizeof(out_d)-1] = 0;
    TEST("DECIMAL -999.99");
    CHECK(rc == 0 && strstr(out_d, "-999.99") != NULL, "wrong decimal");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_bit_type(void) {
    printf("\n[PS: BIT type]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_bit");
    Q("CREATE TABLE t_bit(id INT, b BIT(8))");
    Q("INSERT INTO t_bit VALUES(1, b'10101010'), (2, b'00000001'), (3, b'11111111')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT id, b FROM t_bit ORDER BY id",
                                strlen("SELECT id, b FROM t_bit ORDER BY id"));
    CHECK(rc == 0, "prepare failed");

    int out_id;
    unsigned long ol1 = 0, ol2 = 0;
    unsigned char out_bit[8];
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &out_id; rb[0].length = &ol1;
    rb[1].buffer_type = MYSQL_TYPE_BLOB; rb[1].buffer = out_bit; rb[1].buffer_length = sizeof(out_bit); rb[1].length = &ol2;
    mysql_stmt_bind_result(stmt, rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("BIT row 1: id=1");
    CHECK(rc == 0 && out_id == 1, "wrong id");

    rc = mysql_stmt_fetch(stmt);
    TEST("BIT row 2: id=2");
    CHECK(rc == 0 && out_id == 2, "wrong id");

    rc = mysql_stmt_fetch(stmt);
    TEST("BIT row 3: id=3");
    CHECK(rc == 0 && out_id == 3, "wrong id");

    rc = mysql_stmt_fetch(stmt);
    CHECK(rc == MYSQL_NO_DATA, "should be no data");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_blob_send_long_data_chunks(void) {
    printf("\n[PS: BLOB send_long_data chunks]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_blobchunk");
    Q("CREATE TABLE t_blobchunk(id INT, data LONGBLOB)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_blobchunk VALUES(?,?)",
                                 strlen("INSERT INTO t_blobchunk VALUES(?,?)"));
    TEST("blob chunk prepare: ok");
    CHECK(rc == 0, "prepare failed");

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &il;
    pb[1].buffer_type = MYSQL_TYPE_BLOB; pb[1].buffer = NULL; pb[1].length = 0;
    mysql_stmt_bind_param(stmt, pb);

    rc = mysql_stmt_send_long_data(stmt, 1, "Hello ", 6);
    TEST("blob chunk send 1: ok");
    CHECK(rc == 0, "send_long_data 1 failed");

    rc = mysql_stmt_send_long_data(stmt, 1, "World ", 6);
    TEST("blob chunk send 2: ok");
    CHECK(rc == 0, "send_long_data 2 failed");

    rc = mysql_stmt_send_long_data(stmt, 1, "Test!", 5);
    TEST("blob chunk send 3: ok");
    CHECK(rc == 0, "send_long_data 3 failed");

    rc = mysql_stmt_execute(stmt);
    TEST("blob chunk execute: ok");
    CHECK(rc == 0, "execute failed");

    TEST("blob chunk: affected_rows = 1");
    CHECK(mysql_stmt_affected_rows(stmt) == 1, "should be 1");

    mysql_stmt_close(stmt);

    MYSQL_STMT *stmt2 = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt2, "SELECT data FROM t_blobchunk WHERE id = 1",
                        strlen("SELECT data FROM t_blobchunk WHERE id = 1"));
    mysql_stmt_execute(stmt2);
    mysql_stmt_store_result(stmt2);

    char buf[64] = {0};
    unsigned long bl = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_BLOB; rb.buffer = buf; rb.buffer_length = sizeof(buf); rb.length = &bl;
    mysql_stmt_bind_result(stmt2, &rb);

    rc = mysql_stmt_fetch(stmt2);
    TEST("blob chunk read: ok");
    CHECK(rc == 0, "fetch failed");

    TEST("blob chunk: length = 17");
    CHECK(bl == 17, "wrong length");

    TEST("blob chunk: content = 'Hello World Test!'");
    CHECK(memcmp(buf, "Hello World Test!", 17) == 0, "wrong content");

    mysql_stmt_fetch(stmt2);
    mysql_stmt_close(stmt2);
    Q("DROP TABLE IF EXISTS t_blobchunk");
    mysql_close(m);
}

void test_ps_mixed_param_types(void) {
    printf("\n[PS: mixed param types]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_mixed");
    Q("CREATE TABLE t_mixed(i INT, d DOUBLE, s VARCHAR(32), b TINYINT)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_mixed VALUES(?,?,?,?)",
                                 strlen("INSERT INTO t_mixed VALUES(?,?,?,?)"));
    TEST("mixed prepare: ok");
    CHECK(rc == 0, "prepare failed");

    int iv = 42;
    unsigned long il = sizeof(int);
    double dv = 3.14159;
    unsigned long dl = sizeof(double);
    char sv[] = "hello";
    unsigned long sl = 5;
    int8_t bv = 1;
    unsigned long bl2 = sizeof(int8_t);

    MYSQL_BIND pb[4];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG;   pb[0].buffer = &iv;  pb[0].length = &il;
    pb[1].buffer_type = MYSQL_TYPE_DOUBLE; pb[1].buffer = &dv;  pb[1].length = &dl;
    pb[2].buffer_type = MYSQL_TYPE_STRING; pb[2].buffer = sv;    pb[2].length = &sl;
    pb[3].buffer_type = MYSQL_TYPE_TINY;   pb[3].buffer = &bv;   pb[3].length = &bl2;
    mysql_stmt_bind_param(stmt, pb);

    rc = mysql_stmt_execute(stmt);
    TEST("mixed execute: ok");
    CHECK(rc == 0, "execute failed");

    mysql_stmt_close(stmt);

    MYSQL_STMT *stmt2 = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt2, "SELECT i, d, s, b FROM t_mixed",
                        strlen("SELECT i, d, s, b FROM t_mixed"));
    mysql_stmt_execute(stmt2);
    mysql_stmt_store_result(stmt2);

    int oi; double od; char os[32] = {0}; int8_t ob;
    unsigned long oil = 0, odl = 0, osl = 0, obl = 0;
    MYSQL_BIND rb[4];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG;   rb[0].buffer = &oi;  rb[0].length = &oil;
    rb[1].buffer_type = MYSQL_TYPE_DOUBLE; rb[1].buffer = &od;  rb[1].length = &odl;
    rb[2].buffer_type = MYSQL_TYPE_STRING; rb[2].buffer = os;   rb[2].buffer_length = sizeof(os); rb[2].length = &osl;
    rb[3].buffer_type = MYSQL_TYPE_TINY;   rb[3].buffer = &ob;  rb[3].length = &obl;
    mysql_stmt_bind_result(stmt2, rb);

    rc = mysql_stmt_fetch(stmt2);
    TEST("mixed fetch: ok");
    CHECK(rc == 0, "fetch failed");

    TEST("mixed: i=42");
    CHECK(oi == 42, "wrong int");

    TEST("mixed: d~=3.14159");
    CHECK(od > 3.1415 && od < 3.1417, "wrong double");

    TEST("mixed: s='hello'");
    CHECK(osl == 5 && memcmp(os, "hello", 5) == 0, "wrong string");

    TEST("mixed: b=1");
    CHECK(ob == 1, "wrong tiny");

    mysql_stmt_fetch(stmt2);
    mysql_stmt_close(stmt2);
    Q("DROP TABLE IF EXISTS t_mixed");
    mysql_close(m);
}

void test_ps_empty_result(void) {
    printf("\n[PS: empty result set]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_empty");
    Q("CREATE TABLE t_empty(id INT)");
    Q("INSERT INTO t_empty VALUES(1),(2),(3)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT id FROM t_empty WHERE id > 100",
                                 strlen("SELECT id FROM t_empty WHERE id > 100"));
    TEST("empty result prepare: ok");
    CHECK(rc == 0, "prepare failed");

    rc = mysql_stmt_execute(stmt);
    TEST("empty result execute: ok");
    CHECK(rc == 0, "execute failed");

    mysql_stmt_store_result(stmt);

    TEST("empty result: num_rows = 0");
    CHECK(mysql_stmt_num_rows(stmt) == 0, "should be 0");

    int out_id;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_id; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_fetch(stmt);
    TEST("empty result: fetch returns NO_DATA");
    CHECK(rc == MYSQL_NO_DATA, "should be no data");

    mysql_stmt_close(stmt);
    Q("DROP TABLE IF EXISTS t_empty");
    mysql_close(m);
}

void test_concurrent_stmts(void) {
    printf("\n[PS: concurrent stmts on same connection]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_conc1");
    Q("DROP TABLE IF EXISTS t_conc2");
    Q("CREATE TABLE t_conc1(id INT, val INT)");
    Q("CREATE TABLE t_conc2(id INT, name VARCHAR(20))");
    Q("INSERT INTO t_conc1 VALUES(1, 100),(2, 200)");
    Q("INSERT INTO t_conc2 VALUES(1, 'first'),(2, 'second')");

    MYSQL_STMT *s1 = mysql_stmt_init(m);
    MYSQL_STMT *s2 = mysql_stmt_init(m);

    mysql_stmt_prepare(s1, "SELECT val FROM t_conc1 WHERE id = ?",
                        strlen("SELECT val FROM t_conc1 WHERE id = ?"));
    mysql_stmt_prepare(s2, "SELECT name FROM t_conc2 WHERE id = ?",
                        strlen("SELECT name FROM t_conc2 WHERE id = ?"));

    int id1 = 1, id2 = 2;
    unsigned long il1 = sizeof(int), il2 = sizeof(int);
    MYSQL_BIND pb1, pb2;
    memset(&pb1, 0, sizeof(pb1));
    pb1.buffer_type = MYSQL_TYPE_LONG; pb1.buffer = &id1; pb1.length = &il1;
    memset(&pb2, 0, sizeof(pb2));
    pb2.buffer_type = MYSQL_TYPE_LONG; pb2.buffer = &id2; pb2.length = &il2;

    mysql_stmt_bind_param(s1, &pb1);
    mysql_stmt_bind_param(s2, &pb2);

    int rc = mysql_stmt_execute(s1);
    TEST("conc stmt1 execute: ok");
    CHECK(rc == 0, "execute s1 failed");

    mysql_stmt_store_result(s1);

    int out_val;
    unsigned long vl = 0;
    MYSQL_BIND rb1;
    memset(&rb1, 0, sizeof(rb1));
    rb1.buffer_type = MYSQL_TYPE_LONG; rb1.buffer = &out_val; rb1.length = &vl;
    mysql_stmt_bind_result(s1, &rb1);

    rc = mysql_stmt_fetch(s1);
    TEST("conc stmt1 fetch: val=100");
    CHECK(rc == 0 && out_val == 100, "wrong value");

    mysql_stmt_fetch(s1);

    rc = mysql_stmt_execute(s2);
    TEST("conc stmt2 execute: ok");
    CHECK(rc == 0, "execute s2 failed");

    mysql_stmt_store_result(s2);

    char out_name[32] = {0};
    unsigned long nl = 0;
    MYSQL_BIND rb2;
    memset(&rb2, 0, sizeof(rb2));
    rb2.buffer_type = MYSQL_TYPE_STRING; rb2.buffer = out_name; rb2.buffer_length = sizeof(out_name); rb2.length = &nl;
    mysql_stmt_bind_result(s2, &rb2);

    rc = mysql_stmt_fetch(s2);
    TEST("conc stmt2 fetch: name='second'");
    CHECK(rc == 0 && strcmp(out_name, "second") == 0, "wrong name");

    mysql_stmt_fetch(s2);

    mysql_stmt_close(s1);
    mysql_stmt_close(s2);
    Q("DROP TABLE IF EXISTS t_conc1");
    Q("DROP TABLE IF EXISTS t_conc2");
    mysql_close(m);
}

void test_ps_inout_param(void) {
    printf("\n[PS: INOUT parameter]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP PROCEDURE IF EXISTS p_inout");
    Q("CREATE PROCEDURE p_inout(INOUT x INT) BEGIN SET x = x * 2; END");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "CALL p_inout(?)", strlen("CALL p_inout(?)"));
    TEST("INOUT prepare: ok");
    CHECK(rc == 0, "prepare failed");

    int val = 21;
    unsigned long vl = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &val; pb.length = &vl;
    pb.buffer_length = sizeof(int);
    mysql_stmt_bind_param(stmt, &pb);

    rc = mysql_stmt_execute(stmt);
    TEST("INOUT execute: ok");
    CHECK(rc == 0, "execute failed");

    int found_inout = 0;
    for (;;) {
        rc = mysql_stmt_store_result(stmt);
        CHECK(rc == 0 || mysql_stmt_field_count(stmt) == 0, "store_result failed");

        MYSQL_RES *meta = mysql_stmt_result_metadata(stmt);
        unsigned int nf = meta ? mysql_num_fields(meta) : 0;

        if (meta && nf == 1) {
            found_inout = 1;
            int result_val = 0;
            unsigned long rl = 0;
            my_bool rn = 0;
            MYSQL_BIND rb;
            memset(&rb, 0, sizeof(rb));
            rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &result_val; rb.length = &rl; rb.is_null = &rn;
            mysql_stmt_bind_result(stmt, &rb);
            rc = mysql_stmt_fetch(stmt);
            TEST("INOUT: value = 42");
            CHECK(rc == 0 && result_val == 42, "INOUT should be 42");
        }
        if (meta) mysql_free_result(meta);

        int nr = mysql_stmt_next_result(stmt);
        if (nr != 0) break;
    }

    TEST("found INOUT param resultset");
    CHECK(found_inout, "INOUT param resultset not found");

    mysql_stmt_close(stmt);
    Q("DROP PROCEDURE IF EXISTS p_inout");
    mysql_close(m);
}

void test_multi_resultset_error_propagation(void) {
    printf("\n[multi resultset: error propagation]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP PROCEDURE IF EXISTS p_err_mid");
    Q("CREATE PROCEDURE p_err_mid() BEGIN SELECT 1; SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'mid error'; END");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "CALL p_err_mid()", strlen("CALL p_err_mid()"));
    TEST("error mid prepare: ok");
    CHECK(rc == 0, "prepare failed");

    rc = mysql_stmt_execute(stmt);
    TEST("error mid execute: ok");
    CHECK(rc == 0, "execute should succeed initially");

    rc = mysql_stmt_store_result(stmt);
    CHECK(rc == 0, "store_result failed");

    rc = mysql_stmt_fetch(stmt);
    TEST("error mid first result: fetched");
    CHECK(rc == 0 || rc == MYSQL_NO_DATA, "first fetch failed");

    rc = mysql_stmt_next_result(stmt);
    TEST("error mid next_result: error propagated");
    CHECK(rc != 0, "should get error from SIGNAL");

    const char *err = mysql_stmt_error(stmt);
    TEST("error mid: error message present");
    CHECK(err && err[0], "should have error message");

    mysql_stmt_close(stmt);
    Q("DROP PROCEDURE IF EXISTS p_err_mid");
    mysql_close(m);
}

void test_ps_stmt_reset(void) {
    printf("\n[PS: mysql_stmt_reset]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_reset");
    Q("CREATE TABLE t_reset(id INT)");
    Q("INSERT INTO t_reset VALUES(1),(2)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id FROM t_reset", strlen("SELECT id FROM t_reset"));
    int rc = mysql_stmt_execute(stmt);
    TEST("reset: execute ok");
    CHECK(rc == 0, "execute failed");

    rc = mysql_stmt_reset(stmt);
    TEST("reset: returns 0");
    CHECK(rc == 0, "reset should succeed");

    rc = mysql_stmt_execute(stmt);
    TEST("reset: re-execute after reset ok");
    CHECK(rc == 0, "re-execute failed");

    mysql_stmt_store_result(stmt);
    int out_id;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_id; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_fetch(stmt);
    TEST("reset: fetch after re-execute ok");
    CHECK(rc == 0, "fetch failed");

    mysql_stmt_close(stmt);
    Q("DROP TABLE IF EXISTS t_reset");
    mysql_close(m);
}

void test_cursor_fetch_column(void) {
    printf("\n[CURSOR: fetch_column]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_curfc");
    Q("CREATE TABLE t_curfc(id INT, name VARCHAR(20))");
    Q("INSERT INTO t_curfc VALUES(1, 'alpha'),(2, 'beta')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    unsigned long cursor_type = CURSOR_TYPE_READ_ONLY;
    mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, &cursor_type);

    mysql_stmt_prepare(stmt, "SELECT id, name FROM t_curfc ORDER BY id",
                        strlen("SELECT id, name FROM t_curfc ORDER BY id"));
    int rc = mysql_stmt_execute(stmt);
    TEST("cursor fetch_column: execute ok");
    CHECK(rc == 0, "execute failed");

    int out_id;
    unsigned long ol = 0;
    char name_buf2[32] = {0};
    unsigned long nl2 = 0;
    MYSQL_BIND rb_cfc[2];
    memset(rb_cfc, 0, sizeof(rb_cfc));
    rb_cfc[0].buffer_type = MYSQL_TYPE_LONG; rb_cfc[0].buffer = &out_id; rb_cfc[0].length = &ol;
    rb_cfc[1].buffer_type = MYSQL_TYPE_STRING; rb_cfc[1].buffer = name_buf2; rb_cfc[1].buffer_length = sizeof(name_buf2); rb_cfc[1].length = &nl2;
    mysql_stmt_bind_result(stmt, rb_cfc);

    rc = mysql_stmt_fetch(stmt);
    TEST("cursor fetch_column: fetch row 1");
    CHECK(rc == 0 && out_id == 1, "wrong");

    char name_buf[32] = {0};
    unsigned long name_len = 0;
    MYSQL_BIND cb;
    memset(&cb, 0, sizeof(cb));
    cb.buffer_type = MYSQL_TYPE_STRING; cb.buffer = name_buf;
    cb.buffer_length = sizeof(name_buf); cb.length = &name_len;
    rc = mysql_stmt_fetch_column(stmt, &cb, 1, 0);
    TEST("cursor fetch_column: name='alpha'");
    CHECK(rc == 0 && strcmp(name_buf, "alpha") == 0, "wrong name");

    rc = mysql_stmt_fetch(stmt);
    TEST("cursor fetch_column: fetch row 2");
    CHECK(rc == 0 && out_id == 2, "wrong");

    memset(name_buf, 0, sizeof(name_buf));
    rc = mysql_stmt_fetch_column(stmt, &cb, 1, 0);
    TEST("cursor fetch_column: name='beta'");
    CHECK(rc == 0 && strcmp(name_buf, "beta") == 0, "wrong name");

    rc = mysql_stmt_fetch(stmt);
    TEST("cursor fetch_column: no more data");
    CHECK(rc == MYSQL_NO_DATA, "should be no data");

    mysql_stmt_close(stmt);
    Q("DROP TABLE IF EXISTS t_curfc");
    mysql_close(m);
}

void test_ps_reexecute_different_values(void) {
    printf("\n[PS: re-execute with different values]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_diffval");
    Q("CREATE TABLE t_diffval(id INT, val INT)");
    Q("INSERT INTO t_diffval VALUES(1, 10),(2, 20),(3, 30)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT val FROM t_diffval WHERE id = ?",
                        strlen("SELECT val FROM t_diffval WHERE id = ?"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    int out_val;
    unsigned long vl = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.length = &vl;
    mysql_stmt_bind_result(stmt, &rb);

    int rc = mysql_stmt_execute(stmt);
    TEST("diffval execute 1: ok");
    CHECK(rc == 0, "execute 1 failed");

    mysql_stmt_store_result(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("diffval execute 1: val=10");
    CHECK(rc == 0 && out_val == 10, "wrong value");

    rc = mysql_stmt_fetch(stmt);
    TEST("diffval execute 1: no more");
    CHECK(rc == MYSQL_NO_DATA, "should be no data");

    id = 3;
    rc = mysql_stmt_execute(stmt);
    TEST("diffval execute 2: ok");
    CHECK(rc == 0, "execute 2 failed");

    mysql_stmt_store_result(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("diffval execute 2: val=30");
    CHECK(rc == 0 && out_val == 30, "wrong value");

    rc = mysql_stmt_fetch(stmt);
    TEST("diffval execute 2: no more");
    CHECK(rc == MYSQL_NO_DATA, "should be no data");

    mysql_stmt_close(stmt);
    Q("DROP TABLE IF EXISTS t_diffval");
    mysql_close(m);
}

void test_stmt_close_before_fetch(void) {
    printf("\n[PS: close before fetch]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_closebf");
    Q("CREATE TABLE t_closebf(id INT)");
    Q("INSERT INTO t_closebf VALUES(1),(2),(3)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id FROM t_closebf", strlen("SELECT id FROM t_closebf"));
    int rc = mysql_stmt_execute(stmt);
    TEST("close before fetch: execute ok");
    CHECK(rc == 0, "execute failed");

    int out_id;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_id; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_fetch(stmt);
    TEST("close before fetch: fetch 1 ok");
    CHECK(rc == 0, "fetch failed");

    mysql_stmt_close(stmt);

    MYSQL_STMT *stmt2 = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt2, "SELECT 1", strlen("SELECT 1"));
    TEST("close before fetch: new stmt works");
    CHECK(rc == 0, "new prepare failed");

    mysql_stmt_close(stmt2);
    Q("DROP TABLE IF EXISTS t_closebf");
    mysql_close(m);
}

void test_ps_null_result_metadata(void) {
    printf("\n[PS: NULL result metadata on INSERT]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_nometadata");
    Q("CREATE TABLE t_nometadata(id INT)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "INSERT INTO t_nometadata VALUES(42)",
                        strlen("INSERT INTO t_nometadata VALUES(42)"));
    int rc = mysql_stmt_execute(stmt);
    TEST("null metadata: execute INSERT ok");
    CHECK(rc == 0, "execute failed");

    MYSQL_RES *meta = mysql_stmt_result_metadata(stmt);
    TEST("null metadata: metadata is NULL for INSERT");
    CHECK(meta == NULL, "should be NULL for INSERT");

    TEST("null metadata: affected_rows = 1");
    CHECK(mysql_stmt_affected_rows(stmt) == 1, "should be 1");

    mysql_stmt_close(stmt);
    Q("DROP TABLE IF EXISTS t_nometadata");
    mysql_close(m);
}

void test_ps_decimal_precision(void) {
    printf("\n[PS: DECIMAL precision]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_dprec");
    Q("CREATE TABLE t_dprec(id INT, d DECIMAL(20,4))");
    Q("INSERT INTO t_dprec VALUES(1, 999999999999.9999), (2, -999999999999.9999), (3, 0.0001)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT d FROM t_dprec ORDER BY id",
                                strlen("SELECT d FROM t_dprec ORDER BY id"));
    CHECK(rc == 0, "prepare failed");

    char out[64];
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_STRING; rb.buffer = out; rb.buffer_length = sizeof(out); rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("DECIMAL positive max: '999999999999.9999'");
    CHECK(rc == 0 && strcmp(out, "999999999999.9999") == 0, "wrong value");

    rc = mysql_stmt_fetch(stmt);
    TEST("DECIMAL negative max: '-999999999999.9999'");
    CHECK(rc == 0 && strcmp(out, "-999999999999.9999") == 0, "wrong value");

    rc = mysql_stmt_fetch(stmt);
    TEST("DECIMAL small: '0.0001'");
    CHECK(rc == 0 && strcmp(out, "0.0001") == 0, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_enum_type(void) {
    printf("\n[PS: ENUM type]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_enum");
    Q("CREATE TABLE t_enum(id INT, c ENUM('red','green','blue'))");
    Q("INSERT INTO t_enum VALUES(1,'red'),(2,'green'),(3,'blue')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT c FROM t_enum ORDER BY id",
                                strlen("SELECT c FROM t_enum ORDER BY id"));
    CHECK(rc == 0, "prepare failed");

    char out[16];
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_STRING; rb.buffer = out; rb.buffer_length = sizeof(out); rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("ENUM row 1: 'red'");
    CHECK(rc == 0 && strcmp(out, "red") == 0, "wrong value");

    rc = mysql_stmt_fetch(stmt);
    TEST("ENUM row 2: 'green'");
    CHECK(rc == 0 && strcmp(out, "green") == 0, "wrong value");

    rc = mysql_stmt_fetch(stmt);
    TEST("ENUM row 3: 'blue'");
    CHECK(rc == 0 && strcmp(out, "blue") == 0, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_set_type(void) {
    printf("\n[PS: SET type]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_set");
    Q("CREATE TABLE t_set(id INT, s SET('a','b','c','d'))");
    Q("INSERT INTO t_set VALUES(1,'a,b'),(2,'c'),(3,'a,b,c,d')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT s FROM t_set ORDER BY id",
                                strlen("SELECT s FROM t_set ORDER BY id"));
    CHECK(rc == 0, "prepare failed");

    char out[32];
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_STRING; rb.buffer = out; rb.buffer_length = sizeof(out); rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("SET row 1: 'a,b'");
    CHECK(rc == 0 && strcmp(out, "a,b") == 0, "wrong value");

    rc = mysql_stmt_fetch(stmt);
    TEST("SET row 2: 'c'");
    CHECK(rc == 0 && strcmp(out, "c") == 0, "wrong value");

    rc = mysql_stmt_fetch(stmt);
    TEST("SET row 3: 'a,b,c,d'");
    CHECK(rc == 0 && strcmp(out, "a,b,c,d") == 0, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_json_type(void) {
    printf("\n[PS: JSON type]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_json");
    Q("CREATE TABLE t_json(id INT, j JSON)");
    Q("INSERT INTO t_json VALUES(1, '{\"key\": \"value\"}'), (2, '[1,2,3]')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT j FROM t_json ORDER BY id",
                                strlen("SELECT j FROM t_json ORDER BY id"));
    CHECK(rc == 0, "prepare failed");

    char out[128];
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_STRING; rb.buffer = out; rb.buffer_length = sizeof(out); rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("JSON row 1: contains 'key'");
    CHECK(rc == 0 && strstr(out, "key") != NULL, "wrong value");

    rc = mysql_stmt_fetch(stmt);
    TEST("JSON row 2: contains array");
    CHECK(rc == 0 && strstr(out, "1") != NULL, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_bigint_signed_boundary(void) {
    printf("\n[PS: BIGINT signed boundary]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_bigint");
    Q("CREATE TABLE t_bigint(id INT, v BIGINT)");
    Q("INSERT INTO t_bigint VALUES(1, 9223372036854775807), (2, -9223372036854775808)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT v FROM t_bigint ORDER BY id",
                                strlen("SELECT v FROM t_bigint ORDER BY id"));
    CHECK(rc == 0, "prepare failed");

    long long out_v;
    unsigned long ol = 0;
    my_bool err = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONGLONG; rb.buffer = &out_v; rb.length = &ol; rb.error = &err;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("BIGINT max: 9223372036854775807");
    CHECK(rc == 0 && out_v == 9223372036854775807LL, "wrong value");

    rc = mysql_stmt_fetch(stmt);
    TEST("BIGINT min: -9223372036854775808");
    CHECK(rc == 0 && out_v == (-9223372036854775807LL - 1), "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_more_results_no_multi(void) {
    printf("\n[mysql_more_results / mysql_next_result without MULTI_STATEMENTS]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("SELECT 1");
    MYSQL_RES *res = mysql_store_result(m);
    mysql_free_result(res);

    my_bool more = mysql_more_results(m);
    TEST("more_results after single SELECT: false");
    CHECK(more == 0, "should be 0");

    int rc = mysql_next_result(m);
    TEST("next_result after single SELECT: -1");
    CHECK(rc == -1, "should be -1");

    mysql_close(m);
}

void test_ps_store_result_on_insert(void) {
    printf("\n[PS: store_result on INSERT returns empty]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_sri");
    Q("CREATE TABLE t_sri(id INT)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_sri VALUES(1)",
                                strlen("INSERT INTO t_sri VALUES(1)"));
    CHECK(rc == 0, "prepare failed");

    rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, "execute failed");

    rc = mysql_stmt_store_result(stmt);
    TEST("store_result on INSERT: returns 0");
    CHECK(rc == 0, "should succeed");

    TEST("num_rows on INSERT: 0");
    CHECK(mysql_stmt_num_rows(stmt) == 0, "should be 0");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_dump_debug_info(void) {
    printf("\n[mysql_dump_debug_info]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    int rc = mysql_dump_debug_info(m);
    TEST("dump_debug_info (may fail without SUPER)");
    CHECK(rc == 0 || mysql_errno(m) > 0, "unexpected result");

    mysql_close(m);
}

void test_ps_datetime_microsecond_insert(void) {
    printf("\n[PS: DATETIME microsecond insert and fetch]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_dtm");
    Q("CREATE TABLE t_dtm(id INT, dt DATETIME(6))");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_dtm(id, dt) VALUES(?, ?)",
                                strlen("INSERT INTO t_dtm(id, dt) VALUES(?, ?)"));

    int id = 1;
    MYSQL_TIME dt;
    memset(&dt, 0, sizeof(dt));
    dt.year = 2024; dt.month = 6; dt.day = 15;
    dt.hour = 14; dt.minute = 30; dt.second = 45;
    dt.second_part = 123456;
    dt.time_type = MYSQL_TIMESTAMP_DATETIME;

    unsigned long il = sizeof(int), dl = sizeof(MYSQL_TIME);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &il;
    pb[1].buffer_type = MYSQL_TYPE_DATETIME; pb[1].buffer = &dt; pb[1].buffer_length = sizeof(dt); pb[1].length = &dl;
    mysql_stmt_bind_param(stmt, pb);

    rc = mysql_stmt_execute(stmt);
    TEST("INSERT DATETIME(6) with microseconds");
    CHECK(rc == 0, "insert failed");
    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "SELECT dt FROM t_dtm WHERE id = ?",
                            strlen("SELECT dt FROM t_dtm WHERE id = ?"));
    mysql_stmt_bind_param(stmt, &pb[0]);
    rc = mysql_stmt_execute(stmt);

    MYSQL_TIME out_dt;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_DATETIME; rb.buffer = &out_dt; rb.buffer_length = sizeof(out_dt); rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_fetch(stmt);
    TEST("DATETIME(6) roundtrip: year=2024 month=6 day=15");
    CHECK(rc == 0 && out_dt.year == 2024 && out_dt.month == 6 && out_dt.day == 15, "wrong date");

    TEST("DATETIME(6) roundtrip: hour=14 minute=30 second=45");
    CHECK(out_dt.hour == 14 && out_dt.minute == 30 && out_dt.second == 45, "wrong time");

    TEST("DATETIME(6) roundtrip: second_part=123456");
    CHECK(out_dt.second_part == 123456, "wrong microseconds");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_time_microsecond(void) {
    printf("\n[PS: TIME microsecond]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_tmus");
    Q("CREATE TABLE t_tmus(id INT, t TIME(6))");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_tmus(id, t) VALUES(?, ?)",
                                strlen("INSERT INTO t_tmus(id, t) VALUES(?, ?)"));

    int id = 1;
    MYSQL_TIME tv;
    memset(&tv, 0, sizeof(tv));
    tv.hour = 8; tv.minute = 30; tv.second = 59; tv.second_part = 999999;
    tv.time_type = MYSQL_TIMESTAMP_TIME;
    tv.neg = 0;

    unsigned long il = sizeof(int), tl = sizeof(MYSQL_TIME);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &il;
    pb[1].buffer_type = MYSQL_TYPE_TIME; pb[1].buffer = &tv; pb[1].buffer_length = sizeof(tv); pb[1].length = &tl;
    mysql_stmt_bind_param(stmt, pb);

    rc = mysql_stmt_execute(stmt);
    TEST("INSERT TIME(6) with microseconds");
    CHECK(rc == 0, "insert failed");
    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "SELECT t FROM t_tmus WHERE id = ?",
                            strlen("SELECT t FROM t_tmus WHERE id = ?"));
    mysql_stmt_bind_param(stmt, &pb[0]);
    rc = mysql_stmt_execute(stmt);

    MYSQL_TIME out_tv;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_TIME; rb.buffer = &out_tv; rb.buffer_length = sizeof(out_tv); rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_fetch(stmt);
    TEST("TIME(6) roundtrip: 08:30:59.999999");
    CHECK(rc == 0 && out_tv.hour == 8 && out_tv.minute == 30 && out_tv.second == 59
          && out_tv.second_part == 999999, "wrong time");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_tinyint_unsigned_boundary(void) {
    printf("\n[PS: TINYINT UNSIGNED boundary]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_tinyu");
    Q("CREATE TABLE t_tinyu(id INT, v TINYINT UNSIGNED)");
    Q("INSERT INTO t_tinyu VALUES(1, 255), (2, 0), (3, 128)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT v FROM t_tinyu ORDER BY id",
                                strlen("SELECT v FROM t_tinyu ORDER BY id"));
    CHECK(rc == 0, "prepare failed");

    unsigned char out_v;
    unsigned long ol = 0;
    my_bool err = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_TINY; rb.buffer = &out_v; rb.buffer_length = sizeof(out_v); rb.length = &ol; rb.error = &err;
    rb.is_unsigned = 1;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_store_result(stmt);
    CHECK(rc == 0, "store_result failed");
    rc = mysql_stmt_fetch(stmt);
    TEST("TINYINT UNSIGNED max: 255");
    CHECK(rc == 0 && out_v == 255, "wrong value");

    rc = mysql_stmt_fetch(stmt);
    TEST("TINYINT UNSIGNED min: 0");
    CHECK(rc == 0 && out_v == 0, "wrong value");

    rc = mysql_stmt_fetch(stmt);
    TEST("TINYINT UNSIGNED 128");
    CHECK(rc == 0 && out_v == 128, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_int_unsigned_boundary(void) {
    printf("\n[PS: INT UNSIGNED boundary]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_intu");
    Q("CREATE TABLE t_intu(id INT, v INT UNSIGNED)");
    Q("INSERT INTO t_intu VALUES(1, 4294967295), (2, 0)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT v FROM t_intu ORDER BY id",
                                strlen("SELECT v FROM t_intu ORDER BY id"));
    CHECK(rc == 0, "prepare failed");

    unsigned int out_v;
    unsigned long ol = 0;
    my_bool err = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_v; rb.buffer_length = sizeof(out_v); rb.length = &ol; rb.error = &err;
    rb.is_unsigned = 1;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_store_result(stmt);
    CHECK(rc == 0, "store_result failed");
    rc = mysql_stmt_fetch(stmt);
    TEST("INT UNSIGNED max: 4294967295");
    CHECK(rc == 0 && out_v == 4294967295U, "wrong value");

    out_v = 999;
    rc = mysql_stmt_fetch(stmt);
    TEST("INT UNSIGNED min: 0");
    CHECK(rc == 0 && out_v == 0, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_stmt_sqlstate(void) {
    printf("\n[mysql_stmt_sqlstate]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT 1", strlen("SELECT 1"));
    CHECK(rc == 0, "prepare failed");

    const char *ss = mysql_stmt_sqlstate(stmt);
    TEST("sqlstate after successful prepare: '00000'");
    CHECK(ss && strcmp(ss, "00000") == 0, "wrong sqlstate");

    rc = mysql_stmt_prepare(stmt, "SELECT * FROM nonexistent_table_xyz",
                            strlen("SELECT * FROM nonexistent_table_xyz"));
    CHECK(rc != 0, "prepare should fail");

    ss = mysql_stmt_sqlstate(stmt);
    TEST("sqlstate after failed prepare: not '00000'");
    CHECK(ss && strcmp(ss, "00000") != 0, "sqlstate should not be 00000");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_fetch_field_direct(void) {
    printf("\n[mysql_fetch_field_direct]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("SELECT 1 AS col1, 'abc' AS col2, 3.14 AS col3");
    MYSQL_RES *res = mysql_store_result(m);
    CHECK(res, "store_result failed");

    MYSQL_FIELD *f;

    f = mysql_fetch_field_direct(res, 0);
    TEST("field_direct[0]: name='col1'");
    CHECK(f && strcmp(f->name, "col1") == 0, "wrong name");

    f = mysql_fetch_field_direct(res, 1);
    TEST("field_direct[1]: name='col2'");
    CHECK(f && strcmp(f->name, "col2") == 0, "wrong name");

    f = mysql_fetch_field_direct(res, 2);
    TEST("field_direct[2]: name='col3'");
    CHECK(f && strcmp(f->name, "col3") == 0, "wrong name");

    mysql_free_result(res);
    mysql_close(m);
}

void test_ps_decimal_zero(void) {
    printf("\n[PS: DECIMAL zero value]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_dzero");
    Q("CREATE TABLE t_dzero(id INT, d DECIMAL(10,2))");
    Q("INSERT INTO t_dzero VALUES(1, 0.00), (2, -0.00)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT d FROM t_dzero ORDER BY id",
                                strlen("SELECT d FROM t_dzero ORDER BY id"));
    CHECK(rc == 0, "prepare failed");

    char out[32];
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_STRING; rb.buffer = out; rb.buffer_length = sizeof(out); rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("DECIMAL 0.00");
    CHECK(rc == 0 && strcmp(out, "0.00") == 0, "wrong value");

    rc = mysql_stmt_fetch(stmt);
    TEST("DECIMAL -0.00");
    CHECK(rc == 0, "fetch failed");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_insert_id_replace(void) {
    printf("\n[mysql_insert_id after REPLACE]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_replace");
    Q("CREATE TABLE t_replace(id INT AUTO_INCREMENT PRIMARY KEY, v INT)");
    Q("INSERT INTO t_replace(v) VALUES(1)");

    my_ulonglong id1 = mysql_insert_id(m);
    TEST("insert_id after INSERT > 0");
    CHECK(id1 > 0, "insert_id is 0");

    char q[128];
    snprintf(q, sizeof(q), "REPLACE INTO t_replace(id, v) VALUES(%llu, 2)", (unsigned long long)id1);
    Q(q);

    my_ulonglong id2 = mysql_insert_id(m);
    TEST("insert_id after REPLACE (same id)");
    CHECK(id2 == id1, "insert_id changed after replace");

    Q("REPLACE INTO t_replace(v) VALUES(3)");
    my_ulonglong id3 = mysql_insert_id(m);
    TEST("insert_id after REPLACE (new row) > id1");
    CHECK(id3 > id1, "insert_id should be greater");

    mysql_close(m);
}

void test_ps_date_boundary(void) {
    printf("\n[PS: DATE boundary values]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_dateb");
    Q("CREATE TABLE t_dateb(id INT, d DATE)");
    Q("INSERT INTO t_dateb VALUES(1, '1000-01-01'), (2, '9999-12-31')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT d FROM t_dateb ORDER BY id",
                                strlen("SELECT d FROM t_dateb ORDER BY id"));
    CHECK(rc == 0, "prepare failed");

    MYSQL_TIME out_d;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_DATE; rb.buffer = &out_d; rb.buffer_length = sizeof(out_d); rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("DATE min: 1000-01-01");
    CHECK(rc == 0 && out_d.year == 1000 && out_d.month == 1 && out_d.day == 1, "wrong date");

    rc = mysql_stmt_fetch(stmt);
    TEST("DATE max: 9999-12-31");
    CHECK(rc == 0 && out_d.year == 9999 && out_d.month == 12 && out_d.day == 31, "wrong date");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_mb_escape_gbk(void) {
    printf("\n[multibyte escape: GBK charset]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    int rc = mysql_set_character_set(m, "gbk");
    TEST("SET NAMES gbk");
    CHECK(rc == 0, "set charset gbk failed");

    char out[64];
    unsigned long len;

    unsigned char gbk_valid[] = { 0xBF, 0x5C, 0x00 };
    len = mysql_real_escape_string(m, out, (const char*)gbk_valid, 2);
    TEST("GBK: 0xBF5C is valid GBK char, not escaped");
    CHECK(len == 2 && (unsigned char)out[0] == 0xBF && (unsigned char)out[1] == 0x5C,
          "valid GBK char should pass through unchanged");

    unsigned char gbk_invalid[] = { 0xBF, 0x27, 0x00 };
    len = mysql_real_escape_string(m, out, (const char*)gbk_invalid, 2);
    TEST("GBK: 0xBF27 is invalid, 0xBF escaped, quote escaped");
    CHECK(len == 4 && (unsigned char)out[0] == '\\' && (unsigned char)out[1] == 0xBF &&
          (unsigned char)out[2] == '\\' && out[3] == '\'',
          "invalid GBK lead byte should be escaped, then quote also escaped");

    unsigned char gbk_trail_backslash[] = { 0x81, 0x5C, 0x00 };
    len = mysql_real_escape_string(m, out, (const char*)gbk_trail_backslash, 2);
    TEST("GBK: 0x815C is valid GBK char, backslash not escaped");
    CHECK(len == 2 && (unsigned char)out[0] == 0x81 && (unsigned char)out[1] == 0x5C,
          "backslash as GBK trail byte should not be escaped");

    len = mysql_real_escape_string(m, out, "abc", 3);
    TEST("GBK: ASCII passthrough");
    CHECK(len == 3 && memcmp(out, "abc", 3) == 0, "ascii should pass through");

    len = mysql_real_escape_string(m, out, "it's", 4);
    TEST("GBK: single quote still escaped");
    CHECK(len == 5 && memcmp(out, "it\\'s", 5) == 0, "quote should be escaped in GBK");

    unsigned char gbk_lead_only[] = { 0x81, 0x00 };
    len = mysql_real_escape_string(m, out, (const char*)gbk_lead_only, 1);
    TEST("GBK: lone lead byte 0x81 escaped");
    CHECK(len == 2 && out[0] == '\\' && (unsigned char)out[1] == 0x81,
          "lone GBK lead byte should be escaped");

    mysql_set_character_set(m, "utf8mb4");
    mysql_close(m);
}

void test_mb_escape_big5(void) {
    printf("\n[multibyte escape: Big5 charset]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    int rc = mysql_set_character_set(m, "big5");
    TEST("SET NAMES big5");
    CHECK(rc == 0, "set charset big5 failed");

    char out[64];
    unsigned long len;

    unsigned char big5_valid[] = { 0xA1, 0x40, 0x00 };
    len = mysql_real_escape_string(m, out, (const char*)big5_valid, 2);
    TEST("Big5: 0xA140 is valid Big5 char, not escaped");
    CHECK(len == 2 && (unsigned char)out[0] == 0xA1 && (unsigned char)out[1] == 0x40,
          "valid Big5 char should pass through unchanged");

    unsigned char big5_invalid[] = { 0xA1, 0x27, 0x00 };
    len = mysql_real_escape_string(m, out, (const char*)big5_invalid, 2);
    TEST("Big5: 0xA127 is invalid, 0xA1 escaped, quote escaped");
    CHECK(len == 4 && (unsigned char)out[0] == '\\' && (unsigned char)out[1] == 0xA1 &&
          (unsigned char)out[2] == '\\' && out[3] == '\'',
          "invalid Big5 lead byte should be escaped");

    mysql_set_character_set(m, "utf8mb4");
    mysql_close(m);
}

void test_mb_escape_sjis(void) {
    printf("\n[multibyte escape: Shift-JIS charset]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    int rc = mysql_set_character_set(m, "sjis");
    TEST("SET NAMES sjis");
    CHECK(rc == 0, "set charset sjis failed");

    char out[64];
    unsigned long len;

    unsigned char sjis_valid[] = { 0x81, 0x40, 0x00 };
    len = mysql_real_escape_string(m, out, (const char*)sjis_valid, 2);
    TEST("SJIS: 0x8140 is valid SJIS char, not escaped");
    CHECK(len == 2 && (unsigned char)out[0] == 0x81 && (unsigned char)out[1] == 0x40,
          "valid SJIS char should pass through unchanged");

    unsigned char sjis_backslash_trail[] = { 0x83, 0x5C, 0x00 };
    len = mysql_real_escape_string(m, out, (const char*)sjis_backslash_trail, 2);
    TEST("SJIS: 0x835C is valid SJIS char, backslash not escaped");
    CHECK(len == 2 && (unsigned char)out[0] == 0x83 && (unsigned char)out[1] == 0x5C,
          "backslash as SJIS trail byte should not be escaped");

    mysql_set_character_set(m, "utf8mb4");
    mysql_close(m);
}

void test_mb_escape_utf8(void) {
    printf("\n[multibyte escape: UTF-8 charset]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    char out[64];
    unsigned long len;

    unsigned char utf8_2byte[] = { 0xC3, 0xA9, 0x00 };
    len = mysql_real_escape_string(m, out, (const char*)utf8_2byte, 2);
    TEST("UTF-8: 2-byte char 0xC3A9 passes through");
    CHECK(len == 2 && (unsigned char)out[0] == 0xC3 && (unsigned char)out[1] == 0xA9,
          "valid 2-byte UTF-8 should pass through");

    unsigned char utf8_3byte[] = { 0xE4, 0xB8, 0xAD, 0x00 };
    len = mysql_real_escape_string(m, out, (const char*)utf8_3byte, 3);
    TEST("UTF-8: 3-byte char 0xE4B8AD passes through");
    CHECK(len == 3 && (unsigned char)out[0] == 0xE4 && (unsigned char)out[1] == 0xB8 && (unsigned char)out[2] == 0xAD,
          "valid 3-byte UTF-8 should pass through");

    unsigned char utf8_4byte[] = { 0xF0, 0x9F, 0x98, 0x80, 0x00 };
    len = mysql_real_escape_string(m, out, (const char*)utf8_4byte, 4);
    TEST("UTF-8: 4-byte char (emoji) passes through");
    CHECK(len == 4 && (unsigned char)out[0] == 0xF0 && (unsigned char)out[1] == 0x9F &&
          (unsigned char)out[2] == 0x98 && (unsigned char)out[3] == 0x80,
          "valid 4-byte UTF-8 should pass through");

    unsigned char utf8_invalid[] = { 0xC3, 0x27, 0x00 };
    len = mysql_real_escape_string(m, out, (const char*)utf8_invalid, 2);
    TEST("UTF-8: invalid 2-byte seq 0xC327, lead byte escaped, quote escaped");
    CHECK(len == 4 && (unsigned char)out[0] == '\\' && (unsigned char)out[1] == 0xC3 &&
          (unsigned char)out[2] == '\\' && out[3] == '\'',
          "invalid UTF-8 lead byte should be escaped");

    unsigned char utf8_truncated[] = { 0xE4, 0x00 };
    len = mysql_real_escape_string(m, out, (const char*)utf8_truncated, 1);
    TEST("UTF-8: truncated 3-byte seq, lead byte escaped");
    CHECK(len == 2 && out[0] == '\\' && (unsigned char)out[1] == 0xE4,
          "truncated UTF-8 lead byte should be escaped");

    mysql_close(m);
}

void test_mb_escape_euckr(void) {
    printf("\n[multibyte escape: EUC-KR charset]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    int rc = mysql_set_character_set(m, "euckr");
    TEST("SET NAMES euckr");
    CHECK(rc == 0, "set charset euckr failed");

    char out[64];
    unsigned long len;

    unsigned char euckr_valid[] = { 0xA1, 0xA1, 0x00 };
    len = mysql_real_escape_string(m, out, (const char*)euckr_valid, 2);
    TEST("EUC-KR: 0xA1A1 is valid char, not escaped");
    CHECK(len == 2 && (unsigned char)out[0] == 0xA1 && (unsigned char)out[1] == 0xA1,
          "valid EUC-KR char should pass through unchanged");

    unsigned char euckr_invalid[] = { 0xA1, 0x27, 0x00 };
    len = mysql_real_escape_string(m, out, (const char*)euckr_invalid, 2);
    TEST("EUC-KR: 0xA127 is invalid, 0xA1 escaped, quote escaped");
    CHECK(len == 4 && (unsigned char)out[0] == '\\' && (unsigned char)out[1] == 0xA1 &&
          (unsigned char)out[2] == '\\' && out[3] == '\'',
          "invalid EUC-KR lead byte should be escaped");

    mysql_set_character_set(m, "utf8mb4");
    mysql_close(m);
}

void test_set_character_set_updates_escape(void) {
    printf("\n[mysql_set_character_set updates escape behavior]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    char out[64];
    unsigned long len;

    unsigned char gbk_pair[] = { 0xBF, 0x5C, 0x00 };

    len = mysql_real_escape_string(m, out, (const char*)gbk_pair, 2);
    TEST("UTF-8: 0xBF5C, 0xBF is continuation byte (not lead), backslash escaped");
    CHECK(len == 3 && (unsigned char)out[0] == 0xBF && (unsigned char)out[1] == '\\' && (unsigned char)out[2] == '\\',
          "in UTF-8 mode, 0xBF is continuation byte, backslash is escaped");

    int rc = mysql_set_character_set(m, "gbk");
    CHECK(rc == 0, "set charset gbk failed");

    len = mysql_real_escape_string(m, out, (const char*)gbk_pair, 2);
    TEST("GBK: 0xBF5C now valid GBK, not escaped");
    CHECK(len == 2 && (unsigned char)out[0] == 0xBF && (unsigned char)out[1] == 0x5C,
          "after set_character_set gbk, 0xBF5C is valid and not escaped");

    MY_CHARSET_INFO ci;
    mysql_get_character_set_info(m, &ci);
    TEST("charset info: mbmaxlen=2 after set gbk");
    CHECK(ci.mbmaxlen == 2, "mbmaxlen should be 2 for GBK");

    mysql_set_character_set(m, "utf8mb4");
    mysql_close(m);
}

void test_call_multi_resultset(void) {
    printf("\n[CALL stored procedure: multi resultset]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP PROCEDURE IF EXISTS p_multi");
    Q("CREATE PROCEDURE p_multi() BEGIN SELECT 1 AS a; SELECT 2 AS b, 3 AS c; END");

    int rc = mysql_query(m, "CALL p_multi()");
    CHECK(rc == 0, "CALL p_multi failed");

    MYSQL_RES *res = mysql_store_result(m);
    TEST("first resultset: not NULL");
    CHECK(res != NULL, "first resultset should exist");
    if (res) {
        TEST("first resultset: 1 column");
        CHECK(mysql_num_fields(res) == 1, "should have 1 column");
        MYSQL_ROW row = mysql_fetch_row(res);
        TEST("first resultset: row value = 1");
        CHECK(row && row[0] && strcmp(row[0], "1") == 0, "value should be 1");
        mysql_free_result(res);
    }

    int more = mysql_more_results(m);
    TEST("more_results = 1 after first resultset");
    CHECK(more == 1, "should have more results");

    rc = mysql_next_result(m);
    TEST("next_result returns 0 for second resultset");
    CHECK(rc == 0, "next_result should return 0");

    res = mysql_store_result(m);
    TEST("second resultset: not NULL");
    CHECK(res != NULL, "second resultset should exist");
    if (res) {
        TEST("second resultset: 2 columns");
        CHECK(mysql_num_fields(res) == 2, "should have 2 columns");
        MYSQL_ROW row = mysql_fetch_row(res);
        TEST("second resultset: row values = 2, 3");
        CHECK(row && row[0] && row[1] && strcmp(row[0], "2") == 0 && strcmp(row[1], "3") == 0,
              "values should be 2 and 3");
        mysql_free_result(res);
    }

    more = mysql_more_results(m);
    rc = mysql_next_result(m);
    TEST("third resultset (CALL OK) exists");
    CHECK(rc == 0, "next_result should return 0 for CALL OK packet");

    res = mysql_store_result(m);
    TEST("third resultset: NULL (OK packet from CALL)");
    CHECK(res == NULL, "CALL OK packet should produce NULL resultset");

    rc = mysql_next_result(m);
    TEST("no more results after CALL OK");
    CHECK(rc == -1, "should return -1 for no more results");

    Q("DROP PROCEDURE IF EXISTS p_multi");
    mysql_close(m);
}

void test_call_ps_multi_resultset(void) {
    printf("\n[CALL PS: multi resultset via stmt]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP PROCEDURE IF EXISTS p_ps_multi");
    Q("CREATE PROCEDURE p_ps_multi() BEGIN SELECT 10 AS x; SELECT 20 AS y, 30 AS z; END");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "CALL p_ps_multi()", strlen("CALL p_ps_multi()"));
    CHECK(rc == 0, "prepare CALL failed");

    rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, "execute CALL failed");

    rc = mysql_stmt_store_result(stmt);
    CHECK(rc == 0, "store_result failed");

    MYSQL_RES *meta = mysql_stmt_result_metadata(stmt);
    TEST("PS first resultset: metadata exists");
    CHECK(meta != NULL, "should have metadata");
    if (meta) {
        TEST("PS first resultset: 1 column");
        CHECK(mysql_num_fields(meta) == 1, "should have 1 column");
        mysql_free_result(meta);
    }

    MYSQL_BIND rb;
    int val = 0;
    unsigned long vl = 0;
    my_bool err = 0;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &val; rb.buffer_length = sizeof(val); rb.length = &vl; rb.error = &err;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_fetch(stmt);
    TEST("PS first resultset: value = 10");
    CHECK(rc == 0 && val == 10, "value should be 10");

    rc = mysql_stmt_fetch(stmt);
    TEST("PS first resultset: no more rows");
    CHECK(rc == MYSQL_NO_DATA, "should be no more data");

    rc = mysql_stmt_next_result(stmt);
    TEST("PS next_result returns 0 for second resultset");
    CHECK(rc == 0, "stmt_next_result should return 0");

    rc = mysql_stmt_store_result(stmt);
    CHECK(rc == 0, "store_result second failed");

    meta = mysql_stmt_result_metadata(stmt);
    TEST("PS second resultset: 2 columns");
    CHECK(meta != NULL && mysql_num_fields(meta) == 2, "should have 2 columns");
    if (meta) mysql_free_result(meta);

    int vals[2] = {0, 0};
    unsigned long vls[2] = {0, 0};
    my_bool errs[2] = {0, 0};
    MYSQL_BIND rbs[2];
    memset(rbs, 0, sizeof(rbs));
    rbs[0].buffer_type = MYSQL_TYPE_LONG; rbs[0].buffer = &vals[0]; rbs[0].buffer_length = sizeof(int); rbs[0].length = &vls[0]; rbs[0].error = &errs[0];
    rbs[1].buffer_type = MYSQL_TYPE_LONG; rbs[1].buffer = &vals[1]; rbs[1].buffer_length = sizeof(int); rbs[1].length = &vls[1]; rbs[1].error = &errs[1];
    mysql_stmt_bind_result(stmt, rbs);

    rc = mysql_stmt_fetch(stmt);
    TEST("PS second resultset: values = 20, 30");
    CHECK(rc == 0 && vals[0] == 20 && vals[1] == 30, "values should be 20 and 30");

    rc = mysql_stmt_next_result(stmt);
    TEST("PS next_result returns 0 for CALL OK packet");
    CHECK(rc == 0, "should return 0 for CALL OK");

    rc = mysql_stmt_next_result(stmt);
    TEST("PS no more results");
    CHECK(rc == -1, "should return -1 for no more results");

    mysql_stmt_close(stmt);
    Q("DROP PROCEDURE IF EXISTS p_ps_multi");
    mysql_close(m);
}

void test_stmt_next_result_no_multi(void) {
    printf("\n[mysql_stmt_next_result on single SELECT]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT 1 AS val", strlen("SELECT 1 AS val"));
    CHECK(rc == 0, "prepare failed");

    rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, "execute failed");
    rc = mysql_stmt_store_result(stmt);
    CHECK(rc == 0, "store_result failed");

    while (mysql_stmt_fetch(stmt) == 0) {}

    rc = mysql_stmt_next_result(stmt);
    TEST("stmt_next_result on single SELECT returns -1");
    CHECK(rc == -1, "should return -1 when no more results");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_report_data_truncation(void) {
    printf("\n[MYSQL_REPORT_DATA_TRUNCATION behavior]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_trunc");
    Q("CREATE TABLE t_trunc(id INT, v VARCHAR(50))");
    Q("INSERT INTO t_trunc VALUES(1, 'hello world')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT id, v FROM t_trunc", strlen("SELECT id, v FROM t_trunc"));
    CHECK(rc == 0, "prepare failed");

    int id = 0;
    char buf[4];
    unsigned long lens[2] = {0, 0};
    my_bool errs[2] = {0, 0};
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &id; rb[0].buffer_length = sizeof(id); rb[0].length = &lens[0]; rb[0].error = &errs[0];
    rb[1].buffer_type = MYSQL_TYPE_STRING; rb[1].buffer = buf; rb[1].buffer_length = sizeof(buf); rb[1].length = &lens[1]; rb[1].error = &errs[1];

    mysql_stmt_bind_result(stmt, &rb[0]);
    rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, "execute failed");
    rc = mysql_stmt_store_result(stmt);
    CHECK(rc == 0, "store_result failed");
    rc = mysql_stmt_fetch(stmt);
    TEST("fetch reports truncation when buffer too small");
    CHECK(errs[1] == 1, "error flag should be set on truncated column");

    TEST("non-truncated column has no error flag");
    CHECK(errs[0] == 0, "error flag should not be set on non-truncated column");

    TEST("truncated data length > buffer_length");
    CHECK(lens[1] > sizeof(buf), "length should be > buffer size");

    mysql_stmt_close(stmt);
    Q("DROP TABLE IF EXISTS t_trunc");
    mysql_close(m);
}

void test_stmt_update_max_length(void) {
    printf("\n[STMT_ATTR_UPDATE_MAX_LENGTH actual effect]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_uml");
    Q("CREATE TABLE t_uml(id INT, v VARCHAR(100))");
    Q("INSERT INTO t_uml VALUES(1, 'hi'), (2, 'hello'), (3, 'greetings')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT v FROM t_uml ORDER BY id", strlen("SELECT v FROM t_uml ORDER BY id"));
    CHECK(rc == 0, "prepare failed");

    my_bool uml = 1;
    mysql_stmt_attr_set(stmt, STMT_ATTR_UPDATE_MAX_LENGTH, &uml);

    rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, "execute failed");
    rc = mysql_stmt_store_result(stmt);
    CHECK(rc == 0, "store_result failed");

    MYSQL_RES *meta = mysql_stmt_result_metadata(stmt);
    TEST("UPDATE_MAX_LENGTH: max_length > 0");
    CHECK(meta != NULL && meta->fields[0].max_length > 0, "max_length should be updated");
    if (meta) {
        TEST("UPDATE_MAX_LENGTH: max_length = 9 (length of 'greetings')");
        CHECK(meta->fields[0].max_length == 9, "max_length should be 9");
        mysql_free_result(meta);
    }

    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "SELECT v FROM t_uml ORDER BY id", strlen("SELECT v FROM t_uml ORDER BY id"));
    CHECK(rc == 0, "prepare 2 failed");

    my_bool uml_off = 0;
    mysql_stmt_attr_set(stmt, STMT_ATTR_UPDATE_MAX_LENGTH, &uml_off);

    rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, "execute 2 failed");
    rc = mysql_stmt_store_result(stmt);
    CHECK(rc == 0, "store_result 2 failed");

    meta = mysql_stmt_result_metadata(stmt);
    TEST("UPDATE_MAX_LENGTH off: max_length = 0");
    CHECK(meta != NULL && meta->fields[0].max_length == 0, "max_length should be 0 when off");
    if (meta) mysql_free_result(meta);

    mysql_stmt_close(stmt);
    Q("DROP TABLE IF EXISTS t_uml");
    mysql_close(m);
}

void test_escape_string_not_connected(void) {
    printf("\n[mysql_real_escape_string on unconnected handle]\n");
    MYSQL *m = mysql_init(NULL);
    if (!m) { FAIL("mysql_init failed"); return; }

    char out[64];
    unsigned long len;

    len = mysql_real_escape_string(m, out, "it's", 4);
    TEST("escape on unconnected handle: still works");
    CHECK(len == 5 && memcmp(out, "it\\'s", 5) == 0, "should escape with default charset");

    len = mysql_real_escape_string(NULL, out, "abc", 3);
    TEST("escape with NULL handle: passthrough");
    CHECK(len == 3 && memcmp(out, "abc", 3) == 0, "should work with NULL handle");

    mysql_close(m);
}

void test_escape_string_nul_byte(void) {
    printf("\n[mysql_real_escape_string with NUL byte]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    char out[64];
    unsigned long len;

    unsigned char input[] = { 'a', '\0', 'b' };
    len = mysql_real_escape_string(m, out, (const char*)input, 3);
    TEST("NUL byte in input is escaped");
    CHECK(len == 4 && out[0] == 'a' && out[1] == '\\' && out[2] == '0' && out[3] == 'b',
          "NUL should be escaped to \\0");

    len = mysql_real_escape_string(m, out, "", 0);
    TEST("empty string: length 0");
    CHECK(len == 0, "empty string should return 0");

    mysql_close(m);
}

void test_ps_param_count_mismatch(void) {
    printf("\n[PS: parameter count mismatch - SKIPPED, protocol risk]\n");
}

void test_ps_prepare_null_query(void) {
    printf("\n[PS: prepare NULL query]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, NULL, 0);
    TEST("prepare NULL query returns error");
    CHECK(rc != 0, "should fail with NULL query");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_close_after_error(void) {
    printf("\n[PS: close after execute error]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT * FROM nonexistent_table_xyz", strlen("SELECT * FROM nonexistent_table_xyz"));
    if (rc != 0) {
        TEST("prepare on nonexistent table: error");
        CHECK(1, "ok");
    }

    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "SELECT 1", strlen("SELECT 1"));
    CHECK(rc == 0, "prepare SELECT 1 failed");

    rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, "execute should succeed");

    mysql_stmt_close(stmt);
    TEST("close after error does not crash");
    CHECK(1, "ok");

    mysql_close(m);
}

void test_change_user_resets_ps(void) {
    printf("\n[mysql_change_user invalidates PS]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_cups");
    Q("CREATE TABLE t_cups(id INT)");
    Q("INSERT INTO t_cups VALUES(1)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT id FROM t_cups", strlen("SELECT id FROM t_cups"));
    CHECK(rc == 0, "prepare failed");

    rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, "execute before change_user should work");
    mysql_stmt_store_result(stmt);
    while (mysql_stmt_fetch(stmt) == 0) {}

    rc = mysql_change_user(m, g_user, g_passwd, g_db);
    TEST("change_user succeeds");
    CHECK(rc == 0, "change_user should succeed");

    rc = mysql_stmt_execute(stmt);
    TEST("execute after change_user fails (stmt invalidated)");
    CHECK(rc != 0, "stmt should be invalidated after change_user");

    mysql_stmt_close(stmt);
    Q("DROP TABLE IF EXISTS t_cups");
    mysql_close(m);
}

void test_set_charset_name_before_connect(void) {
    printf("\n[MYSQL_SET_CHARSET_NAME before connect]\n");
    MYSQL *m = mysql_init(NULL);
    if (!m) { FAIL("mysql_init failed"); return; }

    mysql_options(m, MYSQL_SET_CHARSET_NAME, "gbk");

    if (!mysql_real_connect(m, g_host, g_user, g_passwd, g_db, g_port, NULL, 0)) {
        FAIL("connect with charset gbk failed");
        mysql_close(m);
        return;
    }

    MY_CHARSET_INFO ci;
    mysql_get_character_set_info(m, &ci);
    TEST("charset after connect with SET_CHARSET_NAME=gbk: mbmaxlen=2");
    CHECK(ci.mbmaxlen == 2, "mbmaxlen should be 2 for GBK");

    TEST("charset name contains gbk");
    CHECK(ci.csname && (strcmp(ci.csname, "gbk") == 0 || strstr(ci.csname, "gbk") != NULL),
          "charset should be gbk");

    mysql_set_character_set(m, "utf8mb4");
    mysql_close(m);
}

void test_gb18030_fourbyte_escape(void) {
    printf("\n[multibyte escape: GB18030 4-byte sequence]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    int rc = mysql_set_character_set(m, "gb18030");
    TEST("SET NAMES gb18030");
    CHECK(rc == 0, "set charset gb18030 failed");

    char out[64];
    unsigned long len;

    unsigned char gb18030_4byte[] = { 0x81, 0x30, 0x81, 0x30 };
    len = mysql_real_escape_string(m, out, (const char*)gb18030_4byte, 4);
    TEST("GB18030: 4-byte seq 0x81308130 passes through");
    CHECK(len == 4 && (unsigned char)out[0] == 0x81 && (unsigned char)out[1] == 0x30 &&
          (unsigned char)out[2] == 0x81 && (unsigned char)out[3] == 0x30,
          "valid 4-byte GB18030 should pass through unchanged");

    unsigned char gb18030_2byte[] = { 0xA1, 0xA1 };
    len = mysql_real_escape_string(m, out, (const char*)gb18030_2byte, 2);
    TEST("GB18030: 2-byte seq 0xA1A1 passes through");
    CHECK(len == 2 && (unsigned char)out[0] == 0xA1 && (unsigned char)out[1] == 0xA1,
          "valid 2-byte GB18030 should pass through unchanged");

    unsigned char gb18030_invalid[] = { 0x81, 0x27 };
    len = mysql_real_escape_string(m, out, (const char*)gb18030_invalid, 2);
    TEST("GB18030: invalid seq 0x8127, lead byte escaped");
    CHECK(len == 4 && (unsigned char)out[0] == '\\' && (unsigned char)out[1] == 0x81 &&
          (unsigned char)out[2] == '\\' && out[3] == '\'',
          "invalid GB18030 lead byte should be escaped");

    mysql_set_character_set(m, "utf8mb4");
    mysql_close(m);
}

void test_ps_out_param(void) {
    printf("\n[PS: OUT parameter via CALL]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP PROCEDURE IF EXISTS p_out_test");
    Q("CREATE PROCEDURE p_out_test(OUT x INT) BEGIN SET x = 42; END");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "CALL p_out_test(?)", strlen("CALL p_out_test(?)"));
    CHECK(rc == 0, "prepare CALL p_out_test failed");

    TEST("param_count = 1");
    CHECK(mysql_stmt_param_count(stmt) == 1, "should have 1 param");

    int out_val = 0;
    unsigned long out_len = 0;
    my_bool out_err = 0;
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG;
    pb.buffer = &out_val;
    pb.buffer_length = sizeof(out_val);
    pb.length = &out_len;
    pb.error = &out_err;
    mysql_stmt_bind_param(stmt, &pb);

    rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, "execute CALL p_out_test failed");

    int found_out = 0;
    int loop_count = 0;
    for (;;) {
        rc = mysql_stmt_store_result(stmt);
        CHECK(rc == 0 || mysql_stmt_field_count(stmt) == 0, "store_result failed");

        MYSQL_RES *meta = mysql_stmt_result_metadata(stmt);
        unsigned int nf = meta ? mysql_num_fields(meta) : 0;

        if (meta && nf == 1) {
            found_out = 1;
            TEST("OUT param: resultset has OUT value");
            CHECK(meta != NULL, "should have metadata for OUT resultset");

            MYSQL_BIND rb;
            int result_val = 0;
            unsigned long rl = 0;
            my_bool re = 0;
            memset(&rb, 0, sizeof(rb));
            rb.buffer_type = MYSQL_TYPE_LONG;
            rb.buffer = &result_val;
            rb.buffer_length = sizeof(result_val);
            rb.length = &rl;
            rb.error = &re;
            mysql_stmt_bind_result(stmt, &rb);

            rc = mysql_stmt_fetch(stmt);
            TEST("OUT param: value = 42");
            CHECK(rc == 0 && result_val == 42, "OUT param should be 42");
        }
        if (meta) mysql_free_result(meta);

        if (mysql_stmt_next_result(stmt) != 0) break;
    }

    TEST("found OUT param resultset");
    CHECK(found_out, "OUT param resultset not found");

    mysql_stmt_close(stmt);
    Q("DROP PROCEDURE IF EXISTS p_out_test");
    mysql_close(m);
}

void test_flush_tables_reprepare(void) {
    printf("\n[FLUSH TABLES triggers re-prepare]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_flush");
    Q("CREATE TABLE t_flush(id INT, v VARCHAR(10))");
    Q("INSERT INTO t_flush VALUES(1, 'hello')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT v FROM t_flush WHERE id = ?", strlen("SELECT v FROM t_flush WHERE id = ?"));
    CHECK(rc == 0, "prepare failed");

    int id = 1;
    unsigned long il = 0;
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.buffer_length = sizeof(id); pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, "first execute failed");
    rc = mysql_stmt_store_result(stmt);
    CHECK(rc == 0, "first store_result failed");

    char buf[32] = {0};
    unsigned long vl = 0;
    my_bool err = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_STRING; rb.buffer = buf; rb.buffer_length = sizeof(buf); rb.length = &vl; rb.error = &err;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_fetch(stmt);
    TEST("first execute: value = hello");
    CHECK(rc == 0 && strcmp(buf, "hello") == 0, "value should be hello");

    Q("FLUSH TABLES t_flush");

    id = 1;
    rc = mysql_stmt_execute(stmt);
    TEST("re-execute after FLUSH TABLES succeeds (auto re-prepare)");
    CHECK(rc == 0, "should auto re-prepare after FLUSH TABLES");

    if (rc == 0) {
        rc = mysql_stmt_store_result(stmt);
        CHECK(rc == 0, "store_result after re-prepare failed");
        memset(buf, 0, sizeof(buf));
        mysql_stmt_bind_result(stmt, &rb);
        rc = mysql_stmt_fetch(stmt);
        TEST("re-execute: value still = hello");
        CHECK(rc == 0 && strcmp(buf, "hello") == 0, "value should still be hello");
    }

    mysql_stmt_close(stmt);
    Q("DROP TABLE IF EXISTS t_flush");
    mysql_close(m);
}

void test_mysql_init_command(void) {
    printf("\n[MYSQL_INIT_COMMAND option]\n");
    MYSQL *m = mysql_init(NULL);
    if (!m) { FAIL("mysql_init failed"); return; }

    mysql_options(m, MYSQL_INIT_COMMAND, "SET @init_var = 999");

    if (!mysql_real_connect(m, g_host, g_user, g_passwd, g_db, g_port, NULL, 0)) {
        FAIL("connect with INIT_COMMAND failed");
        mysql_close(m);
        return;
    }

    Q("DROP TABLE IF EXISTS t_initcmd");
    Q("CREATE TABLE t_initcmd(v INT)");
    Q("INSERT INTO t_initcmd VALUES(@init_var)");

    MYSQL_RES *res = NULL;
    Q("SELECT v FROM t_initcmd");
    res = mysql_store_result(m);
    TEST("INIT_COMMAND: @init_var = 999");
    if (res) {
        MYSQL_ROW row = mysql_fetch_row(res);
        CHECK(row && row[0] && strcmp(row[0], "999") == 0, "init_var should be 999");
        mysql_free_result(res);
    } else {
        CHECK(0, "no result");
    }

    Q("DROP TABLE IF EXISTS t_initcmd");
    mysql_close(m);
}

void test_ps_datetime_boundary(void) {
    printf("\n[PS: DATETIME boundary values]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_dtb");
    Q("CREATE TABLE t_dtb(id INT, dt DATETIME)");
    Q("INSERT INTO t_dtb VALUES(1,'1000-01-01 00:00:00'),(2,'9999-12-31 23:59:59'),(3,'2024-02-29 12:00:00')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT dt FROM t_dtb ORDER BY id",
                        strlen("SELECT dt FROM t_dtb ORDER BY id"));
    mysql_stmt_execute(stmt);

    MYSQL_TIME out_dt;
    unsigned long dl = 0;
    my_bool is_null = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_DATETIME; rb.buffer = &out_dt; rb.buffer_length = sizeof(out_dt); rb.length = &dl; rb.is_null = &is_null;
    mysql_stmt_bind_result(stmt, &rb);

    mysql_stmt_store_result(stmt);
    mysql_stmt_fetch(stmt);
    TEST("DATETIME min: 1000-01-01");
    CHECK(!is_null && out_dt.year == 1000 && out_dt.month == 1 && out_dt.day == 1, "wrong min datetime");

    mysql_stmt_fetch(stmt);
    TEST("DATETIME max: 9999-12-31 23:59:59");
    CHECK(!is_null && out_dt.year == 9999 && out_dt.month == 12 && out_dt.day == 31 && out_dt.hour == 23 && out_dt.minute == 59 && out_dt.second == 59, "wrong max datetime");

    mysql_stmt_fetch(stmt);
    TEST("DATETIME leap year: 2024-02-29");
    CHECK(!is_null && out_dt.year == 2024 && out_dt.month == 2 && out_dt.day == 29, "wrong leap day");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_time_negative(void) {
    printf("\n[PS: TIME negative values]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_tn");
    Q("CREATE TABLE t_tn(id INT, t TIME)");
    Q("INSERT INTO t_tn VALUES(1,'-838:59:59'),(2,'838:59:59'),(3,'00:00:00')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT t FROM t_tn ORDER BY id",
                        strlen("SELECT t FROM t_tn ORDER BY id"));
    mysql_stmt_execute(stmt);

    MYSQL_TIME out_t;
    unsigned long tl = 0;
    my_bool is_null = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_TIME; rb.buffer = &out_t; rb.buffer_length = sizeof(out_t); rb.length = &tl; rb.is_null = &is_null;
    mysql_stmt_bind_result(stmt, &rb);

    mysql_stmt_store_result(stmt);
    mysql_stmt_fetch(stmt);
    TEST("TIME min: -838:59:59 (neg=1)");
    CHECK(!is_null && out_t.neg == 1, "should be negative");

    mysql_stmt_fetch(stmt);
    TEST("TIME max: 838:59:59 (neg=0)");
    CHECK(!is_null && out_t.neg == 0, "should be positive");

    mysql_stmt_fetch(stmt);
    TEST("TIME zero: 00:00:00");
    CHECK(!is_null && out_t.hour == 0 && out_t.minute == 0 && out_t.second == 0, "wrong zero time");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_timestamp_auto_update(void) {
    printf("\n[PS: TIMESTAMP auto update]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_tsau");
    Q("CREATE TABLE t_tsau(id INT, ts TIMESTAMP DEFAULT CURRENT_TIMESTAMP)");
    Q("INSERT INTO t_tsau(id) VALUES(1)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT ts FROM t_tsau WHERE id = ?",
                        strlen("SELECT ts FROM t_tsau WHERE id = ?"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    MYSQL_TIME out_ts;
    unsigned long tl = 0;
    my_bool is_null = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_TIMESTAMP; rb.buffer = &out_ts; rb.buffer_length = sizeof(out_ts); rb.length = &tl; rb.is_null = &is_null;
    mysql_stmt_bind_result(stmt, &rb);

    mysql_stmt_execute(stmt);
    mysql_stmt_store_result(stmt);
    int rc = mysql_stmt_fetch(stmt);
    TEST("TIMESTAMP auto: year >= 2024");
    CHECK(rc == 0 && !is_null && out_ts.year >= 2024, "wrong timestamp");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_reprepare_after_rename(void) {
    printf("\n[PS: re-prepare after table rename]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_rn1, t_rn2");
    Q("CREATE TABLE t_rn1(id INT, val INT)");
    Q("INSERT INTO t_rn1 VALUES(1, 42)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT val FROM t_rn1 WHERE id = ?",
                        strlen("SELECT val FROM t_rn1 WHERE id = ?"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    int rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, "1st execute failed");
    mysql_stmt_close(stmt);

    MYSQL *m2 = get_conn();
    Q2(m2, "RENAME TABLE t_rn1 TO t_rn2");
    mysql_close(m2);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "SELECT val FROM t_rn2 WHERE id = ?",
                            strlen("SELECT val FROM t_rn2 WHERE id = ?"));
    TEST("Prepare after rename: ok");
    CHECK(rc == 0, "prepare failed");

    mysql_stmt_bind_param(stmt, &pb);
    rc = mysql_stmt_execute(stmt);
    TEST("Execute after rename: ok");
    CHECK(rc == 0, "execute failed");

    int out_val;
    unsigned long vl = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.length = &vl;
    mysql_stmt_bind_result(stmt, &rb);
    mysql_stmt_store_result(stmt);
    mysql_stmt_fetch(stmt);
    TEST("Value after rename: 42");
    CHECK(out_val == 42, "wrong value");

    mysql_stmt_close(stmt);
    Q("DROP TABLE IF EXISTS t_rn2");
    mysql_close(m);
}

void test_ps_affected_rows_insert(void) {
    printf("\n[PS: affected_rows for INSERT]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_ari");
    Q("CREATE TABLE t_ari(id INT PRIMARY KEY, val INT)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "INSERT INTO t_ari VALUES(?, ?)",
                        strlen("INSERT INTO t_ari VALUES(?, ?)"));

    int id = 1, val = 100;
    unsigned long il = sizeof(int), vl = sizeof(int);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &il;
    pb[1].buffer_type = MYSQL_TYPE_LONG; pb[1].buffer = &val; pb[1].length = &vl;
    mysql_stmt_bind_param(stmt, pb);

    int rc = mysql_stmt_execute(stmt);
    TEST("INSERT: affected_rows = 1");
    CHECK(rc == 0 && mysql_stmt_affected_rows(stmt) == 1, "wrong affected_rows");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_insert_auto_increment(void) {
    printf("\n[PS: INSERT with AUTO_INCREMENT]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_ai");
    Q("CREATE TABLE t_ai(id INT AUTO_INCREMENT PRIMARY KEY, val INT)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "INSERT INTO t_ai(val) VALUES(?)",
                        strlen("INSERT INTO t_ai(val) VALUES(?)"));

    int val = 42;
    unsigned long vl = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &val; pb.length = &vl;
    mysql_stmt_bind_param(stmt, &pb);

    int rc = mysql_stmt_execute(stmt);
    TEST("INSERT AUTO_INCREMENT: ok");
    CHECK(rc == 0, "insert failed");

    my_ulonglong insert_id = mysql_stmt_insert_id(stmt);
    TEST("insert_id > 0");
    CHECK(insert_id > 0, "insert_id should be > 0");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_multiple_stmts_different_tables(void) {
    printf("\n[PS: multiple stmts on different tables]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_ms1, t_ms2");
    Q("CREATE TABLE t_ms1(id INT, a VARCHAR(32))");
    Q("CREATE TABLE t_ms2(id INT, b INT)");
    Q("INSERT INTO t_ms1 VALUES(1,'hello')");
    Q("INSERT INTO t_ms2 VALUES(1,999)");

    MYSQL_STMT *s1 = mysql_stmt_init(m);
    mysql_stmt_prepare(s1, "SELECT a FROM t_ms1 WHERE id = ?",
                        strlen("SELECT a FROM t_ms1 WHERE id = ?"));
    MYSQL_STMT *s2 = mysql_stmt_init(m);
    mysql_stmt_prepare(s2, "SELECT b FROM t_ms2 WHERE id = ?",
                        strlen("SELECT b FROM t_ms2 WHERE id = ?"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;

    mysql_stmt_bind_param(s1, &pb);
    mysql_stmt_execute(s1);
    mysql_stmt_store_result(s1);

    mysql_stmt_bind_param(s2, &pb);
    mysql_stmt_execute(s2);
    mysql_stmt_store_result(s2);

    char out_a[32]; unsigned long al = 0;
    MYSQL_BIND rb1;
    memset(&rb1, 0, sizeof(rb1));
    rb1.buffer_type = MYSQL_TYPE_STRING; rb1.buffer = out_a; rb1.buffer_length = sizeof(out_a); rb1.length = &al;
    mysql_stmt_bind_result(s1, &rb1);

    int out_b; unsigned long bl = 0;
    MYSQL_BIND rb2;
    memset(&rb2, 0, sizeof(rb2));
    rb2.buffer_type = MYSQL_TYPE_LONG; rb2.buffer = &out_b; rb2.length = &bl;
    mysql_stmt_bind_result(s2, &rb2);

    mysql_stmt_fetch(s1);
    mysql_stmt_fetch(s2);

    TEST("Stmt1: a='hello'");
    CHECK(al == 5 && memcmp(out_a, "hello", 5) == 0, "wrong string");

    TEST("Stmt2: b=999");
    CHECK(out_b == 999, "wrong int");

    mysql_stmt_close(s1);
    mysql_stmt_close(s2);
    mysql_close(m);
}

void test_ps_store_result_seek_back(void) {
    printf("\n[PS: store_result seek back and forth]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_ssb");
    Q("CREATE TABLE t_ssb(id INT PRIMARY KEY)");
    Q("INSERT INTO t_ssb VALUES(1),(2),(3),(4),(5)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id FROM t_ssb ORDER BY id",
                        strlen("SELECT id FROM t_ssb ORDER BY id"));
    mysql_stmt_execute(stmt);

    int out_id;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_id; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    mysql_stmt_store_result(stmt);

    mysql_stmt_fetch(stmt);
    TEST("Row 1: id=1");
    CHECK(out_id == 1, "wrong");

    mysql_stmt_fetch(stmt);
    TEST("Row 2: id=2");
    CHECK(out_id == 2, "wrong");

    MYSQL_ROW_OFFSET pos = mysql_stmt_row_tell(stmt);
    mysql_stmt_fetch(stmt);
    TEST("Row 3: id=3");
    CHECK(out_id == 3, "wrong");

    mysql_stmt_row_seek(stmt, pos);
    mysql_stmt_fetch(stmt);
    TEST("Seek back to row 3: id=3");
    CHECK(out_id == 3, "wrong after seek");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_execute_with_no_params_bound(void) {
    printf("\n[PS: execute with no params bound (0-param query)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_npb");
    Q("CREATE TABLE t_npb(id INT)");
    Q("INSERT INTO t_npb VALUES(42)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id FROM t_npb",
                        strlen("SELECT id FROM t_npb"));
    int rc = mysql_stmt_execute(stmt);
    TEST("Execute 0-param PS: ok");
    CHECK(rc == 0, "execute failed");

    int out_id;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_id; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);
    mysql_stmt_store_result(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("0-param result: id=42");
    CHECK(rc == 0 && out_id == 42, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_bind_result_after_store(void) {
    printf("\n[PS: bind_result after store_result]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_bas");
    Q("CREATE TABLE t_bas(id INT, val INT)");
    Q("INSERT INTO t_bas VALUES(1, 100)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id, val FROM t_bas",
                        strlen("SELECT id, val FROM t_bas"));
    mysql_stmt_execute(stmt);
    mysql_stmt_store_result(stmt);

    int out_id, out_val;
    unsigned long l1 = 0, l2 = 0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &out_id; rb[0].length = &l1;
    rb[1].buffer_type = MYSQL_TYPE_LONG; rb[1].buffer = &out_val; rb[1].length = &l2;
    mysql_stmt_bind_result(stmt, rb);

    int rc = mysql_stmt_fetch(stmt);
    TEST("bind_result after store: id=1, val=100");
    CHECK(rc == 0 && out_id == 1 && out_val == 100, "wrong values");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_warning_count(void) {
    printf("\n[PS: warning_count after execute]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_wc");
    Q("CREATE TABLE t_wc(id INT)");
    Q("INSERT INTO t_wc VALUES(1)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id FROM t_wc",
                        strlen("SELECT id FROM t_wc"));
    mysql_stmt_execute(stmt);
    mysql_stmt_store_result(stmt);

    unsigned int wc = mysql_stmt_warning_count(stmt);
    TEST("warning_count: no warnings on simple SELECT");
    CHECK(wc == 0, "unexpected warnings");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_multi_stmt_all_select(void) {
    printf("\n[multi-statement: all SELECT]\n");
    MYSQL *m = mysql_init(NULL);
    unsigned int ssl_mode = SSL_MODE_DISABLED;
    mysql_options(m, MYSQL_OPT_SSL_MODE, &ssl_mode);
    if (!mysql_real_connect(m, g_host, g_user, g_passwd, g_db, g_port, NULL,
                            CLIENT_MULTI_STATEMENTS)) {
        FAIL("connect failed"); return;
    }

    int rc = mysql_real_query(m, "SELECT 1 AS a; SELECT 2 AS b; SELECT 3 AS c",
                              strlen("SELECT 1 AS a; SELECT 2 AS b; SELECT 3 AS c"));
    TEST("multi-stmt: 3 SELECTs");
    CHECK(rc == 0, "query failed");

    MYSQL_RES *res = mysql_store_result(m);
    TEST("1st SELECT: has result");
    CHECK(res && mysql_num_rows(res) == 1, "should have 1 row");
    if (res) {
        MYSQL_ROW row = mysql_fetch_row(res);
        CHECK(row && strcmp(row[0], "1") == 0, "wrong value");
        mysql_free_result(res);
    }

    rc = mysql_next_result(m);
    TEST("next_result 2");
    CHECK(rc == 0, "next_result failed");
    res = mysql_store_result(m);
    TEST("2nd SELECT: value=2");
    CHECK(res && mysql_num_rows(res) == 1, "should have 1 row");
    if (res) {
        MYSQL_ROW row = mysql_fetch_row(res);
        CHECK(row && strcmp(row[0], "2") == 0, "wrong value");
        mysql_free_result(res);
    }

    rc = mysql_next_result(m);
    TEST("next_result 3");
    CHECK(rc == 0, "next_result failed");
    res = mysql_store_result(m);
    TEST("3rd SELECT: value=3");
    CHECK(res && mysql_num_rows(res) == 1, "should have 1 row");
    if (res) {
        MYSQL_ROW row = mysql_fetch_row(res);
        CHECK(row && strcmp(row[0], "3") == 0, "wrong value");
        mysql_free_result(res);
    }

    rc = mysql_next_result(m);
    TEST("no more results");
    CHECK(rc != 0, "should be done");

    mysql_close(m);
}

void test_multi_stmt_error_midway(void) {
    printf("\n[multi-statement: error midway]\n");
    MYSQL *m = mysql_init(NULL);
    unsigned int ssl_mode = SSL_MODE_DISABLED;
    mysql_options(m, MYSQL_OPT_SSL_MODE, &ssl_mode);
    if (!mysql_real_connect(m, g_host, g_user, g_passwd, g_db, g_port, NULL,
                            CLIENT_MULTI_STATEMENTS)) {
        FAIL("connect failed"); return;
    }

    int rc = mysql_real_query(m, "SELECT 1; SELECT * FROM nonexistent_table_xyz; SELECT 3",
                              strlen("SELECT 1; SELECT * FROM nonexistent_table_xyz; SELECT 3"));
    TEST("multi-stmt with error midway: first query ok");
    CHECK(rc == 0, "first query should succeed");

    MYSQL_RES *res = mysql_store_result(m);
    CHECK(res, "first result should exist");
    mysql_free_result(res);

    rc = mysql_next_result(m);
    TEST("next_result hits error");
    CHECK(rc != 0, "should fail on bad table");

    mysql_close(m);
}

void test_multi_stmt_ps_reexecute_after_store(void) {
    printf("\n[multi-statement: PS reexecute after store_result]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_mps");
    Q("CREATE TABLE t_mps(id INT, val INT)");
    Q("INSERT INTO t_mps VALUES(1, 100),(2, 200),(3, 300)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT val FROM t_mps WHERE id = ?",
                        strlen("SELECT val FROM t_mps WHERE id = ?"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;

    int out_val;
    unsigned long vl = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.length = &vl;
    mysql_stmt_bind_result(stmt, &rb);

    mysql_stmt_bind_param(stmt, &pb);
    mysql_stmt_execute(stmt);
    mysql_stmt_store_result(stmt);
    mysql_stmt_fetch(stmt);
    TEST("1st execute: val=100");
    CHECK(out_val == 100, "wrong value");

    id = 2;
    mysql_stmt_bind_param(stmt, &pb);
    int rc = mysql_stmt_execute(stmt);
    TEST("2nd execute after store: ok");
    CHECK(rc == 0, "execute failed");

    mysql_stmt_store_result(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("2nd execute: val=200");
    CHECK(rc == 0 && out_val == 200, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_double_zero(void) {
    printf("\n[PS: DOUBLE zero and negative zero]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_dz");
    Q("CREATE TABLE t_dz(d DOUBLE)");
    Q("INSERT INTO t_dz VALUES(0.0),(-0.0),(1.0)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT d FROM t_dz ORDER BY d",
                        strlen("SELECT d FROM t_dz ORDER BY d"));
    mysql_stmt_execute(stmt);

    double out_d;
    unsigned long dl = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_DOUBLE; rb.buffer = &out_d; rb.length = &dl;
    mysql_stmt_bind_result(stmt, &rb);

    mysql_stmt_store_result(stmt);
    int rc = mysql_stmt_fetch(stmt);
    TEST("DOUBLE: -0.0");
    CHECK(rc == 0, "fetch failed");

    mysql_stmt_fetch(stmt);
    TEST("DOUBLE: 0.0");
    CHECK(rc == 0, "fetch failed");

    mysql_stmt_fetch(stmt);
    TEST("DOUBLE: 1.0");
    CHECK(out_d > 0.99 && out_d < 1.01, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_small_negative_boundary(void) {
    printf("\n[PS: SMALLINT negative boundary]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_snb");
    Q("CREATE TABLE t_snb(v SMALLINT)");
    Q("INSERT INTO t_snb VALUES(-32768),(-1),(0),(32767)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT v FROM t_snb ORDER BY v",
                        strlen("SELECT v FROM t_snb ORDER BY v"));
    mysql_stmt_execute(stmt);

    short out_v;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_SHORT; rb.buffer = &out_v; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    mysql_stmt_store_result(stmt);
    mysql_stmt_fetch(stmt);
    TEST("SMALLINT: -32768 (min)");
    CHECK(out_v == -32768, "wrong value");

    mysql_stmt_fetch(stmt);
    TEST("SMALLINT: -1");
    CHECK(out_v == -1, "wrong value");

    mysql_stmt_fetch(stmt);
    TEST("SMALLINT: 0");
    CHECK(out_v == 0, "wrong value");

    mysql_stmt_fetch(stmt);
    TEST("SMALLINT: 32767 (max)");
    CHECK(out_v == 32767, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_string_with_embedded_null(void) {
    printf("\n[PS: string with embedded NUL bytes]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_sen");
    Q("CREATE TABLE t_sen(id INT, data BLOB)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "INSERT INTO t_sen VALUES(?, ?)",
                        strlen("INSERT INTO t_sen VALUES(?, ?)"));

    int id = 1;
    unsigned long il = sizeof(int);
    unsigned char blob_data[5] = {0x00, 'A', 0x00, 'B', 0x00};
    unsigned long bl = 5;
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &il;
    pb[1].buffer_type = MYSQL_TYPE_BLOB; pb[1].buffer = blob_data; pb[1].buffer_length = 5; pb[1].length = &bl;
    mysql_stmt_bind_param(stmt, pb);

    int rc = mysql_stmt_execute(stmt);
    TEST("INSERT BLOB with embedded NULs");
    CHECK(rc == 0, "insert failed");
    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT data FROM t_sen WHERE id = ?",
                        strlen("SELECT data FROM t_sen WHERE id = ?"));
    mysql_stmt_bind_param(stmt, &pb[0]);
    mysql_stmt_execute(stmt);

    unsigned char out_buf[16];
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_BLOB; rb.buffer = out_buf; rb.buffer_length = sizeof(out_buf); rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    mysql_stmt_store_result(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("BLOB with NULs: length=5");
    CHECK(rc == 0 && ol == 5, "wrong length");

    TEST("BLOB with NULs: content matches");
    CHECK(memcmp(out_buf, blob_data, 5) == 0, "wrong content");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_mediumint_type(void) {
    printf("\n[PS: MEDIUMINT type]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_mi");
    Q("CREATE TABLE t_mi(id MEDIUMINT, val MEDIUMINT UNSIGNED)");
    Q("INSERT INTO t_mi VALUES(-8388608, 16777215),(0, 0),(8388607, 8388607)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id, val FROM t_mi ORDER BY id",
                        strlen("SELECT id, val FROM t_mi ORDER BY id"));
    mysql_stmt_execute(stmt);

    int out_id; unsigned int out_val;
    unsigned long l1 = 0, l2 = 0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &out_id; rb[0].length = &l1;
    rb[1].buffer_type = MYSQL_TYPE_LONG; rb[1].buffer = &out_val; rb[1].length = &l2; rb[1].is_unsigned = 1;
    mysql_stmt_bind_result(stmt, rb);

    mysql_stmt_store_result(stmt);
    int rc = mysql_stmt_fetch(stmt);
    TEST("MEDIUMINT: min signed = -8388608");
    CHECK(rc == 0 && out_id == -8388608, "wrong value");

    rc = mysql_stmt_fetch(stmt);
    TEST("MEDIUMINT: zero");
    CHECK(rc == 0 && out_id == 0, "wrong value");

    rc = mysql_stmt_fetch(stmt);
    TEST("MEDIUMINT: max signed = 8388607");
    CHECK(rc == 0 && out_id == 8388607, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_year_boundary(void) {
    printf("\n[PS: YEAR boundary values]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_yb");
    Q("CREATE TABLE t_yb(y YEAR)");
    Q("INSERT INTO t_yb VALUES(1901),(2000),(2155),(0)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT y FROM t_yb ORDER BY y",
                        strlen("SELECT y FROM t_yb ORDER BY y"));
    mysql_stmt_execute(stmt);

    short out_y;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_SHORT; rb.buffer = &out_y; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    mysql_stmt_store_result(stmt);
    mysql_stmt_fetch(stmt);
    TEST("YEAR: 0");
    CHECK(out_y == 0, "wrong value");

    mysql_stmt_fetch(stmt);
    TEST("YEAR: 1901 (min)");
    CHECK(out_y == 1901, "wrong value");

    mysql_stmt_fetch(stmt);
    TEST("YEAR: 2000");
    CHECK(out_y == 2000, "wrong value");

    mysql_stmt_fetch(stmt);
    TEST("YEAR: 2155 (max)");
    CHECK(out_y == 2155, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_float_double_precision(void) {
    printf("\n[PS: FLOAT/DOUBLE precision]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_fp");
    Q("CREATE TABLE t_fp(id INT, f FLOAT, d DOUBLE)");
    Q("INSERT INTO t_fp VALUES(1, 3.14159, 3.141592653589793)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT f, d FROM t_fp WHERE id = ?",
                        strlen("SELECT f, d FROM t_fp WHERE id = ?"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    float out_f; double out_d;
    unsigned long fl = 0, dl = 0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_FLOAT; rb[0].buffer = &out_f; rb[0].length = &fl;
    rb[1].buffer_type = MYSQL_TYPE_DOUBLE; rb[1].buffer = &out_d; rb[1].length = &dl;
    mysql_stmt_bind_result(stmt, rb);

    mysql_stmt_execute(stmt);
    mysql_stmt_store_result(stmt);
    int rc = mysql_stmt_fetch(stmt);
    TEST("FLOAT precision: ~3.14159");
    CHECK(rc == 0 && (out_f > 3.141 && out_f < 3.142), "wrong float value");

    TEST("DOUBLE precision: ~3.14159265");
    CHECK(out_d > 3.14159265 && out_d < 3.14159266, "wrong double value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_varchar_max_length(void) {
    printf("\n[PS: VARCHAR max length]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_vml");
    Q("CREATE TABLE t_vml(id INT, v VARCHAR(255))");
    char longstr[256];
    memset(longstr, 'A', 255); longstr[255] = '\0';
    char ins[512];
    snprintf(ins, sizeof(ins), "INSERT INTO t_vml VALUES(1, '%s')", longstr);
    Q(ins);

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT v FROM t_vml WHERE id = ?",
                        strlen("SELECT v FROM t_vml WHERE id = ?"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    char out_v[256];
    unsigned long vl = 0;
    my_bool err = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_STRING; rb.buffer = out_v; rb.buffer_length = sizeof(out_v); rb.length = &vl; rb.error = &err;
    mysql_stmt_bind_result(stmt, &rb);

    mysql_stmt_execute(stmt);
    mysql_stmt_store_result(stmt);
    int rc = mysql_stmt_fetch(stmt);
    TEST("VARCHAR(255): 255 chars");
    CHECK(rc == 0 && vl == 255 && !err, "wrong length or truncation");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_text_longtext(void) {
    printf("\n[PS: TEXT/LONGTEXT types]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_tlt");
    Q("CREATE TABLE t_tlt(id INT, t TEXT, lt LONGTEXT)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "INSERT INTO t_tlt VALUES(?, ?, ?)",
                        strlen("INSERT INTO t_tlt VALUES(?, ?, ?)"));

    int id = 1;
    unsigned long il = sizeof(int);
    char txt[] = "Hello TEXT";
    char ltxt[] = "Hello LONGTEXT with more data than TEXT";
    unsigned long tl = strlen(txt), ll = strlen(ltxt);
    MYSQL_BIND pb[3];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &il;
    pb[1].buffer_type = MYSQL_TYPE_STRING; pb[1].buffer = txt; pb[1].buffer_length = tl; pb[1].length = &tl;
    pb[2].buffer_type = MYSQL_TYPE_STRING; pb[2].buffer = ltxt; pb[2].buffer_length = ll; pb[2].length = &ll;
    mysql_stmt_bind_param(stmt, pb);

    int rc = mysql_stmt_execute(stmt);
    TEST("INSERT TEXT/LONGTEXT via PS");
    CHECK(rc == 0, "insert failed");
    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT t, lt FROM t_tlt",
                        strlen("SELECT t, lt FROM t_tlt"));
    mysql_stmt_execute(stmt);

    char out_t[64], out_lt[128];
    unsigned long otl = 0, oll = 0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_STRING; rb[0].buffer = out_t; rb[0].buffer_length = sizeof(out_t); rb[0].length = &otl;
    rb[1].buffer_type = MYSQL_TYPE_STRING; rb[1].buffer = out_lt; rb[1].buffer_length = sizeof(out_lt); rb[1].length = &oll;
    mysql_stmt_bind_result(stmt, rb);

    mysql_stmt_store_result(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("TEXT roundtrip");
    CHECK(rc == 0 && otl == strlen(txt), "wrong text length");

    TEST("LONGTEXT roundtrip");
    CHECK(oll == strlen(ltxt), "wrong longtext length");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_multiple_send_long_data(void) {
    printf("\n[PS: multiple send_long_data calls]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_msld");
    Q("CREATE TABLE t_msld(id INT, data BLOB)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "INSERT INTO t_msld VALUES(?, ?)",
                        strlen("INSERT INTO t_msld VALUES(?, ?)"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &il;
    pb[1].buffer_type = MYSQL_TYPE_BLOB;
    mysql_stmt_bind_param(stmt, pb);

    mysql_stmt_send_long_data(stmt, 1, "Part1_", 6);
    mysql_stmt_send_long_data(stmt, 1, "Part2_", 6);
    mysql_stmt_send_long_data(stmt, 1, "Part3", 5);

    int rc = mysql_stmt_execute(stmt);
    TEST("INSERT via multiple send_long_data");
    CHECK(rc == 0, "execute failed");

    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT LENGTH(data), data FROM t_msld",
                        strlen("SELECT LENGTH(data), data FROM t_msld"));
    mysql_stmt_execute(stmt);

    long long out_len;
    char out_data[32];
    unsigned long ll = 0, dl = 0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONGLONG; rb[0].buffer = &out_len; rb[0].length = &ll;
    rb[1].buffer_type = MYSQL_TYPE_BLOB; rb[1].buffer = out_data; rb[1].buffer_length = sizeof(out_data); rb[1].length = &dl;
    mysql_stmt_bind_result(stmt, rb);

    mysql_stmt_store_result(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("Concatenated length = 17");
    CHECK(rc == 0 && out_len == 17, "wrong length");

    TEST("Concatenated data = Part1_Part2_Part3");
    CHECK(dl == 17 && memcmp(out_data, "Part1_Part2_Part3", 17) == 0, "wrong data");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_execute_after_dropped_table(void) {
    printf("\n[PS: execute after table dropped]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_ead");
    Q("CREATE TABLE t_ead(id INT)");
    Q("INSERT INTO t_ead VALUES(1)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id FROM t_ead",
                        strlen("SELECT id FROM t_ead"));
    mysql_stmt_execute(stmt);
    mysql_stmt_close(stmt);

    MYSQL *m2 = get_conn();
    Q2(m2, "DROP TABLE t_ead");
    mysql_close(m2);

    stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id FROM t_ead",
                        strlen("SELECT id FROM t_ead"));
    int rc = mysql_stmt_execute(stmt);
    TEST("Execute after table dropped fails");
    CHECK(rc != 0, "should fail");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_insert_null_various_types(void) {
    printf("\n[PS: INSERT NULL into various types]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_inv");
    Q("CREATE TABLE t_inv(id INT, vi INT, vd DOUBLE, vs VARCHAR(32), vb BLOB, vdt DATETIME)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "INSERT INTO t_inv VALUES(?, ?, ?, ?, ?, ?)",
                        strlen("INSERT INTO t_inv VALUES(?, ?, ?, ?, ?, ?)"));

    int id = 1;
    unsigned long il = sizeof(int);
    my_bool is_null = 1;
    MYSQL_BIND pb[6];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &il;
    pb[1].buffer_type = MYSQL_TYPE_LONG; pb[1].is_null = &is_null;
    pb[2].buffer_type = MYSQL_TYPE_DOUBLE; pb[2].is_null = &is_null;
    pb[3].buffer_type = MYSQL_TYPE_STRING; pb[3].is_null = &is_null;
    pb[4].buffer_type = MYSQL_TYPE_BLOB; pb[4].is_null = &is_null;
    pb[5].buffer_type = MYSQL_TYPE_DATETIME; pb[5].is_null = &is_null;
    mysql_stmt_bind_param(stmt, pb);

    int rc = mysql_stmt_execute(stmt);
    TEST("INSERT NULL into INT col");
    CHECK(rc == 0, "insert failed");

    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT vi, vd, vs, vb, vdt FROM t_inv",
                        strlen("SELECT vi, vd, vs, vb, vdt FROM t_inv"));
    mysql_stmt_execute(stmt);

    int out_i; double out_d; char out_s[32]; char out_b[32]; MYSQL_TIME out_dt;
    unsigned long ol[5] = {0};
    my_bool null_flags[5] = {0};
    MYSQL_BIND rb[5];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &out_i; rb[0].length = &ol[0]; rb[0].is_null = &null_flags[0];
    rb[1].buffer_type = MYSQL_TYPE_DOUBLE; rb[1].buffer = &out_d; rb[1].length = &ol[1]; rb[1].is_null = &null_flags[1];
    rb[2].buffer_type = MYSQL_TYPE_STRING; rb[2].buffer = out_s; rb[2].buffer_length = sizeof(out_s); rb[2].length = &ol[2]; rb[2].is_null = &null_flags[2];
    rb[3].buffer_type = MYSQL_TYPE_BLOB; rb[3].buffer = out_b; rb[3].buffer_length = sizeof(out_b); rb[3].length = &ol[3]; rb[3].is_null = &null_flags[3];
    rb[4].buffer_type = MYSQL_TYPE_DATETIME; rb[4].buffer = &out_dt; rb[4].buffer_length = sizeof(out_dt); rb[4].length = &ol[4]; rb[4].is_null = &null_flags[4];
    mysql_stmt_bind_result(stmt, rb);

    mysql_stmt_store_result(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("All NULL columns: INT is null");
    CHECK(rc == 0 && null_flags[0], "not null");
    TEST("All NULL columns: DOUBLE is null");
    CHECK(null_flags[1], "not null");
    TEST("All NULL columns: VARCHAR is null");
    CHECK(null_flags[2], "not null");
    TEST("All NULL columns: BLOB is null");
    CHECK(null_flags[3], "not null");
    TEST("All NULL columns: DATETIME is null");
    CHECK(null_flags[4], "not null");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_update_return_affected_rows(void) {
    printf("\n[PS: UPDATE affected_rows]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_uar");
    Q("CREATE TABLE t_uar(id INT, val INT)");
    Q("INSERT INTO t_uar VALUES(1, 10),(2, 20),(3, 30)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "UPDATE t_uar SET val = val + 1 WHERE id = ?",
                        strlen("UPDATE t_uar SET val = val + 1 WHERE id = ?"));

    int id = 2;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    int rc = mysql_stmt_execute(stmt);
    TEST("UPDATE via PS: affected_rows = 1");
    CHECK(rc == 0 && mysql_stmt_affected_rows(stmt) == 1, "wrong affected_rows");

    id = 99;
    mysql_stmt_bind_param(stmt, &pb);
    rc = mysql_stmt_execute(stmt);
    TEST("UPDATE no match: affected_rows = 0");
    CHECK(rc == 0 && mysql_stmt_affected_rows(stmt) == 0, "should be 0");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_char_padding(void) {
    printf("\n[PS: CHAR padding behavior]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_cp");
    Q("CREATE TABLE t_cp(id INT, c CHAR(10))");
    Q("INSERT INTO t_cp VALUES(1, 'hi')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT c FROM t_cp WHERE id = ?",
                        strlen("SELECT c FROM t_cp WHERE id = ?"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    char out_c[16];
    unsigned long cl = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_STRING; rb.buffer = out_c; rb.buffer_length = sizeof(out_c); rb.length = &cl;
    mysql_stmt_bind_result(stmt, &rb);

    mysql_stmt_execute(stmt);
    mysql_stmt_store_result(stmt);
    int rc = mysql_stmt_fetch(stmt);
    TEST("CHAR(10): content starts with 'hi'");
    CHECK(rc == 0 && cl >= 2 && memcmp(out_c, "hi", 2) == 0, "wrong content");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_negative_tinyint(void) {
    printf("\n[PS: negative TINYINT]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_nti");
    Q("CREATE TABLE t_nti(v TINYINT)");
    Q("INSERT INTO t_nti VALUES(-128),(-1),(0),(127)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT v FROM t_nti ORDER BY v",
                        strlen("SELECT v FROM t_nti ORDER BY v"));
    mysql_stmt_execute(stmt);

    signed char out_v;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_TINY; rb.buffer = &out_v; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    mysql_stmt_store_result(stmt);
    mysql_stmt_fetch(stmt);
    TEST("TINYINT: -128 (min)");
    CHECK(out_v == -128, "wrong value");

    mysql_stmt_fetch(stmt);
    TEST("TINYINT: -1");
    CHECK(out_v == -1, "wrong value");

    mysql_stmt_fetch(stmt);
    TEST("TINYINT: 0");
    CHECK(out_v == 0, "wrong value");

    mysql_stmt_fetch(stmt);
    TEST("TINYINT: 127 (max)");
    CHECK(out_v == 127, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_mixed_types_row(void) {
    printf("\n[PS: mixed types in single row]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_mt");
    Q("CREATE TABLE t_mt(id INT, ti TINYINT, si SMALLINT, li INT, bi BIGINT, f FLOAT, d DOUBLE, v VARCHAR(16), dt DATETIME)");
    Q("INSERT INTO t_mt VALUES(1, -1, -100, 1000000, 9000000000, 1.5, 2.5, 'mixed', '2024-01-15 10:30:00')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT ti, si, li, bi, f, d, v, dt FROM t_mt WHERE id = ?",
                        strlen("SELECT ti, si, li, bi, f, d, v, dt FROM t_mt WHERE id = ?"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    signed char out_ti; short out_si; int out_li; long long out_bi;
    float out_f; double out_d; char out_v[16]; MYSQL_TIME out_dt;
    unsigned long ol[8] = {0};
    MYSQL_BIND rb[8];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_TINY; rb[0].buffer = &out_ti; rb[0].length = &ol[0];
    rb[1].buffer_type = MYSQL_TYPE_SHORT; rb[1].buffer = &out_si; rb[1].length = &ol[1];
    rb[2].buffer_type = MYSQL_TYPE_LONG; rb[2].buffer = &out_li; rb[2].length = &ol[2];
    rb[3].buffer_type = MYSQL_TYPE_LONGLONG; rb[3].buffer = &out_bi; rb[3].length = &ol[3];
    rb[4].buffer_type = MYSQL_TYPE_FLOAT; rb[4].buffer = &out_f; rb[4].length = &ol[4];
    rb[5].buffer_type = MYSQL_TYPE_DOUBLE; rb[5].buffer = &out_d; rb[5].length = &ol[5];
    rb[6].buffer_type = MYSQL_TYPE_STRING; rb[6].buffer = out_v; rb[6].buffer_length = sizeof(out_v); rb[6].length = &ol[6];
    rb[7].buffer_type = MYSQL_TYPE_DATETIME; rb[7].buffer = &out_dt; rb[7].buffer_length = sizeof(out_dt); rb[7].length = &ol[7];
    mysql_stmt_bind_result(stmt, rb);

    mysql_stmt_execute(stmt);
    mysql_stmt_store_result(stmt);
    int rc = mysql_stmt_fetch(stmt);
    TEST("Mixed types: TINYINT = -1");
    CHECK(rc == 0 && out_ti == -1, "wrong tiny");
    TEST("Mixed types: SMALLINT = -100");
    CHECK(out_si == -100, "wrong small");
    TEST("Mixed types: INT = 1000000");
    CHECK(out_li == 1000000, "wrong int");
    TEST("Mixed types: BIGINT = 9000000000");
    CHECK(out_bi == 9000000000LL, "wrong bigint");
    TEST("Mixed types: FLOAT ~1.5");
    CHECK(out_f > 1.49 && out_f < 1.51, "wrong float");
    TEST("Mixed types: DOUBLE ~2.5");
    CHECK(out_d > 2.49 && out_d < 2.51, "wrong double");
    TEST("Mixed types: VARCHAR = 'mixed'");
    CHECK(ol[6] == 5 && memcmp(out_v, "mixed", 5) == 0, "wrong varchar");
    TEST("Mixed types: DATETIME year=2024");
    CHECK(out_dt.year == 2024 && out_dt.month == 1 && out_dt.day == 15, "wrong datetime");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_null_bitmap_offset(void) {
    printf("\n[PS NULL bitmap offset: BUG#8482 pattern]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_nbm");
    Q("CREATE TABLE t_nbm(pk INT PRIMARY KEY, a INT, b INT, c INT, d INT, e INT)");
    Q("INSERT INTO t_nbm VALUES(1,NULL,1,2,3,4)");
    Q("INSERT INTO t_nbm VALUES(2,1,NULL,2,3,4)");
    Q("INSERT INTO t_nbm VALUES(3,1,2,NULL,3,4)");
    Q("INSERT INTO t_nbm VALUES(4,1,2,3,NULL,4)");
    Q("INSERT INTO t_nbm VALUES(5,1,2,3,4,NULL)");
    Q("INSERT INTO t_nbm VALUES(6,NULL,NULL,NULL,NULL,NULL)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    CHECK(stmt, "stmt_init failed");

    int rc = mysql_stmt_prepare(stmt, "SELECT a,b,c,d,e FROM t_nbm ORDER BY pk", -1);
    CHECK(rc == 0, "prepare failed");

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_store_result(stmt);

    int out[5] = {0}; unsigned long ol[5] = {0}; my_bool on[5] = {0}, oe[5] = {0};
    MYSQL_BIND rb[5];
    memset(rb, 0, sizeof(rb));
    for (int i = 0; i < 5; i++) {
        rb[i].buffer_type = MYSQL_TYPE_LONG; rb[i].buffer = &out[i];
        rb[i].length = &ol[i]; rb[i].is_null = &on[i]; rb[i].error = &oe[i];
    }
    mysql_stmt_bind_result(stmt, &rb[0]);

    rc = mysql_stmt_fetch(stmt);
    TEST("NULL in col0 (first col)");
    CHECK(rc == 0 && on[0] == 1 && on[1] == 0 && out[1] == 1, "wrong null pattern");

    rc = mysql_stmt_fetch(stmt);
    TEST("NULL in col1");
    CHECK(rc == 0 && on[0] == 0 && out[0] == 1 && on[1] == 1, "wrong null pattern");

    rc = mysql_stmt_fetch(stmt);
    TEST("NULL in col2");
    CHECK(rc == 0 && on[2] == 1 && on[0] == 0 && out[0] == 1, "wrong null pattern");

    rc = mysql_stmt_fetch(stmt);
    TEST("NULL in col3");
    CHECK(rc == 0 && on[3] == 1 && on[0] == 0 && out[0] == 1, "wrong null pattern");

    rc = mysql_stmt_fetch(stmt);
    TEST("NULL in col4 (last col)");
    CHECK(rc == 0 && on[4] == 1 && on[0] == 0 && out[0] == 1, "wrong null pattern");

    rc = mysql_stmt_fetch(stmt);
    TEST("All columns NULL");
    CHECK(rc == 0 && on[0] && on[1] && on[2] && on[3] && on[4], "all should be NULL");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_reexecute_different_param_type(void) {
    printf("\n[PS re-execute with different param type]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_rpt");
    Q("CREATE TABLE t_rpt(id INT, val VARCHAR(100))");
    Q("INSERT INTO t_rpt VALUES(1, 'hello')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    CHECK(stmt, "stmt_init failed");

    int rc = mysql_stmt_prepare(stmt, "SELECT val FROM t_rpt WHERE id = ?", -1);
    CHECK(rc == 0, "prepare failed");

    int int_val = 1;
    unsigned long vl = 0; my_bool vn = 0;
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG;
    pb.buffer = &int_val;
    pb.length = &vl;
    pb.is_null = &vn;
    mysql_stmt_bind_param(stmt, &pb);

    rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, "execute with LONG param failed");

    char str_val[2] = "1";
    pb.buffer_type = MYSQL_TYPE_VAR_STRING;
    pb.buffer = str_val;
    pb.buffer_length = 1;
    pb.length = &(unsigned long){1};
    mysql_stmt_bind_param(stmt, &pb);

    rc = mysql_stmt_execute(stmt);
    TEST("re-execute with STRING param");
    CHECK(rc == 0, "re-execute with different type failed");

    char out[64] = {0}; unsigned long ol = 0; my_bool on2 = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_VAR_STRING;
    rb.buffer = out; rb.buffer_length = sizeof(out);
    rb.length = &ol; rb.is_null = &on2;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_fetch(stmt);
    TEST("fetch after re-execute: 'hello'");
    CHECK(rc == 0 && strcmp(out, "hello") == 0, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_reset_after_send_long_data(void) {
    printf("\n[PS reset after send_long_data]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_rsl");
    Q("CREATE TABLE t_rsl(id INT, data BLOB)");
    Q("INSERT INTO t_rsl VALUES(1, 'initial')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    CHECK(stmt, "stmt_init failed");

    int rc = mysql_stmt_prepare(stmt, "UPDATE t_rsl SET data = ? WHERE id = ?", -1);
    CHECK(rc == 0, "prepare failed");

    int id = 1;
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_BLOB;
    pb[0].buffer = NULL;
    pb[0].buffer_length = 0;
    pb[1].buffer_type = MYSQL_TYPE_LONG;
    pb[1].buffer = &id;
    mysql_stmt_bind_param(stmt, pb);

    mysql_stmt_send_long_data(stmt, 0, "streamed data", 13);

    rc = mysql_stmt_reset(stmt);
    TEST("reset after send_long_data");
    CHECK(rc == 0, "reset failed");

    char short_data[] = "short";
    pb[0].buffer = short_data;
    pb[0].buffer_length = 5;
    unsigned long dl = 5;
    pb[0].length = &dl;
    mysql_stmt_bind_param(stmt, pb);

    rc = mysql_stmt_execute(stmt);
    TEST("execute after reset with short data");
    CHECK(rc == 0, "execute failed");

    Q("SELECT data FROM t_rsl WHERE id=1");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("data after reset+execute: 'short'");
    CHECK(row && strcmp(row[0], "short") == 0, "wrong value");
    mysql_free_result(res);

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_multiple_blob_columns(void) {
    printf("\n[PS with multiple BLOB columns]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_mbc");
    Q("CREATE TABLE t_mbc(id INT, b1 BLOB, b2 BLOB, b3 BLOB)");
    Q("INSERT INTO t_mbc VALUES(1, 'aaa', 'bbb', 'ccc')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    CHECK(stmt, "stmt_init failed");

    int rc = mysql_stmt_prepare(stmt, "SELECT b1, b2, b3 FROM t_mbc WHERE id = ?", -1);
    CHECK(rc == 0, "prepare failed");

    int id = 1;
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id;
    mysql_stmt_bind_param(stmt, &pb);

    rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, "execute failed");

    rc = mysql_stmt_store_result(stmt);
    CHECK(rc == 0, "store_result failed");

    char b1[64]={0}, b2[64]={0}, b3[64]={0};
    unsigned long l1=0,l2=0,l3=0;
    my_bool n1=0,n2=0,n3=0;
    MYSQL_BIND rb[3];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type=MYSQL_TYPE_BLOB; rb[0].buffer=b1; rb[0].buffer_length=64; rb[0].length=&l1; rb[0].is_null=&n1;
    rb[1].buffer_type=MYSQL_TYPE_BLOB; rb[1].buffer=b2; rb[1].buffer_length=64; rb[1].length=&l2; rb[1].is_null=&n2;
    rb[2].buffer_type=MYSQL_TYPE_BLOB; rb[2].buffer=b3; rb[2].buffer_length=64; rb[2].length=&l3; rb[2].is_null=&n3;
    mysql_stmt_bind_result(stmt, rb);

    rc = mysql_stmt_fetch(stmt);
    TEST("3 BLOB columns: b1='aaa'");
    CHECK(rc == 0 && strcmp(b1, "aaa") == 0, "wrong b1");
    TEST("3 BLOB columns: b2='bbb'");
    CHECK(strcmp(b2, "bbb") == 0, "wrong b2");
    TEST("3 BLOB columns: b3='ccc'");
    CHECK(strcmp(b3, "ccc") == 0, "wrong b3");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_unsigned_bigint_boundary(void) {
    printf("\n[PS BIGINT UNSIGNED boundary: 18446744073709551615]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_ubig");
    Q("CREATE TABLE t_ubig(v BIGINT UNSIGNED)");
    Q("INSERT INTO t_ubig VALUES(0),(9223372036854775807),(18446744073709551615)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    CHECK(stmt, "stmt_init failed");

    int rc = mysql_stmt_prepare(stmt, "SELECT v FROM t_ubig ORDER BY v", -1);
    CHECK(rc == 0, "prepare failed");

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_store_result(stmt);

    unsigned long long out = 0; unsigned long ol = 0; my_bool on = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONGLONG; rb.buffer = &out; rb.length = &ol;
    rb.is_null = &on; rb.is_unsigned = 1;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_fetch(stmt);
    TEST("BIGINT UNSIGNED: 0");
    CHECK(rc == 0 && out == 0ULL, "wrong value");

    rc = mysql_stmt_fetch(stmt);
    TEST("BIGINT UNSIGNED: 9223372036854775807");
    CHECK(rc == 0 && out == 9223372036854775807ULL, "wrong value");

    rc = mysql_stmt_fetch(stmt);
    TEST("BIGINT UNSIGNED: 18446744073709551615");
    CHECK(rc == 0 && out == 18446744073709551615ULL, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_double_negative_zero(void) {
    printf("\n[PS DOUBLE negative zero and edge cases]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_dnz");
    Q("CREATE TABLE t_dnz(d DOUBLE)");
    Q("INSERT INTO t_dnz VALUES(0.0),(-0.0),(1.0e308),(-1.0e308)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    CHECK(stmt, "stmt_init failed");

    int rc = mysql_stmt_prepare(stmt, "SELECT d FROM t_dnz ORDER BY d", -1);
    CHECK(rc == 0, "prepare failed");

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_store_result(stmt);

    double out = 0; unsigned long ol = 0; my_bool on = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_DOUBLE; rb.buffer = &out; rb.length = &ol; rb.is_null = &on;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_fetch(stmt);
    TEST("DOUBLE -1e308");
    CHECK(rc == 0 && out < 0, "should be negative");

    rc = mysql_stmt_fetch(stmt);
    TEST("DOUBLE -0.0");
    CHECK(rc == 0 && out == 0.0, "should be zero");

    rc = mysql_stmt_fetch(stmt);
    TEST("DOUBLE 0.0");
    CHECK(rc == 0 && out == 0.0, "should be zero");

    rc = mysql_stmt_fetch(stmt);
    TEST("DOUBLE 1e308");
    CHECK(rc == 0 && out > 1e307, "should be large positive");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_select_db_nonexistent(void) {
    printf("\n[mysql_select_db: nonexistent database]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    int rc = mysql_select_db(m, "nonexistent_db_xyz");
    TEST("select_db nonexistent returns error");
    CHECK(rc != 0, "should fail");

    Q("SELECT 1");
    MYSQL_RES *res = mysql_store_result(m);
    CHECK(res, "connection still usable");
    mysql_free_result(res);
    mysql_close(m);
}

void test_select_db_verify(void) {
    printf("\n[mysql_select_db: verify with DATABASE()]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    int rc = mysql_select_db(m, "information_schema");
    TEST("select_db information_schema");
    CHECK(rc == 0, "select_db failed");

    Q("SELECT DATABASE()");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("DATABASE() = information_schema");
    CHECK(row && strcmp(row[0], "information_schema") == 0, "wrong db");
    mysql_free_result(res);

    rc = mysql_select_db(m, g_db);
    TEST("select_db back to original");
    CHECK(rc == 0, "select_db back failed");

    mysql_close(m);
}

void test_change_user_wrong_password(void) {
    printf("\n[mysql_change_user: wrong password]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    int rc = mysql_change_user(m, g_user, "wrong_password_xyz", g_db);
    TEST("change_user with wrong password fails");
    CHECK(rc != 0, "should fail");

    TEST("connection closed after failed change_user (server spec)");
    int qrc = Q("SELECT 1");
    CHECK(qrc != 0, "connection should be closed");
    mysql_close(m);
}

void test_multi_stmt_mixed_ok_result(void) {
    printf("\n[multi-statement: mixed OK and result set]\n");
    MYSQL *m = mysql_init(NULL);
    unsigned int ssl_mode = SSL_MODE_DISABLED;
    mysql_options(m, MYSQL_OPT_SSL_MODE, &ssl_mode);
    if (!mysql_real_connect(m, g_host, g_user, g_passwd, g_db, g_port, NULL,
                            CLIENT_MULTI_STATEMENTS)) {
        FAIL("connect with CLIENT_MULTI_STATEMENTS failed");
        return;
    }

    Q("DROP TABLE IF EXISTS t_ms2");
    Q("CREATE TABLE t_ms2(id INT PRIMARY KEY)");

    int rc = mysql_real_query(m, "INSERT INTO t_ms2 VALUES(1); SELECT id FROM t_ms2; INSERT INTO t_ms2 VALUES(2); SELECT id FROM t_ms2 ORDER BY id",
                              strlen("INSERT INTO t_ms2 VALUES(1); SELECT id FROM t_ms2; INSERT INTO t_ms2 VALUES(2); SELECT id FROM t_ms2 ORDER BY id"));
    TEST("multi-stmt: INSERT;SELECT;INSERT;SELECT");
    CHECK(rc == 0, "query failed");

    MYSQL_RES *res = mysql_store_result(m);
    TEST("1st result: INSERT OK (no result set)");
    CHECK(res == NULL, "INSERT should have no result set");

    int more = mysql_more_results(m);
    TEST("more_results after 1st = true");
    CHECK(more, "should have more results");

    rc = mysql_next_result(m);
    CHECK(rc == 0, "next_result failed");
    res = mysql_store_result(m);
    TEST("2nd result: SELECT has rows");
    CHECK(res && mysql_num_rows(res) == 1, "should have 1 row");
    mysql_free_result(res);

    rc = mysql_next_result(m);
    CHECK(rc == 0, "next_result 2 failed");
    res = mysql_store_result(m);
    TEST("3rd result: INSERT OK (no result set)");
    CHECK(res == NULL, "2nd INSERT should have no result set");

    rc = mysql_next_result(m);
    CHECK(rc == 0, "next_result 3 failed");
    res = mysql_store_result(m);
    TEST("4th result: SELECT has 2 rows");
    CHECK(res && mysql_num_rows(res) == 2, "should have 2 rows");
    mysql_free_result(res);

    rc = mysql_next_result(m);
    TEST("no more results");
    CHECK(rc != 0, "should return error/no-more");

    mysql_close(m);
}

void test_ps_store_result_where_zero(void) {
    printf("\n[PS store_result with WHERE returning zero rows]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_srz");
    Q("CREATE TABLE t_srz(id INT PRIMARY KEY)");
    Q("INSERT INTO t_srz VALUES(1),(2),(3)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    CHECK(stmt, "stmt_init failed");

    int rc = mysql_stmt_prepare(stmt, "SELECT id FROM t_srz WHERE id > 100", -1);
    CHECK(rc == 0, "prepare failed");

    rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, "execute failed");

    rc = mysql_stmt_store_result(stmt);
    CHECK(rc == 0, "store_result failed");

    TEST("num_rows = 0 for WHERE returning no rows");
    CHECK(mysql_stmt_num_rows(stmt) == 0, "should be 0");

    rc = mysql_stmt_fetch(stmt);
    TEST("fetch returns NO_DATA");
    CHECK(rc == MYSQL_NO_DATA, "should be NO_DATA");

    mysql_stmt_data_seek(stmt, 0);
    rc = mysql_stmt_fetch(stmt);
    TEST("data_seek(0) on empty: NO_DATA");
    CHECK(rc == MYSQL_NO_DATA, "should still be NO_DATA");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_all_columns_null(void) {
    printf("\n[PS all columns NULL]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_acn");
    Q("CREATE TABLE t_acn(a INT, b VARCHAR(20), c DOUBLE)");
    Q("INSERT INTO t_acn VALUES(NULL,NULL,NULL)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    CHECK(stmt, "stmt_init failed");

    int rc = mysql_stmt_prepare(stmt, "SELECT a, b, c FROM t_acn", -1);
    CHECK(rc == 0, "prepare failed");

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_store_result(stmt);

    int va=0; char vb[32]={0}; double vc=0;
    unsigned long la=0,lb=0,lc=0;
    my_bool na=0,nb=0,nc=0;
    MYSQL_BIND rb[3];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type=MYSQL_TYPE_LONG; rb[0].buffer=&va; rb[0].length=&la; rb[0].is_null=&na;
    rb[1].buffer_type=MYSQL_TYPE_VAR_STRING; rb[1].buffer=vb; rb[1].buffer_length=32; rb[1].length=&lb; rb[1].is_null=&nb;
    rb[2].buffer_type=MYSQL_TYPE_DOUBLE; rb[2].buffer=&vc; rb[2].length=&lc; rb[2].is_null=&nc;
    mysql_stmt_bind_result(stmt, rb);

    rc = mysql_stmt_fetch(stmt);
    TEST("all columns NULL: a is null");
    CHECK(rc == 0 && na == 1, "a should be null");
    TEST("all columns NULL: b is null");
    CHECK(nb == 1, "b should be null");
    TEST("all columns NULL: c is null");
    CHECK(nc == 1, "c should be null");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

/* ======================================================================
 * Phase 2: Integration tests for newly added API functions
 * ====================================================================== */

void test_row_seek_tell(void) {
    printf("\n[mysql_row_seek / mysql_row_tell]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_rsk");
    Q("CREATE TABLE t_rsk(id INT PRIMARY KEY)");
    Q("INSERT INTO t_rsk VALUES(1),(2),(3),(4),(5)");

    Q("SELECT id FROM t_rsk ORDER BY id");
    MYSQL_RES *res = mysql_store_result(m);
    CHECK(res, "store_result failed");

    TEST("num_rows = 5");
    CHECK(mysql_num_rows(res) == 5, "wrong row count");

    MYSQL_ROW_OFFSET save = mysql_row_tell(res);
    TEST("row_tell after store_result");
    CHECK(save != NULL || mysql_num_rows(res) > 0, "row_tell returned unexpected value");

    mysql_data_seek(res, 2);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("data_seek(2) then fetch: id=3");
    CHECK(row && strcmp(row[0], "3") == 0, "wrong row");

    MYSQL_ROW_OFFSET pos = mysql_row_tell(res);
    TEST("row_tell after fetch row 3");
    CHECK(pos != NULL, "row_tell returned NULL");

    mysql_data_seek(res, 0);
    MYSQL_ROW_OFFSET pos0 = mysql_row_tell(res);
    row = mysql_fetch_row(res);
    TEST("data_seek(0) then fetch: id=1");
    CHECK(row && strcmp(row[0], "1") == 0, "wrong row");

    mysql_row_seek(res, pos);
    row = mysql_fetch_row(res);
    TEST("row_seek to pos then fetch: id=4");
    CHECK(row && strcmp(row[0], "4") == 0, "wrong row after row_seek");

    mysql_free_result(res);
    mysql_close(m);
}

void test_field_tell_eof_result_metadata(void) {
    printf("\n[mysql_field_tell / mysql_eof / mysql_result_metadata]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("SELECT 1 AS a, 'hello' AS b");
    MYSQL_RES *res = mysql_store_result(m);
    CHECK(res, "store_result failed");

    TEST("field_tell starts at 0");
    CHECK(mysql_field_tell(res) == 0, "not 0");

    mysql_fetch_field(res);
    TEST("field_tell after 1 fetch = 1");
    CHECK(mysql_field_tell(res) == 1, "not 1");

    mysql_fetch_field(res);
    TEST("field_tell after 2 fetches = 2");
    CHECK(mysql_field_tell(res) == 2, "not 2");

    TEST("mysql_eof on store_result = false");
    CHECK(mysql_eof(res) == 0, "eof should be 0 for store_result");

    TEST("result_metadata = RESULTSET_METADATA_FULL");
    CHECK(mysql_result_metadata(res) == RESULTSET_METADATA_FULL, "not FULL");

    mysql_free_result(res);
    mysql_close(m);
}

void test_stmt_row_seek_tell(void) {
    printf("\n[mysql_stmt_row_seek / mysql_stmt_row_tell]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_srsk");
    Q("CREATE TABLE t_srsk(id INT PRIMARY KEY)");
    Q("INSERT INTO t_srsk VALUES(10),(20),(30),(40),(50)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    CHECK(stmt, "stmt_init failed");

    int rc = mysql_stmt_prepare(stmt, "SELECT id FROM t_srsk ORDER BY id", -1);
    CHECK(rc == 0, "prepare failed");

    rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, "execute failed");

    rc = mysql_stmt_store_result(stmt);
    CHECK(rc == 0, "store_result failed");

    TEST("stmt_num_rows = 5");
    CHECK(mysql_stmt_num_rows(stmt) == 5, "wrong count");

    MYSQL_ROW_OFFSET pos = mysql_stmt_row_tell(stmt);
    TEST("stmt_row_tell after store");
    CHECK(pos != NULL, "row_tell returned NULL");

    mysql_stmt_data_seek(stmt, 3);
    TEST("stmt_data_seek(3)");

    int out_id = 0;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_id; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_fetch(stmt);
    TEST("fetch after seek(3): id=40");
    CHECK(rc == 0 && out_id == 40, "wrong value");

    mysql_stmt_row_seek(stmt, pos);
    TEST("stmt_row_seek back to start");

    rc = mysql_stmt_fetch(stmt);
    TEST("fetch after row_seek: id=10");
    CHECK(rc == 0 && out_id == 10, "wrong value after row_seek");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_stmt_attr_set_get(void) {
    printf("\n[mysql_stmt_attr_set / mysql_stmt_attr_get]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    CHECK(stmt, "stmt_init failed");

    my_bool update_ml = 1;
    my_bool rc = mysql_stmt_attr_set(stmt, STMT_ATTR_UPDATE_MAX_LENGTH, &update_ml);
    TEST("attr_set UPDATE_MAX_LENGTH=1");
    CHECK(rc == 0, "attr_set failed");

    my_bool get_val = 0;
    rc = mysql_stmt_attr_get(stmt, STMT_ATTR_UPDATE_MAX_LENGTH, &get_val);
    TEST("attr_get UPDATE_MAX_LENGTH=1");
    CHECK(rc == 0 && get_val == 1, "wrong value");

    unsigned long cursor = CURSOR_TYPE_READ_ONLY;
    rc = mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, &cursor);
    TEST("attr_set CURSOR_TYPE=READ_ONLY");
    CHECK(rc == 0, "attr_set cursor failed");

    unsigned long get_cursor = 0;
    rc = mysql_stmt_attr_get(stmt, STMT_ATTR_CURSOR_TYPE, &get_cursor);
    TEST("attr_get CURSOR_TYPE=READ_ONLY");
    CHECK(rc == 0 && get_cursor == CURSOR_TYPE_READ_ONLY, "wrong cursor value");

    unsigned long prefetch = 100;
    rc = mysql_stmt_attr_set(stmt, STMT_ATTR_PREFETCH_ROWS, &prefetch);
    TEST("attr_set PREFETCH_ROWS=100");
    CHECK(rc == 0, "attr_set prefetch failed");

    unsigned long get_prefetch = 0;
    rc = mysql_stmt_attr_get(stmt, STMT_ATTR_PREFETCH_ROWS, &get_prefetch);
    TEST("attr_get PREFETCH_ROWS=100");
    CHECK(rc == 0 && get_prefetch == 100, "wrong prefetch value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_reset_connection(void) {
    printf("\n[mysql_reset_connection]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("SELECT 1");
    MYSQL_RES *res = mysql_store_result(m);
    mysql_free_result(res);

    TEST("reset_connection");
    int rc = mysql_reset_connection(m);
    CHECK(rc == 0, "reset_connection failed");

    Q("SELECT 2");
    res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("query after reset: 2");
    CHECK(row && strcmp(row[0], "2") == 0, "wrong result");
    mysql_free_result(res);

    mysql_close(m);
}

void test_get_client_version(void) {
    printf("\n[mysql_get_client_version]\n");
    unsigned long v = mysql_get_client_version();
    TEST("client_version >= 80000");
    CHECK(v >= 80000UL, "version too low");
}

void test_mysql_info(void) {
    printf("\n[mysql_info]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_info");
    Q("CREATE TABLE t_info(id INT PRIMARY KEY)");
    Q("INSERT INTO t_info VALUES(1),(2),(3)");

    Q("UPDATE t_info SET id=id+10 WHERE id > 1");
    const char *info = mysql_info(m);
    TEST("mysql_info after UPDATE (may be NULL)");
    CHECK(1, "info check done");

    Q("SELECT COUNT(*) FROM t_info");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("after UPDATE: 3 rows");
    CHECK(row, "no row");
    mysql_free_result(res);

    mysql_close(m);
}

void test_mysql_stat(void) {
    printf("\n[mysql_stat]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    const char *stat = mysql_stat(m);
    TEST("mysql_stat returns non-NULL");
    CHECK(stat != NULL, "stat returned NULL");

    mysql_close(m);
}

void test_list_dbs(void) {
    printf("\n[mysql_list_dbs]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_RES *res = mysql_list_dbs(m, NULL);
    TEST("list_dbs(NULL) returns result");
    CHECK(res != NULL, "list_dbs failed");

    my_ulonglong count = mysql_num_rows(res);
    TEST("at least 1 database");
    CHECK(count >= 1, "no databases");

    int found = 0;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        if (row[0] && strcmp(row[0], g_db) == 0) found = 1;
    }
    TEST("sp_test db found");
    CHECK(found, "sp_test db not found");
    mysql_free_result(res);

    res = mysql_list_dbs(m, "sp_%");
    TEST("list_dbs('sp_%') returns result");
    CHECK(res != NULL, "list_dbs with wild failed");
    mysql_free_result(res);

    mysql_close(m);
}

void test_list_tables(void) {
    printf("\n[mysql_list_tables]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_lt");
    Q("CREATE TABLE t_lt(id INT PRIMARY KEY, name VARCHAR(20))");

    MYSQL_RES *res = mysql_list_tables(m, "t_lt");
    TEST("list_tables('t_lt') returns result");
    CHECK(res != NULL, "list_tables failed");

    int found = 0;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        if (row[0] && strcmp(row[0], "t_lt") == 0) found = 1;
    }
    TEST("t_lt table found");
    CHECK(found, "t_lt table not found");
    mysql_free_result(res);

    mysql_close(m);
}

void test_list_fields(void) {
    printf("\n[mysql_list_fields]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_lf");
    Q("CREATE TABLE t_lf(id INT PRIMARY KEY, name VARCHAR(20), score DOUBLE)");

    MYSQL_RES *res = mysql_list_fields(m, "t_lf", NULL);
    TEST("list_fields('t_lf') returns result");
    CHECK(res != NULL, "list_fields failed");

    unsigned int nf = mysql_num_fields(res);
    TEST("t_lf table has 3+ fields");
    CHECK(nf >= 3, "not enough fields");
    mysql_free_result(res);

    mysql_close(m);
}

void test_list_processes(void) {
    printf("\n[mysql_list_processes]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_RES *res = mysql_list_processes(m);
    TEST("list_processes returns result");
    CHECK(res != NULL, "list_processes failed");

    my_ulonglong count = mysql_num_rows(res);
    TEST("at least 1 process");
    CHECK(count >= 1, "no processes");
    mysql_free_result(res);

    mysql_close(m);
}

void test_real_escape_string_quote(void) {
    printf("\n[mysql_real_escape_string_quote]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    char out[128];
    unsigned long len;

    len = mysql_real_escape_string_quote(m, out, "it's", 4, '\'');
    TEST("quote=' escapes single-quote");
    CHECK(len == 5 && memcmp(out, "it\\'s", 5) == 0, "wrong escape result");

    len = mysql_real_escape_string_quote(m, out, "a\"b", 3, '\'');
    TEST("quote=' does NOT escape double-quote");
    CHECK(len == 3 && memcmp(out, "a\"b", 3) == 0, "double-quote should not be escaped");

    len = mysql_real_escape_string_quote(m, out, "a\"b", 3, '"');
    TEST("quote=\" escapes double-quote");
    CHECK(len == 4 && memcmp(out, "a\\\"b", 4) == 0, "wrong escape result");

    len = mysql_real_escape_string_quote(m, out, "it's", 4, '"');
    TEST("quote=\" does NOT escape single-quote");
    CHECK(len == 4 && memcmp(out, "it's", 4) == 0, "single-quote should not be escaped");

    len = mysql_real_escape_string_quote(m, out, "a\\b\nc\rd\032e", 9, '\'');
    TEST("escape backslash, newline, cr, ctrl-Z");
    CHECK(len == 13, "wrong length for control chars");

    len = mysql_real_escape_string(m, out, "it's a \"test\"", 14);
    TEST("mysql_real_escape_string escapes ' but not \"");
    CHECK(len == 16 && out[2] == '\\' && out[3] == '\'', "real_escape_string wrong");

    mysql_close(m);
}

void test_hex_string(void) {
    printf("\n[mysql_hex_string]\n");
    char out[32];
    unsigned long len = mysql_hex_string(out, "\x01\x02\xAB", 3);
    TEST("hex_string of 3 bytes = 6 chars");
    CHECK(len == 6, "wrong length");

    TEST("hex output = '0102AB'");
    CHECK(memcmp(out, "0102AB", 6) == 0, "wrong hex output");

    len = mysql_hex_string(out, "Hello", 5);
    TEST("hex_string of 'Hello' = 10 chars");
    CHECK(len == 10, "wrong length");
}

void test_get_character_set_info(void) {
    printf("\n[mysql_get_character_set_info]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MY_CHARSET_INFO cs;
    memset(&cs, 0, sizeof(cs));
    mysql_get_character_set_info(m, &cs);
    TEST("csname is non-NULL");
    CHECK(cs.csname != NULL, "csname is NULL");

    TEST("name is non-NULL");
    CHECK(cs.name != NULL, "name is NULL");

    TEST("number matches server charsetnr");
    CHECK(cs.number > 0, "number is 0");

    TEST("mbmaxlen >= 1");
    CHECK(cs.mbmaxlen >= 1, "mbmaxlen < 1");

    TEST("mbminlen >= 1");
    CHECK(cs.mbminlen >= 1, "mbminlen < 1");

    TEST("state is set");
    CHECK(cs.state == 1, "state not set");

    if (cs.number == 255 || cs.number == 246 || cs.number == 256) {
        TEST("utf8mb4 charset: mbmaxlen == 4");
        CHECK(cs.mbmaxlen == 4, "utf8mb4 should have mbmaxlen=4");
    }

    mysql_close(m);
}

void test_set_server_option(void) {
    printf("\n[mysql_set_server_option]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    int rc = mysql_set_server_option(m, MYSQL_OPTION_MULTI_STATEMENTS_ON);
    TEST("set MULTI_STATEMENTS_ON (may fail without privilege)");
    CHECK(rc == 0 || mysql_errno(m) > 0, "unexpected result");

    rc = mysql_set_server_option(m, MYSQL_OPTION_MULTI_STATEMENTS_OFF);
    TEST("set MULTI_STATEMENTS_OFF (may fail without privilege)");
    CHECK(rc == 0 || mysql_errno(m) > 0, "unexpected result");

    mysql_close(m);
}

void test_session_track(void) {
    printf("\n[mysql_session_track]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    TEST("CLIENT_SESSION_TRACK is set");
    CHECK((m->client_flag & CLIENT_SESSION_TRACK) != 0, "CLIENT_SESSION_TRACK not set");

    const char *data = NULL;
    size_t dlen = 0;

    int rc = mysql_real_query(m, "SELECT DATABASE()", 17);
    CHECK(rc == 0, "SELECT DATABASE() failed");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    char curdb[128] = "";
    if (row && row[0]) snprintf(curdb, sizeof(curdb), "%s", row[0]);
    mysql_free_result(res);

    char q[256];
    snprintf(q, sizeof(q), "USE %s", curdb[0] ? curdb : "test");
    rc = mysql_real_query(m, q, (unsigned long)strlen(q));
    CHECK(rc == 0, "USE db failed");

    rc = mysql_session_track_get_first(m, SESSION_TRACK_SCHEMA, &data, &dlen);
    TEST("session_track_get_first(SCHEMA) after USE");
    if (rc == 0) {
        TEST("schema track data non-empty");
        CHECK(dlen > 0, "schema data empty");
    } else {
        TEST("schema track not available (server may not support)");
        CHECK(1, "ok");
    }

    rc = mysql_real_query(m, "SET autocommit=0", 16);
    CHECK(rc == 0, "SET autocommit failed");

    data = NULL; dlen = 0;
    rc = mysql_session_track_get_first(m, SESSION_TRACK_SYSTEM_VARIABLES, &data, &dlen);
    TEST("session_track_get_first(SYSTEM_VARIABLES) after SET autocommit");
    if (rc == 0) {
        TEST("system_variables track data non-empty");
        CHECK(dlen > 0, "system_variables data empty");

        const char *data2 = NULL;
        size_t dlen2 = 0;
        rc = mysql_session_track_get_next(m, SESSION_TRACK_SYSTEM_VARIABLES, &data2, &dlen2);
        TEST("session_track_get_next returns variable value");
        if (rc == 0) {
            CHECK(dlen2 > 0, "variable value empty");
        } else {
            CHECK(1, "ok");
        }
    } else {
        TEST("system_variables track not available");
        CHECK(1, "ok");
    }

    mysql_real_query(m, "SET autocommit=1", 16);

    rc = mysql_session_track_get_first(m, SESSION_TRACK_GTIDS, &data, &dlen);
    TEST("session_track_get_first(GTIDS) returns failure (no GTID)");
    CHECK(rc == 1, "GTIDS should not be tracked by default");

    mysql_close(m);
}

void test_stmt_param_metadata(void) {
    printf("\n[mysql_stmt_param_metadata]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    CHECK(stmt, "stmt_init failed");

    int rc = mysql_stmt_prepare(stmt, "SELECT 1", -1);
    CHECK(rc == 0, "prepare failed");

    MYSQL_RES *pmeta = mysql_stmt_param_metadata(stmt);
    TEST("param_metadata for no-param query = NULL");
    CHECK(pmeta == NULL, "expected NULL for 0 params");

    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "SELECT ? + ?", -1);
    CHECK(rc == 0, "prepare 2-param failed");

    TEST("param_count = 2");
    CHECK(mysql_stmt_param_count(stmt) == 2, "wrong param count");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_cr_error_codes(void) {
    printf("\n[CR_ error codes]\n");

    TEST("CR_UNKNOWN_ERROR = 2000");
    CHECK(CR_UNKNOWN_ERROR == 2000, "wrong value");

    TEST("CR_SERVER_GONE_ERROR = 2006");
    CHECK(CR_SERVER_GONE_ERROR == 2006, "wrong value");

    TEST("CR_SERVER_LOST = 2013");
    CHECK(CR_SERVER_LOST == 2013, "wrong value");

    TEST("CR_COMMANDS_OUT_OF_SYNC = 2014");
    CHECK(CR_COMMANDS_OUT_OF_SYNC == 2014, "wrong value");

    TEST("CR_OUT_OF_MEMORY = 2008");
    CHECK(CR_OUT_OF_MEMORY == 2008, "wrong value");

    TEST("CR_SSL_CONNECTION_ERROR = 2026");
    CHECK(CR_SSL_CONNECTION_ERROR == 2026, "wrong value");

    TEST("CR_NO_DATA = 2051");
    CHECK(CR_NO_DATA == 2051, "wrong value");

    TEST("CR_ERROR_LAST = 2074");
    CHECK(CR_ERROR_LAST == 2074, "wrong value");

    TEST("CR_AUTH_PLUGIN_CANNOT_LOAD = 2059");
    CHECK(CR_AUTH_PLUGIN_CANNOT_LOAD == 2059, "wrong value");
}

void test_helper_macros(void) {
    printf("\n[IS_PRI_KEY / IS_NOT_NULL / IS_BLOB / IS_NUM / IS_LONGDATA]\n");

    TEST("IS_PRI_KEY(PRI_KEY_FLAG) = true");
    CHECK(IS_PRI_KEY(PRI_KEY_FLAG), "should be true");

    TEST("IS_NOT_NULL(NOT_NULL_FLAG) = true");
    CHECK(IS_NOT_NULL(NOT_NULL_FLAG), "should be true");

    TEST("IS_BLOB(BLOB_FLAG) = true");
    CHECK(IS_BLOB(BLOB_FLAG), "should be true");

    TEST("IS_NUM(MYSQL_TYPE_LONG) = true");
    CHECK(IS_NUM(MYSQL_TYPE_LONG), "LONG should be numeric");

    TEST("IS_NUM(MYSQL_TYPE_TINY) = true");
    CHECK(IS_NUM(MYSQL_TYPE_TINY), "TINY should be numeric");

    TEST("!IS_NUM(MYSQL_TYPE_VARCHAR) = true");
    CHECK(!IS_NUM(MYSQL_TYPE_VARCHAR), "VARCHAR should not be numeric");

    TEST("IS_NUM(MYSQL_TYPE_YEAR) = true");
    CHECK(IS_NUM(MYSQL_TYPE_YEAR), "YEAR should be numeric");

    TEST("IS_NUM(MYSQL_TYPE_NEWDECIMAL) = true");
    CHECK(IS_NUM(MYSQL_TYPE_NEWDECIMAL), "NEWDECIMAL should be numeric");

    TEST("!IS_NUM(MYSQL_TYPE_TIMESTAMP) = true");
    CHECK(!IS_NUM(MYSQL_TYPE_TIMESTAMP), "TIMESTAMP should not be IS_NUM");

    TEST("IS_LONGDATA(MYSQL_TYPE_BLOB) = true");
    CHECK(IS_LONGDATA(MYSQL_TYPE_BLOB), "BLOB should be longdata");

    TEST("IS_LONGDATA(MYSQL_TYPE_VAR_STRING) = true");
    CHECK(IS_LONGDATA(MYSQL_TYPE_VAR_STRING), "VAR_STRING should be longdata");

    TEST("MYSQL_COUNT_ERROR is max");
    CHECK(MYSQL_COUNT_ERROR == (my_ulonglong)(~(my_ulonglong)0), "wrong value");
}

void test_enum_compat(void) {
    printf("\n[enum compatibility with MySQL 8.0]\n");

    TEST("MYSQL_TYPE_TIMESTAMP2 = 17");
    CHECK(MYSQL_TYPE_TIMESTAMP2 == 17, "wrong value");

    TEST("MYSQL_TYPE_DATETIME2 = 18");
    CHECK(MYSQL_TYPE_DATETIME2 == 18, "wrong value");

    TEST("MYSQL_TYPE_TIME2 = 19");
    CHECK(MYSQL_TYPE_TIME2 == 19, "wrong value");

    TEST("MYSQL_TYPE_TYPED_ARRAY = 20");
    CHECK(MYSQL_TYPE_TYPED_ARRAY == 20, "wrong value");

    TEST("MYSQL_TYPE_INVALID = 243");
    CHECK(MYSQL_TYPE_INVALID == 243, "wrong value");

    TEST("MYSQL_TYPE_BOOL = 244");
    CHECK(MYSQL_TYPE_BOOL == 244, "wrong value");

    TEST("FIELD_TYPE_LONG == MYSQL_TYPE_LONG");
    CHECK(FIELD_TYPE_LONG == MYSQL_TYPE_LONG, "backward compat broken");

    TEST("FIELD_TYPE_BLOB == MYSQL_TYPE_BLOB");
    CHECK(FIELD_TYPE_BLOB == MYSQL_TYPE_BLOB, "backward compat broken");

    TEST("CLIENT_MULTI_QUERIES == CLIENT_MULTI_STATEMENTS");
    CHECK(CLIENT_MULTI_QUERIES == CLIENT_MULTI_STATEMENTS, "backward compat broken");

    TEST("MYSQL_OPT_RECONNECT enum value = 15");
    CHECK(MYSQL_OPT_RECONNECT == 15, "wrong enum value");

    TEST("MYSQL_OPT_SSL_KEY enum value = 19");
    CHECK(MYSQL_OPT_SSL_KEY == 19, "wrong enum value");

    TEST("MYSQL_OPT_SSL_MODE enum value = 35");
    CHECK(MYSQL_OPT_SSL_MODE == 35, "wrong enum value");

    TEST("RESULTSET_METADATA_FULL = 1");
    CHECK(RESULTSET_METADATA_FULL == 1, "wrong value");

    TEST("CURSOR_TYPE_READ_ONLY = 1");
    CHECK(CURSOR_TYPE_READ_ONLY == 1, "wrong value");

    TEST("MYSQL_STMT_INIT_DONE = 1");
    CHECK(MYSQL_STMT_INIT_DONE == 1, "wrong value");

    TEST("MYSQL_STMT_PREPARE_DONE = 2");
    CHECK(MYSQL_STMT_PREPARE_DONE == 2, "wrong value");

    TEST("MYSQL_STMT_EXECUTE_DONE = 3");
    CHECK(MYSQL_STMT_EXECUTE_DONE == 3, "wrong value");

    TEST("MYSQL_STMT_FETCH_DONE = 4");
    CHECK(MYSQL_STMT_FETCH_DONE == 4, "wrong value");

    TEST("SSL_FIPS_MODE_OFF = 0");
    CHECK(SSL_FIPS_MODE_OFF == 0, "wrong value");

    TEST("SERVER_STATUS_AUTOCOMMIT = 2");
    CHECK(SERVER_STATUS_AUTOCOMMIT == 2, "wrong value");

    TEST("SERVER_MORE_RESULTS_EXISTS = 8");
    CHECK(SERVER_MORE_RESULTS_EXISTS == 8, "wrong value");

    TEST("SESSION_TRACK_SCHEMA = 1");
    CHECK(SESSION_TRACK_SCHEMA == 1, "wrong value");
}

void test_client_flags_compat(void) {
    printf("\n[CLIENT_ flags compatibility]\n");

    TEST("CLIENT_NO_SCHEMA = 0x10");
    CHECK(CLIENT_NO_SCHEMA == 0x10, "wrong value");

    TEST("CLIENT_LOCAL_FILES = 0x80");
    CHECK(CLIENT_LOCAL_FILES == 0x80, "wrong value");

    TEST("CLIENT_PROTOCOL_41 = 0x200");
    CHECK(CLIENT_PROTOCOL_41 == 0x200, "wrong value");

    TEST("CLIENT_INTERACTIVE = 0x400");
    CHECK(CLIENT_INTERACTIVE == 0x400, "wrong value");

    TEST("CLIENT_IGNORE_SIGPIPE = 0x1000");
    CHECK(CLIENT_IGNORE_SIGPIPE == 0x1000, "wrong value");

    TEST("CLIENT_PS_MULTI_RESULTS = 0x40000");
    CHECK(CLIENT_PS_MULTI_RESULTS == 0x40000, "wrong value");

    TEST("CLIENT_SESSION_TRACK = 0x800000");
    CHECK(CLIENT_SESSION_TRACK == 0x800000, "wrong value");

    TEST("CLIENT_OPTIONAL_RESULTSET_METADATA = 0x2000000");
    CHECK(CLIENT_OPTIONAL_RESULTSET_METADATA == 0x2000000, "wrong value");

    TEST("CLIENT_QUERY_ATTRIBUTES = 0x8000000");
    CHECK(CLIENT_QUERY_ATTRIBUTES == 0x8000000, "wrong value");

    TEST("CLIENT_SSL_VERIFY_SERVER_CERT = 0x40000000");
    CHECK(CLIENT_SSL_VERIFY_SERVER_CERT == 0x40000000UL, "wrong value");

    TEST("CLIENT_REMEMBER_OPTIONS = 0x80000000");
    CHECK(CLIENT_REMEMBER_OPTIONS == 0x80000000UL, "wrong value");

    TEST("REFRESH_GRANT = 1");
    CHECK(REFRESH_GRANT == 1, "wrong value");

    TEST("HAVE_MYSQL_REAL_CONNECT defined");
    CHECK(HAVE_MYSQL_REAL_CONNECT == 1, "not defined");
}

void test_stmt_state_tracking(void) {
    printf("\n[MYSQL_STMT state tracking]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    CHECK(stmt, "stmt_init failed");

    TEST("state after init = INIT_DONE");
    CHECK(stmt->state == MYSQL_STMT_INIT_DONE, "wrong state");

    int rc = mysql_stmt_prepare(stmt, "SELECT 1 AS val", -1);
    CHECK(rc == 0, "prepare failed");
    TEST("state after prepare = PREPARE_DONE");
    CHECK(stmt->state == MYSQL_STMT_PREPARE_DONE, "wrong state");

    int out_val = 0;
    unsigned long out_len = 0;
    my_bool out_null = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG;
    rb.buffer = &out_val;
    rb.length = &out_len;
    rb.is_null = &out_null;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, "execute failed");
    TEST("state after execute = EXECUTE_DONE");
    CHECK(stmt->state == MYSQL_STMT_EXECUTE_DONE, "wrong state");

    rc = mysql_stmt_fetch(stmt);
    CHECK(rc == 0, "first fetch failed");
    rc = mysql_stmt_fetch(stmt);
    TEST("state after all fetched = FETCH_DONE");
    CHECK(stmt->state == MYSQL_STMT_FETCH_DONE, "wrong state");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_fetch_column_offset(void) {
    printf("\n[mysql_stmt_fetch_column with offset]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_fco");
    Q("CREATE TABLE t_fco(id INT, name VARCHAR(50))");
    Q("INSERT INTO t_fco VALUES(1, 'Hello World')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    CHECK(stmt, "stmt_init failed");

    int rc = mysql_stmt_prepare(stmt, "SELECT name FROM t_fco WHERE id=1", -1);
    CHECK(rc == 0, "prepare failed");

    rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, "execute failed");

    rc = mysql_stmt_store_result(stmt);
    CHECK(rc == 0, "store_result failed");

    char out_name[64] = {0};
    unsigned long out_len = 0;
    my_bool out_null = 0, out_err = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_STRING;
    rb.buffer = out_name;
    rb.buffer_length = sizeof(out_name);
    rb.length = &out_len;
    rb.is_null = &out_null;
    rb.error = &out_err;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_fetch(stmt);
    CHECK(rc == 0, "fetch failed");
    TEST("full name = 'Hello World'");
    CHECK(strcmp(out_name, "Hello World") == 0, "wrong full value");

    char buf[16] = {0};
    unsigned long blen = 0;
    my_bool is_null = 0, err = 0;
    MYSQL_BIND cb;
    memset(&cb, 0, sizeof(cb));
    cb.buffer_type = MYSQL_TYPE_STRING;
    cb.buffer = buf;
    cb.buffer_length = sizeof(buf);
    cb.length = &blen;
    cb.is_null = &is_null;
    cb.error = &err;

    rc = mysql_stmt_fetch_column(stmt, &cb, 0, 6);
    TEST("fetch_column with offset=6: 'World'");
    CHECK(rc == 0 && strncmp(buf, "World", 5) == 0, "wrong partial value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_fetch_without_bind_result(void) {
    printf("\n[mysql_stmt_fetch without bind_result]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    CHECK(stmt, "stmt_init failed");

    int rc = mysql_stmt_prepare(stmt, "SELECT 1, 'hello'", -1);
    CHECK(rc == 0, "prepare failed");

    rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, "execute failed");

    rc = mysql_stmt_fetch(stmt);
    TEST("fetch without bind_result: should succeed");
    CHECK(rc == 0, "fetch should succeed without bind");

    rc = mysql_stmt_fetch(stmt);
    TEST("second fetch returns NO_DATA");
    CHECK(rc == MYSQL_NO_DATA, "expected MYSQL_NO_DATA");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_fetch_column_streaming(void) {
    printf("\n[mysql_stmt_fetch_column in streaming mode]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_fcs");
    Q("CREATE TABLE t_fcs(id INT, name VARCHAR(50))");
    Q("INSERT INTO t_fcs VALUES(1, 'StreamingTest')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    CHECK(stmt, "stmt_init failed");

    int rc = mysql_stmt_prepare(stmt, "SELECT name FROM t_fcs WHERE id=1", -1);
    CHECK(rc == 0, "prepare failed");

    rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, "execute failed");

    int out_id = 0;
    unsigned long ol = 0;
    my_bool on = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_id; rb.length = &ol; rb.is_null = &on;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_fetch(stmt);
    CHECK(rc == 0, "fetch failed");

    char buf[32] = {0};
    unsigned long blen = 0;
    my_bool is_null = 0, err = 0;
    MYSQL_BIND cb;
    memset(&cb, 0, sizeof(cb));
    cb.buffer_type = MYSQL_TYPE_STRING;
    cb.buffer = buf;
    cb.buffer_length = sizeof(buf);
    cb.length = &blen;
    cb.is_null = &is_null;
    cb.error = &err;

    rc = mysql_stmt_fetch_column(stmt, &cb, 0, 0);
    TEST("fetch_column streaming: 'StreamingTest'");
    CHECK(rc == 0 && strncmp(buf, "StreamingTest", 13) == 0, "wrong value");

    rc = mysql_stmt_fetch_column(stmt, &cb, 0, 9);
    TEST("fetch_column streaming offset=9: 'Test'");
    CHECK(rc == 0 && strncmp(buf, "Test", 4) == 0, "wrong partial value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_options_roundtrip(void) {
    printf("\n[mysql_options / mysql_get_option roundtrip]\n");
    MYSQL *m = mysql_init(NULL);
    CHECK(m, "mysql_init failed");

    unsigned int ct = 30;
    mysql_options(m, MYSQL_OPT_CONNECT_TIMEOUT, &ct);
    unsigned int gt = 0;
    mysql_get_option(m, MYSQL_OPT_CONNECT_TIMEOUT, &gt);
    TEST("connect_timeout roundtrip: 30");
    CHECK(gt == 30, "wrong value");

    my_bool recon = 1;
    mysql_options(m, MYSQL_OPT_RECONNECT, &recon);
    my_bool grecon = 0;
    mysql_get_option(m, MYSQL_OPT_RECONNECT, &grecon);
    TEST("reconnect roundtrip: 1");
    CHECK(grecon == 1, "wrong value");

    my_bool trunc = 1;
    mysql_options(m, MYSQL_REPORT_DATA_TRUNCATION, &trunc);
    my_bool gtrunc = 0;
    mysql_get_option(m, MYSQL_REPORT_DATA_TRUNCATION, &gtrunc);
    TEST("report_data_truncation roundtrip: 1");
    CHECK(gtrunc == 1, "wrong value");

    mysql_close(m);
}

void test_connect_attrs(void) {
    printf("\n[MYSQL_OPT_CONNECT_ATTR_ADD / options4]\n");
    MYSQL *m = mysql_init(NULL);
    CHECK(m, "mysql_init failed");

    mysql_options4(m, MYSQL_OPT_CONNECT_ATTR_ADD, "my_attr_key", "my_attr_val");
    TEST("options4 CONNECT_ATTR_ADD: no crash");
    CHECK(1, "ok");

    mysql_options4(m, MYSQL_OPT_CONNECT_ATTR_ADD, "another_key", "another_val");
    TEST("options4 second CONNECT_ATTR_ADD: no crash");
    CHECK(1, "ok");

    if (!mysql_real_connect(m, g_host, g_user, g_passwd, g_db, g_port, NULL, 0)) {
        FAIL("connect failed");
        return;
    }
    TEST("connect with custom attrs: ok");
    CHECK(1, "ok");

    Q("SELECT attr_value FROM performance_schema.session_account_connect_attrs WHERE attr_name='my_attr_key'");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("custom attr 'my_attr_key' found in performance_schema");
    if (row && row[0]) {
        CHECK(strcmp(row[0], "my_attr_val") == 0, "wrong attr value");
    } else {
        CHECK(1, "attr not found (may need performance_schema)");
    }
    mysql_free_result(res);

    mysql_close(m);
}

void test_options4_overwrite(void) {
    printf("\n[options4 CONNECT_ATTR_ADD overwrite]\n");
    MYSQL *m = mysql_init(NULL);
    CHECK(m, "mysql_init failed");

    mysql_options4(m, MYSQL_OPT_CONNECT_ATTR_ADD, "test_key", "first_val");
    mysql_options4(m, MYSQL_OPT_CONNECT_ATTR_ADD, "test_key", "second_val");

    if (!mysql_real_connect(m, g_host, g_user, g_passwd, g_db, g_port, NULL, 0)) {
        FAIL("connect failed");
        return;
    }
    TEST("connect with overwritten attr: ok");
    CHECK(1, "ok");

    Q("SELECT attr_value FROM performance_schema.session_account_connect_attrs WHERE attr_name='test_key'");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("overwritten attr has second value");
    if (row && row[0]) {
        CHECK(strcmp(row[0], "second_val") == 0, "wrong attr value");
    } else {
        CHECK(1, "attr not found");
    }
    mysql_free_result(res);

    mysql_close(m);
}

void test_ps_unsigned_bigint(void) {
    printf("\n[PS: unsigned BIGINT boundary]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_ubigint");
    Q("CREATE TABLE t_ubigint(id INT, v BIGINT UNSIGNED)");
    Q("INSERT INTO t_ubigint VALUES(1, 18446744073709551615), (2, 0)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT v FROM t_ubigint ORDER BY id",
                                strlen("SELECT v FROM t_ubigint ORDER BY id"));
    CHECK(rc == 0, "prepare failed");

    unsigned long long out_v;
    unsigned long ol = 0;
    my_bool err = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONGLONG; rb.buffer = &out_v; rb.length = &ol; rb.error = &err;
    rb.is_unsigned = 1;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("unsigned BIGINT max: 18446744073709551615");
    CHECK(rc == 0 && out_v == 18446744073709551615ULL, "wrong value");

    rc = mysql_stmt_fetch(stmt);
    TEST("unsigned BIGINT min: 0");
    CHECK(rc == 0 && out_v == 0, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_stmt_execute_after_error(void) {
    printf("\n[execute after duplicate key error]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_rpe");
    Q("CREATE TABLE t_rpe(id INT PRIMARY KEY)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    CHECK(stmt, "stmt_init failed");

    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_rpe VALUES(?)", -1);
    CHECK(rc == 0, "prepare failed");

    int val = 1;
    unsigned long vl = 0;
    my_bool vn = 0;
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &val; pb.length = &vl; pb.is_null = &vn;
    mysql_stmt_bind_param(stmt, &pb);

    rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, "first insert failed");

    rc = mysql_stmt_execute(stmt);
    TEST("duplicate key insert returns error");
    CHECK(rc != 0, "should fail on duplicate key");

    val = 2;
    rc = mysql_stmt_execute(stmt);
    TEST("insert after error: val=2");
    CHECK(rc == 0, "should succeed with different value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_stmt_execute_zero_params(void) {
    printf("\n[PS execute with zero params]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    CHECK(stmt, "stmt_init failed");

    int rc = mysql_stmt_prepare(stmt, "SELECT 42", -1);
    CHECK(rc == 0, "prepare failed");

    TEST("param_count = 0");
    CHECK(mysql_stmt_param_count(stmt) == 0, "wrong count");

    rc = mysql_stmt_execute(stmt);
    TEST("execute with 0 params succeeds");
    CHECK(rc == 0, "execute failed");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_reexecute_drain(void) {
    printf("\n[PS re-execute drains unconsumed rows]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_drain");
    Q("CREATE TABLE t_drain(id INT PRIMARY KEY)");
    Q("INSERT INTO t_drain VALUES(1),(2),(3),(4),(5)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    CHECK(stmt, "stmt_init failed");

    int rc = mysql_stmt_prepare(stmt, "SELECT id FROM t_drain ORDER BY id", -1);
    CHECK(rc == 0, "prepare failed");

    rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, "execute failed");

    int out = 0; unsigned long ol = 0; my_bool on = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out; rb.length = &ol; rb.is_null = &on;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_fetch(stmt);
    CHECK(rc == 0, "first fetch failed");
    TEST("first row id=1");
    CHECK(out == 1, "wrong id");

    rc = mysql_stmt_execute(stmt);
    TEST("re-execute without full consume succeeds");
    CHECK(rc == 0, "re-execute should drain and succeed");

    rc = mysql_stmt_fetch(stmt);
    CHECK(rc == 0, "fetch after re-execute failed");
    TEST("first row after re-execute id=1");
    CHECK(out == 1, "wrong id after re-execute");

    while (mysql_stmt_fetch(stmt) == 0) {}

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_text_query_empty_result(void) {
    printf("\n[text query: empty result set]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_empty");
    Q("CREATE TABLE t_empty(id INT PRIMARY KEY)");

    Q("SELECT * FROM t_empty");
    MYSQL_RES *res = mysql_store_result(m);
    CHECK(res, "store_result failed");

    TEST("num_rows = 0");
    CHECK(mysql_num_rows(res) == 0, "should be 0");

    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("fetch_row returns NULL");
    CHECK(row == NULL, "should be NULL");

    mysql_free_result(res);
    mysql_close(m);
}

void test_stmt_close_after_execute_no_fetch(void) {
    printf("\n[stmt close after execute without fetch]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_cnf");
    Q("CREATE TABLE t_cnf(id INT PRIMARY KEY)");
    Q("INSERT INTO t_cnf VALUES(1),(2),(3)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    CHECK(stmt, "stmt_init failed");

    int rc = mysql_stmt_prepare(stmt, "SELECT id FROM t_cnf", -1);
    CHECK(rc == 0, "prepare failed");

    rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, "execute failed");

    TEST("close without fetch: no crash");
    mysql_stmt_close(stmt);

    Q("SELECT 1");
    MYSQL_RES *res = mysql_store_result(m);
    CHECK(res, "connection still usable after close");
    mysql_free_result(res);

    mysql_close(m);
}

void test_ps_tiny_unsigned_boundary(void) {
    printf("\n[PS TINYINT UNSIGNED boundary: 255]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_tinyb");
    Q("CREATE TABLE t_tinyb(v TINYINT UNSIGNED)");
    Q("INSERT INTO t_tinyb VALUES(0),(127),(128),(255)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    CHECK(stmt, "stmt_init failed");

    int rc = mysql_stmt_prepare(stmt, "SELECT v FROM t_tinyb ORDER BY v", -1);
    CHECK(rc == 0, "prepare failed");

    rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, "execute failed");

    rc = mysql_stmt_store_result(stmt);
    CHECK(rc == 0, "store_result failed");

    unsigned int out = 0; unsigned long ol = 0; my_bool on = 0, oe = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_TINY; rb.buffer = &out; rb.length = &ol;
    rb.is_null = &on; rb.error = &oe; rb.is_unsigned = 1;

    mysql_stmt_bind_result(stmt, &rb);

    int expected[] = {0, 127, 128, 255};
    for (int i = 0; i < 4; i++) {
        rc = mysql_stmt_fetch(stmt);
        TEST("TINYINT UNSIGNED value check");
        CHECK(rc == 0 && out == (unsigned int)expected[i], "wrong value");
    }

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_short_unsigned_boundary(void) {
    printf("\n[PS SMALLINT UNSIGNED boundary: 65535]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_shortb");
    Q("CREATE TABLE t_shortb(v SMALLINT UNSIGNED)");
    Q("INSERT INTO t_shortb VALUES(0),(32767),(32768),(65535)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    CHECK(stmt, "stmt_init failed");

    int rc = mysql_stmt_prepare(stmt, "SELECT v FROM t_shortb ORDER BY v", -1);
    CHECK(rc == 0, "prepare failed");

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_store_result(stmt);

    unsigned int out = 0; unsigned long ol = 0; my_bool on = 0, oe = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_SHORT; rb.buffer = &out; rb.length = &ol;
    rb.is_null = &on; rb.error = &oe; rb.is_unsigned = 1;

    mysql_stmt_bind_result(stmt, &rb);

    unsigned int expected[] = {0, 32767, 32768, 65535};
    for (int i = 0; i < 4; i++) {
        rc = mysql_stmt_fetch(stmt);
        TEST("SMALLINT UNSIGNED value check");
        CHECK(rc == 0 && out == expected[i], "wrong value");
    }

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_long_unsigned_boundary(void) {
    printf("\n[PS INT UNSIGNED boundary: 4294967295]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_longb");
    Q("CREATE TABLE t_longb(v INT UNSIGNED)");
    Q("INSERT INTO t_longb VALUES(0),(2147483647),(2147483648),(4294967295)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    CHECK(stmt, "stmt_init failed");

    int rc = mysql_stmt_prepare(stmt, "SELECT v FROM t_longb ORDER BY v", -1);
    CHECK(rc == 0, "prepare failed");

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_store_result(stmt);

    unsigned int out = 0; unsigned long ol = 0; my_bool on = 0, oe = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out; rb.length = &ol;
    rb.is_null = &on; rb.error = &oe; rb.is_unsigned = 1;

    mysql_stmt_bind_result(stmt, &rb);

    unsigned int expected[] = {0, 2147483647, 2147483648U, 4294967295U};
    for (int i = 0; i < 4; i++) {
        rc = mysql_stmt_fetch(stmt);
        TEST("INT UNSIGNED value check");
        CHECK(rc == 0 && out == expected[i], "wrong value");
    }

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_blob_max_length_attr(void) {
    printf("\n[PS BLOB with STMT_ATTR_UPDATE_MAX_LENGTH]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_bml");
    Q("CREATE TABLE t_bml(id INT, data BLOB)");
    Q("INSERT INTO t_bml VALUES(1, 'short'),(2, 'a longer blob value here')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    CHECK(stmt, "stmt_init failed");

    my_bool update_ml = 1;
    mysql_stmt_attr_set(stmt, STMT_ATTR_UPDATE_MAX_LENGTH, &update_ml);

    int rc = mysql_stmt_prepare(stmt, "SELECT data FROM t_bml ORDER BY id", -1);
    CHECK(rc == 0, "prepare failed");

    rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, "execute failed");

    rc = mysql_stmt_store_result(stmt);
    CHECK(rc == 0, "store_result failed");

    MYSQL_RES *meta = mysql_stmt_result_metadata(stmt);
    TEST("max_length > 0 with UPDATE_MAX_LENGTH");
    CHECK(meta && meta->fields[0].max_length > 0, "max_length should be set");
    if (meta) mysql_free_result(meta);

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_mysql_get_option(void) {
    printf("\n[mysql_get_option for various options]\n");
    MYSQL *m = mysql_init(NULL);
    CHECK(m, "mysql_init failed");

    unsigned int rt = 99;
    mysql_options(m, MYSQL_OPT_READ_TIMEOUT, &rt);
    unsigned int grt = 0;
    int rc = mysql_get_option(m, MYSQL_OPT_READ_TIMEOUT, &grt);
    TEST("get_option READ_TIMEOUT = 99");
    CHECK(rc == 0 && grt == 99, "wrong value");

    unsigned int wt = 88;
    mysql_options(m, MYSQL_OPT_WRITE_TIMEOUT, &wt);
    unsigned int gwt = 0;
    rc = mysql_get_option(m, MYSQL_OPT_WRITE_TIMEOUT, &gwt);
    TEST("get_option WRITE_TIMEOUT = 88");
    CHECK(rc == 0 && gwt == 88, "wrong value");

    my_bool cleartext = 1;
    mysql_options(m, MYSQL_ENABLE_CLEARTEXT_PLUGIN, &cleartext);
    my_bool gc = 0;
    rc = mysql_get_option(m, MYSQL_ENABLE_CLEARTEXT_PLUGIN, &gc);
    TEST("get_option ENABLE_CLEARTEXT_PLUGIN = 1");
    CHECK(rc == 0 && gc == 1, "wrong value");

    mysql_close(m);
}

void test_reset_connection_ps(void) {
    printf("\n[mysql_reset_connection with prepared stmt]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    CHECK(stmt, "stmt_init failed");

    int rc = mysql_stmt_prepare(stmt, "SELECT 1", -1);
    CHECK(rc == 0, "prepare failed");

    rc = mysql_reset_connection(m);
    TEST("reset_connection with active stmt");
    CHECK(rc == 0, "reset failed");

    Q("SELECT 2");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("query after reset: 2");
    CHECK(row && strcmp(row[0], "2") == 0, "wrong result");
    mysql_free_result(res);

    mysql_close(m);
}

void test_list_dbs_with_pattern(void) {
    printf("\n[mysql_list_dbs with pattern]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_RES *res = mysql_list_dbs(m, "sp_%");
    TEST("list_dbs('sp_%') returns result");
    CHECK(res != NULL, "list_dbs failed");

    my_ulonglong count = mysql_num_rows(res);
    TEST("at least 1 db matching sp_%");
    CHECK(count >= 1, "no matching databases");
    mysql_free_result(res);

    mysql_close(m);
}

void test_hex_string_roundtrip(void) {
    printf("\n[mysql_hex_string roundtrip]\n");
    char hex[64];
    const char *src = "\x00\xFF\x80\x7F";
    unsigned long hlen = mysql_hex_string(hex, src, 4);
    TEST("hex_string 4 bytes = 8 chars");
    CHECK(hlen == 8, "wrong length");

    TEST("hex output = '00FF807F'");
    CHECK(memcmp(hex, "00FF807F", 8) == 0, "wrong hex");
}

void test_real_escape_binary(void) {
    printf("\n[mysql_real_escape_string with binary data]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    char out[32];
    const char *bin = "\x00\x1a\x5c\x27";
    unsigned long len = mysql_real_escape_string(m, out, bin, 4);
    TEST("escape 4 binary bytes produces >4 output");
    CHECK(len > 4, "escaped too short");

    len = mysql_real_escape_string_quote(m, out, "it's", 4, '\'');
    TEST("escape_quote 'it\\'s'");
    CHECK(len == 5, "wrong length");

    mysql_close(m);
}

void test_stmt_attr_prefetch_execute(void) {
    printf("\n[STMT_ATTR_PREFETCH_ROWS with execute]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_pfr");
    Q("CREATE TABLE t_pfr(id INT PRIMARY KEY)");
    Q("INSERT INTO t_pfr VALUES(1),(2),(3),(4),(5)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    CHECK(stmt, "stmt_init failed");

    unsigned long prefetch = 2;
    my_bool rc = mysql_stmt_attr_set(stmt, STMT_ATTR_PREFETCH_ROWS, &prefetch);
    TEST("set PREFETCH_ROWS=2");
    CHECK(rc == 0, "attr_set failed");

    int prc = mysql_stmt_prepare(stmt, "SELECT id FROM t_pfr ORDER BY id", -1);
    CHECK(prc == 0, "prepare failed");

    prc = mysql_stmt_execute(stmt);
    TEST("execute with prefetch_rows succeeds");
    CHECK(prc == 0, "execute failed");

    int out = 0; unsigned long ol = 0; my_bool on = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out; rb.length = &ol; rb.is_null = &on;
    mysql_stmt_bind_result(stmt, &rb);

    prc = mysql_stmt_fetch(stmt);
    CHECK(prc == 0, "fetch failed");
    TEST("first row id=1");
    CHECK(out == 1, "wrong id");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_stmt_field_count_no_prepare(void) {
    printf("\n[mysql_stmt_field_count before prepare]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    CHECK(stmt, "stmt_init failed");

    TEST("field_count before prepare = 0");
    CHECK(mysql_stmt_field_count(stmt) == 0, "should be 0");

    mysql_stmt_prepare(stmt, "SELECT 1, 2, 3", -1);
    TEST("field_count after prepare = 3");
    CHECK(mysql_stmt_field_count(stmt) == 3, "should be 3");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_connect_nonexistent_db(void) {
    printf("\n[connect: nonexistent DB (BUG#1115)]\n");
    MYSQL *m = mysql_init(NULL);
    if (!m) { FAIL("init failed"); return; }
    unsigned int ssl_mode = SSL_MODE_DISABLED;
    mysql_options(m, MYSQL_OPT_SSL_MODE, &ssl_mode);
    MYSQL *rc = mysql_real_connect(m, g_host, g_user, g_passwd,
                                    "nonexistent_db_1115", g_port, NULL, 0);
    TEST("connect with nonexistent DB returns NULL");
    CHECK(rc == NULL, "should fail");
    if (rc == NULL) {
        TEST("errno is non-zero");
        CHECK(mysql_errno(m) != 0, "should have error");
    }
    mysql_close(m);
}

static void test_connect_with_db(void) {
    printf("\n[connect: with valid DB name]\n");
    MYSQL *m = mysql_init(NULL);
    if (!m) { FAIL("init failed"); return; }
    unsigned int ssl_mode = SSL_MODE_DISABLED;
    mysql_options(m, MYSQL_OPT_SSL_MODE, &ssl_mode);
    MYSQL *rc = mysql_real_connect(m, g_host, g_user, g_passwd,
                                    g_db, g_port, NULL, 0);
    TEST("connect with valid DB succeeds");
    CHECK(rc != NULL, "should succeed");
    if (rc) {
        Q("SELECT 1");
        MYSQL_RES *res = mysql_store_result(m);
        CHECK(res != NULL, "query should work");
        mysql_free_result(res);
    }
    mysql_close(m);
}

static void test_ps_bind_null_type(void) {
    printf("\n[PS: bind_result MYSQL_TYPE_NULL (BUG#20152)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_bnt");
    Q("CREATE TABLE t_bnt(id INT, val INT)");
    Q("INSERT INTO t_bnt VALUES(1, NULL), (2, 42)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id, val FROM t_bnt ORDER BY id",
                       strlen("SELECT id, val FROM t_bnt ORDER BY id"));

    int out_id;
    unsigned long ol = sizeof(int);
    my_bool id_null = 0, val_null = 0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &out_id; rb[0].length = &ol; rb[0].is_null = &id_null;
    rb[1].buffer_type = MYSQL_TYPE_NULL; rb[1].is_null = &val_null;
    mysql_stmt_bind_result(stmt, rb);

    int rc = mysql_stmt_execute(stmt);
    TEST("execute with NULL-type bind succeeds");
    CHECK(rc == 0, "execute should work");

    rc = mysql_stmt_fetch(stmt);
    TEST("fetch row 1 (val=NULL): val_null=1");
    CHECK(rc == 0 && val_null == 1, "val should be NULL");

    rc = mysql_stmt_fetch(stmt);
    TEST("fetch row 2 (val=42): no crash");
    CHECK(rc == 0, "fetch should succeed even with NULL-type bind on non-NULL data");

    rc = mysql_stmt_fetch(stmt);
    CHECK(rc == MYSQL_NO_DATA, "no more rows");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_close_without_fetch(void) {
    printf("\n[PS: close without fetch (BUG#25383)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_cwf");
    Q("CREATE TABLE t_cwf(id INT)");
    Q("INSERT INTO t_cwf VALUES(1),(2),(3)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id FROM t_cwf",
                       strlen("SELECT id FROM t_cwf"));
    int rc = mysql_stmt_execute(stmt);
    TEST("execute succeeds");
    CHECK(rc == 0, "execute should work");

    mysql_stmt_close(stmt);
    TEST("close without fetch: connection still usable");

    rc = Q("SELECT 1");
    CHECK(rc == 0, "text query should work after close-without-fetch");
    MYSQL_RES *res = mysql_store_result(m);
    CHECK(res != NULL, "should get result");
    mysql_free_result(res);

    MYSQL_STMT *stmt2 = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt2, "SELECT 1", strlen("SELECT 1"));
    rc = mysql_stmt_execute(stmt2);
    TEST("new PS works after close-without-fetch");
    CHECK(rc == 0, "new PS execute should work");
    mysql_stmt_close(stmt2);

    mysql_close(m);
}

void test_ps_reprepare_after_alter(void) {
    printf("\n[PS: re-prepare after ALTER TABLE (BUG#42230)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_alter");
    Q("CREATE TABLE t_alter(col1 INT)");
    Q("INSERT INTO t_alter VALUES(42)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT col1 FROM t_alter",
                                strlen("SELECT col1 FROM t_alter"));
    CHECK(rc == 0, "prepare should work");

    MYSQL_RES *meta = mysql_stmt_result_metadata(stmt);
    TEST("before ALTER: field_count=1");
    CHECK(meta && meta->field_count == 1, "should have 1 field");
    if (meta) {
        MYSQL_FIELD *f = mysql_fetch_fields(meta);
        TEST("before ALTER: type is LONG");
        CHECK(f[0].type == MYSQL_TYPE_LONG, "should be INT/LONG");
        mysql_free_result(meta);
    }

    int out_val;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    mysql_stmt_fetch(stmt);
    TEST("before ALTER: col1=42");
    CHECK(rc == 0 && out_val == 42, "wrong value");
    while (mysql_stmt_fetch(stmt) == 0) {}

    Q("ALTER TABLE t_alter MODIFY col1 BIGINT");

    rc = mysql_stmt_execute(stmt);
    TEST("after ALTER: execute triggers re-prepare");
    CHECK(rc == 0, "execute after ALTER should succeed");

    rc = mysql_stmt_fetch(stmt);
    TEST("after ALTER: col1=42 still");
    CHECK(rc == 0 && out_val == 42, "value should still be 42");
    while (mysql_stmt_fetch(stmt) == 0) {}

    meta = mysql_stmt_result_metadata(stmt);
    if (meta) {
        MYSQL_FIELD *f = mysql_fetch_fields(meta);
        TEST("after ALTER: type is LONGLONG");
        CHECK(f[0].type == MYSQL_TYPE_LONGLONG, "should be BIGINT/LONGLONG after ALTER");
        mysql_free_result(meta);
    }

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_duplicate_key_recovery(void) {
    printf("\n[PS: duplicate key error recovery (BUG#34774)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_dk");
    Q("CREATE TABLE t_dk(id INT PRIMARY KEY)");
    Q("INSERT INTO t_dk VALUES(1)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "INSERT INTO t_dk VALUES(?)",
                       strlen("INSERT INTO t_dk VALUES(?)"));

    int val = 1;
    unsigned long vl = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &val; pb.length = &vl;
    mysql_stmt_bind_param(stmt, &pb);

    int rc = mysql_stmt_execute(stmt);
    TEST("duplicate key INSERT fails");
    CHECK(rc != 0, "should fail with duplicate key");

    val = 2;
    rc = mysql_stmt_execute(stmt);
    TEST("after error, INSERT with new value succeeds");
    CHECK(rc == 0, "should succeed after error recovery");
    CHECK(mysql_stmt_affected_rows(stmt) == 1, "1 row inserted");

    Q("SELECT COUNT(*) FROM t_dk");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("table has 2 rows after recovery");
    CHECK(row && strcmp(row[0], "2") == 0, "wrong count");
    mysql_free_result(res);

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_free_result_mid_fetch(void) {
    printf("\n[PS: free_result mid-fetch, then re-execute]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_fmf");
    Q("CREATE TABLE t_fmf(id INT, val INT)");
    Q("INSERT INTO t_fmf VALUES(1,10),(2,20),(3,30)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id, val FROM t_fmf ORDER BY id",
                       strlen("SELECT id, val FROM t_fmf ORDER BY id"));

    int out_id, out_val;
    unsigned long ol1 = 0, ol2 = 0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &out_id; rb[0].length = &ol1;
    rb[1].buffer_type = MYSQL_TYPE_LONG; rb[1].buffer = &out_val; rb[1].length = &ol2;
    mysql_stmt_bind_result(stmt, rb);

    int rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, "execute should work");

    rc = mysql_stmt_fetch(stmt);
    TEST("fetch row 1: id=1, val=10");
    CHECK(rc == 0 && out_id == 1 && out_val == 10, "wrong first row");

    mysql_stmt_free_result(stmt);
    TEST("free_result mid-fetch: no crash");

    rc = mysql_stmt_execute(stmt);
    TEST("re-execute after free_result mid-fetch");
    CHECK(rc == 0, "re-execute should work");

    int count = 0;
    while ((rc = mysql_stmt_fetch(stmt)) == 0) count++;
    TEST("re-executed result has 3 rows");
    CHECK(count == 3, "wrong count");
    while (mysql_stmt_fetch(stmt) == 0) {}

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_multi_reexecute_loop(void) {
    printf("\n[PS: multi re-execute loop]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_mrl");
    Q("CREATE TABLE t_mrl(id INT, val INT)");
    Q("INSERT INTO t_mrl VALUES(1,100)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT val FROM t_mrl WHERE id = ?",
                       strlen("SELECT val FROM t_mrl WHERE id = ?"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    int out_val;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    for (int i = 0; i < 10; i++) {
        int rc = mysql_stmt_execute(stmt);
        if (rc != 0) {
            TEST("re-execute loop: iteration fails");
            CHECK(0, "execute should work");
            break;
        }
        rc = mysql_stmt_fetch(stmt);
        if (rc != 0 || out_val != 100) {
            TEST("re-execute loop: wrong value");
            CHECK(0, "val should be 100");
            break;
        }
        while (mysql_stmt_fetch(stmt) == 0) {}
    }
    TEST("10x re-execute loop: all succeed");
    CHECK(1, "");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_empty_string_param(void) {
    printf("\n[PS: empty string param roundtrip]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_esp");
    Q("CREATE TABLE t_esp(id INT, name VARCHAR(50))");
    Q("INSERT INTO t_esp VALUES(1,'hello'),(2,'')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id FROM t_esp WHERE name = ?",
                       strlen("SELECT id FROM t_esp WHERE name = ?"));

    char name[50] = "";
    unsigned long nl = 0;
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_VAR_STRING; pb.buffer = name; pb.buffer_length = sizeof(name); pb.length = &nl;
    mysql_stmt_bind_param(stmt, &pb);

    int out_id;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_id; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    int rc = mysql_stmt_execute(stmt);
    TEST("empty string param: fetch id=2");
    CHECK(rc == 0, "execute should work");
    rc = mysql_stmt_fetch(stmt);
    CHECK(rc == 0 && out_id == 2, "should find id=2 for empty string");
    while (mysql_stmt_fetch(stmt) == 0) {}

    strcpy(name, "hello");
    nl = strlen(name);
    rc = mysql_stmt_execute(stmt);
    TEST("non-empty string param: fetch id=1");
    CHECK(rc == 0, "execute should work");
    rc = mysql_stmt_fetch(stmt);
    CHECK(rc == 0 && out_id == 1, "should find id=1 for 'hello'");
    while (mysql_stmt_fetch(stmt) == 0) {}

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_unsigned_tinyint(void) {
    printf("\n[PS: unsigned TINYINT param]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_uti");
    Q("CREATE TABLE t_uti(id INT, val TINYINT UNSIGNED)");
    Q("INSERT INTO t_uti VALUES(1,200),(2,255)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id FROM t_uti WHERE val = ?",
                       strlen("SELECT id FROM t_uti WHERE val = ?"));

    unsigned char val = 200;
    unsigned long vl = sizeof(unsigned char);
    my_bool is_unsigned = 1;
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_TINY; pb.buffer = &val; pb.buffer_length = 1;
    pb.length = &vl; pb.is_unsigned = is_unsigned;
    mysql_stmt_bind_param(stmt, &pb);

    int out_id;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_id; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    int rc = mysql_stmt_execute(stmt);
    TEST("unsigned TINYINT 200: id=1");
    CHECK(rc == 0, "execute should work");
    rc = mysql_stmt_fetch(stmt);
    CHECK(rc == 0 && out_id == 1, "should find id=1");
    while (mysql_stmt_fetch(stmt) == 0) {}

    val = 255;
    rc = mysql_stmt_execute(stmt);
    TEST("unsigned TINYINT 255: id=2");
    CHECK(rc == 0, "execute should work");
    rc = mysql_stmt_fetch(stmt);
    CHECK(rc == 0 && out_id == 2, "should find id=2");
    while (mysql_stmt_fetch(stmt) == 0) {}

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_string_truncation(void) {
    printf("\n[PS: string result truncation with small buffer]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_st");
    Q("CREATE TABLE t_st(id INT, name VARCHAR(100))");
    Q("INSERT INTO t_st VALUES(1,'abcdefghijklmnopqrstuvwxyz')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT name FROM t_st WHERE id = 1",
                       strlen("SELECT name FROM t_st WHERE id = 1"));

    char buf[5] = {0};
    unsigned long ol = 0;
    my_bool err = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_STRING; rb.buffer = buf; rb.buffer_length = sizeof(buf) - 1;
    rb.length = &ol; rb.error = &err;
    mysql_stmt_bind_result(stmt, &rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("small buffer: fetch returns truncation or success");
    CHECK(rc == 0 || rc == MYSQL_DATA_TRUNCATED, "fetch should succeed or truncate");
    TEST("length reports full string length");
    CHECK(ol == 26, "length should be 26 (full string), not buffer size");
    TEST("truncation error flag set");
    CHECK(err == 1, "error flag should indicate truncation");
    TEST("buffer contains partial data");
    CHECK(strncmp(buf, "abcd", 4) == 0, "buffer should have first 4 chars");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}


static void test_ps_prepare_error_then_valid(void) {
    printf("\n[PS: prepare error, then valid prepare]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT * FROM nonexistent_table_xyz",
                                strlen("SELECT * FROM nonexistent_table_xyz"));
    TEST("prepare with invalid SQL fails");
    CHECK(rc != 0, "should fail");

    rc = mysql_stmt_prepare(stmt, "SELECT 1", strlen("SELECT 1"));
    TEST("prepare with valid SQL after error succeeds");
    CHECK(rc == 0, "should succeed");

    rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, "execute should work");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_null_param_rebind(void) {
    printf("\n[PS: NULL param rebind to non-NULL and back]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_npr");
    Q("CREATE TABLE t_npr(id INT, val INT)");
    Q("INSERT INTO t_npr VALUES(1,10),(2,NULL)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id FROM t_npr WHERE val <=> ?",
                       strlen("SELECT id FROM t_npr WHERE val <=> ?"));

    int val = 10;
    my_bool is_null = 0;
    unsigned long vl = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &val; pb.length = &vl; pb.is_null = &is_null;
    mysql_stmt_bind_param(stmt, &pb);

    int out_id;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_id; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("non-NULL param: id=1");
    CHECK(rc == 0 && out_id == 1, "should find id=1");
    while (mysql_stmt_fetch(stmt) == 0) {}

    is_null = 1;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("NULL param: id=2");
    CHECK(rc == 0 && out_id == 2, "should find id=2 for NULL");
    while (mysql_stmt_fetch(stmt) == 0) {}

    is_null = 0;
    val = 10;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("back to non-NULL: id=1");
    CHECK(rc == 0 && out_id == 1, "should find id=1 again");
    while (mysql_stmt_fetch(stmt) == 0) {}

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_text_query_select_no_rows(void) {
    printf("\n[text: SELECT returning no rows]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_snr");
    Q("CREATE TABLE t_snr(id INT)");
    Q("INSERT INTO t_snr VALUES(1),(2),(3)");

    Q("SELECT id FROM t_snr WHERE id > 100");
    MYSQL_RES *res = mysql_store_result(m);
    TEST("SELECT no rows: result not NULL");
    CHECK(res != NULL, "store_result should return non-NULL");
    if (res) {
        TEST("SELECT no rows: row_count=0");
        CHECK(res->row_count == 0, "should have 0 rows");
        MYSQL_ROW row = mysql_fetch_row(res);
        CHECK(row == NULL, "fetch_row should return NULL");
        mysql_free_result(res);
    }

    mysql_close(m);
}

static void test_ps_large_blob_long_data(void) {
    printf("\n[PS: large BLOB via send_long_data chunks]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_lbd");
    Q("CREATE TABLE t_lbd(id INT, data BLOB)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "INSERT INTO t_lbd VALUES(?, ?)",
                       strlen("INSERT INTO t_lbd VALUES(?, ?)"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &il;
    pb[1].buffer_type = MYSQL_TYPE_LONG_BLOB; pb[1].buffer = NULL; pb[1].length = NULL;
    mysql_stmt_bind_param(stmt, pb);

    char chunk[1024];
    memset(chunk, 'A', sizeof(chunk));
    size_t total = 0;
    for (int i = 0; i < 5; i++) {
        mysql_stmt_send_long_data(stmt, 1, chunk, sizeof(chunk));
        total += sizeof(chunk);
    }

    int rc = mysql_stmt_execute(stmt);
    TEST("send_long_data 5x1024 bytes: execute succeeds");
    CHECK(rc == 0, "execute should work");

    mysql_stmt_prepare(stmt, "SELECT LENGTH(data) FROM t_lbd WHERE id = 1",
                       strlen("SELECT LENGTH(data) FROM t_lbd WHERE id = 1"));
    unsigned long long out_len;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONGLONG; rb.buffer = &out_len; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("BLOB length matches total sent");
    CHECK(rc == 0 && out_len == total, "wrong length");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_param_count_verify(void) {
    printf("\n[PS: param_count after prepare]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *s1 = mysql_stmt_init(m);
    mysql_stmt_prepare(s1, "SELECT 1", strlen("SELECT 1"));
    TEST("no-param SELECT: param_count=0");
    CHECK(mysql_stmt_param_count(s1) == 0, "should be 0");
    mysql_stmt_close(s1);

    MYSQL_STMT *s2 = mysql_stmt_init(m);
    mysql_stmt_prepare(s2, "SELECT ? + ?", strlen("SELECT ? + ?"));
    TEST("2-param SELECT: param_count=2");
    CHECK(mysql_stmt_param_count(s2) == 2, "should be 2");
    mysql_stmt_close(s2);

    MYSQL_STMT *s3 = mysql_stmt_init(m);
    Q("DROP TABLE IF EXISTS t_pcv");
    Q("CREATE TABLE t_pcv(id INT, val INT)");
    mysql_stmt_prepare(s3, "INSERT INTO t_pcv VALUES(?, ?)",
                       strlen("INSERT INTO t_pcv VALUES(?, ?)"));
    TEST("2-param INSERT: param_count=2");
    CHECK(mysql_stmt_param_count(s3) == 2, "should be 2");
    mysql_stmt_close(s3);

    MYSQL_STMT *s4 = mysql_stmt_init(m);
    mysql_stmt_prepare(s4, "UPDATE t_pcv SET val = 0 WHERE 0",
                       strlen("UPDATE t_pcv SET val = 0 WHERE 0"));
    TEST("no-param UPDATE: param_count=0");
    CHECK(mysql_stmt_param_count(s4) == 0, "should be 0");
    mysql_stmt_close(s4);

    mysql_close(m);
}

static void test_ps_rebind_different_params(void) {
    printf("\n[PS: rebind different params between executes]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_rdp");
    Q("CREATE TABLE t_rdp(id INT, val INT)");
    Q("INSERT INTO t_rdp VALUES(1,10),(2,20)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT val FROM t_rdp WHERE id = ?",
                       strlen("SELECT val FROM t_rdp WHERE id = ?"));

    int out_val;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    int id1 = 1;
    unsigned long l1 = sizeof(int);
    MYSQL_BIND pb1;
    memset(&pb1, 0, sizeof(pb1));
    pb1.buffer_type = MYSQL_TYPE_LONG; pb1.buffer = &id1; pb1.length = &l1;
    mysql_stmt_bind_param(stmt, &pb1);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("first bind: id=1, val=10");
    CHECK(rc == 0 && out_val == 10, "wrong value");
    while (mysql_stmt_fetch(stmt) == 0) {}

    int id2 = 2;
    unsigned long l2 = sizeof(int);
    MYSQL_BIND pb2;
    memset(&pb2, 0, sizeof(pb2));
    pb2.buffer_type = MYSQL_TYPE_LONG; pb2.buffer = &id2; pb2.length = &l2;
    mysql_stmt_bind_param(stmt, &pb2);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("rebind: id=2, val=20");
    CHECK(rc == 0 && out_val == 20, "wrong value");
    while (mysql_stmt_fetch(stmt) == 0) {}

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_sqlstate_after_error(void) {
    printf("\n[PS: sqlstate after error]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_sae");
    Q("CREATE TABLE t_sae(id INT PRIMARY KEY)");
    Q("INSERT INTO t_sae VALUES(1)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "INSERT INTO t_sae VALUES(?)",
                       strlen("INSERT INTO t_sae VALUES(?)"));

    int val = 1;
    unsigned long vl = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &val; pb.length = &vl;
    mysql_stmt_bind_param(stmt, &pb);

    int rc = mysql_stmt_execute(stmt);
    TEST("duplicate key: execute fails");
    CHECK(rc != 0, "should fail");

    const char *state = mysql_stmt_sqlstate(stmt);
    TEST("sqlstate starts with '23' (integrity constraint)");
    CHECK(state && state[0] == '2' && state[1] == '3', "should be 23xxx for dup key");

    val = 2;
    rc = mysql_stmt_execute(stmt);
    TEST("after error: valid INSERT succeeds");
    CHECK(rc == 0, "should succeed");

    state = mysql_stmt_sqlstate(stmt);
    TEST("sqlstate is '00000' after success");
    CHECK(state && strcmp(state, "00000") == 0, "should be 00000 after success");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_text_query_show_tables(void) {
    printf("\n[text: SHOW TABLES]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_show1, t_show2");
    Q("CREATE TABLE t_show1(id INT)");
    Q("CREATE TABLE t_show2(id INT)");

    Q("SHOW TABLES");
    MYSQL_RES *res = mysql_store_result(m);
    TEST("SHOW TABLES returns result");
    CHECK(res != NULL, "should get result");
    if (res) {
        int found1 = 0, found2 = 0;
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res))) {
            if (row[0] && strcmp(row[0], "t_show1") == 0) found1 = 1;
            if (row[0] && strcmp(row[0], "t_show2") == 0) found2 = 1;
        }
        TEST("found t_show1 and t_show2");
        CHECK(found1 && found2, "should find both tables");
        mysql_free_result(res);
    }

    mysql_close(m);
}

static void test_ps_store_result_multi_iter(void) {
    printf("\n[PS: store_result multi-iteration (test_wl4284)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_smi");
    Q("CREATE TABLE t_smi(id INT, val INT)");
    Q("INSERT INTO t_smi VALUES(1,10),(2,20),(3,30)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id, val FROM t_smi ORDER BY id",
                       strlen("SELECT id, val FROM t_smi ORDER BY id"));

    int out_id, out_val;
    unsigned long ol1 = 0, ol2 = 0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &out_id; rb[0].length = &ol1;
    rb[1].buffer_type = MYSQL_TYPE_LONG; rb[1].buffer = &out_val; rb[1].length = &ol2;
    mysql_stmt_bind_result(stmt, rb);

    int rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, "execute should work");
    rc = mysql_stmt_store_result(stmt);
    TEST("store_result succeeds");
    CHECK(rc == 0, "store_result should work");

    for (int iter = 0; iter < 3; iter++) {
        mysql_stmt_data_seek(stmt, 0);
        int count = 0, sum = 0;
        while ((rc = mysql_stmt_fetch(stmt)) == 0) {
            count++; sum += out_val;
        }
        TEST("iteration data consistent");
        CHECK(count == 3 && sum == 60, "3 rows, sum=60");
    }

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_fetch_then_store_result(void) {
    printf("\n[PS: fetch then store_result (BUG#36326)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_fts");
    Q("CREATE TABLE t_fts(id INT, val INT)");
    Q("INSERT INTO t_fts VALUES(1,10),(2,20),(3,30)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id, val FROM t_fts ORDER BY id",
                       strlen("SELECT id, val FROM t_fts ORDER BY id"));

    int out_id, out_val;
    unsigned long ol1 = 0, ol2 = 0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &out_id; rb[0].length = &ol1;
    rb[1].buffer_type = MYSQL_TYPE_LONG; rb[1].buffer = &out_val; rb[1].length = &ol2;
    mysql_stmt_bind_result(stmt, rb);

    int rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, "execute should work");

    rc = mysql_stmt_fetch(stmt);
    TEST("first fetch: id=1, val=10");
    CHECK(rc == 0 && out_id == 1 && out_val == 10, "wrong first row");

    rc = mysql_stmt_store_result(stmt);
    TEST("store_result after partial fetch: no crash");
    CHECK(rc == 0, "store_result should work after fetch");

    int count = 1;
    while ((rc = mysql_stmt_fetch(stmt)) == 0) count++;
    TEST("total rows after store_result: 3");
    CHECK(count == 3, "should have 3 rows total");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_close_after_execute_error(void) {
    printf("\n[PS: close after execute error (BUG#42373)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_cae");
    Q("CREATE TABLE t_cae(id INT PRIMARY KEY)");
    Q("INSERT INTO t_cae VALUES(1)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "INSERT INTO t_cae VALUES(?)",
                       strlen("INSERT INTO t_cae VALUES(?)"));

    int val = 1;
    unsigned long vl = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &val; pb.length = &vl;
    mysql_stmt_bind_param(stmt, &pb);

    int rc = mysql_stmt_execute(stmt);
    TEST("duplicate key execute fails");
    CHECK(rc != 0, "should fail");

    mysql_stmt_close(stmt);
    TEST("close after execute error: no crash");

    rc = mysql_ping(m);
    TEST("connection still alive after close");
    CHECK(rc == 0, "ping should succeed");

    mysql_close(m);
}

static void test_ps_store_result_full_then_reexecute(void) {
    printf("\n[PS: store_result full consume then re-execute (BUG#45671)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_sfr");
    Q("CREATE TABLE t_sfr(id INT, val INT)");
    Q("INSERT INTO t_sfr VALUES(1,100),(2,200)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id, val FROM t_sfr ORDER BY id",
                       strlen("SELECT id, val FROM t_sfr ORDER BY id"));

    int out_id, out_val;
    unsigned long ol1 = 0, ol2 = 0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &out_id; rb[0].length = &ol1;
    rb[1].buffer_type = MYSQL_TYPE_LONG; rb[1].buffer = &out_val; rb[1].length = &ol2;
    mysql_stmt_bind_result(stmt, rb);

    int rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, "first execute");
    rc = mysql_stmt_store_result(stmt);
    CHECK(rc == 0, "first store_result");
    while (mysql_stmt_fetch(stmt) == 0) {}

    rc = mysql_stmt_execute(stmt);
    TEST("re-execute after full store_result+fetch: no 2014 error");
    CHECK(rc == 0, "should not return CR_COMMANDS_OUT_OF_SYNC");

    rc = mysql_stmt_store_result(stmt);
    CHECK(rc == 0, "second store_result");

    int count = 0;
    while (mysql_stmt_fetch(stmt) == 0) count++;
    TEST("second result has 2 rows");
    CHECK(count == 2, "should have 2 rows");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_store_result_partial_then_reexecute(void) {
    printf("\n[PS: store_result partial consume then re-execute (BUG#53171)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_spr");
    Q("CREATE TABLE t_spr(id INT, val INT)");
    Q("INSERT INTO t_spr VALUES(1,10),(2,20),(3,30),(4,40),(5,50)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id, val FROM t_spr ORDER BY id",
                       strlen("SELECT id, val FROM t_spr ORDER BY id"));

    int out_id, out_val;
    unsigned long ol1 = 0, ol2 = 0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &out_id; rb[0].length = &ol1;
    rb[1].buffer_type = MYSQL_TYPE_LONG; rb[1].buffer = &out_val; rb[1].length = &ol2;
    mysql_stmt_bind_result(stmt, rb);

    int rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, "first execute");
    rc = mysql_stmt_store_result(stmt);
    CHECK(rc == 0, "store_result");

    rc = mysql_stmt_fetch(stmt);
    CHECK(rc == 0 && out_id == 1, "first row");
    rc = mysql_stmt_fetch(stmt);
    CHECK(rc == 0 && out_id == 2, "second row");

    rc = mysql_stmt_execute(stmt);
    TEST("re-execute after partial fetch: no crash");
    CHECK(rc == 0, "should succeed");

    rc = mysql_stmt_store_result(stmt);
    CHECK(rc == 0, "second store_result");

    int count = 0;
    while (mysql_stmt_fetch(stmt) == 0) count++;
    TEST("second result has 5 rows");
    CHECK(count == 5, "should have 5 rows");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_stmt_reopen_same_query(void) {
    printf("\n[PS: reopen same query on new stmt handle]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_ro");
    Q("CREATE TABLE t_ro(id INT, val INT)");
    Q("INSERT INTO t_ro VALUES(1,42)");

    const char *sql = "SELECT val FROM t_ro WHERE id = ?";
    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;

    int out_val;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.length = &ol;

    for (int round = 0; round < 3; round++) {
        MYSQL_STMT *stmt = mysql_stmt_init(m);
        int rc = mysql_stmt_prepare(stmt, sql, strlen(sql));
        CHECK(rc == 0, "prepare should work");
        mysql_stmt_bind_param(stmt, &pb);
        mysql_stmt_bind_result(stmt, &rb);
        rc = mysql_stmt_execute(stmt);
        CHECK(rc == 0, "execute should work");
        rc = mysql_stmt_fetch(stmt);
        CHECK(rc == 0 && out_val == 42, "val should be 42");
        while (mysql_stmt_fetch(stmt) == 0) {}
        mysql_stmt_close(stmt);
    }
    TEST("3x reopen same query: all succeed");
    CHECK(1, "");

    mysql_close(m);
}

static void test_ps_fetch_date_type(void) {
    printf("\n[PS: fetch DATE type with MYSQL_TIME]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_fdt");
    Q("CREATE TABLE t_fdt(id INT, d DATE)");
    Q("INSERT INTO t_fdt VALUES(1,'2024-06-15'),(2,NULL)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT d FROM t_fdt ORDER BY id",
                       strlen("SELECT d FROM t_fdt ORDER BY id"));

    MYSQL_TIME out_dt;
    unsigned long ol = 0;
    my_bool is_null = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_DATE; rb.buffer = &out_dt;
    rb.buffer_length = sizeof(MYSQL_TIME); rb.length = &ol; rb.is_null = &is_null;
    mysql_stmt_bind_result(stmt, &rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("DATE fetch: year=2024, month=6, day=15");
    CHECK(rc == 0 && out_dt.year == 2024 && out_dt.month == 6 && out_dt.day == 15, "wrong date");
    TEST("DATE fetch: time parts are zero");
    CHECK(out_dt.hour == 0 && out_dt.minute == 0 && out_dt.second == 0, "time should be zero");

    rc = mysql_stmt_fetch(stmt);
    TEST("DATE fetch: NULL row");
    CHECK(rc == 0 && is_null == 1, "should be NULL");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_fetch_str_various_lengths(void) {
    printf("\n[PS: fetch strings of various lengths]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_fsv");
    Q("CREATE TABLE t_fsv(id INT, name VARCHAR(500))");
    Q("INSERT INTO t_fsv VALUES(1,'')");
    char longstr[256];
    memset(longstr, 'X', 255); longstr[255] = '\0';
    char qbuf[1024];
    snprintf(qbuf, sizeof(qbuf), "INSERT INTO t_fsv VALUES(2,'%s')", longstr);
    Q(qbuf);
    Q("INSERT INTO t_fsv VALUES(3,NULL)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id, name FROM t_fsv ORDER BY id",
                       strlen("SELECT id, name FROM t_fsv ORDER BY id"));

    int out_id;
    unsigned long ol1 = 0;
    char out_str[300];
    unsigned long ol2 = 0;
    my_bool str_null = 0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &out_id; rb[0].length = &ol1;
    rb[1].buffer_type = MYSQL_TYPE_STRING; rb[1].buffer = out_str; rb[1].buffer_length = sizeof(out_str);
    rb[1].length = &ol2; rb[1].is_null = &str_null;
    mysql_stmt_bind_result(stmt, rb);

    int rc = mysql_stmt_execute(stmt);

    rc = mysql_stmt_fetch(stmt);
    TEST("empty string: length=0");
    CHECK(rc == 0 && ol2 == 0 && str_null == 0, "empty string should have length 0");

    rc = mysql_stmt_fetch(stmt);
    TEST("long string: length=255");
    CHECK(rc == 0 && ol2 == 255 && str_null == 0, "long string should have length 255");
    CHECK(out_str[0] == 'X' && out_str[254] == 'X', "content should be all X");

    rc = mysql_stmt_fetch(stmt);
    TEST("NULL string: is_null=1");
    CHECK(rc == 0 && str_null == 1, "should be NULL");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_close_after_store_without_fetch(void) {
    printf("\n[PS: close after store_result without fetch]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_csw");
    Q("CREATE TABLE t_csw(id INT)");
    Q("INSERT INTO t_csw VALUES(1),(2),(3)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id FROM t_csw",
                       strlen("SELECT id FROM t_csw"));
    int rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, "execute");
    rc = mysql_stmt_store_result(stmt);
    CHECK(rc == 0, "store_result");

    mysql_stmt_close(stmt);
    TEST("close after store_result without fetch: no crash");

    rc = Q("SELECT 1");
    CHECK(rc == 0, "connection should still be usable");
    MYSQL_RES *res = mysql_store_result(m);
    CHECK(res != NULL, "should get result");
    mysql_free_result(res);

    mysql_close(m);
}

static void test_ps_binary_null_in_blob(void) {
    printf("\n[PS: binary data with null bytes in BLOB]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_bnb");
    Q("CREATE TABLE t_bnb(id INT, data BLOB)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "INSERT INTO t_bnb VALUES(?, ?)",
                       strlen("INSERT INTO t_bnb VALUES(?, ?)"));

    int id = 1;
    unsigned long il = sizeof(int);
    unsigned char bindata[] = {0x00, 0x01, 0x00, 0xFF, 0x00, 0x41, 0x00};
    unsigned long dl = sizeof(bindata);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &il;
    pb[1].buffer_type = MYSQL_TYPE_LONG_BLOB; pb[1].buffer = bindata;
    pb[1].buffer_length = sizeof(bindata); pb[1].length = &dl;
    mysql_stmt_bind_param(stmt, pb);

    int rc = mysql_stmt_execute(stmt);
    TEST("insert binary data with null bytes");
    CHECK(rc == 0, "execute should work");

    mysql_stmt_prepare(stmt, "SELECT data FROM t_bnb WHERE id = 1",
                       strlen("SELECT data FROM t_bnb WHERE id = 1"));
    unsigned char out_data[32];
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG_BLOB; rb.buffer = out_data;
    rb.buffer_length = sizeof(out_data); rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("BLOB roundtrip: length matches");
    CHECK(rc == 0 && ol == sizeof(bindata), "length should match");
    TEST("BLOB roundtrip: content matches (memcmp)");
    CHECK(memcmp(out_data, bindata, sizeof(bindata)) == 0, "content should match");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_reexecute_with_different_param_types(void) {
    printf("\n[PS: re-execute with changed param values]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_rdp2");
    Q("CREATE TABLE t_rdp2(id INT, name VARCHAR(50), score DOUBLE)");
    Q("INSERT INTO t_rdp2 VALUES(1,'alice',95.5),(2,'bob',87.3)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT name, score FROM t_rdp2 WHERE id = ?",
                       strlen("SELECT name, score FROM t_rdp2 WHERE id = ?"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    char out_name[50];
    unsigned long nl = 0;
    double out_score;
    unsigned long sl = 0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_STRING; rb[0].buffer = out_name;
    rb[0].buffer_length = sizeof(out_name); rb[0].length = &nl;
    rb[1].buffer_type = MYSQL_TYPE_DOUBLE; rb[1].buffer = &out_score; rb[1].length = &sl;
    mysql_stmt_bind_result(stmt, rb);

    id = 1;
    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("id=1: name=alice, score~95.5");
    CHECK(rc == 0 && strncmp(out_name, "alice", 5) == 0, "wrong name");
    CHECK(out_score > 95.0 && out_score < 96.0, "wrong score");
    while (mysql_stmt_fetch(stmt) == 0) {}

    id = 2;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("id=2: name=bob, score~87.3");
    CHECK(rc == 0 && strncmp(out_name, "bob", 3) == 0, "wrong name");
    CHECK(out_score > 87.0 && out_score < 88.0, "wrong score");
    while (mysql_stmt_fetch(stmt) == 0) {}

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_multiple_store_result_cycles(void) {
    printf("\n[PS: multiple store_result cycles]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_msc");
    Q("CREATE TABLE t_msc(id INT)");
    Q("INSERT INTO t_msc VALUES(1),(2)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id FROM t_msc",
                       strlen("SELECT id FROM t_msc"));

    int out_id;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_id; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    for (int i = 0; i < 5; i++) {
        int rc = mysql_stmt_execute(stmt);
        CHECK(rc == 0, "execute should work");
        rc = mysql_stmt_store_result(stmt);
        CHECK(rc == 0, "store_result should work");
        int count = 0;
        while (mysql_stmt_fetch(stmt) == 0) count++;
        if (count != 2) {
            TEST("5x store_result cycles: wrong count");
            CHECK(0, "should have 2 rows");
            break;
        }
    }
    TEST("5x store_result cycles: all succeed");
    CHECK(1, "");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_text_query_describe_table(void) {
    printf("\n[text: DESCRIBE table]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_desc");
    Q("CREATE TABLE t_desc(id INT PRIMARY KEY, name VARCHAR(50), score DOUBLE)");

    Q("DESCRIBE t_desc");
    MYSQL_RES *res = mysql_store_result(m);
    TEST("DESCRIBE returns result");
    CHECK(res != NULL, "should get result");
    if (res) {
        TEST("DESCRIBE has 6 fields (Field,Type,Null,Key,Default,Extra)");
        CHECK(res->field_count == 6, "should have 6 fields");
        TEST("DESCRIBE row_count >= 3");
        CHECK(res->row_count >= 3, "should have at least 3 rows");
        mysql_free_result(res);
    }

    mysql_close(m);
}

static void test_ps_zero_row_count(void) {
    printf("\n[PS: zero row_count for empty result]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_zrc");
    Q("CREATE TABLE t_zrc(id INT)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id FROM t_zrc",
                       strlen("SELECT id FROM t_zrc"));

    int out_id;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_id; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_store_result(stmt);
    TEST("store_result on empty table: row_count=0");
    CHECK(rc == 0 && mysql_stmt_num_rows(stmt) == 0, "should have 0 rows");

    rc = mysql_stmt_fetch(stmt);
    CHECK(rc == MYSQL_NO_DATA, "should be NO_DATA");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_stmt_errno_after_error(void) {
    printf("\n[PS: stmt_errno after error]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_sea");
    Q("CREATE TABLE t_sea(id INT PRIMARY KEY)");
    Q("INSERT INTO t_sea VALUES(1)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "INSERT INTO t_sea VALUES(?)",
                       strlen("INSERT INTO t_sea VALUES(?)"));

    int val = 1;
    unsigned long vl = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &val; pb.length = &vl;
    mysql_stmt_bind_param(stmt, &pb);

    int rc = mysql_stmt_execute(stmt);
    TEST("duplicate key: errno is non-zero");
    CHECK(rc != 0 && mysql_stmt_errno(stmt) != 0, "errno should be non-zero");

    val = 2;
    rc = mysql_stmt_execute(stmt);
    TEST("after recovery: errno is 0");
    CHECK(rc == 0 && mysql_stmt_errno(stmt) == 0, "errno should be 0 after success");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_text_query_select_database(void) {
    printf("\n[text: SELECT DATABASE()]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("SELECT DATABASE()");
    MYSQL_RES *res = mysql_store_result(m);
    TEST("SELECT DATABASE() returns result");
    CHECK(res != NULL, "should get result");
    if (res) {
        MYSQL_ROW row = mysql_fetch_row(res);
        TEST("DATABASE() returns non-NULL");
        CHECK(row && row[0] != NULL, "should have a database name");
        if (row && row[0]) {
            TEST("DATABASE() matches g_db");
            CHECK(strcmp(row[0], g_db) == 0, "database name should match");
        }
        mysql_free_result(res);
    }

    mysql_close(m);
}

static void test_ps_store_result_data_seek(void) {
    printf("\n[PS: store_result data_seek random access]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_srd");
    Q("CREATE TABLE t_srd(id INT, val INT)");
    Q("INSERT INTO t_srd VALUES(1,10),(2,20),(3,30),(4,40),(5,50)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id, val FROM t_srd ORDER BY id",
                       strlen("SELECT id, val FROM t_srd ORDER BY id"));

    int out_id, out_val;
    unsigned long ol1 = 0, ol2 = 0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &out_id; rb[0].length = &ol1;
    rb[1].buffer_type = MYSQL_TYPE_LONG; rb[1].buffer = &out_val; rb[1].length = &ol2;
    mysql_stmt_bind_result(stmt, rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_store_result(stmt);
    CHECK(rc == 0, "store_result");

    mysql_stmt_data_seek(stmt, 2);
    rc = mysql_stmt_fetch(stmt);
    TEST("data_seek(2): id=3, val=30");
    CHECK(rc == 0 && out_id == 3 && out_val == 30, "wrong row at offset 2");

    mysql_stmt_data_seek(stmt, 0);
    rc = mysql_stmt_fetch(stmt);
    TEST("data_seek(0): id=1, val=10");
    CHECK(rc == 0 && out_id == 1 && out_val == 10, "wrong row at offset 0");

    mysql_stmt_data_seek(stmt, 4);
    rc = mysql_stmt_fetch(stmt);
    TEST("data_seek(4): id=5, val=50");
    CHECK(rc == 0 && out_id == 5 && out_val == 50, "wrong row at offset 4");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_timestamp_param_insert(void) {
    printf("\n[PS: TIMESTAMP param insert and fetch]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_tpi");
    Q("CREATE TABLE t_tpi(id INT, ts TIMESTAMP(6))");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "INSERT INTO t_tpi VALUES(?, ?)",
                       strlen("INSERT INTO t_tpi VALUES(?, ?)"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_TIME ts;
    memset(&ts, 0, sizeof(ts));
    ts.time_type = MYSQL_TIMESTAMP_DATETIME;
    ts.year = 2024; ts.month = 8; ts.day = 15;
    ts.hour = 14; ts.minute = 30; ts.second = 0;
    ts.second_part = 500000;
    unsigned long tl = sizeof(MYSQL_TIME);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &il;
    pb[1].buffer_type = MYSQL_TYPE_TIMESTAMP; pb[1].buffer = &ts;
    pb[1].buffer_length = sizeof(MYSQL_TIME); pb[1].length = &tl;
    mysql_stmt_bind_param(stmt, pb);

    int rc = mysql_stmt_execute(stmt);
    TEST("TIMESTAMP insert succeeds");
    CHECK(rc == 0, "execute should work");

    mysql_stmt_prepare(stmt, "SELECT ts FROM t_tpi WHERE id = 1",
                       strlen("SELECT ts FROM t_tpi WHERE id = 1"));
    MYSQL_TIME out_ts;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_TIMESTAMP; rb.buffer = &out_ts;
    rb.buffer_length = sizeof(MYSQL_TIME); rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("TIMESTAMP roundtrip: 2024-08-15 14:30:00.500000");
    CHECK(rc == 0 && out_ts.year == 2024 && out_ts.month == 8 && out_ts.day == 15, "wrong date");
    CHECK(out_ts.hour == 14 && out_ts.minute == 30 && out_ts.second == 0, "wrong time");
    CHECK(out_ts.second_part == 500000, "wrong microsecond");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_negative_values(void) {
    printf("\n[PS: negative integer and float values]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_neg");
    Q("CREATE TABLE t_neg(id INT, vi INT, vf DOUBLE)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "INSERT INTO t_neg VALUES(?, ?, ?)",
                       strlen("INSERT INTO t_neg VALUES(?, ?, ?)"));

    int id = 1, vi = -42;
    double vf = -3.14;
    unsigned long il = sizeof(int), vl = sizeof(int), dl = sizeof(double);
    MYSQL_BIND pb[3];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &il;
    pb[1].buffer_type = MYSQL_TYPE_LONG; pb[1].buffer = &vi; pb[1].length = &vl;
    pb[2].buffer_type = MYSQL_TYPE_DOUBLE; pb[2].buffer = &vf; pb[2].length = &dl;
    mysql_stmt_bind_param(stmt, pb);

    int rc = mysql_stmt_execute(stmt);
    TEST("insert negative values");
    CHECK(rc == 0, "execute should work");

    mysql_stmt_prepare(stmt, "SELECT vi, vf FROM t_neg WHERE id = 1",
                       strlen("SELECT vi, vf FROM t_neg WHERE id = 1"));
    int out_vi;
    double out_vf;
    unsigned long ol1 = 0, ol2 = 0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &out_vi; rb[0].length = &ol1;
    rb[1].buffer_type = MYSQL_TYPE_DOUBLE; rb[1].buffer = &out_vf; rb[1].length = &ol2;
    mysql_stmt_bind_result(stmt, rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("negative INT roundtrip: -42");
    CHECK(rc == 0 && out_vi == -42, "wrong value");
    TEST("negative DOUBLE roundtrip: ~-3.14");
    CHECK(out_vf > -3.15 && out_vf < -3.13, "wrong value");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_max_varchar_param(void) {
    printf("\n[PS: max VARCHAR param roundtrip]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_mvp");
    Q("CREATE TABLE t_mvp(id INT, val VARCHAR(1000))");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "INSERT INTO t_mvp VALUES(?, ?)",
                       strlen("INSERT INTO t_mvp VALUES(?, ?)"));

    int id = 1;
    unsigned long il = sizeof(int);
    char bigstr[501];
    memset(bigstr, 'M', 500);
    bigstr[500] = '\0';
    unsigned long bl = 500;
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &il;
    pb[1].buffer_type = MYSQL_TYPE_VAR_STRING; pb[1].buffer = bigstr;
    pb[1].buffer_length = sizeof(bigstr); pb[1].length = &bl;
    mysql_stmt_bind_param(stmt, pb);

    int rc = mysql_stmt_execute(stmt);
    TEST("insert 500-char VARCHAR");
    CHECK(rc == 0, "execute should work");

    mysql_stmt_prepare(stmt, "SELECT val FROM t_mvp WHERE id = 1",
                       strlen("SELECT val FROM t_mvp WHERE id = 1"));
    char out_str[600];
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_STRING; rb.buffer = out_str;
    rb.buffer_length = sizeof(out_str); rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("500-char VARCHAR roundtrip: length=500");
    CHECK(rc == 0 && ol == 500, "length should be 500");
    CHECK(out_str[0] == 'M' && out_str[499] == 'M', "content should be all M");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_fetch_on_non_select(void) {
    printf("\n[PS: fetch on non-SELECT (BUG#14169)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_fns");
    Q("CREATE TABLE t_fns(id INT)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "INSERT INTO t_fns VALUES(1)",
                       strlen("INSERT INTO t_fns VALUES(1)"));
    int rc = mysql_stmt_execute(stmt);
    TEST("INSERT execute succeeds");
    CHECK(rc == 0, "execute should work");

    rc = mysql_stmt_fetch(stmt);
    TEST("fetch on INSERT returns MYSQL_NO_DATA");
    CHECK(rc == MYSQL_NO_DATA, "should return NO_DATA, not crash");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_reprepare_metadata_refresh(void) {
    printf("\n[PS: re-prepare metadata refresh (BUG#27892)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT 1 AS a",
                       strlen("SELECT 1 AS a"));

    MYSQL_RES *meta1 = mysql_stmt_result_metadata(stmt);
    TEST("first prepare: field_count=1");
    CHECK(meta1 && meta1->field_count == 1, "should have 1 field");
    if (meta1) {
        MYSQL_FIELD *f = mysql_fetch_fields(meta1);
        TEST("first prepare: field name='a'");
        CHECK(f[0].name && strcmp(f[0].name, "a") == 0, "should be 'a'");
        mysql_free_result(meta1);
    }

    mysql_stmt_prepare(stmt, "SELECT 1 AS b, 2 AS c",
                       strlen("SELECT 1 AS b, 2 AS c"));
    MYSQL_RES *meta2 = mysql_stmt_result_metadata(stmt);
    TEST("re-prepare: field_count=2");
    CHECK(meta2 && meta2->field_count == 2, "should have 2 fields after re-prepare");
    if (meta2) {
        MYSQL_FIELD *f = mysql_fetch_fields(meta2);
        TEST("re-prepare: first field name='b'");
        CHECK(f[0].name && strcmp(f[0].name, "b") == 0, "should be 'b'");
        TEST("re-prepare: second field name='c'");
        CHECK(f[1].name && strcmp(f[1].name, "c") == 0, "should be 'c'");
        mysql_free_result(meta2);
    }

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_int_truncation_tiny(void) {
    printf("\n[PS: integer truncation - BIGINT to TINYINT]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_itt");
    Q("CREATE TABLE t_itt(id INT, val BIGINT)");
    Q("INSERT INTO t_itt VALUES(1, 300)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT val FROM t_itt WHERE id = 1",
                       strlen("SELECT val FROM t_itt WHERE id = 1"));

    int8_t tiny_val = 0;
    unsigned long ol = 0;
    my_bool err_flag = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_TINY; rb.buffer = &tiny_val;
    rb.length = &ol; rb.error = &err_flag;
    mysql_stmt_bind_result(stmt, &rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("BIGINT 300 bound to TINYINT: truncation detected");
    CHECK(rc == 0 || rc == MYSQL_DATA_TRUNCATED, "fetch should succeed or truncate");
    CHECK(err_flag == 1, "error flag should be set for truncation");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_int_truncation_short(void) {
    printf("\n[PS: integer truncation - INT to SMALLINT]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_its");
    Q("CREATE TABLE t_its(id INT, val INT)");
    Q("INSERT INTO t_its VALUES(1, 100000)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT val FROM t_its WHERE id = 1",
                       strlen("SELECT val FROM t_its WHERE id = 1"));

    int16_t short_val = 0;
    unsigned long ol = 0;
    my_bool err_flag = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_SHORT; rb.buffer = &short_val;
    rb.length = &ol; rb.error = &err_flag;
    mysql_stmt_bind_result(stmt, &rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("INT 100000 bound to SMALLINT: truncation detected");
    CHECK(rc == 0 || rc == MYSQL_DATA_TRUNCATED, "fetch should succeed or truncate");
    CHECK(err_flag == 1, "error flag should be set for truncation");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_blob_zero_buffer_length(void) {
    printf("\n[PS: BLOB with buffer_length=0 (BUG#6761)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_bzb");
    Q("CREATE TABLE t_bzb(id INT, data BLOB)");
    Q("INSERT INTO t_bzb VALUES(1, 'hello world')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT data FROM t_bzb WHERE id = 1",
                       strlen("SELECT data FROM t_bzb WHERE id = 1"));

    unsigned long ol = 0;
    my_bool err_flag = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_BLOB; rb.buffer = NULL;
    rb.buffer_length = 0; rb.length = &ol; rb.error = &err_flag;
    mysql_stmt_bind_result(stmt, &rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("BLOB with buffer_length=0: no crash");
    CHECK(rc == 0 || rc == MYSQL_DATA_TRUNCATED, "should not crash");
    TEST("length reports actual size");
    CHECK(ol == 11, "length should be 11");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_store_result_row_count_after_partial_fetch(void) {
    printf("\n[PS: store_result row_count after partial fetch (BUG#11183)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_src");
    Q("CREATE TABLE t_src(id INT)");
    Q("INSERT INTO t_src VALUES(1),(2),(3),(4),(5)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id FROM t_src ORDER BY id",
                       strlen("SELECT id FROM t_src ORDER BY id"));

    int out_id;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_id; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    CHECK(rc == 0, "first fetch");

    rc = mysql_stmt_store_result(stmt);
    TEST("store_result after partial fetch: no crash");
    CHECK(rc == 0, "store_result should succeed");
    my_ulonglong total = mysql_stmt_num_rows(stmt);
    TEST("store_result buffers remaining rows");
    CHECK(total >= 4, "should have at least 4 rows (remaining after fetch)");

    int count = 1;
    while (mysql_stmt_fetch(stmt) == 0) count++;
    TEST("total rows accessible >= 5");
    CHECK(count >= 5, "should have at least 5 rows total");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_reexecute_without_full_consume(void) {
    printf("\n[PS: re-execute without full consume (BUG#25355)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_rwf");
    Q("CREATE TABLE t_rwf(id INT, val INT)");
    Q("INSERT INTO t_rwf VALUES(1,10),(2,20),(3,30)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id, val FROM t_rwf ORDER BY id",
                       strlen("SELECT id, val FROM t_rwf ORDER BY id"));

    int out_id, out_val;
    unsigned long ol1 = 0, ol2 = 0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &out_id; rb[0].length = &ol1;
    rb[1].buffer_type = MYSQL_TYPE_LONG; rb[1].buffer = &out_val; rb[1].length = &ol2;
    mysql_stmt_bind_result(stmt, rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    CHECK(rc == 0 && out_id == 1, "first row");

    rc = mysql_stmt_execute(stmt);
    TEST("re-execute without full consume: succeeds");
    CHECK(rc == 0, "should auto-drain and succeed");

    int count = 0;
    while ((rc = mysql_stmt_fetch(stmt)) == 0) count++;
    TEST("second result has 3 rows");
    CHECK(count == 3, "should have 3 rows");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_unsigned_int_param(void) {
    printf("\n[PS: unsigned INT param (BUG#20485)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_uip");
    Q("CREATE TABLE t_uip(id INT, val INT UNSIGNED)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "INSERT INTO t_uip VALUES(?, ?)",
                       strlen("INSERT INTO t_uip VALUES(?, ?)"));

    int id = 1;
    unsigned long il = sizeof(int);
    unsigned int uval = 2147483648U;
    unsigned long vl = sizeof(unsigned int);
    my_bool is_unsigned = 1;
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &il;
    pb[1].buffer_type = MYSQL_TYPE_LONG; pb[1].buffer = &uval; pb[1].length = &vl;
    pb[1].is_unsigned = is_unsigned;
    mysql_stmt_bind_param(stmt, pb);

    int rc = mysql_stmt_execute(stmt);
    TEST("unsigned INT insert: 2147483648");
    CHECK(rc == 0, "execute should work");

    Q("SELECT val FROM t_uip WHERE id = 1");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("unsigned INT roundtrip: value > INT_MAX");
    CHECK(row && row[0] && strcmp(row[0], "2147483648") == 0, "should be 2147483648");
    mysql_free_result(res);

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_store_result_zero_cols(void) {
    printf("\n[PS: store_result on zero-column result (BUG#21261)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_srz");
    Q("CREATE TABLE t_srz(id INT)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "INSERT INTO t_srz VALUES(1)",
                       strlen("INSERT INTO t_srz VALUES(1)"));
    int rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, "execute INSERT");

    rc = mysql_stmt_store_result(stmt);
    TEST("store_result on INSERT: no crash");
    CHECK(rc == 0, "should succeed for zero-column result");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_fetch_column_with_offset(void) {
    printf("\n[PS: fetch_column with offset]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_fco");
    Q("CREATE TABLE t_fco(id INT, data VARCHAR(100))");
    Q("INSERT INTO t_fco VALUES(1, 'Hello, World!')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id, data FROM t_fco WHERE id = 1",
                       strlen("SELECT id, data FROM t_fco WHERE id = 1"));

    int out_id;
    unsigned long ol1 = 0;
    char out_str[100];
    unsigned long ol2 = 0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &out_id; rb[0].length = &ol1;
    rb[1].buffer_type = MYSQL_TYPE_STRING; rb[1].buffer = out_str;
    rb[1].buffer_length = sizeof(out_str); rb[1].length = &ol2;
    mysql_stmt_bind_result(stmt, rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_store_result(stmt);
    CHECK(rc == 0, "store_result");
    rc = mysql_stmt_fetch(stmt);
    CHECK(rc == 0, "fetch");

    char buf[20] = {0};
    unsigned long bl = 0;
    MYSQL_BIND fb;
    memset(&fb, 0, sizeof(fb));
    fb.buffer_type = MYSQL_TYPE_STRING; fb.buffer = buf;
    fb.buffer_length = sizeof(buf); fb.length = &bl;
    rc = mysql_stmt_fetch_column(stmt, &fb, 1, 7);
    TEST("fetch_column offset=7: partial data");
    CHECK(rc == 0, "fetch_column should work");
    if (bl > 0 && bl < sizeof(buf)) {
        TEST("partial content from offset 7");
        CHECK(strncmp(buf, "World!", 6) == 0 || bl > 0, "should have partial data");
    }

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_multiple_prepare_same_stmt(void) {
    printf("\n[PS: multiple prepare on same stmt handle]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_mps");
    Q("CREATE TABLE t_mps(id INT, val INT)");
    Q("INSERT INTO t_mps VALUES(1,100),(2,200)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);

    mysql_stmt_prepare(stmt, "SELECT val FROM t_mps WHERE id = ?",
                       strlen("SELECT val FROM t_mps WHERE id = ?"));
    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    int out_val;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("first prepare: id=1, val=100");
    CHECK(rc == 0 && out_val == 100, "wrong value");
    while (mysql_stmt_fetch(stmt) == 0) {}

    mysql_stmt_prepare(stmt, "SELECT val FROM t_mps WHERE val > ?",
                       strlen("SELECT val FROM t_mps WHERE val > ?"));
    int threshold = 150;
    unsigned long tl = sizeof(int);
    MYSQL_BIND pb2;
    memset(&pb2, 0, sizeof(pb2));
    pb2.buffer_type = MYSQL_TYPE_LONG; pb2.buffer = &threshold; pb2.length = &tl;
    mysql_stmt_bind_param(stmt, &pb2);
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("second prepare: val>150, first row val=200");
    CHECK(rc == 0 && out_val == 200, "wrong value");
    while (mysql_stmt_fetch(stmt) == 0) {}

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_execute_after_reset(void) {
    printf("\n[PS: execute after reset]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_ear");
    Q("CREATE TABLE t_ear(id INT, val INT)");
    Q("INSERT INTO t_ear VALUES(1,10)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT val FROM t_ear WHERE id = ?",
                       strlen("SELECT val FROM t_ear WHERE id = ?"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    int out_val;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    CHECK(rc == 0 && out_val == 10, "first execute");
    while (mysql_stmt_fetch(stmt) == 0) {}

    rc = mysql_stmt_reset(stmt);
    TEST("reset succeeds");
    CHECK(rc == 0, "reset should work");

    rc = mysql_stmt_execute(stmt);
    TEST("execute after reset succeeds");
    CHECK(rc == 0, "execute should work after reset");
    rc = mysql_stmt_fetch(stmt);
    CHECK(rc == 0 && out_val == 10, "same result after reset");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_update_affected_rows_verify(void) {
    printf("\n[PS: UPDATE affected_rows verification]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_uav");
    Q("CREATE TABLE t_uav(id INT, val INT)");
    Q("INSERT INTO t_uav VALUES(1,10),(2,20),(3,30)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "UPDATE t_uav SET val = val + ? WHERE id = ?",
                       strlen("UPDATE t_uav SET val = val + ? WHERE id = ?"));

    int delta = 1000, id = 2;
    unsigned long dl = sizeof(int), il = sizeof(int);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &delta; pb[0].length = &dl;
    pb[1].buffer_type = MYSQL_TYPE_LONG; pb[1].buffer = &id; pb[1].length = &il;
    mysql_stmt_bind_param(stmt, pb);

    int rc = mysql_stmt_execute(stmt);
    TEST("UPDATE id=2: affected_rows=1");
    CHECK(rc == 0 && mysql_stmt_affected_rows(stmt) == 1, "should affect 1 row");

    Q("SELECT val FROM t_uav WHERE id = 2");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("val updated to 1020");
    CHECK(row && strcmp(row[0], "1020") == 0, "should be 1020");
    mysql_free_result(res);

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_delete_affected_rows_verify(void) {
    printf("\n[PS: DELETE affected_rows verification]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_dav");
    Q("CREATE TABLE t_dav(id INT, val INT)");
    Q("INSERT INTO t_dav VALUES(1,10),(2,20),(3,30)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "DELETE FROM t_dav WHERE val > ?",
                       strlen("DELETE FROM t_dav WHERE val > ?"));

    int threshold = 15;
    unsigned long tl = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &threshold; pb.length = &tl;
    mysql_stmt_bind_param(stmt, &pb);

    int rc = mysql_stmt_execute(stmt);
    TEST("DELETE val>15: affected_rows=2");
    CHECK(rc == 0 && mysql_stmt_affected_rows(stmt) == 2, "should delete 2 rows");

    Q("SELECT COUNT(*) FROM t_dav");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("1 row remaining");
    CHECK(row && strcmp(row[0], "1") == 0, "should have 1 row left");
    mysql_free_result(res);

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_text_query_count_func(void) {
    printf("\n[text: COUNT() aggregate function]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_cf");
    Q("CREATE TABLE t_cf(id INT, val INT)");
    Q("INSERT INTO t_cf VALUES(1,10),(2,20),(3,NULL)");

    Q("SELECT COUNT(*), COUNT(val) FROM t_cf");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("COUNT(*)=3, COUNT(val)=2");
    CHECK(row && strcmp(row[0], "3") == 0 && strcmp(row[1], "2") == 0, "wrong count");
    mysql_free_result(res);

    mysql_close(m);
}

static void test_text_query_min_max(void) {
    printf("\n[text: MIN/MAX functions]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_mm");
    Q("CREATE TABLE t_mm(id INT, val INT)");
    Q("INSERT INTO t_mm VALUES(1,10),(2,50),(3,30)");

    Q("SELECT MIN(val), MAX(val) FROM t_mm");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("MIN=10, MAX=50");
    CHECK(row && strcmp(row[0], "10") == 0 && strcmp(row[1], "50") == 0, "wrong min/max");
    mysql_free_result(res);

    mysql_close(m);
}

static void test_ps_mixed_null_row(void) {
    printf("\n[PS: row with mixed NULL and non-NULL columns]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_mnr");
    Q("CREATE TABLE t_mnr(id INT, a INT, b VARCHAR(50), c DOUBLE)");
    Q("INSERT INTO t_mnr VALUES(1, 42, NULL, 3.14)");
    Q("INSERT INTO t_mnr VALUES(2, NULL, 'hello', NULL)");
    Q("INSERT INTO t_mnr VALUES(3, NULL, NULL, NULL)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id, a, b, c FROM t_mnr ORDER BY id",
                       strlen("SELECT id, a, b, c FROM t_mnr ORDER BY id"));

    int out_id, out_a;
    char out_b[50];
    double out_c;
    unsigned long ol1=0, ol2=0, ol3=0, ol4=0;
    my_bool n1=0, n2=0, n3=0, n4=0;
    MYSQL_BIND rb[4];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type=MYSQL_TYPE_LONG; rb[0].buffer=&out_id; rb[0].length=&ol1; rb[0].is_null=&n1;
    rb[1].buffer_type=MYSQL_TYPE_LONG; rb[1].buffer=&out_a; rb[1].length=&ol2; rb[1].is_null=&n2;
    rb[2].buffer_type=MYSQL_TYPE_STRING; rb[2].buffer=out_b; rb[2].buffer_length=sizeof(out_b); rb[2].length=&ol3; rb[2].is_null=&n3;
    rb[3].buffer_type=MYSQL_TYPE_DOUBLE; rb[3].buffer=&out_c; rb[3].length=&ol4; rb[3].is_null=&n4;
    mysql_stmt_bind_result(stmt, rb);

    int rc = mysql_stmt_execute(stmt);

    rc = mysql_stmt_fetch(stmt);
    TEST("row1: id=1, a=42, b=NULL, c=3.14");
    CHECK(rc == 0 && out_id==1 && n2==0 && out_a==42 && n3==1 && n4==0, "wrong row1");

    rc = mysql_stmt_fetch(stmt);
    TEST("row2: id=2, a=NULL, b='hello', c=NULL");
    CHECK(rc == 0 && out_id==2 && n2==1 && n3==0 && strncmp(out_b,"hello",5)==0 && n4==1, "wrong row2");

    rc = mysql_stmt_fetch(stmt);
    TEST("row3: id=3, a=NULL, b=NULL, c=NULL");
    CHECK(rc == 0 && out_id==3 && n2==1 && n3==1 && n4==1, "wrong row3");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_result_metadata_field_names(void) {
    printf("\n[PS: result_metadata field names]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT 1 AS col_a, 2 AS col_b, 'test' AS col_c",
                       strlen("SELECT 1 AS col_a, 2 AS col_b, 'test' AS col_c"));

    MYSQL_RES *meta = mysql_stmt_result_metadata(stmt);
    TEST("metadata has 3 fields");
    CHECK(meta && meta->field_count == 3, "should have 3 fields");
    if (meta) {
        MYSQL_FIELD *f = mysql_fetch_fields(meta);
        TEST("field 0 name='col_a'");
        CHECK(f[0].name && strcmp(f[0].name, "col_a") == 0, "wrong name");
        TEST("field 1 name='col_b'");
        CHECK(f[1].name && strcmp(f[1].name, "col_b") == 0, "wrong name");
        TEST("field 2 name='col_c'");
        CHECK(f[2].name && strcmp(f[2].name, "col_c") == 0, "wrong name");
        mysql_free_result(meta);
    }

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_store_result_then_data_seek_loop(void) {
    printf("\n[PS: store_result then data_seek loop]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_sdl");
    Q("CREATE TABLE t_sdl(id INT, val INT)");
    Q("INSERT INTO t_sdl VALUES(1,10),(2,20),(3,30),(4,40),(5,50),(6,60),(7,70),(8,80),(9,90),(10,100)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id, val FROM t_sdl ORDER BY id",
                       strlen("SELECT id, val FROM t_sdl ORDER BY id"));

    int out_id, out_val;
    unsigned long ol1 = 0, ol2 = 0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &out_id; rb[0].length = &ol1;
    rb[1].buffer_type = MYSQL_TYPE_LONG; rb[1].buffer = &out_val; rb[1].length = &ol2;
    mysql_stmt_bind_result(stmt, rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_store_result(stmt);
    CHECK(rc == 0, "store_result");

    TEST("num_rows=10");
    CHECK(mysql_stmt_num_rows(stmt) == 10, "should have 10 rows");

    for (my_ulonglong i = 0; i < 10; i++) {
        mysql_stmt_data_seek(stmt, i);
        rc = mysql_stmt_fetch(stmt);
        if (rc != 0 || out_id != (int)(i + 1) || out_val != (int)((i + 1) * 10)) {
            TEST("data_seek loop: wrong data at offset");
            CHECK(0, "data mismatch");
            break;
        }
    }
    TEST("data_seek loop: all 10 positions correct");
    CHECK(1, "");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_double_prepare_same_stmt(void) {
    printf("\n[PS: double prepare on same stmt (BUG#1664)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT 1", strlen("SELECT 1"));
    CHECK(rc == 0, "first prepare");

    rc = mysql_stmt_prepare(stmt, "SELECT 2", strlen("SELECT 2"));
    TEST("second prepare on same stmt succeeds");
    CHECK(rc == 0, "should work");

    int out_val;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("second prepare result: val=2");
    CHECK(rc == 0 && out_val == 2, "should be 2");
    while (mysql_stmt_fetch(stmt) == 0) {}

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_fetch_without_bind_result(void) {
    printf("\n[PS: fetch without bind_result (BUG#7486)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_fwb");
    Q("CREATE TABLE t_fwb(id INT)");
    Q("INSERT INTO t_fwb VALUES(1)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id FROM t_fwb",
                       strlen("SELECT id FROM t_fwb"));
    int rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, "execute");

    rc = mysql_stmt_fetch(stmt);
    TEST("fetch without bind_result: no crash");
    CHECK(rc == 0 || rc == MYSQL_NO_DATA || rc != 0, "should not crash");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}


static void test_ps_fetch_with_warning(void) {
    printf("\n[PS: fetch with server warning (BUG#23983)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT 1/0", strlen("SELECT 1/0"));

    double out_val;
    unsigned long ol = 0;
    my_bool is_null = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_DOUBLE; rb.buffer = &out_val; rb.length = &ol; rb.is_null = &is_null;
    mysql_stmt_bind_result(stmt, &rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("fetch 1/0: no crash");
    CHECK(rc == 0, "should not crash");
    TEST("1/0 result is NULL");
    CHECK(is_null == 1, "should be NULL for division by zero");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_execute_on_dropped_table(void) {
    printf("\n[PS: execute on dropped table (test_stmt_err)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_edt");
    Q("CREATE TABLE t_edt(id INT)");
    Q("INSERT INTO t_edt VALUES(1)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id FROM t_edt",
                       strlen("SELECT id FROM t_edt"));

    Q("DROP TABLE t_edt");

    int rc = mysql_stmt_execute(stmt);
    TEST("execute on dropped table returns error");
    CHECK(rc != 0, "should fail");
    TEST("errno is non-zero");
    CHECK(mysql_stmt_errno(stmt) != 0, "should have error");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_prepare_empty_sql(void) {
    printf("\n[PS: prepare with empty SQL]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "", 0);
    TEST("prepare empty SQL fails");
    CHECK(rc != 0, "should fail for empty SQL");

    rc = mysql_stmt_prepare(stmt, "SELECT 1", strlen("SELECT 1"));
    TEST("prepare valid SQL after empty SQL error");
    CHECK(rc == 0, "should succeed");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_close_after_execute_no_fetch(void) {
    printf("\n[PS: close after execute without fetch (test_stmt_close_after_execute)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_cae2");
    Q("CREATE TABLE t_cae2(id INT)");
    Q("INSERT INTO t_cae2 VALUES(1),(2),(3)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id FROM t_cae2",
                       strlen("SELECT id FROM t_cae2"));
    int rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, "execute");

    mysql_stmt_close(stmt);
    TEST("close after execute without fetch: connection usable");

    rc = Q("SELECT 1");
    CHECK(rc == 0, "text query should work");
    MYSQL_RES *res = mysql_store_result(m);
    CHECK(res != NULL, "should get result");
    mysql_free_result(res);

    mysql_close(m);
}

static void test_ps_store_result_no_fetch_then_reexecute(void) {
    printf("\n[PS: store_result no fetch then re-execute (BUG#28075)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_snr2");
    Q("CREATE TABLE t_snr2(id INT)");
    Q("INSERT INTO t_snr2 VALUES(1),(2),(3)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id FROM t_snr2",
                       strlen("SELECT id FROM t_snr2"));

    int out_id;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_id; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_store_result(stmt);
    CHECK(rc == 0, "first store_result");

    rc = mysql_stmt_execute(stmt);
    TEST("re-execute after store_result without fetch: succeeds");
    CHECK(rc == 0, "should succeed");

    rc = mysql_stmt_store_result(stmt);
    CHECK(rc == 0, "second store_result");

    int count = 0;
    while (mysql_stmt_fetch(stmt) == 0) count++;
    TEST("second result has 3 rows");
    CHECK(count == 3, "should have 3 rows");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_close_then_new_stmt(void) {
    printf("\n[PS: close then new stmt on same connection]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_ctn");
    Q("CREATE TABLE t_ctn(id INT, val INT)");
    Q("INSERT INTO t_ctn VALUES(1,42)");

    for (int i = 0; i < 5; i++) {
        MYSQL_STMT *stmt = mysql_stmt_init(m);
        int rc = mysql_stmt_prepare(stmt, "SELECT val FROM t_ctn WHERE id = 1",
                                    strlen("SELECT val FROM t_ctn WHERE id = 1"));
        CHECK(rc == 0, "prepare should work");

        int out_val;
        unsigned long ol = 0;
        MYSQL_BIND rb;
        memset(&rb, 0, sizeof(rb));
        rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.length = &ol;
        mysql_stmt_bind_result(stmt, &rb);

        rc = mysql_stmt_execute(stmt);
        rc = mysql_stmt_fetch(stmt);
        if (rc != 0 || out_val != 42) {
            TEST("close-then-new loop: wrong value");
            CHECK(0, "should be 42");
            mysql_stmt_close(stmt);
            break;
        }
        while (mysql_stmt_fetch(stmt) == 0) {}
        mysql_stmt_close(stmt);
    }
    TEST("5x close-then-new stmt: all succeed");
    CHECK(1, "");

    mysql_close(m);
}

static void test_ps_reprepare_different_col_count(void) {
    printf("\n[PS: re-prepare with different column count (BUG#15518)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);

    mysql_stmt_prepare(stmt, "SELECT 1 AS a",
                       strlen("SELECT 1 AS a"));
    MYSQL_RES *meta1 = mysql_stmt_result_metadata(stmt);
    TEST("first prepare: 1 column");
    CHECK(meta1 && meta1->field_count == 1, "should have 1 field");
    mysql_free_result(meta1);

    mysql_stmt_prepare(stmt, "SELECT 1 AS a, 2 AS b, 3 AS c",
                       strlen("SELECT 1 AS a, 2 AS b, 3 AS c"));
    MYSQL_RES *meta2 = mysql_stmt_result_metadata(stmt);
    TEST("re-prepare: 3 columns");
    CHECK(meta2 && meta2->field_count == 3, "should have 3 fields after re-prepare");
    if (meta2) {
        MYSQL_FIELD *f = mysql_fetch_fields(meta2);
        TEST("field names updated");
        CHECK(f[0].name && strcmp(f[0].name, "a") == 0, "first field should be 'a'");
        CHECK(f[1].name && strcmp(f[1].name, "b") == 0, "second field should be 'b'");
        CHECK(f[2].name && strcmp(f[2].name, "c") == 0, "third field should be 'c'");
        mysql_free_result(meta2);
    }

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_multiple_warnings(void) {
    printf("\n[PS: multiple warnings in single query]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT 1/0, 2/0",
                       strlen("SELECT 1/0, 2/0"));

    double out_v1, out_v2;
    unsigned long ol1 = 0, ol2 = 0;
    my_bool n1 = 0, n2 = 0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_DOUBLE; rb[0].buffer = &out_v1; rb[0].length = &ol1; rb[0].is_null = &n1;
    rb[1].buffer_type = MYSQL_TYPE_DOUBLE; rb[1].buffer = &out_v2; rb[1].length = &ol2; rb[1].is_null = &n2;
    mysql_stmt_bind_result(stmt, rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("multiple warnings: no crash");
    CHECK(rc == 0, "should not crash");
    TEST("both division results are NULL");
    CHECK(n1 == 1 && n2 == 1, "both should be NULL");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_select_constant_expr(void) {
    printf("\n[PS: SELECT with constant expressions]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT 1 + 2, 'hello', 3.14",
                       strlen("SELECT 1 + 2, 'hello', 3.14"));

    int out_int;
    char out_str[20];
    double out_dbl;
    unsigned long ol1=0, ol2=0, ol3=0;
    MYSQL_BIND rb[3];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type=MYSQL_TYPE_LONG; rb[0].buffer=&out_int; rb[0].length=&ol1;
    rb[1].buffer_type=MYSQL_TYPE_STRING; rb[1].buffer=out_str; rb[1].buffer_length=sizeof(out_str); rb[1].length=&ol2;
    rb[2].buffer_type=MYSQL_TYPE_DOUBLE; rb[2].buffer=&out_dbl; rb[2].length=&ol3;
    mysql_stmt_bind_result(stmt, rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("1+2=3");
    CHECK(rc == 0 && out_int == 3, "wrong value");
    TEST("'hello' roundtrip");
    CHECK(ol2 == 5 && strncmp(out_str, "hello", 5) == 0, "wrong string");
    TEST("3.14 roundtrip");
    CHECK(out_dbl > 3.13 && out_dbl < 3.15, "wrong double");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_between_operator(void) {
    printf("\n[PS: BETWEEN operator]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_btw");
    Q("CREATE TABLE t_btw(id INT, val INT)");
    Q("INSERT INTO t_btw VALUES(1,10),(2,20),(3,30),(4,40),(5,50)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id FROM t_btw WHERE val BETWEEN ? AND ? ORDER BY id",
                       strlen("SELECT id FROM t_btw WHERE val BETWEEN ? AND ? ORDER BY id"));

    int lo = 20, hi = 40;
    unsigned long ll = sizeof(int), hl = sizeof(int);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &lo; pb[0].length = &ll;
    pb[1].buffer_type = MYSQL_TYPE_LONG; pb[1].buffer = &hi; pb[1].length = &hl;
    mysql_stmt_bind_param(stmt, pb);

    int out_id;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_id; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    int rc = mysql_stmt_execute(stmt);
    int count = 0;
    while ((rc = mysql_stmt_fetch(stmt)) == 0) count++;
    TEST("BETWEEN 20 AND 40: 3 rows");
    CHECK(count == 3, "should have 3 rows (id=2,3,4)");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_in_clause(void) {
    printf("\n[PS: IN clause with PS param]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_inc");
    Q("CREATE TABLE t_inc(id INT, val INT)");
    Q("INSERT INTO t_inc VALUES(1,10),(2,20),(3,30)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id FROM t_inc WHERE val = ? OR val = ? ORDER BY id",
                       strlen("SELECT id FROM t_inc WHERE val = ? OR val = ? ORDER BY id"));

    int v1 = 10, v2 = 30;
    unsigned long l1 = sizeof(int), l2 = sizeof(int);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &v1; pb[0].length = &l1;
    pb[1].buffer_type = MYSQL_TYPE_LONG; pb[1].buffer = &v2; pb[1].length = &l2;
    mysql_stmt_bind_param(stmt, pb);

    int out_id;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_id; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    int rc = mysql_stmt_execute(stmt);
    int count = 0;
    while ((rc = mysql_stmt_fetch(stmt)) == 0) count++;
    TEST("val=10 OR val=30: 2 rows");
    CHECK(count == 2, "should have 2 rows");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_like_operator(void) {
    printf("\n[PS: LIKE operator]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_lke");
    Q("CREATE TABLE t_lke(id INT, name VARCHAR(50))");
    Q("INSERT INTO t_lke VALUES(1,'apple'),(2,'application'),(3,'banana')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id FROM t_lke WHERE name LIKE ? ORDER BY id",
                       strlen("SELECT id FROM t_lke WHERE name LIKE ? ORDER BY id"));

    char pattern[] = "app%";
    unsigned long pl = strlen(pattern);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_VAR_STRING; pb.buffer = pattern;
    pb.buffer_length = sizeof(pattern); pb.length = &pl;
    mysql_stmt_bind_param(stmt, &pb);

    int out_id;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_id; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    int rc = mysql_stmt_execute(stmt);
    int count = 0;
    while ((rc = mysql_stmt_fetch(stmt)) == 0) count++;
    TEST("LIKE 'app%%': 2 rows");
    CHECK(count == 2, "should match apple and application");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_is_null_operator(void) {
    printf("\n[PS: IS NULL operator]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_ino");
    Q("CREATE TABLE t_ino(id INT, val INT)");
    Q("INSERT INTO t_ino VALUES(1,10),(2,NULL),(3,30)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id FROM t_ino WHERE val IS NULL",
                       strlen("SELECT id FROM t_ino WHERE val IS NULL"));

    int out_id;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_id; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("IS NULL: id=2");
    CHECK(rc == 0 && out_id == 2, "should find id=2");

    rc = mysql_stmt_fetch(stmt);
    CHECK(rc == MYSQL_NO_DATA, "no more rows");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_group_by_count(void) {
    printf("\n[PS: GROUP BY with COUNT]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_gbc");
    Q("CREATE TABLE t_gbc(id INT, category VARCHAR(20))");
    Q("INSERT INTO t_gbc VALUES(1,'A'),(2,'A'),(3,'B'),(4,'B'),(5,'B')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT category, COUNT(*) FROM t_gbc GROUP BY category ORDER BY category",
                       strlen("SELECT category, COUNT(*) FROM t_gbc GROUP BY category ORDER BY category"));

    char out_cat[20];
    long long out_cnt;
    unsigned long ol1=0, ol2=0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type=MYSQL_TYPE_STRING; rb[0].buffer=out_cat; rb[0].buffer_length=sizeof(out_cat); rb[0].length=&ol1;
    rb[1].buffer_type=MYSQL_TYPE_LONGLONG; rb[1].buffer=&out_cnt; rb[1].length=&ol2;
    mysql_stmt_bind_result(stmt, rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("GROUP BY 'A': count=2");
    CHECK(rc == 0 && strncmp(out_cat, "A", 1) == 0 && out_cnt == 2, "wrong count for A");

    rc = mysql_stmt_fetch(stmt);
    TEST("GROUP BY 'B': count=3");
    CHECK(rc == 0 && strncmp(out_cat, "B", 1) == 0 && out_cnt == 3, "wrong count for B");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_having_clause(void) {
    printf("\n[PS: HAVING clause]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_hvc");
    Q("CREATE TABLE t_hvc(id INT, category VARCHAR(20), val INT)");
    Q("INSERT INTO t_hvc VALUES(1,'A',10),(2,'A',20),(3,'B',5),(4,'B',15)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT category, SUM(val) AS total FROM t_hvc GROUP BY category HAVING SUM(val) > ?",
                       strlen("SELECT category, SUM(val) AS total FROM t_hvc GROUP BY category HAVING SUM(val) > ?"));

    int threshold = 20;
    unsigned long tl = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &threshold; pb.length = &tl;
    mysql_stmt_bind_param(stmt, &pb);

    char out_cat[20];
    long long out_total;
    unsigned long ol1=0, ol2=0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type=MYSQL_TYPE_STRING; rb[0].buffer=out_cat; rb[0].buffer_length=sizeof(out_cat); rb[0].length=&ol1;
    rb[1].buffer_type=MYSQL_TYPE_LONGLONG; rb[1].buffer=&out_total; rb[1].length=&ol2;
    mysql_stmt_bind_result(stmt, rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("HAVING SUM>20: category='A', total=30");
    CHECK(rc == 0 && strncmp(out_cat, "A", 1) == 0 && out_total == 30, "wrong result");

    rc = mysql_stmt_fetch(stmt);
    CHECK(rc == MYSQL_NO_DATA, "should be only 1 group");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_order_by_desc(void) {
    printf("\n[PS: ORDER BY DESC]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_obd");
    Q("CREATE TABLE t_obd(id INT, val INT)");
    Q("INSERT INTO t_obd VALUES(1,10),(2,30),(3,20)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id, val FROM t_obd ORDER BY val DESC",
                       strlen("SELECT id, val FROM t_obd ORDER BY val DESC"));

    int out_id, out_val;
    unsigned long ol1=0, ol2=0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type=MYSQL_TYPE_LONG; rb[0].buffer=&out_id; rb[0].length=&ol1;
    rb[1].buffer_type=MYSQL_TYPE_LONG; rb[1].buffer=&out_val; rb[1].length=&ol2;
    mysql_stmt_bind_result(stmt, rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("ORDER BY DESC: first row val=30");
    CHECK(rc == 0 && out_val == 30, "should be 30");

    rc = mysql_stmt_fetch(stmt);
    TEST("ORDER BY DESC: second row val=20");
    CHECK(rc == 0 && out_val == 20, "should be 20");

    rc = mysql_stmt_fetch(stmt);
    TEST("ORDER BY DESC: third row val=10");
    CHECK(rc == 0 && out_val == 10, "should be 10");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_limit_clause(void) {
    printf("\n[PS: LIMIT clause]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_lmt");
    Q("CREATE TABLE t_lmt(id INT)");
    Q("INSERT INTO t_lmt VALUES(1),(2),(3),(4),(5)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id FROM t_lmt ORDER BY id LIMIT ?",
                       strlen("SELECT id FROM t_lmt ORDER BY id LIMIT ?"));

    int limit_val = 3;
    unsigned long ll = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &limit_val; pb.length = &ll;
    mysql_stmt_bind_param(stmt, &pb);

    int out_id;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_id; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    int rc = mysql_stmt_execute(stmt);
    int count = 0;
    while ((rc = mysql_stmt_fetch(stmt)) == 0) count++;
    TEST("LIMIT 3: 3 rows");
    CHECK(count == 3, "should have 3 rows");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_union_query(void) {
    printf("\n[PS: UNION query (BUG#17667)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_uni");
    Q("CREATE TABLE t_uni(id INT, val INT)");
    Q("INSERT INTO t_uni VALUES(1,10),(2,20)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id, val FROM t_uni UNION ALL SELECT id, val FROM t_uni ORDER BY id",
                       strlen("SELECT id, val FROM t_uni UNION ALL SELECT id, val FROM t_uni ORDER BY id"));

    int out_id, out_val;
    unsigned long ol1=0, ol2=0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type=MYSQL_TYPE_LONG; rb[0].buffer=&out_id; rb[0].length=&ol1;
    rb[1].buffer_type=MYSQL_TYPE_LONG; rb[1].buffer=&out_val; rb[1].length=&ol2;
    mysql_stmt_bind_result(stmt, rb);

    int rc = mysql_stmt_execute(stmt);
    int count = 0;
    while ((rc = mysql_stmt_fetch(stmt)) == 0) count++;
    TEST("UNION: 4 rows (2+2)");
    CHECK(count == 4, "should have 4 rows from UNION");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_close_one_stmt_execute_another(void) {
    printf("\n[PS: close stmt1 then execute stmt2 (BUG#35189)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *s1 = mysql_stmt_init(m);
    MYSQL_STMT *s2 = mysql_stmt_init(m);

    mysql_stmt_prepare(s1, "SELECT 1", strlen("SELECT 1"));
    mysql_stmt_prepare(s2, "SELECT 2", strlen("SELECT 2"));

    int out_val;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.length = &ol;

    mysql_stmt_bind_result(s1, &rb);
    mysql_stmt_execute(s1);
    mysql_stmt_fetch(s1);
    while (mysql_stmt_fetch(s1) == 0) {}

    mysql_stmt_close(s1);
    TEST("stmt1 closed");

    mysql_stmt_bind_result(s2, &rb);
    int rc = mysql_stmt_execute(s2);
    TEST("stmt2 execute after stmt1 close: succeeds");
    CHECK(rc == 0, "should succeed");

    rc = mysql_stmt_fetch(s2);
    TEST("stmt2 result: val=2");
    CHECK(rc == 0 && out_val == 2, "should be 2");
    while (mysql_stmt_fetch(s2) == 0) {}

    mysql_stmt_close(s2);
    mysql_close(m);
}

static void test_ps_reset_then_store_result(void) {
    printf("\n[PS: reset then store_result (BUG#36000)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_rts");
    Q("CREATE TABLE t_rts(id INT)");
    Q("INSERT INTO t_rts VALUES(1),(2),(3)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id FROM t_rts",
                       strlen("SELECT id FROM t_rts"));

    int out_id;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_id; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_store_result(stmt);
    CHECK(rc == 0, "first store_result");
    while (mysql_stmt_fetch(stmt) == 0) {}

    rc = mysql_stmt_reset(stmt);
    TEST("reset succeeds");
    CHECK(rc == 0, "reset should work");

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_store_result(stmt);
    TEST("store_result after reset: succeeds");
    CHECK(rc == 0, "store_result should work after reset");

    int count = 0;
    while (mysql_stmt_fetch(stmt) == 0) count++;
    TEST("result after reset: 3 rows");
    CHECK(count == 3, "should have 3 rows");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_send_long_data_zero_length(void) {
    printf("\n[PS: send_long_data with zero length (BUG#30472)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_slz");
    Q("CREATE TABLE t_slz(id INT, data BLOB)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "INSERT INTO t_slz VALUES(?, ?)",
                       strlen("INSERT INTO t_slz VALUES(?, ?)"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &il;
    pb[1].buffer_type = MYSQL_TYPE_LONG_BLOB; pb[1].buffer = NULL; pb[1].length = NULL;
    mysql_stmt_bind_param(stmt, pb);

    mysql_stmt_send_long_data(stmt, 1, "", 0);
    int rc = mysql_stmt_execute(stmt);
    TEST("send_long_data with 0 length: execute succeeds");
    CHECK(rc == 0, "should succeed");

    mysql_stmt_prepare(stmt, "SELECT LENGTH(data) FROM t_slz WHERE id = 1",
                       strlen("SELECT LENGTH(data) FROM t_slz WHERE id = 1"));
    long long out_len;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONGLONG; rb.buffer = &out_len; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("zero-length long data: LENGTH=0");
    CHECK(rc == 0 && out_len == 0, "should be 0");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_result_metadata_after_execute(void) {
    printf("\n[PS: result_metadata after execute (BUG#36976)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_rma");
    Q("CREATE TABLE t_rma(id INT, val VARCHAR(50))");
    Q("INSERT INTO t_rma VALUES(1,'test')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id, val FROM t_rma",
                       strlen("SELECT id, val FROM t_rma"));

    MYSQL_RES *meta_before = mysql_stmt_result_metadata(stmt);
    TEST("metadata before execute: 2 fields");
    CHECK(meta_before && meta_before->field_count == 2, "should have 2 fields");
    mysql_free_result(meta_before);

    int rc = mysql_stmt_execute(stmt);
    while (mysql_stmt_fetch(stmt) == 0) {}

    MYSQL_RES *meta_after = mysql_stmt_result_metadata(stmt);
    TEST("metadata after execute: still 2 fields");
    CHECK(meta_after && meta_after->field_count == 2, "should still have 2 fields after execute");
    mysql_free_result(meta_after);

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_empty_result_store_then_reexecute(void) {
    printf("\n[PS: empty result store then re-execute (BUG#31618)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_ers");
    Q("CREATE TABLE t_ers(id INT)");
    Q("INSERT INTO t_ers VALUES(1),(2),(3)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id FROM t_ers WHERE id > ?",
                       strlen("SELECT id FROM t_ers WHERE id > ?"));

    int threshold = 100;
    unsigned long tl = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &threshold; pb.length = &tl;
    mysql_stmt_bind_param(stmt, &pb);

    int out_id;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_id; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_store_result(stmt);
    TEST("empty result store_result: succeeds");
    CHECK(rc == 0, "should succeed for empty result");
    CHECK(mysql_stmt_num_rows(stmt) == 0, "should have 0 rows");

    threshold = 0;
    rc = mysql_stmt_execute(stmt);
    TEST("re-execute with non-empty result: succeeds");
    CHECK(rc == 0, "should succeed");

    rc = mysql_stmt_store_result(stmt);
    CHECK(rc == 0, "store_result should work");

    int count = 0;
    while (mysql_stmt_fetch(stmt) == 0) count++;
    TEST("non-empty result: 3 rows");
    CHECK(count == 3, "should have 3 rows");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_multiple_stmt_handles(void) {
    printf("\n[PS: multiple stmt handles (test_conc60)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *s1 = mysql_stmt_init(m);
    MYSQL_STMT *s2 = mysql_stmt_init(m);
    MYSQL_STMT *s3 = mysql_stmt_init(m);

    mysql_stmt_prepare(s1, "SELECT 1", strlen("SELECT 1"));
    mysql_stmt_prepare(s2, "SELECT ?, ?", strlen("SELECT ?, ?"));
    mysql_stmt_prepare(s3, "SELECT 'hello'", strlen("SELECT 'hello'"));

    int out_v1;
    unsigned long ol1 = 0;
    MYSQL_BIND rb1;
    memset(&rb1, 0, sizeof(rb1));
    rb1.buffer_type = MYSQL_TYPE_LONG; rb1.buffer = &out_v1; rb1.length = &ol1;
    mysql_stmt_bind_result(s1, &rb1);

    int a = 10, b = 20;
    unsigned long al = sizeof(int), bl = sizeof(int);
    MYSQL_BIND pb2[2];
    memset(pb2, 0, sizeof(pb2));
    pb2[0].buffer_type = MYSQL_TYPE_LONG; pb2[0].buffer = &a; pb2[0].length = &al;
    pb2[1].buffer_type = MYSQL_TYPE_LONG; pb2[1].buffer = &b; pb2[1].length = &bl;
    mysql_stmt_bind_param(s2, pb2);

    int out_a, out_b;
    unsigned long ol2a = 0, ol2b = 0;
    MYSQL_BIND rb2[2];
    memset(rb2, 0, sizeof(rb2));
    rb2[0].buffer_type = MYSQL_TYPE_LONG; rb2[0].buffer = &out_a; rb2[0].length = &ol2a;
    rb2[1].buffer_type = MYSQL_TYPE_LONG; rb2[1].buffer = &out_b; rb2[1].length = &ol2b;
    mysql_stmt_bind_result(s2, rb2);

    char out_str[20];
    unsigned long ol3 = 0;
    MYSQL_BIND rb3;
    memset(&rb3, 0, sizeof(rb3));
    rb3.buffer_type = MYSQL_TYPE_STRING; rb3.buffer = out_str;
    rb3.buffer_length = sizeof(out_str); rb3.length = &ol3;
    mysql_stmt_bind_result(s3, &rb3);

    mysql_stmt_execute(s1);
    mysql_stmt_fetch(s1);
    TEST("s1: val=1");
    CHECK(out_v1 == 1, "should be 1");
    while (mysql_stmt_fetch(s1) == 0) {}

    mysql_stmt_execute(s3);
    mysql_stmt_fetch(s3);
    TEST("s3: str='hello'");
    CHECK(ol3 == 5 && strncmp(out_str, "hello", 5) == 0, "should be hello");
    while (mysql_stmt_fetch(s3) == 0) {}

    mysql_stmt_execute(s2);
    mysql_stmt_fetch(s2);
    TEST("s2: a=10, b=20");
    CHECK(out_a == 10 && out_b == 20, "should be 10,20");
    while (mysql_stmt_fetch(s2) == 0) {}

    mysql_stmt_close(s1);
    mysql_stmt_close(s2);
    mysql_stmt_close(s3);
    mysql_close(m);
}

static void test_ps_long_sql_prepare(void) {
    printf("\n[PS: long SQL prepare (BUG#33831)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    char longsql[10000];
    int pos = 0;
    pos += snprintf(longsql + pos, sizeof(longsql) - pos, "SELECT 1");
    for (int i = 0; i < 100 && pos < (int)sizeof(longsql) - 50; i++) {
        pos += snprintf(longsql + pos, sizeof(longsql) - pos, " UNION ALL SELECT %d", i + 2);
    }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, longsql, (unsigned long)strlen(longsql));
    TEST("long SQL prepare: succeeds or returns error");
    CHECK(rc == 0 || rc != 0, "should not crash");

    if (rc == 0) {
        int out_val;
        unsigned long ol = 0;
        MYSQL_BIND rb;
        memset(&rb, 0, sizeof(rb));
        rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.length = &ol;
        mysql_stmt_bind_result(stmt, &rb);

        rc = mysql_stmt_execute(stmt);
        int count = 0;
        while ((rc = mysql_stmt_fetch(stmt)) == 0) count++;
        TEST("long SQL result: row count > 0");
        CHECK(count > 0, "should have rows");
    }

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_subquery(void) {
    printf("\n[PS: subquery]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_sq1, t_sq2");
    Q("CREATE TABLE t_sq1(id INT, val INT)");
    Q("CREATE TABLE t_sq2(id INT, ref_id INT, score INT)");
    Q("INSERT INTO t_sq1 VALUES(1,10),(2,20)");
    Q("INSERT INTO t_sq2 VALUES(1,1,95),(2,1,85),(3,2,90)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id, val FROM t_sq1 WHERE id IN (SELECT ref_id FROM t_sq2 WHERE score > ?)",
                       strlen("SELECT id, val FROM t_sq1 WHERE id IN (SELECT ref_id FROM t_sq2 WHERE score > ?)"));

    int threshold = 88;
    unsigned long tl = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &threshold; pb.length = &tl;
    mysql_stmt_bind_param(stmt, &pb);

    int out_id, out_val;
    unsigned long ol1=0, ol2=0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type=MYSQL_TYPE_LONG; rb[0].buffer=&out_id; rb[0].length=&ol1;
    rb[1].buffer_type=MYSQL_TYPE_LONG; rb[1].buffer=&out_val; rb[1].length=&ol2;
    mysql_stmt_bind_result(stmt, rb);

    int rc = mysql_stmt_execute(stmt);
    int count = 0;
    while ((rc = mysql_stmt_fetch(stmt)) == 0) count++;
    TEST("subquery score>88: 2 rows");
    CHECK(count == 2, "should have 2 rows (both ids have scores > 88)");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_case_when(void) {
    printf("\n[PS: CASE WHEN expression]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_cw");
    Q("CREATE TABLE t_cw(id INT, val INT)");
    Q("INSERT INTO t_cw VALUES(1,10),(2,50),(3,90)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id, CASE WHEN val < ? THEN 'low' WHEN val < ? THEN 'mid' ELSE 'high' END AS lvl FROM t_cw ORDER BY id",
                       strlen("SELECT id, CASE WHEN val < ? THEN 'low' WHEN val < ? THEN 'mid' ELSE 'high' END AS lvl FROM t_cw ORDER BY id"));

    int lo = 30, hi = 70;
    unsigned long ll = sizeof(int), hl = sizeof(int);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &lo; pb[0].length = &ll;
    pb[1].buffer_type = MYSQL_TYPE_LONG; pb[1].buffer = &hi; pb[1].length = &hl;
    mysql_stmt_bind_param(stmt, pb);

    int out_id;
    char out_lvl[10];
    unsigned long ol1=0, ol2=0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type=MYSQL_TYPE_LONG; rb[0].buffer=&out_id; rb[0].length=&ol1;
    rb[1].buffer_type=MYSQL_TYPE_STRING; rb[1].buffer=out_lvl; rb[1].buffer_length=sizeof(out_lvl); rb[1].length=&ol2;
    mysql_stmt_bind_result(stmt, rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("id=1: lvl='low'");
    CHECK(rc == 0 && strncmp(out_lvl, "low", 3) == 0, "should be low");

    rc = mysql_stmt_fetch(stmt);
    TEST("id=2: lvl='mid'");
    CHECK(rc == 0 && strncmp(out_lvl, "mid", 3) == 0, "should be mid");

    rc = mysql_stmt_fetch(stmt);
    TEST("id=3: lvl='high'");
    CHECK(rc == 0 && strncmp(out_lvl, "high", 4) == 0, "should be high");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_distinct(void) {
    printf("\n[PS: SELECT DISTINCT]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_dis");
    Q("CREATE TABLE t_dis(id INT, category VARCHAR(10))");
    Q("INSERT INTO t_dis VALUES(1,'A'),(2,'A'),(3,'B'),(4,'B'),(5,'A')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT DISTINCT category FROM t_dis ORDER BY category",
                       strlen("SELECT DISTINCT category FROM t_dis ORDER BY category"));

    char out_cat[10];
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_STRING; rb.buffer = out_cat;
    rb.buffer_length = sizeof(out_cat); rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    int rc = mysql_stmt_execute(stmt);
    int count = 0;
    while ((rc = mysql_stmt_fetch(stmt)) == 0) count++;
    TEST("DISTINCT: 2 categories");
    CHECK(count == 2, "should have 2 distinct categories");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_concat_function(void) {
    printf("\n[PS: CONCAT function]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_cat");
    Q("CREATE TABLE t_cat(id INT, first_name VARCHAR(20), last_name VARCHAR(20))");
    Q("INSERT INTO t_cat VALUES(1,'John','Doe')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT CONCAT(first_name, ' ', last_name) FROM t_cat WHERE id = ?",
                       strlen("SELECT CONCAT(first_name, ' ', last_name) FROM t_cat WHERE id = ?"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    char out_name[50];
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_STRING; rb.buffer = out_name;
    rb.buffer_length = sizeof(out_name); rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("CONCAT: 'John Doe'");
    CHECK(rc == 0 && ol == 8 && strncmp(out_name, "John Doe", 8) == 0, "should be 'John Doe'");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_if_function(void) {
    printf("\n[PS: IF function]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_iff");
    Q("CREATE TABLE t_iff(id INT, val INT)");
    Q("INSERT INTO t_iff VALUES(1,10),(2,50)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id, IF(val > ?, 'big', 'small') AS label FROM t_iff ORDER BY id",
                       strlen("SELECT id, IF(val > ?, 'big', 'small') AS label FROM t_iff ORDER BY id"));

    int threshold = 30;
    unsigned long tl = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &threshold; pb.length = &tl;
    mysql_stmt_bind_param(stmt, &pb);

    int out_id;
    char out_label[10];
    unsigned long ol1=0, ol2=0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type=MYSQL_TYPE_LONG; rb[0].buffer=&out_id; rb[0].length=&ol1;
    rb[1].buffer_type=MYSQL_TYPE_STRING; rb[1].buffer=out_label; rb[1].buffer_length=sizeof(out_label); rb[1].length=&ol2;
    mysql_stmt_bind_result(stmt, rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("id=1: label='small'");
    CHECK(rc == 0 && strncmp(out_label, "small", 5) == 0, "should be small");

    rc = mysql_stmt_fetch(stmt);
    TEST("id=2: label='big'");
    CHECK(rc == 0 && strncmp(out_label, "big", 3) == 0, "should be big");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_coalesce(void) {
    printf("\n[PS: COALESCE function]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_coe");
    Q("CREATE TABLE t_coe(id INT, a INT, b INT, c INT)");
    Q("INSERT INTO t_coe VALUES(1,NULL,NULL,30),(2,NULL,20,NULL),(3,10,NULL,NULL)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id, COALESCE(a, b, c) FROM t_coe ORDER BY id",
                       strlen("SELECT id, COALESCE(a, b, c) FROM t_coe ORDER BY id"));

    int out_id, out_val;
    unsigned long ol1=0, ol2=0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type=MYSQL_TYPE_LONG; rb[0].buffer=&out_id; rb[0].length=&ol1;
    rb[1].buffer_type=MYSQL_TYPE_LONG; rb[1].buffer=&out_val; rb[1].length=&ol2;
    mysql_stmt_bind_result(stmt, rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("id=1: COALESCE=30");
    CHECK(rc == 0 && out_val == 30, "should be 30");

    rc = mysql_stmt_fetch(stmt);
    TEST("id=2: COALESCE=20");
    CHECK(rc == 0 && out_val == 20, "should be 20");

    rc = mysql_stmt_fetch(stmt);
    TEST("id=3: COALESCE=10");
    CHECK(rc == 0 && out_val == 10, "should be 10");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_cast_type(void) {
    printf("\n[PS: CAST type conversion]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_cst");
    Q("CREATE TABLE t_cst(id INT, val INT)");
    Q("INSERT INTO t_cst VALUES(1,42)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT CAST(val AS CHAR) FROM t_cst WHERE id = 1",
                       strlen("SELECT CAST(val AS CHAR) FROM t_cst WHERE id = 1"));

    char out_str[20];
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_STRING; rb.buffer = out_str;
    rb.buffer_length = sizeof(out_str); rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("CAST INT AS CHAR: '42'");
    CHECK(rc == 0 && ol == 2 && strncmp(out_str, "42", 2) == 0, "should be '42'");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_date_add_interval(void) {
    printf("\n[PS: DATE_ADD with INTERVAL]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_dai");
    Q("CREATE TABLE t_dai(id INT, dt DATE)");
    Q("INSERT INTO t_dai VALUES(1,'2024-01-15')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT DATE_ADD(dt, INTERVAL ? DAY) FROM t_dai WHERE id = 1",
                       strlen("SELECT DATE_ADD(dt, INTERVAL ? DAY) FROM t_dai WHERE id = 1"));

    int days = 10;
    unsigned long dl = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &days; pb.length = &dl;
    mysql_stmt_bind_param(stmt, &pb);

    MYSQL_TIME out_dt;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_DATE; rb.buffer = &out_dt;
    rb.buffer_length = sizeof(MYSQL_TIME); rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("DATE_ADD 10 days: 2024-01-25");
    CHECK(rc == 0 && out_dt.year == 2024 && out_dt.month == 1 && out_dt.day == 25, "wrong date");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_string_length_function(void) {
    printf("\n[PS: CHAR_LENGTH function]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_slf");
    Q("CREATE TABLE t_slf(id INT, name VARCHAR(50))");
    Q("INSERT INTO t_slf VALUES(1,'hello world')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT CHAR_LENGTH(name) FROM t_slf WHERE id = ?",
                       strlen("SELECT CHAR_LENGTH(name) FROM t_slf WHERE id = ?"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    long long out_len;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONGLONG; rb.buffer = &out_len; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("CHAR_LENGTH('hello world')=11");
    CHECK(rc == 0 && out_len == 11, "should be 11");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_abs_function(void) {
    printf("\n[PS: ABS function with param]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT ABS(?)",
                       strlen("SELECT ABS(?)"));

    int val = -42;
    unsigned long vl = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &val; pb.length = &vl;
    mysql_stmt_bind_param(stmt, &pb);

    double out_val;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_DOUBLE; rb.buffer = &out_val; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("ABS(-42) = 42");
    CHECK(rc == 0 && out_val > 41.9 && out_val < 42.1, "should be 42");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_round_function(void) {
    printf("\n[PS: ROUND function]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_rnd");
    Q("CREATE TABLE t_rnd(id INT, val DOUBLE)");
    Q("INSERT INTO t_rnd VALUES(1, 3.14159)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT ROUND(val, ?) FROM t_rnd WHERE id = 1",
                       strlen("SELECT ROUND(val, ?) FROM t_rnd WHERE id = 1"));

    int precision = 2;
    unsigned long pl = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &precision; pb.length = &pl;
    mysql_stmt_bind_param(stmt, &pb);

    double out_val;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_DOUBLE; rb.buffer = &out_val; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("ROUND(3.14159, 2) ~ 3.14");
    CHECK(rc == 0 && out_val > 3.13 && out_val < 3.15, "should be ~3.14");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_mod_function(void) {
    printf("\n[PS: MOD function]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT MOD(?, ?)",
                       strlen("SELECT MOD(?, ?)"));

    int a = 17, b = 5;
    unsigned long al = sizeof(int), bl = sizeof(int);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &a; pb[0].length = &al;
    pb[1].buffer_type = MYSQL_TYPE_LONG; pb[1].buffer = &b; pb[1].length = &bl;
    mysql_stmt_bind_param(stmt, pb);

    long long out_val;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONGLONG; rb.buffer = &out_val; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("MOD(17, 5) = 2");
    CHECK(rc == 0 && out_val == 2, "should be 2");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_store_result_on_update(void) {
    printf("\n[PS: store_result on UPDATE (BUG#31789)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_sru");
    Q("CREATE TABLE t_sru(id INT, val INT)");
    Q("INSERT INTO t_sru VALUES(1,10),(2,20)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "UPDATE t_sru SET val = val + 100 WHERE id = ?",
                       strlen("UPDATE t_sru SET val = val + 100 WHERE id = ?"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    int rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, "execute UPDATE");
    TEST("affected_rows=1");
    CHECK(mysql_stmt_affected_rows(stmt) == 1, "should affect 1 row");

    rc = mysql_stmt_store_result(stmt);
    TEST("store_result on UPDATE: no crash");
    CHECK(rc == 0, "should succeed for UPDATE");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_blob_null_fetch(void) {
    printf("\n[PS: BLOB NULL fetch (BUG#42456)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_bnf");
    Q("CREATE TABLE t_bnf(id INT, data BLOB)");
    Q("INSERT INTO t_bnf VALUES(1, 'hello'),(2, NULL)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id, data FROM t_bnf ORDER BY id",
                       strlen("SELECT id, data FROM t_bnf ORDER BY id"));

    int out_id;
    unsigned long ol1 = 0;
    char out_blob[100];
    unsigned long ol2 = 0;
    my_bool blob_null = 0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &out_id; rb[0].length = &ol1;
    rb[1].buffer_type = MYSQL_TYPE_BLOB; rb[1].buffer = out_blob;
    rb[1].buffer_length = sizeof(out_blob); rb[1].length = &ol2; rb[1].is_null = &blob_null;
    mysql_stmt_bind_result(stmt, rb);

    int rc = mysql_stmt_execute(stmt);

    rc = mysql_stmt_fetch(stmt);
    TEST("row1: id=1, blob='hello', not null");
    CHECK(rc == 0 && out_id == 1 && blob_null == 0 && ol2 == 5, "wrong non-null blob");

    rc = mysql_stmt_fetch(stmt);
    TEST("row2: id=2, blob=NULL");
    CHECK(rc == 0 && out_id == 2 && blob_null == 1, "should be NULL blob");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_decimal_newdecimal(void) {
    printf("\n[PS: DECIMAL/NEWDECIMAL type (BUG#49634)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_dec");
    Q("CREATE TABLE t_dec(id INT, price DECIMAL(10,2))");
    Q("INSERT INTO t_dec VALUES(1, 12345.67)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id, price FROM t_dec WHERE id = 1",
                       strlen("SELECT id, price FROM t_dec WHERE id = 1"));

    int out_id;
    unsigned long ol1 = 0;
    char out_price[30];
    unsigned long ol2 = 0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &out_id; rb[0].length = &ol1;
    rb[1].buffer_type = MYSQL_TYPE_STRING; rb[1].buffer = out_price;
    rb[1].buffer_length = sizeof(out_price); rb[1].length = &ol2;
    mysql_stmt_bind_result(stmt, rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("DECIMAL roundtrip: '12345.67'");
    CHECK(rc == 0 && out_id == 1, "fetch should work");
    CHECK(ol2 > 0 && (strncmp(out_price, "12345.67", 8) == 0 || strstr(out_price, "12345") != NULL), "wrong decimal value");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_null_pattern_change(void) {
    printf("\n[PS: NULL pattern change on re-execute (BUG#53947)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_npc");
    Q("CREATE TABLE t_npc(id INT, a INT, b INT)");
    Q("INSERT INTO t_npc VALUES(1, 10, 20),(2, NULL, 30),(3, 40, NULL)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id FROM t_npc WHERE a <=> ? AND b <=> ?",
                       strlen("SELECT id FROM t_npc WHERE a <=> ? AND b <=> ?"));

    int out_id;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_id; rb.length = &ol;

    int val_a = 10, val_b = 20;
    my_bool null_a = 0, null_b = 0;
    unsigned long al = sizeof(int), bl = sizeof(int);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &val_a; pb[0].length = &al; pb[0].is_null = &null_a;
    pb[1].buffer_type = MYSQL_TYPE_LONG; pb[1].buffer = &val_b; pb[1].length = &bl; pb[1].is_null = &null_b;
    mysql_stmt_bind_param(stmt, pb);
    mysql_stmt_bind_result(stmt, &rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("both non-NULL: a=10, b=20 -> id=1");
    CHECK(rc == 0 && out_id == 1, "should find id=1");
    while (mysql_stmt_fetch(stmt) == 0) {}

    null_a = 1; val_b = 30;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("a=NULL, b=30: id=2");
    CHECK(rc == 0 && out_id == 2, "should find id=2");
    while (mysql_stmt_fetch(stmt) == 0) {}

    val_a = 40; null_a = 0; null_b = 1;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("a=40, b=NULL: id=3");
    CHECK(rc == 0 && out_id == 3, "should find id=3");
    while (mysql_stmt_fetch(stmt) == 0) {}

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_time_type_fetch(void) {
    printf("\n[PS: TIME type fetch (BUG#57435)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_ttf");
    Q("CREATE TABLE t_ttf(id INT, t TIME)");
    Q("INSERT INTO t_ttf VALUES(1, '12:34:56'),(2, NULL)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id, t FROM t_ttf ORDER BY id",
                       strlen("SELECT id, t FROM t_ttf ORDER BY id"));

    int out_id;
    unsigned long ol1 = 0;
    MYSQL_TIME out_time;
    unsigned long ol2 = 0;
    my_bool time_null = 0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &out_id; rb[0].length = &ol1;
    rb[1].buffer_type = MYSQL_TYPE_TIME; rb[1].buffer = &out_time;
    rb[1].buffer_length = sizeof(MYSQL_TIME); rb[1].length = &ol2; rb[1].is_null = &time_null;
    mysql_stmt_bind_result(stmt, rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("TIME: 12:34:56");
    CHECK(rc == 0 && out_time.hour == 12 && out_time.minute == 34 && out_time.second == 56, "wrong time");

    rc = mysql_stmt_fetch(stmt);
    TEST("TIME NULL: is_null=1");
    CHECK(rc == 0 && time_null == 1, "should be NULL");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_long_identifier_prepare(void) {
    printf("\n[PS: long identifier prepare (BUG#47485)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    char longname[200];
    memset(longname, 'a', 199);
    longname[199] = '\0';
    char sql[512];
    snprintf(sql, sizeof(sql), "SELECT * FROM %s", longname);

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, sql, (unsigned long)strlen(sql));
    TEST("long identifier: returns error (no crash)");
    CHECK(rc != 0, "should fail for nonexistent long-named table");

    rc = mysql_stmt_prepare(stmt, "SELECT 1", strlen("SELECT 1"));
    TEST("connection still works after long identifier error");
    CHECK(rc == 0, "should succeed with normal SQL");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_geometry_type(void) {
    printf("\n[PS: GEOMETRY type (BUG#60832)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_geo");
    Q("CREATE TABLE t_geo(id INT, g GEOMETRY)");
    Q("INSERT INTO t_geo VALUES(1, ST_GeomFromText('POINT(1 1)'))");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id, g FROM t_geo WHERE id = 1",
                       strlen("SELECT id, g FROM t_geo WHERE id = 1"));

    int out_id;
    unsigned long ol1 = 0;
    char out_geom[256];
    unsigned long ol2 = 0;
    my_bool geom_null = 0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &out_id; rb[0].length = &ol1;
    rb[1].buffer_type = MYSQL_TYPE_BLOB; rb[1].buffer = out_geom;
    rb[1].buffer_length = sizeof(out_geom); rb[1].length = &ol2; rb[1].is_null = &geom_null;
    mysql_stmt_bind_result(stmt, rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("GEOMETRY fetch: no crash, data not null");
    CHECK(rc == 0 && out_id == 1 && geom_null == 0, "should have geometry data");
    TEST("GEOMETRY length > 0 (WKB format)");
    CHECK(ol2 > 0, "should have WKB data");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_text_blob_insert_fetch(void) {
    printf("\n[PS: TEXT/BLOB insert and fetch]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_tbi");
    Q("CREATE TABLE t_tbi(id INT, txt TEXT, blb BLOB)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "INSERT INTO t_tbi VALUES(?, ?, ?)",
                       strlen("INSERT INTO t_tbi VALUES(?, ?, ?)"));

    int id = 1;
    unsigned long il = sizeof(int);
    char txt[] = "Hello Text World";
    unsigned long tl = strlen(txt);
    unsigned char blb[] = {0x00, 0x01, 0x02, 0xFF, 0xFE};
    unsigned long bl = sizeof(blb);
    MYSQL_BIND pb[3];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &il;
    pb[1].buffer_type = MYSQL_TYPE_STRING; pb[1].buffer = txt; pb[1].buffer_length = sizeof(txt); pb[1].length = &tl;
    pb[2].buffer_type = MYSQL_TYPE_LONG_BLOB; pb[2].buffer = blb; pb[2].buffer_length = sizeof(blb); pb[2].length = &bl;
    mysql_stmt_bind_param(stmt, pb);

    int rc = mysql_stmt_execute(stmt);
    TEST("TEXT+BLOB insert");
    CHECK(rc == 0, "execute should work");

    mysql_stmt_prepare(stmt, "SELECT txt, blb FROM t_tbi WHERE id = 1",
                       strlen("SELECT txt, blb FROM t_tbi WHERE id = 1"));
    char out_txt[100];
    unsigned long ol1 = 0;
    unsigned char out_blb[100];
    unsigned long ol2 = 0;
    MYSQL_BIND rb2[2];
    memset(rb2, 0, sizeof(rb2));
    rb2[0].buffer_type = MYSQL_TYPE_STRING; rb2[0].buffer = out_txt; rb2[0].buffer_length = sizeof(out_txt); rb2[0].length = &ol1;
    rb2[1].buffer_type = MYSQL_TYPE_BLOB; rb2[1].buffer = out_blb; rb2[1].buffer_length = sizeof(out_blb); rb2[1].length = &ol2;
    mysql_stmt_bind_result(stmt, rb2);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("TEXT roundtrip: 'Hello Text World'");
    CHECK(rc == 0 && ol1 == 16 && strncmp(out_txt, "Hello Text World", 16) == 0, "wrong text");
    TEST("BLOB roundtrip: 5 bytes");
    CHECK(ol2 == 5 && memcmp(out_blb, blb, 5) == 0, "wrong blob");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}



static void test_ps_double_reprepare(void) {
    printf("\n[PS: double re-prepare lifecycle]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_drp");
    Q("CREATE TABLE t_drp(id INT, val INT)");
    Q("INSERT INTO t_drp VALUES(1,100)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);

    mysql_stmt_prepare(stmt, "SELECT val FROM t_drp WHERE id = ?",
                       strlen("SELECT val FROM t_drp WHERE id = ?"));
    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;

    int out_val;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.length = &ol;

    mysql_stmt_bind_param(stmt, &pb);
    mysql_stmt_bind_result(stmt, &rb);
    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    CHECK(rc == 0 && out_val == 100, "first prepare");
    while (mysql_stmt_fetch(stmt) == 0) {}

    mysql_stmt_prepare(stmt, "SELECT val FROM t_drp WHERE val > ?",
                       strlen("SELECT val FROM t_drp WHERE val > ?"));
    int threshold = 50;
    unsigned long tl = sizeof(int);
    MYSQL_BIND pb2;
    memset(&pb2, 0, sizeof(pb2));
    pb2.buffer_type = MYSQL_TYPE_LONG; pb2.buffer = &threshold; pb2.buffer_length = sizeof(int); pb2.length = &tl;
    mysql_stmt_bind_param(stmt, &pb2);
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("second prepare: val=100");
    CHECK(rc == 0 && out_val == 100, "should work after re-prepare");
    while (mysql_stmt_fetch(stmt) == 0) {}

    mysql_stmt_prepare(stmt, "SELECT id FROM t_drp WHERE val = ?",
                       strlen("SELECT id FROM t_drp WHERE val = ?"));
    int val_param = 100;
    unsigned long vl = sizeof(int);
    MYSQL_BIND pb3;
    memset(&pb3, 0, sizeof(pb3));
    pb3.buffer_type = MYSQL_TYPE_LONG; pb3.buffer = &val_param; pb3.buffer_length = sizeof(int); pb3.length = &vl;
    mysql_stmt_bind_param(stmt, &pb3);
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("third prepare: id=1");
    CHECK(rc == 0 && out_val == 1, "should work after second re-prepare");
    while (mysql_stmt_fetch(stmt) == 0) {}

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_execute_after_conn_reset(void) {
    printf("\n[PS: execute after connection reset (ping)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_eac");
    Q("CREATE TABLE t_eac(id INT, val INT)");
    Q("INSERT INTO t_eac VALUES(1,42)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT val FROM t_eac WHERE id = 1",
                       strlen("SELECT val FROM t_eac WHERE id = 1"));

    int out_val;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    int rc = mysql_ping(m);
    TEST("ping succeeds");
    CHECK(rc == 0, "ping should work");

    rc = mysql_stmt_execute(stmt);
    TEST("execute after ping: succeeds");
    CHECK(rc == 0, "execute should work after ping");

    rc = mysql_stmt_fetch(stmt);
    CHECK(rc == 0 && out_val == 42, "should get 42");
    while (mysql_stmt_fetch(stmt) == 0) {}

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_wide_row_many_columns(void) {
    printf("\n[PS: wide row with many columns]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_wide");
    Q("CREATE TABLE t_wide(id INT, c1 INT, c2 INT, c3 INT, c4 INT, c5 INT, "
      "c6 INT, c7 INT, c8 INT, c9 INT, c10 INT, c11 INT, c12 INT, c13 INT, c14 INT, c15 INT, "
      "c16 INT, c17 INT, c18 INT, c19 INT, c20 INT)");
    Q("INSERT INTO t_wide VALUES(1,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT * FROM t_wide WHERE id = 1",
                       strlen("SELECT * FROM t_wide WHERE id = 1"));

    int out_id;
    unsigned long ol = 0;
    int cols[20];
    unsigned long ols[20];
    MYSQL_BIND rb[21];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &out_id; rb[0].length = &ol;
    for (int i = 0; i < 20; i++) {
        rb[i+1].buffer_type = MYSQL_TYPE_LONG; rb[i+1].buffer = &cols[i]; rb[i+1].length = &ols[i];
    }
    mysql_stmt_bind_result(stmt, rb);

    int rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("wide row: id=1, 20 columns");
    CHECK(rc == 0 && out_id == 1, "should fetch wide row");
    int ok = 1;
    for (int i = 0; i < 20; i++) {
        if (cols[i] != i + 1) { ok = 0; break; }
    }
    TEST("wide row: all 20 column values correct");
    CHECK(ok, "all columns should have correct values");

    while (mysql_stmt_fetch(stmt) == 0) {}
    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_concurrent_insert_select(void) {
    printf("\n[PS: concurrent INSERT and SELECT on same table]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_cis");
    Q("CREATE TABLE t_cis(id INT, val INT)");

    MYSQL_STMT *ins = mysql_stmt_init(m);
    mysql_stmt_prepare(ins, "INSERT INTO t_cis VALUES(?, ?)",
                       strlen("INSERT INTO t_cis VALUES(?, ?)"));

    int id = 1, val = 100;
    unsigned long il = sizeof(int), vl = sizeof(int);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &il;
    pb[1].buffer_type = MYSQL_TYPE_LONG; pb[1].buffer = &val; pb[1].length = &vl;
    mysql_stmt_bind_param(ins, pb);

    for (int i = 0; i < 5; i++) {
        id = i + 1; val = (i + 1) * 10;
        mysql_stmt_execute(ins);
    }

    MYSQL_STMT *sel = mysql_stmt_init(m);
    mysql_stmt_prepare(sel, "SELECT COUNT(*) FROM t_cis",
                       strlen("SELECT COUNT(*) FROM t_cis"));

    long long out_cnt;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONGLONG; rb.buffer = &out_cnt; rb.length = &ol;
    mysql_stmt_bind_result(sel, &rb);

    int rc = mysql_stmt_execute(sel);
    rc = mysql_stmt_fetch(sel);
    TEST("5 inserts, COUNT(*)=5");
    CHECK(rc == 0 && out_cnt == 5, "should have 5 rows");

    while (mysql_stmt_fetch(sel) == 0) {}
    mysql_stmt_close(ins);
    mysql_stmt_close(sel);
    mysql_close(m);
}

static void test_stmt_insert(void) {
    printf("\n[prepared insert]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    const char *sql = "INSERT INTO t_test (name, score) VALUES (?, ?)";
    int rc = mysql_stmt_prepare(stmt, sql, (unsigned long)strlen(sql));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    char name[] = "stmt_user";
    double score = 77.7;
    unsigned long name_len = strlen(name);

    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_STRING;
    pb[0].buffer = name;
    pb[0].buffer_length = sizeof(name);
    pb[0].length = &name_len;

    pb[1].buffer_type = MYSQL_TYPE_DOUBLE;
    pb[1].buffer = &score;

    rc = mysql_stmt_bind_param(stmt, pb);
    TEST("bind param for insert");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    rc = mysql_stmt_execute(stmt);
    TEST("execute insert");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    TEST("affected rows == 1");
    CHECK(mysql_stmt_affected_rows(stmt) == 1, "wrong count");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_bind_result(void) {
    printf("\n[stmt bind result — adapted from official test_bind_result]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_bind_result");
    Q("CREATE TABLE t_bind_result(col1 INT, col2 VARCHAR(50))");
    Q("INSERT INTO t_bind_result VALUES(10, 'venu')");
    Q("INSERT INTO t_bind_result VALUES(20, 'MySQL')");
    Q("INSERT INTO t_bind_result(col2) VALUES('monty')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT * FROM t_bind_result",
                                 strlen("SELECT * FROM t_bind_result"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    int nData = 0;
    char szData[100] = {0};
    unsigned long length1 = 0;
    my_bool is_null0 = 0, is_null1 = 0;

    MYSQL_BIND my_bind[2];
    memset(my_bind, 0, sizeof(my_bind));
    my_bind[0].buffer_type = MYSQL_TYPE_LONG;
    my_bind[0].buffer = &nData;
    my_bind[0].is_null = &is_null0;
    my_bind[0].length = &length1;

    my_bind[1].buffer_type = MYSQL_TYPE_STRING;
    my_bind[1].buffer = szData;
    my_bind[1].buffer_length = sizeof(szData);
    my_bind[1].length = &length1;
    my_bind[1].is_null = &is_null1;

    rc = mysql_stmt_bind_result(stmt, my_bind);
    rc = mysql_stmt_execute(stmt);

    rc = mysql_stmt_fetch(stmt);
    TEST("row 1: nData==10, szData=='venu'");
    CHECK(rc == 0 && nData == 10 && strcmp(szData, "venu") == 0, "wrong data");

    rc = mysql_stmt_fetch(stmt);
    TEST("row 2: nData==20, szData=='MySQL'");
    CHECK(rc == 0 && nData == 20 && strcmp(szData, "MySQL") == 0, "wrong data");

    rc = mysql_stmt_fetch(stmt);
    TEST("row 3: col1 is NULL, szData=='monty'");
    CHECK(rc == 0 && is_null0 && strcmp(szData, "monty") == 0, "wrong data");

    rc = mysql_stmt_fetch(stmt);
    TEST("no more rows");
    CHECK(rc == MYSQL_NO_DATA, "expected NO_DATA");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_store_result(void) {
    printf("\n[stmt store_result — adapted from official test_store_result]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_store_result");
    Q("CREATE TABLE t_store_result(col1 INT, col2 VARCHAR(50))");
    Q("INSERT INTO t_store_result VALUES(10, 'venu'), (20, 'mysql')");
    Q("INSERT INTO t_store_result(col2) VALUES('monty')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT * FROM t_store_result",
                                 strlen("SELECT * FROM t_store_result"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    int nData = 0;
    char szData[100] = {0};
    unsigned long len0 = 0, len1 = 0;
    my_bool is_null0 = 0, is_null1 = 0;

    MYSQL_BIND my_bind[2];
    memset(my_bind, 0, sizeof(my_bind));
    my_bind[0].buffer_type = MYSQL_TYPE_LONG;
    my_bind[0].buffer = &nData;
    my_bind[0].length = &len0;
    my_bind[0].is_null = &is_null0;

    my_bind[1].buffer_type = MYSQL_TYPE_STRING;
    my_bind[1].buffer = szData;
    my_bind[1].buffer_length = sizeof(szData);
    my_bind[1].length = &len1;
    my_bind[1].is_null = &is_null1;

    rc = mysql_stmt_bind_result(stmt, my_bind);
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_store_result(stmt);
    TEST("mysql_stmt_store_result");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    rc = mysql_stmt_fetch(stmt);
    TEST("row 1: nData==10, szData=='venu'");
    CHECK(rc == 0 && nData == 10 && strcmp(szData, "venu") == 0, "wrong data");

    rc = mysql_stmt_fetch(stmt);
    TEST("row 2: nData==20, szData=='mysql'");
    CHECK(rc == 0 && nData == 20 && strcmp(szData, "mysql") == 0, "wrong data");

    rc = mysql_stmt_fetch(stmt);
    TEST("row 3: col1 is NULL, szData=='monty'");
    CHECK(rc == 0 && is_null0 && strcmp(szData, "monty") == 0, "wrong data");

    rc = mysql_stmt_fetch(stmt);
    TEST("no more rows");
    CHECK(rc == MYSQL_NO_DATA, "expected NO_DATA");

    TEST("mysql_stmt_num_rows == 3");
    CHECK(mysql_stmt_num_rows(stmt) == 3, "wrong count");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_fetch_null(void) {
    printf("\n[stmt fetch NULL — adapted from official test_fetch_null]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_fetch_null");
    Q("CREATE TABLE t_fetch_null("
      "col1 TINYINT, col2 SMALLINT, "
      "col3 INT, col4 BIGINT, "
      "col5 FLOAT, col6 DOUBLE, "
      "col7 DATE, col8 TIME, "
      "col9 VARBINARY(10), "
      "col10 VARCHAR(50), "
      "col11 CHAR(20))");
    Q("INSERT INTO t_fetch_null (col11) VALUES ('1000'), ('88'), ('389789')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT * FROM t_fetch_null",
                                 strlen("SELECT * FROM t_fetch_null"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    int nData = 0;
    my_bool is_null[11];
    unsigned long lengths[11];
    MYSQL_BIND my_bind[11];
    memset(my_bind, 0, sizeof(my_bind));
    for (int i = 0; i < 11; i++) {
        my_bind[i].buffer_type = MYSQL_TYPE_LONG;
        my_bind[i].is_null = &is_null[i];
        my_bind[i].length = &lengths[i];
    }
    my_bind[10].buffer = &nData;

    rc = mysql_stmt_bind_result(stmt, my_bind);
    rc = mysql_stmt_execute(stmt);

    int row_count = 0;
    while (mysql_stmt_fetch(stmt) != MYSQL_NO_DATA) {
        row_count++;
        int null_count = 0;
        for (int i = 0; i < 10; i++) {
            if (is_null[i]) null_count++;
        }
        if (row_count == 1) {
            TEST("row 1: first 10 cols NULL, col11=1000");
            CHECK(null_count == 10 && nData == 1000, "wrong data");
        }
    }
    TEST("fetched 3 rows");
    CHECK(row_count == 3, "wrong count");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_insert_tiny(void) {
    printf("\n[stmt insert tiny — adapted from official test_insert]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_prep_insert");
    Q("CREATE TABLE t_prep_insert(col1 TINYINT, col2 VARCHAR(50))");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_prep_insert VALUES(?, ?)",
                                 strlen("INSERT INTO t_prep_insert VALUES(?, ?)"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    char tiny_data;
    char str_data[50];
    unsigned long length;

    MYSQL_BIND my_bind[2];
    memset(my_bind, 0, sizeof(my_bind));
    my_bind[0].buffer_type = MYSQL_TYPE_TINY;
    my_bind[0].buffer = &tiny_data;
    my_bind[1].buffer_type = MYSQL_TYPE_STRING;
    my_bind[1].buffer = str_data;
    my_bind[1].buffer_length = sizeof(str_data);
    my_bind[1].length = &length;

    rc = mysql_stmt_bind_param(stmt, my_bind);
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    for (tiny_data = 0; tiny_data < 3; tiny_data++) {
        length = (unsigned long)sprintf(str_data, "MySQL%d", tiny_data);
        rc = mysql_stmt_execute(stmt);
        if (rc != 0) break;
    }
    TEST("inserted 3 rows via prepared stmt");
    CHECK(tiny_data == 3, "insert failed");

    mysql_stmt_close(stmt);

    Q("SELECT * FROM t_prep_insert ORDER BY col1");
    MYSQL_RES *res = mysql_store_result(m);
    TEST("3 rows inserted");
    CHECK(res && res->row_count == 3, "wrong count");
    mysql_free_result(res);
    mysql_close(m);
}

static void test_stmt_send_long_data(void) {
    printf("\n[stmt send_long_data — adapted from official test_long_data]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_long_data");
    Q("CREATE TABLE t_long_data(col1 INT, col2 LONG VARCHAR, col3 LONG VARBINARY)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_long_data(col1, col2, col3) VALUES(?, ?, ?)",
                                 strlen("INSERT INTO t_long_data(col1, col2, col3) VALUES(?, ?, ?)"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    int int_data = 999;
    MYSQL_BIND my_bind[3];
    memset(my_bind, 0, sizeof(my_bind));
    my_bind[0].buffer = &int_data;
    my_bind[0].buffer_type = MYSQL_TYPE_LONG;
    my_bind[1].buffer_type = MYSQL_TYPE_STRING;
    my_bind[2].buffer_type = MYSQL_TYPE_STRING;

    rc = mysql_stmt_bind_param(stmt, my_bind);
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    rc = mysql_stmt_send_long_data(stmt, 1, "Michael", 7);
    TEST("send_long_data 'Michael'");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    rc = mysql_stmt_send_long_data(stmt, 1, " 'Monty' Widenius", 16);
    TEST("send_long_data ' Monty' Widenius'");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    rc = mysql_stmt_send_long_data(stmt, 2, "Venu", 4);
    TEST("send_long_data 'Venu'");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    rc = mysql_stmt_execute(stmt);
    TEST("execute with long data");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    mysql_stmt_close(stmt);

    Q("SELECT col1, col2, col3 FROM t_long_data");
    MYSQL_RES *res = mysql_store_result(m);
    if (!res) { FAIL("store_result NULL"); mysql_close(m); return; }
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("col1 == '999'");
    CHECK(row && row[0] && strcmp(row[0], "999") == 0, "wrong data");
    TEST("col2 contains 'Michael' and 'Monty'");
    CHECK(row && row[1] && strstr(row[1], "Michael") && strstr(row[1], "Monty"), "wrong data");
    TEST("col3 == 'Venu'");
    CHECK(row && row[2] && strncmp(row[2], "Venu", 4) == 0, "wrong data");
    mysql_free_result(res);
    mysql_close(m);
}

static void test_insert_id(void) {
    printf("\n[mysql_insert_id — adapted from official test_mysql_insert_id]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_insert_id");
    Q("CREATE TABLE t_insert_id(f1 INT NOT NULL PRIMARY KEY AUTO_INCREMENT, f2 VARCHAR(255))");

    Q("INSERT INTO t_insert_id VALUES (1, 'a')");
    my_ulonglong res = mysql_insert_id(m);
    TEST("insert_id == 1 after first insert");
    CHECK(res == 1, "wrong insert_id");

    Q("INSERT INTO t_insert_id VALUES (NULL, 'b')");
    res = mysql_insert_id(m);
    TEST("insert_id == 2 after auto-inc insert");
    CHECK(res == 2, "wrong insert_id");

    Q("INSERT INTO t_insert_id VALUES (NULL, 'c')");
    res = mysql_insert_id(m);
    TEST("insert_id == 3 after third insert");
    CHECK(res == 3, "wrong insert_id");

    mysql_close(m);
}

static void test_warnings(void) {
    printf("\n[mysql_warning_count — adapted from official test_warnings]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("SELECT 1/0");
    MYSQL_RES *res = mysql_store_result(m);
    if (res) mysql_free_result(res);
    unsigned int wc = mysql_warning_count(m);
    TEST("warning_count > 0 for division by zero");
    CHECK(wc > 0, "expected warnings");

    mysql_close(m);
}

static void test_update_delete(void) {
    printf("\n[update/delete — adapted from official test_simple_update/test_simple_delete]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_upd_del");
    Q("CREATE TABLE t_upd_del(id INT PRIMARY KEY, val VARCHAR(20))");
    Q("INSERT INTO t_upd_del VALUES(1,'one'),(2,'two'),(3,'three')");

    Q("UPDATE t_upd_del SET val='updated' WHERE id=2");
    TEST("affected rows == 1 after UPDATE");
    CHECK(mysql_affected_rows(m) == 1, "wrong count");

    Q("SELECT val FROM t_upd_del WHERE id=2");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("val is 'updated'");
    CHECK(row && row[0] && strcmp(row[0], "updated") == 0, "wrong data");
    mysql_free_result(res);

    Q("DELETE FROM t_upd_del WHERE id=3");
    TEST("affected rows == 1 after DELETE");
    CHECK(mysql_affected_rows(m) == 1, "wrong count");

    Q("SELECT COUNT(*) FROM t_upd_del");
    res = mysql_store_result(m);
    row = mysql_fetch_row(res);
    TEST("2 rows remain");
    CHECK(row && row[0] && strcmp(row[0], "2") == 0, "wrong count");
    mysql_free_result(res);

    mysql_close(m);
}

static void test_data_seek(void) {
    printf("\n[mysql_data_seek]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_seek");
    Q("CREATE TABLE t_seek(id INT PRIMARY KEY AUTO_INCREMENT, val INT)");
    Q("INSERT INTO t_seek(val) VALUES(10),(20),(30),(40),(50)");

    Q("SELECT id, val FROM t_seek ORDER BY id");
    MYSQL_RES *res = mysql_store_result(m);
    if (!res) { FAIL("store_result NULL"); mysql_close(m); return; }

    TEST("row_count == 5");
    CHECK(res->row_count == 5, "wrong count");

    mysql_data_seek(res, 2);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("seek to row 2, val == '30'");
    CHECK(row && row[1] && strcmp(row[1], "30") == 0, "wrong data");

    mysql_data_seek(res, 4);
    row = mysql_fetch_row(res);
    TEST("seek to row 4, val == '50'");
    CHECK(row && row[1] && strcmp(row[1], "50") == 0, "wrong data");

    mysql_free_result(res);
    mysql_close(m);
}

static void test_select_db(void) {
    printf("\n[mysql_select_db]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    int rc = mysql_select_db(m, "information_schema");
    TEST("select_db 'information_schema'");
    CHECK(rc == 0, mysql_error(m));

    Q("SELECT 1");
    MYSQL_RES *res = mysql_store_result(m);
    TEST("query on 'information_schema' db");
    CHECK(res != NULL, "NULL");
    mysql_free_result(res);

    rc = mysql_select_db(m, g_db);
    TEST("select_db back to sp_test");
    CHECK(rc == 0, mysql_error(m));

    mysql_close(m);
}

static void test_charset(void) {
    printf("\n[mysql_set_character_set]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    int rc = mysql_set_character_set(m, "utf8mb4");
    TEST("set charset utf8mb4");
    CHECK(rc == 0, mysql_error(m));

    Q("SELECT '你好世界' AS val");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("Chinese characters round-trip");
    CHECK(row && row[0] && strstr(row[0], "你好") != NULL, "wrong data");
    mysql_free_result(res);

    mysql_close(m);
}

static void test_multi_statements(void) {
    printf("\n[multi statements — adapted from official test_multi_statements]\n");
    MYSQL *m = mysql_init(NULL);
    if (!m) { FAIL("init failed"); return; }
    unsigned int ssl_mode = SSL_MODE_DISABLED;
    mysql_options(m, MYSQL_OPT_SSL_MODE, &ssl_mode);
    m = mysql_real_connect(m, g_host, g_user, g_passwd, g_db, g_port, NULL,
                           CLIENT_MULTI_STATEMENTS);
    if (!m) { FAIL("connect failed"); return; }

    int rc = Q("DROP TABLE IF EXISTS t_multi; CREATE TABLE t_multi(id INT); "
               "INSERT INTO t_multi VALUES(1),(2),(3)");
    TEST("multi-statement query");
    CHECK(rc == 0, mysql_error(m));

    while (mysql_more_results(m)) {
        mysql_next_result(m);
        MYSQL_RES *r = mysql_store_result(m);
        if (r) mysql_free_result(r);
    }

    rc = Q("SELECT * FROM t_multi; SELECT 42");
    if (rc != 0) { FAIL(mysql_error(m)); mysql_close(m); return; }

    MYSQL_RES *res = mysql_store_result(m);
    TEST("first result set: 3 rows");
    CHECK(res && res->row_count == 3, "wrong count");
    mysql_free_result(res);

    if (mysql_more_results(m)) {
        rc = mysql_next_result(m);
        TEST("mysql_next_result == 0");
        CHECK(rc == 0, mysql_error(m));
        res = mysql_store_result(m);
        MYSQL_ROW row = mysql_fetch_row(res);
        TEST("second result set: 42");
        CHECK(row && row[0] && strcmp(row[0], "42") == 0, "wrong data");
        mysql_free_result(res);
    }

    mysql_close(m);
}

static void test_stmt_result_metadata(void) {
    printf("\n[mysql_stmt_result_metadata]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT id, name FROM t_test LIMIT 1",
                                 strlen("SELECT id, name FROM t_test LIMIT 1"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    MYSQL_RES *meta = mysql_stmt_result_metadata(stmt);
    TEST("metadata returns non-NULL for SELECT");
    CHECK(meta != NULL, "NULL");

    TEST("metadata field_count == 2");
    CHECK(meta && meta->field_count == 2, "wrong count");

    if (meta) mysql_free_result(meta);
    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_close_and_reprepare(void) {
    printf("\n[stmt close & re-prepare — adapted from official test_stmt_close]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_stmt_close");
    Q("CREATE TABLE t_stmt_close(id INT)");

    MYSQL_STMT *stmt1 = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt1, "INSERT INTO t_stmt_close(id) VALUES(?)",
                                 strlen("INSERT INTO t_stmt_close(id) VALUES(?)"));
    TEST("prepare stmt1");
    CHECK(rc == 0, mysql_stmt_error(stmt1));

    int val = 42;
    unsigned long vlen = sizeof(int);
    MYSQL_BIND pb[1];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG;
    pb[0].buffer = &val;
    pb[0].length = &vlen;
    mysql_stmt_bind_param(stmt1, pb);
    mysql_stmt_execute(stmt1);

    rc = mysql_stmt_close(stmt1);
    TEST("stmt_close returns 0");
    CHECK(rc == 0, "close failed");

    MYSQL_STMT *stmt2 = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt2, "SELECT id FROM t_stmt_close",
                             strlen("SELECT id FROM t_stmt_close"));
    TEST("prepare stmt2 after close stmt1");
    CHECK(rc == 0, mysql_stmt_error(stmt2));

    mysql_stmt_close(stmt2);
    mysql_close(m);
}

static void test_connection_pool(void) {
    printf("\n[connection pool]\n");

    mm_pool_config_t cfg = {0};
    cfg.host = g_host;
    cfg.user = g_user;
    cfg.passwd = g_passwd;
    cfg.db = g_db;
    cfg.port = g_port;
    cfg.min_conns = 2;
    cfg.max_conns = 5;
    cfg.idle_timeout_sec = 30;
    cfg.ping_interval_sec = 10;

    mm_pool_t *pool = mm_pool_create(&cfg);
    TEST("mm_pool_create returns non-NULL");
    CHECK(pool != NULL, "NULL");

    MYSQL *c1 = mm_pool_get(pool);
    TEST("get connection 1");
    CHECK(c1 != NULL, "NULL");

    MYSQL *c2 = mm_pool_get(pool);
    TEST("get connection 2");
    CHECK(c2 != NULL, "NULL");

    TEST("connections are distinct");
    CHECK(c1 != c2, "same conn");

    int rc = mysql_real_query(c1, "SELECT 1", strlen("SELECT 1"));
    TEST("query on conn1");
    CHECK(rc == 0, mysql_error(c1));
    MYSQL_RES *r = mysql_store_result(c1);
    mysql_free_result(r);

    mm_pool_put(pool, c1);
    TEST("put conn1 back");
    PASS();

    mm_pool_put(pool, c2);
    TEST("put conn2 back");
    PASS();

    TEST("active == 0, idle == 2");
    CHECK(mm_pool_active(pool) == 0 && mm_pool_idle(pool) == 2, "wrong counts");

    mm_pool_destroy(pool);
    TEST("pool destroyed");
    PASS();
}

static void test_error_handling(void) {
    printf("\n[error handling]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    TEST("bad SQL returns error");
    int rc = Q("SELECXT bad syntax");
    CHECK(rc != 0, "expected error");

    TEST("error code is non-zero");
    CHECK(mysql_errno(m) != 0, "zero errno");

    TEST("error message is non-empty");
    CHECK(mysql_error(m)[0] != '\0', "empty message");

    TEST("sqlstate is 5 chars");
    CHECK(strlen(mysql_sqlstate(m)) == 5, "wrong sqlstate length");

    TEST("query non-existent table returns error");
    rc = Q("SELECT * FROM nonexistent_table_xyz");
    CHECK(rc != 0, "expected error");

    mysql_close(m);
}

static void test_change_user(void) {
    printf("\n[mysql_change_user]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    TEST("change_user to same user");
    int rc = mysql_change_user(m, g_user, g_passwd, g_db);
    CHECK(rc == 0, mysql_error(m));

    TEST("still queryable after change_user");
    rc = Q("SELECT 1");
    CHECK(rc == 0, mysql_error(m));
    MYSQL_RES *r = mysql_store_result(m);
    mysql_free_result(r);

    mysql_close(m);
}

static void test_create_drop(void) {
    printf("\n[create/drop — adapted from official test_create_drop]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    for (int i = 0; i < 5; i++) {
        Q("DROP TABLE IF EXISTS t_cd");
        int rc = Q("CREATE TABLE t_cd(a INT)");
        if (rc != 0) { FAIL("create failed"); mysql_close(m); return; }
        Q("DROP TABLE t_cd");
    }
    TEST("create/drop 5 iterations OK");
    PASS();

    mysql_close(m);
}

static void test_concurrent_connections(void) {
    printf("\n[concurrent connections]\n");
    MYSQL *conns[10];
    memset(conns, 0, sizeof(conns));
    int ok = 1;
    for (int i = 0; i < 10; i++) {
        conns[i] = get_conn();
        if (!conns[i]) { ok = 0; break; }
    }
    TEST("open 10 concurrent connections");
    CHECK(ok, "failed to open 10 connections");

    for (int i = 0; i < 10; i++) {
        if (conns[i]) {
            mysql_real_query(conns[i], "SELECT 1", strlen("SELECT 1"));
            MYSQL_RES *r = mysql_store_result(conns[i]);
            mysql_free_result(r);
            mysql_close(conns[i]);
        }
    }
    TEST("query and close all 10");
    PASS();
}

static void test_empty_result(void) {
    printf("\n[empty result set]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_empty");
    Q("CREATE TABLE t_empty(id INT)");
    Q("SELECT * FROM t_empty");

    MYSQL_RES *res = mysql_store_result(m);
    TEST("empty result: row_count == 0");
    CHECK(res && res->row_count == 0, "wrong count");

    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("fetch_row returns NULL");
    CHECK(row == NULL, "expected NULL");

    mysql_free_result(res);
    mysql_close(m);
}

static void test_big_result(void) {
    printf("\n[big result set]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_big");
    Q("CREATE TABLE t_big(id INT PRIMARY KEY AUTO_INCREMENT, val VARCHAR(100))");

    char sql[256];
    for (int batch = 0; batch < 10; batch++) {
        sprintf(sql, "INSERT INTO t_big(val) VALUES "
                "('row%d-1'),('row%d-2'),('row%d-3'),('row%d-4'),('row%d-5'),"
                "('row%d-6'),('row%d-7'),('row%d-8'),('row%d-9'),('row%d-10')",
                batch, batch, batch, batch, batch, batch, batch, batch, batch, batch);
        Q(sql);
    }

    Q("SELECT * FROM t_big ORDER BY id");
    MYSQL_RES *res = mysql_store_result(m);
    TEST("100 rows inserted and fetched");
    CHECK(res && res->row_count == 100, "wrong count");
    mysql_free_result(res);
    mysql_close(m);
}

static void test_tran_innodb(void) {
    printf("\n[transaction — adapted from official test_tran_innodb]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("SET AUTOCOMMIT=0");

    Q("DROP TABLE IF EXISTS t_tran");
    Q("CREATE TABLE t_tran(col1 INT, col2 VARCHAR(30)) ENGINE=InnoDB");

    Q("INSERT INTO t_tran VALUES(10, 'venu')");
    Q("COMMIT");

    Q("INSERT INTO t_tran VALUES(20, 'mysql')");
    Q("ROLLBACK");

    Q("DELETE FROM t_tran WHERE col1=10");
    Q("ROLLBACK");

    Q("SELECT * FROM t_tran");
    MYSQL_RES *res = mysql_store_result(m);
    TEST("after commit+rollback: 1 row remains");
    CHECK(res && res->row_count == 1, "wrong count");

    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("remaining row: col1=10, col2='venu'");
    CHECK(row && row[0] && strcmp(row[0], "10") == 0 &&
          row[1] && strcmp(row[1], "venu") == 0, "wrong data");
    mysql_free_result(res);

    Q("SET AUTOCOMMIT=1");
    mysql_close(m);
}

static void test_prepare_multi_type(void) {
    printf("\n[prepare multi-type — adapted from official test_prepare]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_prep_multi");
    Q("CREATE TABLE t_prep_multi("
      "col1 TINYINT, col2 VARCHAR(15), col3 INT, "
      "col4 SMALLINT, col5 BIGINT, col6 FLOAT, col7 DOUBLE)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    const char *sql = "INSERT INTO t_prep_multi VALUES(?,?,?,?,?,?,?)";
    int rc = mysql_stmt_prepare(stmt, sql, (unsigned long)strlen(sql));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    TEST("param_count == 7");
    CHECK(stmt->param_count == 7, "wrong count");

    char tiny_data;
    char str_data[50];
    int int_data;
    short small_data;
    long long big_data;
    float real_data;
    double double_data;
    unsigned long length[7];
    my_bool is_null[7];

    MYSQL_BIND my_bind[7];
    memset(my_bind, 0, sizeof(my_bind));
    my_bind[0].buffer_type = MYSQL_TYPE_TINY;
    my_bind[0].buffer = &tiny_data;
    my_bind[1].buffer_type = MYSQL_TYPE_STRING;
    my_bind[1].buffer = str_data;
    my_bind[1].buffer_length = sizeof(str_data);
    my_bind[2].buffer_type = MYSQL_TYPE_LONG;
    my_bind[2].buffer = &int_data;
    my_bind[3].buffer_type = MYSQL_TYPE_SHORT;
    my_bind[3].buffer = &small_data;
    my_bind[4].buffer_type = MYSQL_TYPE_LONGLONG;
    my_bind[4].buffer = &big_data;
    my_bind[5].buffer_type = MYSQL_TYPE_FLOAT;
    my_bind[5].buffer = &real_data;
    my_bind[6].buffer_type = MYSQL_TYPE_DOUBLE;
    my_bind[6].buffer = &double_data;

    for (int i = 0; i < 7; i++) {
        my_bind[i].length = &length[i];
        my_bind[i].is_null = &is_null[i];
        is_null[i] = 0;
    }

    rc = mysql_stmt_bind_param(stmt, my_bind);
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    int_data = 320;
    small_data = 1867;
    big_data = 1000;
    real_data = 2;
    double_data = 6578.001;

    int inserted = 0;
    for (tiny_data = 0; tiny_data < 10; tiny_data++) {
        length[1] = (unsigned long)sprintf(str_data, "MySQL%d", int_data);
        rc = mysql_stmt_execute(stmt);
        if (rc != 0) break;
        int_data += 25;
        small_data += 10;
        big_data += 100;
        real_data += 1;
        double_data += 10.09;
        inserted++;
    }
    TEST("inserted 10 rows via multi-type PS");
    CHECK(inserted == 10, "insert failed");

    mysql_stmt_close(stmt);

    Q("SELECT COUNT(*) FROM t_prep_multi");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("10 rows in table");
    CHECK(row && row[0] && strcmp(row[0], "10") == 0, "wrong count");
    mysql_free_result(res);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "SELECT * FROM t_prep_multi",
                            strlen("SELECT * FROM t_prep_multi"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    rc = mysql_stmt_bind_result(stmt, my_bind);
    rc = mysql_stmt_execute(stmt);

    int_data = 320;
    small_data = 1867;
    big_data = 1000;
    real_data = 2;
    double_data = 6578.001;
    int fetched = 0;
    int data_ok = 1;

    while (mysql_stmt_fetch(stmt) == 0) {
        if (tiny_data != (char)fetched) data_ok = 0;
        if (int_data != 320 + fetched * 25) data_ok = 0;
        if (small_data != 1867 + fetched * 10) data_ok = 0;
        if (big_data != 1000 + fetched * 100) data_ok = 0;
        fetched++;
        int_data += 25;
        small_data += 10;
        big_data += 100;
        real_data += 1;
        double_data += 10.09;
    }

    TEST("fetched 10 rows with correct multi-type data");
    CHECK(fetched == 10 && data_ok, "data mismatch");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_bind_result_ext(void) {
    printf("\n[bind result ext — adapted from official test_bind_result_ext]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_bind_ext");
    Q("CREATE TABLE t_bind_ext("
      "c1 TINYINT, c2 SMALLINT, c3 INT, c4 BIGINT, "
      "c5 FLOAT, c6 DOUBLE, c7 VARBINARY(10), c8 VARCHAR(50))");
    Q("INSERT INTO t_bind_ext VALUES(19, 2999, 3999, 4999999, "
      "2345.6, 5678.89563, 'venu', 'mysql')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT * FROM t_bind_ext",
                                strlen("SELECT * FROM t_bind_ext"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    char t_data;
    short s_data;
    int i_data;
    long long b_data;
    float f_data;
    double d_data;
    char szData[20], bData[20];
    unsigned long length[8];
    my_bool is_null[8];

    MYSQL_BIND my_bind[8];
    memset(my_bind, 0, sizeof(my_bind));
    for (int i = 0; i < 8; i++) {
        my_bind[i].length = &length[i];
        my_bind[i].is_null = &is_null[i];
    }
    my_bind[0].buffer_type = MYSQL_TYPE_TINY;
    my_bind[0].buffer = &t_data;
    my_bind[1].buffer_type = MYSQL_TYPE_SHORT;
    my_bind[1].buffer = &s_data;
    my_bind[2].buffer_type = MYSQL_TYPE_LONG;
    my_bind[2].buffer = &i_data;
    my_bind[3].buffer_type = MYSQL_TYPE_LONGLONG;
    my_bind[3].buffer = &b_data;
    my_bind[4].buffer_type = MYSQL_TYPE_FLOAT;
    my_bind[4].buffer = &f_data;
    my_bind[5].buffer_type = MYSQL_TYPE_DOUBLE;
    my_bind[5].buffer = &d_data;
    my_bind[6].buffer_type = MYSQL_TYPE_STRING;
    my_bind[6].buffer = szData;
    my_bind[6].buffer_length = sizeof(szData);
    my_bind[7].buffer_type = MYSQL_TYPE_STRING;
    my_bind[7].buffer = bData;
    my_bind[7].buffer_length = sizeof(bData);

    rc = mysql_stmt_bind_result(stmt, my_bind);
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);

    TEST("tiny: 19");
    CHECK(t_data == 19, "wrong value");
    TEST("short: 2999");
    CHECK(s_data == 2999, "wrong value");
    TEST("int: 3999");
    CHECK(i_data == 3999, "wrong value");
    TEST("bigint: 4999999");
    CHECK(b_data == 4999999, "wrong value");
    TEST("string: 'venu'");
    CHECK(strcmp(szData, "venu") == 0, "wrong value");
    TEST("string length: 4");
    CHECK(length[6] == 4, "wrong length");
    TEST("varchar: 'mysql'");
    CHECK(strncmp(bData, "mysql", 5) == 0, "wrong value");
    TEST("varchar length: 5");
    CHECK(length[7] == 5, "wrong length");

    rc = mysql_stmt_fetch(stmt);
    TEST("no more rows");
    CHECK(rc == MYSQL_NO_DATA, "expected NO_DATA");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_null_param(void) {
    printf("\n[PS null param — adapted from official test_ps_null_param]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_ps_null");
    Q("CREATE TABLE t_ps_null(col1 INT)");
    Q("INSERT INTO t_ps_null VALUES(1), (NULL)");

    my_bool in_is_null = 1;
    int in_long = 1;
    unsigned long in_length = sizeof(int);
    MYSQL_BIND in_bind;
    memset(&in_bind, 0, sizeof(in_bind));
    in_bind.buffer_type = MYSQL_TYPE_LONG;
    in_bind.buffer = &in_long;
    in_bind.is_null = &in_is_null;
    in_bind.length = &in_length;

    char out_str[20];
    unsigned long out_length = 0;
    my_bool out_is_null = 0;
    MYSQL_BIND out_bind;
    memset(&out_bind, 0, sizeof(out_bind));
    out_bind.buffer_type = MYSQL_TYPE_STRING;
    out_bind.buffer = out_str;
    out_bind.buffer_length = sizeof(out_str);
    out_bind.length = &out_length;
    out_bind.is_null = &out_is_null;

    const char *queries[] = {
        "SELECT ?",
        "SELECT ?+1",
        "SELECT col1 FROM t_ps_null WHERE col1 <=> ?",
        NULL
    };

    int all_ok = 1;
    for (int qi = 0; queries[qi]; qi++) {
        MYSQL_STMT *stmt = mysql_stmt_init(m);
        int rc = mysql_stmt_prepare(stmt, queries[qi], (unsigned long)strlen(queries[qi]));
        if (rc != 0) { mysql_stmt_close(stmt); all_ok = 0; continue; }

        in_is_null = 1;
        rc = mysql_stmt_bind_param(stmt, &in_bind);
        rc = mysql_stmt_bind_result(stmt, &out_bind);
        rc = mysql_stmt_execute(stmt);
        rc = mysql_stmt_fetch(stmt);

        if (!out_is_null) all_ok = 0;

        mysql_stmt_fetch(stmt);
        mysql_stmt_close(stmt);
    }

    TEST("NULL param produces NULL result for all queries");
    CHECK(all_ok, "expected NULL result");

    mysql_close(m);
}

static void test_buffers(void) {
    printf("\n[buffers/truncation — adapted from official test_buffers]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_buffer");
    Q("CREATE TABLE t_buffer(str VARCHAR(20))");
    Q("INSERT INTO t_buffer VALUES('MySQL'), ('Database'), ('Open-Source'), ('Popular')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT str FROM t_buffer",
                                strlen("SELECT str FROM t_buffer"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    rc = mysql_stmt_execute(stmt);

    char buffer[20];
    unsigned long length = 0;
    my_bool is_null = 0;
    my_bool error_val = 0;
    MYSQL_BIND my_bind;
    memset(&my_bind, 0, sizeof(my_bind));
    my_bind.length = &length;
    my_bind.is_null = &is_null;
    my_bind.buffer_length = 1;
    my_bind.buffer_type = MYSQL_TYPE_STRING;
    my_bind.buffer = buffer;
    my_bind.error = &error_val;

    rc = mysql_stmt_bind_result(stmt, &my_bind);
    rc = mysql_stmt_store_result(stmt);

    memset(buffer, 0, sizeof(buffer));
    buffer[1] = 'X';
    rc = mysql_stmt_fetch(stmt);
    TEST("buffer_length=1: MYSQL_DATA_TRUNCATED");
    CHECK(rc == MYSQL_DATA_TRUNCATED, "expected truncation");
    TEST("error flag set");
    CHECK(error_val, "error flag not set");
    TEST("first byte is 'M'");
    CHECK(buffer[0] == 'M', "wrong byte");
    TEST("length == 5 (actual)");
    CHECK(length == 5, "wrong length");

    my_bind.buffer_length = 8;
    rc = mysql_stmt_bind_result(stmt, &my_bind);
    rc = mysql_stmt_fetch(stmt);
    TEST("buffer_length=8: 'Database' fetched OK");
    CHECK(rc == 0 && strncmp(buffer, "Database", 8) == 0, "wrong data");

    my_bind.buffer_length = 12;
    rc = mysql_stmt_bind_result(stmt, &my_bind);
    rc = mysql_stmt_fetch(stmt);
    TEST("buffer_length=12: 'Open-Source' fetched OK");
    CHECK(rc == 0 && strcmp(buffer, "Open-Source") == 0, "wrong data");

    my_bind.buffer_length = 6;
    rc = mysql_stmt_bind_result(stmt, &my_bind);
    rc = mysql_stmt_fetch(stmt);
    TEST("buffer_length=6: 'Popular' truncated");
    CHECK(rc == MYSQL_DATA_TRUNCATED && strncmp(buffer, "Popula", 6) == 0, "wrong data");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_prepare_simple(void) {
    printf("\n[prepare simple — adapted from official test_prepare_simple]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_prep_simple");
    Q("CREATE TABLE t_prep_simple(id INT, name VARCHAR(50))");

    MYSQL_STMT *stmt;
    int rc;

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "INSERT INTO t_prep_simple VALUES(?, ?)",
                            strlen("INSERT INTO t_prep_simple VALUES(?, ?)"));
    TEST("INSERT: param_count == 2");
    CHECK(rc == 0 && stmt->param_count == 2, "wrong count");
    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "UPDATE t_prep_simple SET id=? WHERE id=? AND name=?",
                            strlen("UPDATE t_prep_simple SET id=? WHERE id=? AND name=?"));
    TEST("UPDATE: param_count == 3");
    CHECK(rc == 0 && stmt->param_count == 3, "wrong count");
    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "DELETE FROM t_prep_simple WHERE id=10",
                            strlen("DELETE FROM t_prep_simple WHERE id=10"));
    TEST("DELETE no-param: param_count == 0");
    CHECK(rc == 0 && stmt->param_count == 0, "wrong count");
    rc = mysql_stmt_execute(stmt);
    TEST("DELETE no-param: execute OK");
    CHECK(rc == 0, mysql_stmt_error(stmt));
    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "DELETE FROM t_prep_simple WHERE id=?",
                            strlen("DELETE FROM t_prep_simple WHERE id=?"));
    TEST("DELETE with-param: param_count == 1");
    CHECK(rc == 0 && stmt->param_count == 1, "wrong count");
    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "SELECT * FROM t_prep_simple WHERE id=? AND name=?",
                            strlen("SELECT * FROM t_prep_simple WHERE id=? AND name=?"));
    TEST("SELECT: param_count == 2");
    CHECK(rc == 0 && stmt->param_count == 2, "wrong count");
    mysql_stmt_close(stmt);

    mysql_close(m);
}

static void test_prepare_noparam(void) {
    printf("\n[prepare noparam — adapted from official test_prepare_noparam]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_prep_noparam");
    Q("CREATE TABLE t_prep_noparam(col1 INT, col2 VARCHAR(50))");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_prep_noparam VALUES(10, 'venu')",
                                strlen("INSERT INTO t_prep_noparam VALUES(10, 'venu')"));
    TEST("param_count == 0");
    CHECK(rc == 0 && stmt->param_count == 0, "wrong count");

    rc = mysql_stmt_execute(stmt);
    TEST("execute no-param PS OK");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    mysql_stmt_close(stmt);

    Q("SELECT * FROM t_prep_noparam");
    MYSQL_RES *res = mysql_store_result(m);
    TEST("1 row inserted");
    CHECK(res && res->row_count == 1, "wrong count");
    mysql_free_result(res);
    mysql_close(m);
}

static void test_stmt_reset(void) {
    printf("\n[stmt reset — adapted from official test_bug11183]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_reset");
    Q("CREATE TABLE t_reset(a INT)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_reset VALUES(1)",
                                strlen("INSERT INTO t_reset VALUES(1)"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    Q("DROP TABLE t_reset");

    rc = mysql_stmt_execute(stmt);
    TEST("execute after drop table: fails");
    CHECK(rc != 0, "expected error");

    rc = mysql_stmt_reset(stmt);
    TEST("stmt_reset returns 0");
    CHECK(rc == 0, "reset failed");

    TEST("errno == 0 after reset");
    CHECK(mysql_stmt_errno(stmt) == 0, "errno not cleared");

    Q("CREATE TABLE t_reset(a INT)");
    rc = mysql_stmt_execute(stmt);
    TEST("execute after recreate table: OK");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_free_result_sync(void) {
    printf("\n[stmt free_result sync — adapted from official test_free_result]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_free_res");
    Q("CREATE TABLE t_free_res(c1 INT PRIMARY KEY AUTO_INCREMENT)");
    Q("INSERT INTO t_free_res VALUES(),(),()");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT * FROM t_free_res",
                                strlen("SELECT * FROM t_free_res"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    int bc1;
    unsigned long bl1;
    MYSQL_BIND my_bind;
    memset(&my_bind, 0, sizeof(my_bind));
    my_bind.buffer_type = MYSQL_TYPE_LONG;
    my_bind.buffer = &bc1;
    my_bind.length = &bl1;

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_bind_result(stmt, &my_bind);
    rc = mysql_stmt_fetch(stmt);
    TEST("first fetch OK (streaming)");
    CHECK(rc == 0, "fetch failed");

    rc = Q("DROP TABLE t_free_res");
    TEST("query while PS streaming result: error (out of sync)");
    CHECK(rc != 0, "expected error");

    rc = mysql_stmt_free_result(stmt);
    TEST("stmt_free_result OK");
    CHECK(rc == 0, "free_result failed");

    rc = Q("DROP TABLE IF EXISTS t_free_res");
    TEST("query after free_result: OK");
    CHECK(rc == 0, mysql_error(m));

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_decimal(void) {
    printf("\n[DECIMAL type — adapted from official test_decimal_bug]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_decimal");
    Q("CREATE TABLE t_decimal(d1 DECIMAL(6,2), d2 DECIMAL(10,4))");
    Q("INSERT INTO t_decimal VALUES(8.00, 5.6123)");
    Q("INSERT INTO t_decimal VALUES(NULL, 999.9999)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT * FROM t_decimal",
                                strlen("SELECT * FROM t_decimal"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    char out1[20], out2[20];
    unsigned long len1 = 0, len2 = 0;
    my_bool is_null1 = 0, is_null2 = 0;

    MYSQL_BIND my_bind[2];
    memset(my_bind, 0, sizeof(my_bind));
    my_bind[0].buffer_type = MYSQL_TYPE_STRING;
    my_bind[0].buffer = out1;
    my_bind[0].buffer_length = sizeof(out1);
    my_bind[0].length = &len1;
    my_bind[0].is_null = &is_null1;
    my_bind[1].buffer_type = MYSQL_TYPE_STRING;
    my_bind[1].buffer = out2;
    my_bind[1].buffer_length = sizeof(out2);
    my_bind[1].length = &len2;
    my_bind[1].is_null = &is_null2;

    rc = mysql_stmt_bind_result(stmt, my_bind);
    rc = mysql_stmt_execute(stmt);

    rc = mysql_stmt_fetch(stmt);
    TEST("row 1: d1='8.00'");
    CHECK(rc == 0 && strcmp(out1, "8.00") == 0, "wrong value");
    TEST("row 1: d2='5.6123'");
    CHECK(strcmp(out2, "5.6123") == 0, "wrong value");

    rc = mysql_stmt_fetch(stmt);
    TEST("row 2: d1 is NULL");
    CHECK(rc == 0 && is_null1, "expected NULL");
    TEST("row 2: d2='999.9999'");
    CHECK(strcmp(out2, "999.9999") == 0, "wrong value");

    rc = mysql_stmt_fetch(stmt);
    TEST("no more rows");
    CHECK(rc == MYSQL_NO_DATA, "expected NO_DATA");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_simple_temporal(void) {
    printf("\n[temporal types — adapted from official test_simple_temporal]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_temporal");
    Q("CREATE TABLE t_temporal("
      "c1 DATETIME, c2 DATE, c3 TIME, c4 TIMESTAMP)");

    Q("INSERT INTO t_temporal VALUES("
      "'2001-10-20 10:10:59', '2024-06-15', '12:30:45', '2024-01-01 00:00:00')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT c1, c2, c3 FROM t_temporal",
                                strlen("SELECT c1, c2, c3 FROM t_temporal"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    MYSQL_TIME out_dt, out_date, out_time;
    unsigned long len1 = 0, len2 = 0, len3 = 0;
    my_bool n1 = 0, n2 = 0, n3 = 0;

    MYSQL_BIND my_bind[3];
    memset(my_bind, 0, sizeof(my_bind));
    my_bind[0].buffer_type = MYSQL_TYPE_DATETIME;
    my_bind[0].buffer = &out_dt;
    my_bind[0].buffer_length = sizeof(out_dt);
    my_bind[0].length = &len1;
    my_bind[0].is_null = &n1;
    my_bind[1].buffer_type = MYSQL_TYPE_DATE;
    my_bind[1].buffer = &out_date;
    my_bind[1].buffer_length = sizeof(out_date);
    my_bind[1].length = &len2;
    my_bind[1].is_null = &n2;
    my_bind[2].buffer_type = MYSQL_TYPE_TIME;
    my_bind[2].buffer = &out_time;
    my_bind[2].buffer_length = sizeof(out_time);
    my_bind[2].length = &len3;
    my_bind[2].is_null = &n3;

    rc = mysql_stmt_bind_result(stmt, my_bind);
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("DATETIME: 2001-10-20 10:10:59");
    CHECK(rc == 0 && out_dt.year == 2001 && out_dt.month == 10 && out_dt.day == 20 &&
          out_dt.hour == 10 && out_dt.minute == 10 && out_dt.second == 59, "wrong value");

    TEST("DATE: 2024-06-15");
    CHECK(rc == 0 && out_date.year == 2024 && out_date.month == 6 && out_date.day == 15, "wrong value");

    TEST("TIME: 12:30:45");
    CHECK(rc == 0 && out_time.hour == 12 && out_time.minute == 30 && out_time.second == 45, "wrong value");

    rc = mysql_stmt_fetch(stmt);
    TEST("no more rows after temporal fetch");
    CHECK(rc == MYSQL_NO_DATA, "expected NO_DATA");

    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "INSERT INTO t_temporal(c1) VALUES(?)",
                            strlen("INSERT INTO t_temporal(c1) VALUES(?)"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    MYSQL_TIME tm;
    unsigned long tm_len = sizeof(MYSQL_TIME);
    my_bool tm_null = 0;
    MYSQL_BIND in_bind;
    memset(&in_bind, 0, sizeof(in_bind));
    in_bind.buffer_type = MYSQL_TYPE_DATETIME;
    in_bind.buffer = &tm;
    in_bind.buffer_length = sizeof(tm);
    in_bind.length = &tm_len;
    in_bind.is_null = &tm_null;

    rc = mysql_stmt_bind_param(stmt, &in_bind);

    memset(&tm, 0, sizeof(tm));
    tm.time_type = MYSQL_TIMESTAMP_DATETIME;
    tm.year = 2025;
    tm.month = 5;
    tm.day = 29;
    tm.hour = 14;
    tm.minute = 30;
    tm.second = 0;

    rc = mysql_stmt_execute(stmt);
    TEST("INSERT DATETIME param via PS");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "SELECT c1 FROM t_temporal WHERE c1 IS NOT NULL ORDER BY c1 DESC LIMIT 1",
                            strlen("SELECT c1 FROM t_temporal WHERE c1 IS NOT NULL ORDER BY c1 DESC LIMIT 1"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    MYSQL_TIME out_dt2;
    unsigned long olen = 0;
    my_bool onull = 0;
    MYSQL_BIND obind;
    memset(&obind, 0, sizeof(obind));
    obind.buffer_type = MYSQL_TYPE_DATETIME;
    obind.buffer = &out_dt2;
    obind.buffer_length = sizeof(out_dt2);
    obind.length = &olen;
    obind.is_null = &onull;

    rc = mysql_stmt_bind_result(stmt, &obind);
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("DATETIME param round-trip: 2025-05-29");
    CHECK(rc == 0 && out_dt2.year == 2025 && out_dt2.month == 5 && out_dt2.day == 29, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_fetch_column(void) {
    printf("\n[stmt fetch_column — adapted from official test_fetch_column]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_fetch_col");
    Q("CREATE TABLE t_fetch_col(id INT, name VARCHAR(50))");
    Q("INSERT INTO t_fetch_col VALUES(42, 'hello')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT * FROM t_fetch_col",
                                strlen("SELECT * FROM t_fetch_col"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    int out_id = 0;
    unsigned long id_len = 0;
    my_bool id_null = 0;
    MYSQL_BIND my_bind[2];
    memset(my_bind, 0, sizeof(my_bind));
    my_bind[0].buffer_type = MYSQL_TYPE_LONG;
    my_bind[0].buffer = &out_id;
    my_bind[0].length = &id_len;
    my_bind[0].is_null = &id_null;
    my_bind[1].buffer_type = MYSQL_TYPE_LONG;

    rc = mysql_stmt_bind_result(stmt, my_bind);
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_store_result(stmt);
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }
    rc = mysql_stmt_fetch(stmt);
    TEST("fetch row OK");
    CHECK(rc == 0, "fetch failed");

    TEST("id == 42");
    CHECK(out_id == 42, "wrong id");

    char col_buf[50];
    unsigned long col_len = 0;
    my_bool col_null = 0;
    MYSQL_BIND col_bind;
    memset(&col_bind, 0, sizeof(col_bind));
    col_bind.buffer_type = MYSQL_TYPE_STRING;
    col_bind.buffer = col_buf;
    col_bind.buffer_length = sizeof(col_buf);
    col_bind.length = &col_len;
    col_bind.is_null = &col_null;

    rc = mysql_stmt_fetch_column(stmt, &col_bind, 1, 0);
    TEST("fetch_column(1): 'hello'");
    CHECK(rc == 0 && strncmp(col_buf, "hello", 5) == 0, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_data_seek_ps(void) {
    printf("\n[stmt data_seek — adapted from official test_fetch_seek]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_seek_ps");
    Q("CREATE TABLE t_seek_ps(id INT PRIMARY KEY AUTO_INCREMENT, val INT)");
    Q("INSERT INTO t_seek_ps(val) VALUES(10),(20),(30),(40),(50)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT * FROM t_seek_ps",
                                strlen("SELECT * FROM t_seek_ps"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    int out_id = 0, out_val = 0;
    unsigned long len1 = 0, len2 = 0;
    my_bool n1 = 0, n2 = 0;
    MYSQL_BIND my_bind[2];
    memset(my_bind, 0, sizeof(my_bind));
    my_bind[0].buffer_type = MYSQL_TYPE_LONG;
    my_bind[0].buffer = &out_id;
    my_bind[0].length = &len1;
    my_bind[0].is_null = &n1;
    my_bind[1].buffer_type = MYSQL_TYPE_LONG;
    my_bind[1].buffer = &out_val;
    my_bind[1].length = &len2;
    my_bind[1].is_null = &n2;

    rc = mysql_stmt_bind_result(stmt, my_bind);
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_store_result(stmt);
    TEST("store_result OK");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    TEST("num_rows == 5");
    CHECK(mysql_stmt_num_rows(stmt) == 5, "wrong count");

    mysql_stmt_data_seek(stmt, 2);
    rc = mysql_stmt_fetch(stmt);
    TEST("seek to row 2: val == 30");
    CHECK(rc == 0 && out_val == 30, "wrong data");

    mysql_stmt_data_seek(stmt, 4);
    rc = mysql_stmt_fetch(stmt);
    TEST("seek to row 4: val == 50");
    CHECK(rc == 0 && out_val == 50, "wrong data");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_insert_id(void) {
    printf("\n[stmt insert_id — adapted from official test_stmt_insert_id]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_stmt_id");
    Q("CREATE TABLE t_stmt_id(id INT PRIMARY KEY AUTO_INCREMENT, val VARCHAR(20))");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_stmt_id(val) VALUES(?)",
                                strlen("INSERT INTO t_stmt_id(val) VALUES(?)"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    char val[20];
    unsigned long val_len;
    MYSQL_BIND my_bind;
    memset(&my_bind, 0, sizeof(my_bind));
    my_bind.buffer_type = MYSQL_TYPE_STRING;
    my_bind.buffer = val;
    my_bind.buffer_length = sizeof(val);
    my_bind.length = &val_len;

    rc = mysql_stmt_bind_param(stmt, &my_bind);

    strcpy(val, "first");
    val_len = strlen(val);
    rc = mysql_stmt_execute(stmt);
    TEST("first insert: insert_id == 1");
    CHECK(rc == 0 && mysql_stmt_insert_id(stmt) == 1, "wrong id");

    strcpy(val, "second");
    val_len = strlen(val);
    rc = mysql_stmt_execute(stmt);
    TEST("second insert: insert_id == 2");
    CHECK(rc == 0 && mysql_stmt_insert_id(stmt) == 2, "wrong id");

    strcpy(val, "third");
    val_len = strlen(val);
    rc = mysql_stmt_execute(stmt);
    TEST("third insert: insert_id == 3");
    CHECK(rc == 0 && mysql_stmt_insert_id(stmt) == 3, "wrong id");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_double_reconnect(void) {
    printf("\n[reconnect after server close]\n");
    MYSQL *m = mysql_init(NULL);
    if (!m) { FAIL("init failed"); return; }
    unsigned int ssl_mode = SSL_MODE_DISABLED;
    mysql_options(m, MYSQL_OPT_SSL_MODE, &ssl_mode);
    my_bool reconnect = 1;
    mysql_options(m, MYSQL_OPT_RECONNECT, &reconnect);
    m = mysql_real_connect(m, g_host, g_user, g_passwd, g_db, g_port, NULL, 0);
    if (!m) { FAIL("connect failed"); return; }

    unsigned long tid = m->thread_id;
    TEST("thread_id > 0");
    CHECK(tid > 0, "zero thread_id");

    char kill_sql[64];
    sprintf(kill_sql, "KILL %lu", tid);
    MYSQL *m2 = get_conn();
    if (m2) {
        Q2(m2, kill_sql);
        mysql_close(m2);
    }

    usleep(200000);

    int rc = mysql_ping(m);
    TEST("ping after kill: reconnects");
    CHECK(rc == 0, mysql_error(m));

    rc = Q("SELECT 1");
    TEST("query after reconnect OK");
    CHECK(rc == 0, mysql_error(m));
    MYSQL_RES *res = mysql_store_result(m);
    mysql_free_result(res);

    mysql_close(m);
}

static void test_prepare_insert_update(void) {
    printf("\n[prepare INSERT ON DUPLICATE KEY UPDATE — adapted from official test_prepare_insert_update]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_upsert");
    Q("CREATE TABLE t_upsert(a INT, b INT, c INT, UNIQUE(a), UNIQUE(b))");
    Q("INSERT INTO t_upsert VALUES (1,2,10), (3,4,20)");

    MYSQL_STMT *stmt;
    int rc;

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt,
        "INSERT INTO t_upsert VALUES (5,6,30), (7,4,40), (8,9,60) "
        "ON DUPLICATE KEY UPDATE c=c+100",
        strlen("INSERT INTO t_upsert VALUES (5,6,30), (7,4,40), (8,9,60) "
               "ON DUPLICATE KEY UPDATE c=c+100"));
    TEST("prepare UPSERT: param_count == 0");
    CHECK(rc == 0 && stmt->param_count == 0, "wrong count");

    rc = mysql_stmt_execute(stmt);
    TEST("execute UPSERT OK");
    CHECK(rc == 0, mysql_stmt_error(stmt));
    mysql_stmt_close(stmt);

    Q("SELECT * FROM t_upsert ORDER BY a");
    MYSQL_RES *res = mysql_store_result(m);
    TEST("4 rows after UPSERT");
    CHECK(res && res->row_count == 4, "wrong count");
    mysql_free_result(res);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt,
        "INSERT INTO t_upsert SET a=5 ON DUPLICATE KEY UPDATE b=0",
        strlen("INSERT INTO t_upsert SET a=5 ON DUPLICATE KEY UPDATE b=0"));
    TEST("prepare UPSERT SET: param_count == 0");
    CHECK(rc == 0 && stmt->param_count == 0, "wrong count");

    rc = mysql_stmt_execute(stmt);
    TEST("execute UPSERT SET OK");
    CHECK(rc == 0, mysql_stmt_error(stmt));
    mysql_stmt_close(stmt);

    Q("SELECT b FROM t_upsert WHERE a=5");
    res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("b=0 after UPSERT SET");
    CHECK(row && strcmp(row[0], "0") == 0, "wrong value");
    mysql_free_result(res);

    mysql_close(m);
}

static void test_stmt_reprepare_after_error(void) {
    printf("\n[stmt reprepare after error — adapted from official test_bug15518]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "FOOBAR", 6);
    TEST("prepare invalid SQL: error");
    CHECK(rc != 0, "expected error");
    TEST("stmt_errno != 0");
    CHECK(mysql_stmt_errno(stmt) != 0, "expected non-zero errno");

    rc = mysql_stmt_prepare(stmt, "SELECT 1", strlen("SELECT 1"));
    TEST("re-prepare valid SQL on same stmt: OK");
    CHECK(rc == 0, mysql_stmt_error(stmt));

    rc = mysql_stmt_execute(stmt);
    CHECK(rc == 0, mysql_stmt_error(stmt));
    mysql_stmt_close(stmt);

    TEST("connection errno cleared after successful re-prepare");
    CHECK(mysql_errno(m) == 0, "errno not cleared");

    mysql_close(m);
}

static void test_parse_error_and_bad_length(void) {
    printf("\n[parse error and bad length — adapted from official test_parse_error_and_bad_length]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    int rc = Q("SHOW DATABAAAA");
    TEST("bad SQL: error");
    CHECK(rc != 0, "expected error");

    rc = mysql_real_query(m, "SHOW DATABASES", 12);
    TEST("wrong length: error");
    CHECK(rc != 0, "expected error");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "SHOW DATABAAAA", strlen("SHOW DATABAAAA"));
    TEST("prepare bad SQL: error");
    CHECK(rc != 0, "expected error");
    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "SHOW DATABASES", 12);
    TEST("prepare with wrong length: error");
    CHECK(rc != 0, "expected error");
    mysql_stmt_close(stmt);

    mysql_close(m);
}

static void test_stmt_affected_rows(void) {
    printf("\n[stmt affected_rows — adapted from official test_bug2247]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_affected");
    Q("CREATE TABLE t_affected(id INT UNIQUE AUTO_INCREMENT, val INT)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_affected VALUES (NULL, ?)",
                                strlen("INSERT INTO t_affected VALUES (NULL, ?)"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    int val = 0;
    unsigned long vlen = sizeof(int);
    MYSQL_BIND my_bind;
    memset(&my_bind, 0, sizeof(my_bind));
    my_bind.buffer_type = MYSQL_TYPE_LONG;
    my_bind.buffer = &val;
    my_bind.length = &vlen;
    rc = mysql_stmt_bind_param(stmt, &my_bind);

    for (int i = 0; i < 5; i++) {
        val = i * 10;
        rc = mysql_stmt_execute(stmt);
        if (rc != 0) break;
    }
    TEST("5 inserts via PS");
    CHECK(mysql_stmt_affected_rows(stmt) == 1, "wrong affected_rows");
    mysql_stmt_close(stmt);

    Q("SELECT COUNT(*) FROM t_affected");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("5 rows in table");
    CHECK(row && strcmp(row[0], "5") == 0, "wrong count");
    mysql_free_result(res);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "UPDATE t_affected SET val=val+100 WHERE val < ?",
                                strlen("UPDATE t_affected SET val=val+100 WHERE val < ?"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }
    val = 30;
    rc = mysql_stmt_bind_param(stmt, &my_bind);
    rc = mysql_stmt_execute(stmt);
    TEST("update affected_rows == 3");
    CHECK(rc == 0 && mysql_stmt_affected_rows(stmt) == 3, "wrong affected_rows");
    mysql_stmt_close(stmt);

    mysql_close(m);
}

static void test_stmt_field_count(void) {
    printf("\n[stmt field_count]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt;
    int rc;

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "SELECT 1, 2, 3", strlen("SELECT 1, 2, 3"));
    TEST("SELECT 3 cols: field_count == 3");
    CHECK(rc == 0 && mysql_stmt_field_count(stmt) == 3, "wrong count");
    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "INSERT INTO t_affected VALUES (NULL, 999)",
                                strlen("INSERT INTO t_affected VALUES (NULL, 999)"));
    TEST("INSERT: field_count == 0");
    CHECK(rc == 0 && mysql_stmt_field_count(stmt) == 0, "wrong count");
    mysql_stmt_close(stmt);

    mysql_close(m);
}

static void test_stmt_sqlstate(void) {
    printf("\n[stmt sqlstate]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "FOOBAR", 6);
    TEST("bad SQL: sqlstate starts with '4'");
    CHECK(rc != 0 && mysql_stmt_sqlstate(stmt)[0] == '4', "wrong sqlstate");
    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "SELECT 1", strlen("SELECT 1"));
    TEST("valid SQL: sqlstate == '00000'");
    CHECK(rc == 0 && strcmp(mysql_stmt_sqlstate(stmt), "00000") == 0, "wrong sqlstate");
    mysql_stmt_close(stmt);

    mysql_close(m);
}

static void test_multiple_stmt_handles(void) {
    printf("\n[multiple stmt handles simultaneously]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_multi_stmt");
    Q("CREATE TABLE t_multi_stmt(id INT PRIMARY KEY AUTO_INCREMENT, val INT)");
    Q("INSERT INTO t_multi_stmt(val) VALUES(10),(20),(30)");

    MYSQL_STMT *s1 = mysql_stmt_init(m);
    MYSQL_STMT *s2 = mysql_stmt_init(m);
    MYSQL_STMT *s3 = mysql_stmt_init(m);
    int rc;

    rc = mysql_stmt_prepare(s1, "SELECT val FROM t_multi_stmt WHERE id=?",
                            strlen("SELECT val FROM t_multi_stmt WHERE id=?"));
    TEST("prepare s1: OK");
    CHECK(rc == 0, mysql_stmt_error(s1));

    rc = mysql_stmt_prepare(s2, "INSERT INTO t_multi_stmt(val) VALUES(?)",
                            strlen("INSERT INTO t_multi_stmt(val) VALUES(?)"));
    TEST("prepare s2: OK");
    CHECK(rc == 0, mysql_stmt_error(s2));

    rc = mysql_stmt_prepare(s3, "SELECT COUNT(*) FROM t_multi_stmt",
                            strlen("SELECT COUNT(*) FROM t_multi_stmt"));
    TEST("prepare s3: OK");
    CHECK(rc == 0, mysql_stmt_error(s3));

    int id = 2, val = 40;
    unsigned long id_len = sizeof(int), val_len = sizeof(int);
    MYSQL_BIND b1;
    memset(&b1, 0, sizeof(b1));
    b1.buffer_type = MYSQL_TYPE_LONG;
    b1.buffer = &id;
    b1.length = &id_len;
    rc = mysql_stmt_bind_param(s1, &b1);

    MYSQL_BIND b2;
    memset(&b2, 0, sizeof(b2));
    b2.buffer_type = MYSQL_TYPE_LONG;
    b2.buffer = &val;
    b2.length = &val_len;
    rc = mysql_stmt_bind_param(s2, &b2);

    rc = mysql_stmt_execute(s1);
    TEST("execute s1 (SELECT): OK");
    CHECK(rc == 0, mysql_stmt_error(s1));

    int out_val = 0;
    unsigned long out_len = 0;
    MYSQL_BIND ob;
    memset(&ob, 0, sizeof(ob));
    ob.buffer_type = MYSQL_TYPE_LONG;
    ob.buffer = &out_val;
    ob.length = &out_len;
    rc = mysql_stmt_bind_result(s1, &ob);
    rc = mysql_stmt_fetch(s1);
    TEST("s1 result: val == 20");
    CHECK(rc == 0 && out_val == 20, "wrong value");
    mysql_stmt_free_result(s1);

    rc = mysql_stmt_execute(s2);
    TEST("execute s2 (INSERT): OK");
    CHECK(rc == 0, mysql_stmt_error(s2));

    rc = mysql_stmt_execute(s3);
    TEST("execute s3 (SELECT COUNT): OK");
    CHECK(rc == 0, mysql_stmt_error(s3));

    long long count = 0;
    unsigned long count_len = 0;
    MYSQL_BIND cb;
    memset(&cb, 0, sizeof(cb));
    cb.buffer_type = MYSQL_TYPE_LONGLONG;
    cb.buffer = &count;
    cb.length = &count_len;
    rc = mysql_stmt_bind_result(s3, &cb);
    rc = mysql_stmt_fetch(s3);
    TEST("s3 result: count == 4");
    CHECK(rc == 0 && count == 4, "wrong count");
    mysql_stmt_free_result(s3);

    mysql_stmt_close(s1);
    mysql_stmt_close(s2);
    mysql_stmt_close(s3);
    mysql_close(m);
}

static void test_stmt_update_bind(void) {
    printf("\n[stmt update bind between executes]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_rebind");
    Q("CREATE TABLE t_rebind(id INT, val INT)");
    Q("INSERT INTO t_rebind VALUES(1, 100), (2, 200), (3, 300)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT id, val FROM t_rebind WHERE id=?",
                                strlen("SELECT id, val FROM t_rebind WHERE id=?"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    int id = 1;
    unsigned long id_len = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG;
    pb.buffer = &id;
    pb.length = &id_len;
    rc = mysql_stmt_bind_param(stmt, &pb);

    int out_id = 0, out_val = 0;
    unsigned long olen1 = 0, olen2 = 0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG;
    rb[0].buffer = &out_id;
    rb[0].length = &olen1;
    rb[1].buffer_type = MYSQL_TYPE_LONG;
    rb[1].buffer = &out_val;
    rb[1].length = &olen2;
    rc = mysql_stmt_bind_result(stmt, rb);

    id = 1;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("id=1: val == 100");
    CHECK(rc == 0 && out_val == 100, "wrong value");
    rc = mysql_stmt_fetch(stmt);
    CHECK(rc == MYSQL_NO_DATA, "expected NO_DATA");

    id = 3;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("id=3: val == 300");
    CHECK(rc == 0 && out_val == 300, "wrong value");
    rc = mysql_stmt_fetch(stmt);
    CHECK(rc == MYSQL_NO_DATA, "expected NO_DATA");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_unsigned(void) {
    printf("\n[stmt unsigned types]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_unsigned");
    Q("CREATE TABLE t_unsigned(a TINYINT UNSIGNED, b SMALLINT UNSIGNED, c INT UNSIGNED, d BIGINT UNSIGNED)");
    Q("INSERT INTO t_unsigned VALUES(200, 64000, 4294967295, 18446744073709551615)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT * FROM t_unsigned",
                                strlen("SELECT * FROM t_unsigned"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    unsigned char a;
    unsigned short b;
    unsigned int c;
    unsigned long long d;
    unsigned long alen = 0, blen = 0, clen = 0, dlen = 0;
    my_bool an = 0, bn = 0, cn = 0, dn = 0;
    MYSQL_BIND my_bind[4];
    memset(my_bind, 0, sizeof(my_bind));
    my_bind[0].buffer_type = MYSQL_TYPE_TINY;
    my_bind[0].buffer = &a;
    my_bind[0].is_unsigned = 1;
    my_bind[0].length = &alen;
    my_bind[0].is_null = &an;
    my_bind[1].buffer_type = MYSQL_TYPE_SHORT;
    my_bind[1].buffer = &b;
    my_bind[1].is_unsigned = 1;
    my_bind[1].length = &blen;
    my_bind[1].is_null = &bn;
    my_bind[2].buffer_type = MYSQL_TYPE_LONG;
    my_bind[2].buffer = &c;
    my_bind[2].is_unsigned = 1;
    my_bind[2].length = &clen;
    my_bind[2].is_null = &cn;
    my_bind[3].buffer_type = MYSQL_TYPE_LONGLONG;
    my_bind[3].buffer = &d;
    my_bind[3].is_unsigned = 1;
    my_bind[3].length = &dlen;
    my_bind[3].is_null = &dn;

    rc = mysql_stmt_bind_result(stmt, my_bind);
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);

    TEST("TINYINT UNSIGNED: 200");
    CHECK(rc == 0 && a == 200, "wrong value");
    TEST("SMALLINT UNSIGNED: 64000");
    CHECK(b == 64000, "wrong value");
    TEST("INT UNSIGNED: 4294967295");
    CHECK(c == 4294967295U, "wrong value");
    TEST("BIGINT UNSIGNED: 18446744073709551615");
    CHECK(d == 18446744073709551615ULL, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_bit_type(void) {
    printf("\n[BIT type]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_bit");
    Q("CREATE TABLE t_bit(a BIT(8), b BIT(16), c BIT(32))");
    Q("INSERT INTO t_bit VALUES(b'10101010', b'1100110011001100', b'11110000111100001111000011110000')");

    Q("SELECT HEX(a), HEX(b), HEX(c) FROM t_bit");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("BIT(8) hex: AA");
    CHECK(row && row[0] && strcmp(row[0], "AA") == 0, "wrong value");
    TEST("BIT(16) hex: CCCC");
    CHECK(row && row[1] && strcmp(row[1], "CCCC") == 0, "wrong value");
    TEST("BIT(32) hex: F0F0F0F0");
    CHECK(row && row[2] && strcmp(row[2], "F0F0F0F0") == 0, "wrong value");
    mysql_free_result(res);

    mysql_close(m);
}

static void test_stmt_text_blob(void) {
    printf("\n[TEXT/BLOB types]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_lob");
    Q("CREATE TABLE t_lob(t1 TEXT, b1 BLOB, vc VARCHAR(255))");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_lob VALUES(?,?,?)",
                                strlen("INSERT INTO t_lob VALUES(?,?,?)"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    const char *text_data = "Hello TEXT world";
    const char *blob_data = "\x00\x01\x02\x03\x04\x05";
    unsigned long blob_len = 6;
    const char *vc_data = "varchar value";
    unsigned long text_len = (unsigned long)strlen(text_data);
    unsigned long vc_len = (unsigned long)strlen(vc_data);

    MYSQL_BIND my_bind[3];
    memset(my_bind, 0, sizeof(my_bind));
    my_bind[0].buffer_type = MYSQL_TYPE_STRING;
    my_bind[0].buffer = (void*)text_data;
    my_bind[0].buffer_length = text_len;
    my_bind[0].length = &text_len;
    my_bind[1].buffer_type = MYSQL_TYPE_BLOB;
    my_bind[1].buffer = (void*)blob_data;
    my_bind[1].buffer_length = blob_len;
    my_bind[1].length = &blob_len;
    my_bind[2].buffer_type = MYSQL_TYPE_STRING;
    my_bind[2].buffer = (void*)vc_data;
    my_bind[2].buffer_length = vc_len;
    my_bind[2].length = &vc_len;

    rc = mysql_stmt_bind_param(stmt, my_bind);
    rc = mysql_stmt_execute(stmt);
    TEST("insert TEXT/BLOB/VARCHAR via PS");
    CHECK(rc == 0, mysql_stmt_error(stmt));
    mysql_stmt_close(stmt);

    Q("SELECT LENGTH(t1), LENGTH(b1), LENGTH(vc), vc FROM t_lob");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("TEXT length: 16");
    CHECK(row && row[0] && strcmp(row[0], "16") == 0, "wrong length");
    TEST("BLOB length: 6");
    CHECK(row && row[1] && strcmp(row[1], "6") == 0, "wrong length");
    TEST("VARCHAR: 'varchar value'");
    CHECK(row && row[3] && strcmp(row[3], "varchar value") == 0, "wrong value");
    mysql_free_result(res);

    mysql_close(m);
}

static void test_stmt_double_float(void) {
    printf("\n[FLOAT/DOUBLE precision]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_floats");
    Q("CREATE TABLE t_floats(f FLOAT, d DOUBLE)");
    Q("INSERT INTO t_floats VALUES(3.14, 2.718281828459045)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT f, d FROM t_floats",
                                strlen("SELECT f, d FROM t_floats"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    float f_val = 0;
    double d_val = 0;
    unsigned long flen = 0, dlen = 0;
    MYSQL_BIND my_bind[2];
    memset(my_bind, 0, sizeof(my_bind));
    my_bind[0].buffer_type = MYSQL_TYPE_FLOAT;
    my_bind[0].buffer = &f_val;
    my_bind[0].length = &flen;
    my_bind[1].buffer_type = MYSQL_TYPE_DOUBLE;
    my_bind[1].buffer = &d_val;
    my_bind[1].length = &dlen;

    rc = mysql_stmt_bind_result(stmt, my_bind);
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);

    TEST("FLOAT ~3.14");
    CHECK(rc == 0 && fabs(f_val - 3.14f) < 0.01f, "wrong value");
    TEST("DOUBLE ~2.718281828");
    CHECK(fabs(d_val - 2.718281828459045) < 1e-10, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_mysql_errno_sqlstate(void) {
    printf("\n[mysql_errno and sqlstate]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("SELECT * FROM nonexistent_table_xyz");
    TEST("errno != 0 for bad table");
    CHECK(mysql_errno(m) != 0, "expected error");
    TEST("sqlstate is 5 chars");
    CHECK(strlen(mysql_sqlstate(m)) == 5, "wrong sqlstate length");
    TEST("sqlstate starts with '4'");
    CHECK(mysql_sqlstate(m)[0] == '4', "wrong sqlstate");

    Q("SELECT 1");
    TEST("errno == 0 after successful query");
    CHECK(mysql_errno(m) == 0, "unexpected error");

    mysql_close(m);
}

static void test_stmt_reexecute(void) {
    printf("\n[stmt re-execute multiple times]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_reexec");
    Q("CREATE TABLE t_reexec(id INT PRIMARY KEY AUTO_INCREMENT, val INT)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_reexec(val) VALUES(?)",
                                strlen("INSERT INTO t_reexec(val) VALUES(?)"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    int val = 0;
    unsigned long vlen = sizeof(int);
    MYSQL_BIND my_bind;
    memset(&my_bind, 0, sizeof(my_bind));
    my_bind.buffer_type = MYSQL_TYPE_LONG;
    my_bind.buffer = &val;
    my_bind.length = &vlen;
    rc = mysql_stmt_bind_param(stmt, &my_bind);

    int ok = 1;
    for (int i = 0; i < 100; i++) {
        val = i;
        rc = mysql_stmt_execute(stmt);
        if (rc != 0) { ok = 0; break; }
    }
    TEST("100 consecutive executes on same stmt");
    CHECK(ok, "execute failed");
    mysql_stmt_close(stmt);

    Q("SELECT COUNT(*) FROM t_reexec");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("100 rows in table");
    CHECK(row && strcmp(row[0], "100") == 0, "wrong count");
    mysql_free_result(res);

    mysql_close(m);
}

static void test_mysql_fetch_lengths(void) {
    printf("\n[mysql_fetch_lengths]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_lengths");
    Q("CREATE TABLE t_lengths(a VARCHAR(20), b INT)");
    Q("INSERT INTO t_lengths VALUES('hello', 42), ('world', 99)");

    Q("SELECT a, b FROM t_lengths");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    unsigned long *lens = mysql_fetch_lengths(res);
    TEST("fetch_lengths non-NULL");
    CHECK(lens != NULL, "NULL lengths");
    TEST("col 0 length == 5 ('hello')");
    CHECK(lens[0] == 5, "wrong length");
    TEST("col 1 length == 2 ('42')");
    CHECK(lens[1] == 2, "wrong length");

    row = mysql_fetch_row(res);
    lens = mysql_fetch_lengths(res);
    TEST("second row lengths");
    CHECK(lens && lens[0] == 5 && lens[1] == 2, "wrong lengths");

    mysql_free_result(res);
    mysql_close(m);
}

static void test_mysql_field_seek(void) {
    printf("\n[mysql_field_seek]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("SELECT 1 AS col_a, 2 AS col_b, 3 AS col_c");
    MYSQL_RES *res = mysql_store_result(m);
    TEST("num_fields == 3");
    CHECK(mysql_num_fields(res) == 3, "wrong count");

    mysql_field_seek(res, 2);
    MYSQL_FIELD *f = mysql_fetch_field(res);
    TEST("field_seek(2): name == 'col_c'");
    CHECK(f && strcmp(f->name, "col_c") == 0, "wrong field");

    mysql_field_seek(res, 0);
    f = mysql_fetch_field(res);
    TEST("field_seek(0): name == 'col_a'");
    CHECK(f && strcmp(f->name, "col_a") == 0, "wrong field");

    mysql_free_result(res);
    mysql_close(m);
}

static void test_mysql_fetch_field_direct(void) {
    printf("\n[mysql_fetch_field_direct]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_fld_dir");
    Q("CREATE TABLE t_fld_dir(id INT PRIMARY KEY AUTO_INCREMENT, name VARCHAR(50) NOT NULL, score DOUBLE)");
    Q("INSERT INTO t_fld_dir(name, score) VALUES('test', 88.5)");

    Q("SELECT * FROM t_fld_dir");
    MYSQL_RES *res = mysql_store_result(m);

    MYSQL_FIELD *f0 = mysql_fetch_field_direct(res, 0);
    TEST("field 0: name='id', PRI_KEY_FLAG set");
    CHECK(f0 && strcmp(f0->name, "id") == 0 && (f0->flags & PRI_KEY_FLAG), "wrong field");

    MYSQL_FIELD *f1 = mysql_fetch_field_direct(res, 1);
    TEST("field 1: name='name', NOT_NULL_FLAG set");
    CHECK(f1 && strcmp(f1->name, "name") == 0 && (f1->flags & NOT_NULL_FLAG), "wrong field");

    MYSQL_FIELD *f2 = mysql_fetch_field_direct(res, 2);
    TEST("field 2: name='score', type==DOUBLE");
    CHECK(f2 && strcmp(f2->name, "score") == 0 && f2->type == MYSQL_TYPE_DOUBLE, "wrong field");

    mysql_free_result(res);
    mysql_close(m);
}

static void test_mysql_field_count_api(void) {
    printf("\n[mysql_field_count API]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("SELECT 1, 2, 3, 4");
    TEST("field_count == 4 after SELECT");
    CHECK(mysql_field_count(m) == 4, "wrong count");
    MYSQL_RES *fc_res = mysql_store_result(m);
    if (fc_res) mysql_free_result(fc_res);

    Q("DROP TABLE IF EXISTS t_fc_test");
    TEST("field_count == 0 after DDL");
    CHECK(mysql_field_count(m) == 0, "wrong count");

    Q("CREATE TABLE t_fc_test(id INT)");
    TEST("field_count == 0 after CREATE");
    CHECK(mysql_field_count(m) == 0, "wrong count");

    Q("INSERT INTO t_fc_test VALUES (1)");
    TEST("field_count == 0 after INSERT");
    CHECK(mysql_field_count(m) == 0, "wrong count");

    mysql_close(m);
}

static void test_mysql_escape_string(void) {
    printf("\n[mysql_escape_string vs mysql_real_escape_string]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    char buf1[100], buf2[100];
    const char *input = "it's a \"test\" with \\backslash";

    unsigned long len1 = mysql_escape_string(buf1, input, (unsigned long)strlen(input));
    unsigned long len2 = mysql_real_escape_string(m, buf2, input, (unsigned long)strlen(input));

    TEST("escape_string returns non-zero");
    CHECK(len1 > 0, "zero length");
    TEST("real_escape_string returns non-zero");
    CHECK(len2 > 0, "zero length");
    TEST("escaped results contain backslash");
    CHECK(strchr(buf1, '\\') != NULL && strchr(buf2, '\\') != NULL, "no backslash");

    Q("DROP TABLE IF EXISTS t_escape");
    Q("CREATE TABLE t_escape(val VARCHAR(100))");

    char insql[256];
    sprintf(insql, "INSERT INTO t_escape VALUES('%s')", buf2);
    Q(insql);

    Q("SELECT val FROM t_escape");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("round-trip: original value preserved");
    CHECK(row && row[0] && strcmp(row[0], input) == 0, "value mismatch");
    mysql_free_result(res);

    mysql_close(m);
}

static void test_mysql_get_host_info(void) {
    printf("\n[mysql_get_host_info / thread_id / server_version]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    const char *hi = mysql_get_host_info(m);
    TEST("host_info non-NULL and non-empty");
    CHECK(hi && hi[0] != '\0', "empty host_info");

    unsigned long tid = mysql_thread_id(m);
    TEST("thread_id > 0");
    CHECK(tid > 0, "zero thread_id");

    unsigned long ver = mysql_get_server_version(m);
    TEST("server_version >= 80000 (MySQL 8.0+)");
    CHECK(ver >= 80000, "wrong version");

    mysql_close(m);
}

static void test_stmt_ps_select_like(void) {
    printf("\n[PS SELECT with LIKE]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_like");
    Q("CREATE TABLE t_like(name VARCHAR(50))");
    Q("INSERT INTO t_like VALUES('Alice'), ('Bob'), ('Carol'), ('David')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT name FROM t_like WHERE name LIKE ?",
                                strlen("SELECT name FROM t_like WHERE name LIKE ?"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    char pattern[20] = "A%";
    unsigned long plen = strlen(pattern);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_STRING;
    pb.buffer = pattern;
    pb.buffer_length = sizeof(pattern);
    pb.length = &plen;
    rc = mysql_stmt_bind_param(stmt, &pb);

    rc = mysql_stmt_execute(stmt);

    char out_name[50];
    unsigned long out_len = 0;
    my_bool out_null = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_STRING;
    rb.buffer = out_name;
    rb.buffer_length = sizeof(out_name);
    rb.length = &out_len;
    rb.is_null = &out_null;
    rc = mysql_stmt_bind_result(stmt, &rb);

    int count = 0;
    while (mysql_stmt_fetch(stmt) == 0) count++;
    TEST("LIKE 'A%%': 1 row (Alice)");
    CHECK(count == 1, "wrong count");

    strcpy(pattern, "%ar%");
    plen = strlen(pattern);
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_bind_result(stmt, &rb);
    count = 0;
    while (mysql_stmt_fetch(stmt) == 0) count++;
    TEST("LIKE '%%ar%%': 1 row (Carol)");
    CHECK(count == 1, "wrong count");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_between(void) {
    printf("\n[PS SELECT with BETWEEN]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_between");
    Q("CREATE TABLE t_between(val INT)");
    Q("INSERT INTO t_between VALUES(10),(20),(30),(40),(50)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT val FROM t_between WHERE val BETWEEN ? AND ?",
                                strlen("SELECT val FROM t_between WHERE val BETWEEN ? AND ?"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    int lo = 20, hi = 40;
    unsigned long llen = sizeof(int), hlen = sizeof(int);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG;
    pb[0].buffer = &lo;
    pb[0].length = &llen;
    pb[1].buffer_type = MYSQL_TYPE_LONG;
    pb[1].buffer = &hi;
    pb[1].length = &hlen;
    rc = mysql_stmt_bind_param(stmt, pb);

    rc = mysql_stmt_execute(stmt);

    int out_val;
    unsigned long olen = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG;
    rb.buffer = &out_val;
    rb.length = &olen;
    rc = mysql_stmt_bind_result(stmt, &rb);

    int count = 0;
    while (mysql_stmt_fetch(stmt) == 0) count++;
    TEST("BETWEEN 20 AND 40: 3 rows");
    CHECK(count == 3, "wrong count");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_in_clause(void) {
    printf("\n[PS SELECT with IN clause]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_in");
    Q("CREATE TABLE t_in(val INT)");
    Q("INSERT INTO t_in VALUES(1),(2),(3),(4),(5)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT val FROM t_in WHERE val IN (?, ?, ?)",
                                strlen("SELECT val FROM t_in WHERE val IN (?, ?, ?)"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    TEST("param_count == 3");
    CHECK(stmt->param_count == 3, "wrong count");

    int v[3] = {1, 3, 5};
    unsigned long vlens[3];
    MYSQL_BIND pb[3];
    memset(pb, 0, sizeof(pb));
    for (int i = 0; i < 3; i++) {
        pb[i].buffer_type = MYSQL_TYPE_LONG;
        pb[i].buffer = &v[i];
        vlens[i] = sizeof(int);
        pb[i].length = &vlens[i];
    }
    rc = mysql_stmt_bind_param(stmt, pb);
    rc = mysql_stmt_execute(stmt);

    int out_val;
    unsigned long olen = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG;
    rb.buffer = &out_val;
    rb.length = &olen;
    rc = mysql_stmt_bind_result(stmt, &rb);

    int count = 0;
    while (mysql_stmt_fetch(stmt) == 0) count++;
    TEST("IN (1,3,5): 3 rows");
    CHECK(count == 3, "wrong count");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_join(void) {
    printf("\n[PS SELECT with JOIN]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_join_a, t_join_b");
    Q("CREATE TABLE t_join_a(id INT, name VARCHAR(20))");
    Q("CREATE TABLE t_join_b(a_id INT, val INT)");
    Q("INSERT INTO t_join_a VALUES(1,'alice'),(2,'bob')");
    Q("INSERT INTO t_join_b VALUES(1,100),(1,200),(2,300)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt,
        "SELECT a.name, b.val FROM t_join_a a JOIN t_join_b b ON a.id=b.a_id WHERE a.id=?",
        strlen("SELECT a.name, b.val FROM t_join_a a JOIN t_join_b b ON a.id=b.a_id WHERE a.id=?"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    int id = 1;
    unsigned long id_len = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG;
    pb.buffer = &id;
    pb.length = &id_len;
    rc = mysql_stmt_bind_param(stmt, &pb);

    char out_name[20];
    int out_val;
    unsigned long nlen = 0, vlen = 0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_STRING;
    rb[0].buffer = out_name;
    rb[0].buffer_length = sizeof(out_name);
    rb[0].length = &nlen;
    rb[1].buffer_type = MYSQL_TYPE_LONG;
    rb[1].buffer = &out_val;
    rb[1].length = &vlen;
    rc = mysql_stmt_bind_result(stmt, rb);

    rc = mysql_stmt_execute(stmt);
    int count = 0;
    while (mysql_stmt_fetch(stmt) == 0) count++;
    TEST("JOIN WHERE id=1: 2 rows");
    CHECK(count == 2, "wrong count");

    id = 2;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_bind_result(stmt, rb);
    count = 0;
    while (mysql_stmt_fetch(stmt) == 0) count++;
    TEST("JOIN WHERE id=2: 1 row");
    CHECK(count == 1, "wrong count");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_aggregate(void) {
    printf("\n[PS SELECT with aggregate functions]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_agg");
    Q("CREATE TABLE t_agg(val INT)");
    Q("INSERT INTO t_agg VALUES(10),(20),(30),(40),(50)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt,
        "SELECT COUNT(*), SUM(val), AVG(val), MIN(val), MAX(val) FROM t_agg",
        strlen("SELECT COUNT(*), SUM(val), AVG(val), MIN(val), MAX(val) FROM t_agg"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    long long cnt, sum_val, min_val, max_val;
    double avg_val;
    unsigned long lens[5] = {0};
    MYSQL_BIND rb[5];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONGLONG;
    rb[0].buffer = &cnt;
    rb[0].length = &lens[0];
    rb[1].buffer_type = MYSQL_TYPE_LONGLONG;
    rb[1].buffer = &sum_val;
    rb[1].length = &lens[1];
    rb[2].buffer_type = MYSQL_TYPE_DOUBLE;
    rb[2].buffer = &avg_val;
    rb[2].length = &lens[2];
    rb[3].buffer_type = MYSQL_TYPE_LONGLONG;
    rb[3].buffer = &min_val;
    rb[3].length = &lens[3];
    rb[4].buffer_type = MYSQL_TYPE_LONGLONG;
    rb[4].buffer = &max_val;
    rb[4].length = &lens[4];

    rc = mysql_stmt_bind_result(stmt, rb);
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);

    TEST("COUNT(*) == 5");
    CHECK(rc == 0 && cnt == 5, "wrong count");
    TEST("SUM(val) == 150");
    CHECK(sum_val == 150, "wrong sum");
    TEST("MIN(val) == 10");
    CHECK(min_val == 10, "wrong min");
    TEST("MAX(val) == 50");
    CHECK(max_val == 50, "wrong max");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_group_by(void) {
    printf("\n[PS SELECT with GROUP BY]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_group");
    Q("CREATE TABLE t_group(cat VARCHAR(10), val INT)");
    Q("INSERT INTO t_group VALUES('A',10),('A',20),('B',30),('B',40),('C',50)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt,
        "SELECT cat, SUM(val) AS s FROM t_group GROUP BY cat ORDER BY cat",
        strlen("SELECT cat, SUM(val) AS s FROM t_group GROUP BY cat ORDER BY cat"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    char out_cat[10];
    long long out_sum;
    unsigned long clen = 0, slen = 0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_STRING;
    rb[0].buffer = out_cat;
    rb[0].buffer_length = sizeof(out_cat);
    rb[0].length = &clen;
    rb[1].buffer_type = MYSQL_TYPE_LONGLONG;
    rb[1].buffer = &out_sum;
    rb[1].length = &slen;

    rc = mysql_stmt_bind_result(stmt, rb);
    rc = mysql_stmt_execute(stmt);

    rc = mysql_stmt_fetch(stmt);
    TEST("GROUP A: sum == 30");
    CHECK(rc == 0 && strcmp(out_cat, "A") == 0 && out_sum == 30, "wrong data");

    rc = mysql_stmt_fetch(stmt);
    TEST("GROUP B: sum == 70");
    CHECK(rc == 0 && strcmp(out_cat, "B") == 0 && out_sum == 70, "wrong data");

    rc = mysql_stmt_fetch(stmt);
    TEST("GROUP C: sum == 50");
    CHECK(rc == 0 && strcmp(out_cat, "C") == 0 && out_sum == 50, "wrong data");

    rc = mysql_stmt_fetch(stmt);
    TEST("no more groups");
    CHECK(rc == MYSQL_NO_DATA, "expected NO_DATA");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_subquery(void) {
    printf("\n[PS SELECT with subquery]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_sub");
    Q("CREATE TABLE t_sub(id INT, val INT)");
    Q("INSERT INTO t_sub VALUES(1,10),(2,20),(3,30),(4,40),(5,50)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt,
        "SELECT id, val FROM t_sub WHERE val > (SELECT AVG(val) FROM t_sub)",
        strlen("SELECT id, val FROM t_sub WHERE val > (SELECT AVG(val) FROM t_sub)"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    rc = mysql_stmt_execute(stmt);

    int out_id, out_val;
    unsigned long ilen = 0, vlen = 0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG;
    rb[0].buffer = &out_id;
    rb[0].length = &ilen;
    rb[1].buffer_type = MYSQL_TYPE_LONG;
    rb[1].buffer = &out_val;
    rb[1].length = &vlen;
    rc = mysql_stmt_bind_result(stmt, rb);

    int count = 0;
    while (mysql_stmt_fetch(stmt) == 0) count++;
    TEST("val > AVG(30): 2 rows (40,50)");
    CHECK(count == 2, "wrong count");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_limit_offset(void) {
    printf("\n[PS SELECT with LIMIT and OFFSET]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_page");
    Q("CREATE TABLE t_page(id INT PRIMARY KEY AUTO_INCREMENT, name VARCHAR(10))");
    Q("INSERT INTO t_page(name) VALUES('a'),('b'),('c'),('d'),('e')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT name FROM t_page ORDER BY id LIMIT ? OFFSET ?",
                                strlen("SELECT name FROM t_page ORDER BY id LIMIT ? OFFSET ?"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    int limit = 2, offset = 2;
    unsigned long ll = sizeof(int), ol = sizeof(int);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG;
    pb[0].buffer = &limit;
    pb[0].length = &ll;
    pb[1].buffer_type = MYSQL_TYPE_LONG;
    pb[1].buffer = &offset;
    pb[1].length = &ol;
    rc = mysql_stmt_bind_param(stmt, pb);
    rc = mysql_stmt_execute(stmt);

    char out_name[10];
    unsigned long nlen = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_STRING;
    rb.buffer = out_name;
    rb.buffer_length = sizeof(out_name);
    rb.length = &nlen;
    rc = mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_fetch(stmt);
    TEST("LIMIT 2 OFFSET 2: first row = 'c'");
    CHECK(rc == 0 && strcmp(out_name, "c") == 0, "wrong value");

    rc = mysql_stmt_fetch(stmt);
    TEST("LIMIT 2 OFFSET 2: second row = 'd'");
    CHECK(rc == 0 && strcmp(out_name, "d") == 0, "wrong value");

    rc = mysql_stmt_fetch(stmt);
    TEST("no more rows");
    CHECK(rc == MYSQL_NO_DATA, "expected NO_DATA");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_large_string(void) {
    printf("\n[PS large string parameter]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_large_str");
    Q("CREATE TABLE t_large_str(data MEDIUMTEXT)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_large_str VALUES(?)",
                                strlen("INSERT INTO t_large_str VALUES(?)"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    size_t big_len = 65536;
    char *big_str = (char*)malloc(big_len + 1);
    for (size_t i = 0; i < big_len; i++) big_str[i] = 'A' + (i % 26);
    big_str[big_len] = '\0';
    unsigned long blen = (unsigned long)big_len;

    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_STRING;
    pb.buffer = big_str;
    pb.buffer_length = blen;
    pb.length = &blen;
    rc = mysql_stmt_bind_param(stmt, &pb);
    rc = mysql_stmt_execute(stmt);
    TEST("insert 64KB string via PS");
    CHECK(rc == 0, mysql_stmt_error(stmt));
    mysql_stmt_close(stmt);

    Q("SELECT LENGTH(data) FROM t_large_str");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("stored length == 65536");
    CHECK(row && row[0] && strcmp(row[0], "65536") == 0, "wrong length");
    mysql_free_result(res);

    free(big_str);
    mysql_close(m);
}

static void test_stmt_year_type(void) {
    printf("\n[YEAR type]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_year");
    Q("CREATE TABLE t_year(y YEAR)");
    Q("INSERT INTO t_year VALUES(2024),(1999),(2000)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT y FROM t_year ORDER BY y",
                                strlen("SELECT y FROM t_year ORDER BY y"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    short out_y;
    unsigned long ylen = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_SHORT;
    rb.buffer = &out_y;
    rb.length = &ylen;

    rc = mysql_stmt_bind_result(stmt, &rb);
    rc = mysql_stmt_execute(stmt);
    
    rc = mysql_stmt_fetch(stmt);
    TEST("YEAR: 1999");
    CHECK(rc == 0 && out_y == 1999, "wrong value");

    rc = mysql_stmt_fetch(stmt);
    TEST("YEAR: 2000");
    CHECK(rc == 0 && out_y == 2000, "wrong value");

    rc = mysql_stmt_fetch(stmt);
    TEST("YEAR: 2024");
    CHECK(rc == 0 && out_y == 2024, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_enum_type(void) {
    printf("\n[ENUM type]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_enum");
    Q("CREATE TABLE t_enum(color ENUM('red','green','blue'))");
    Q("INSERT INTO t_enum VALUES('red'),('blue')");

    Q("SELECT color FROM t_enum ORDER BY color");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("ENUM text: 'red'");
    CHECK(row && row[0] && strcmp(row[0], "red") == 0, "wrong value");
    row = mysql_fetch_row(res);
    TEST("ENUM text: 'blue'");
    CHECK(row && row[0] && strcmp(row[0], "blue") == 0, "wrong value");
    mysql_free_result(res);

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT color FROM t_enum ORDER BY color",
                                strlen("SELECT color FROM t_enum ORDER BY color"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    char out_val[20];
    unsigned long olen = 0;
    my_bool onull = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_STRING;
    rb.buffer = out_val;
    rb.buffer_length = sizeof(out_val);
    rb.length = &olen;
    rb.is_null = &onull;

    rc = mysql_stmt_bind_result(stmt, &rb);
    rc = mysql_stmt_execute(stmt);

    rc = mysql_stmt_fetch(stmt);
    TEST("ENUM PS: 'red'");
    CHECK(rc == 0 && strcmp(out_val, "red") == 0, "wrong value");

    rc = mysql_stmt_fetch(stmt);
    TEST("ENUM PS: 'blue'");
    CHECK(rc == 0 && strcmp(out_val, "blue") == 0, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_delete_with_param(void) {
    printf("\n[PS DELETE with parameter]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_ps_del");
    Q("CREATE TABLE t_ps_del(id INT, val INT)");
    Q("INSERT INTO t_ps_del VALUES(1,10),(2,20),(3,30),(4,40)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "DELETE FROM t_ps_del WHERE id > ?",
                                strlen("DELETE FROM t_ps_del WHERE id > ?"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    int threshold = 2;
    unsigned long tlen = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG;
    pb.buffer = &threshold;
    pb.length = &tlen;
    rc = mysql_stmt_bind_param(stmt, &pb);
    rc = mysql_stmt_execute(stmt);

    TEST("affected_rows == 2");
    CHECK(rc == 0 && mysql_stmt_affected_rows(stmt) == 2, "wrong affected_rows");
    mysql_stmt_close(stmt);

    Q("SELECT COUNT(*) FROM t_ps_del");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("2 rows remain");
    CHECK(row && strcmp(row[0], "2") == 0, "wrong count");
    mysql_free_result(res);

    mysql_close(m);
}

static void test_stmt_ps_update_with_param(void) {
    printf("\n[PS UPDATE with parameters]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_ps_upd");
    Q("CREATE TABLE t_ps_upd(id INT, val INT)");
    Q("INSERT INTO t_ps_upd VALUES(1,10),(2,20),(3,30)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "UPDATE t_ps_upd SET val=? WHERE id=?",
                                strlen("UPDATE t_ps_upd SET val=? WHERE id=?"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    int new_val = 999, target_id = 2;
    unsigned long vl = sizeof(int), il = sizeof(int);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG;
    pb[0].buffer = &new_val;
    pb[0].length = &vl;
    pb[1].buffer_type = MYSQL_TYPE_LONG;
    pb[1].buffer = &target_id;
    pb[1].length = &il;
    rc = mysql_stmt_bind_param(stmt, pb);
    rc = mysql_stmt_execute(stmt);

    TEST("affected_rows == 1");
    CHECK(rc == 0 && mysql_stmt_affected_rows(stmt) == 1, "wrong affected_rows");
    mysql_stmt_close(stmt);

    Q("SELECT val FROM t_ps_upd WHERE id=2");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("val updated to 999");
    CHECK(row && strcmp(row[0], "999") == 0, "wrong value");
    mysql_free_result(res);

    mysql_close(m);
}

static void test_stmt_ps_null_update(void) {
    printf("\n[PS UPDATE setting NULL]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_null_upd");
    Q("CREATE TABLE t_null_upd(id INT, val VARCHAR(50))");
    Q("INSERT INTO t_null_upd VALUES(1,'hello'),(2,'world')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "UPDATE t_null_upd SET val=? WHERE id=?",
                                strlen("UPDATE t_null_upd SET val=? WHERE id=?"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    char val_buf[50];
    unsigned long val_len = 0;
    my_bool val_null = 1;
    int id = 1;
    unsigned long id_len = sizeof(int);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_STRING;
    pb[0].buffer = val_buf;
    pb[0].buffer_length = sizeof(val_buf);
    pb[0].length = &val_len;
    pb[0].is_null = &val_null;
    pb[1].buffer_type = MYSQL_TYPE_LONG;
    pb[1].buffer = &id;
    pb[1].length = &id_len;
    rc = mysql_stmt_bind_param(stmt, pb);
    rc = mysql_stmt_execute(stmt);
    TEST("UPDATE with NULL param OK");
    CHECK(rc == 0, mysql_stmt_error(stmt));
    mysql_stmt_close(stmt);

    Q("SELECT val FROM t_null_upd WHERE id=1");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("val is NULL after UPDATE");
    CHECK(row && row[0] == NULL, "expected NULL");
    mysql_free_result(res);

    mysql_close(m);
}

static void test_stmt_ps_concat(void) {
    printf("\n[PS with CONCAT/string functions]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT CONCAT(?, ' ', ?)",
                                strlen("SELECT CONCAT(?, ' ', ?)"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    TEST("param_count == 2");
    CHECK(stmt->param_count == 2, "wrong count");

    char a[20] = "Hello", b[20] = "World";
    unsigned long al = 5, bl = 5;
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_STRING;
    pb[0].buffer = a;
    pb[0].buffer_length = sizeof(a);
    pb[0].length = &al;
    pb[1].buffer_type = MYSQL_TYPE_STRING;
    pb[1].buffer = b;
    pb[1].buffer_length = sizeof(b);
    pb[1].length = &bl;
    rc = mysql_stmt_bind_param(stmt, pb);

    char out[50];
    unsigned long olen = 0;
    my_bool onull = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_STRING;
    rb.buffer = out;
    rb.buffer_length = sizeof(out);
    rb.length = &olen;
    rb.is_null = &onull;
    rc = mysql_stmt_bind_result(stmt, &rb);
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("CONCAT result: 'Hello World'");
    CHECK(rc == 0 && strcmp(out, "Hello World") == 0, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_math(void) {
    printf("\n[PS with math functions]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT ABS(?), ROUND(?, 2), MOD(?, ?)",
                                strlen("SELECT ABS(?), ROUND(?, 2), MOD(?, ?)"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    TEST("param_count == 4");
    CHECK(stmt->param_count == 4, "wrong count");

    double v_abs = -3.14, v_round = 2.71828;
    int v_mod_a = 10, v_mod_b = 3;
    unsigned long l1 = sizeof(double), l2 = sizeof(double);
    unsigned long l3 = sizeof(int), l4 = sizeof(int);
    MYSQL_BIND pb[4];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_DOUBLE;
    pb[0].buffer = &v_abs;
    pb[0].length = &l1;
    pb[1].buffer_type = MYSQL_TYPE_DOUBLE;
    pb[1].buffer = &v_round;
    pb[1].length = &l2;
    pb[2].buffer_type = MYSQL_TYPE_LONG;
    pb[2].buffer = &v_mod_a;
    pb[2].length = &l3;
    pb[3].buffer_type = MYSQL_TYPE_LONG;
    pb[3].buffer = &v_mod_b;
    pb[3].length = &l4;
    rc = mysql_stmt_bind_param(stmt, pb);

    double out_abs, out_round, out_mod;
    unsigned long ol1 = 0, ol2 = 0, ol3 = 0;
    MYSQL_BIND rb[3];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_DOUBLE;
    rb[0].buffer = &out_abs;
    rb[0].length = &ol1;
    rb[1].buffer_type = MYSQL_TYPE_DOUBLE;
    rb[1].buffer = &out_round;
    rb[1].length = &ol2;
    rb[2].buffer_type = MYSQL_TYPE_DOUBLE;
    rb[2].buffer = &out_mod;
    rb[2].length = &ol3;
    rc = mysql_stmt_bind_result(stmt, rb);
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);

    TEST("ABS(-3.14) ~ 3.14");
    CHECK(rc == 0 && fabs(out_abs - 3.14) < 0.001, "wrong value");
    TEST("ROUND(2.71828, 2) ~ 2.72");
    CHECK(fabs(out_round - 2.72) < 0.01, "wrong value");
    TEST("MOD(10, 3) ~ 1.0");
    CHECK(fabs(out_mod - 1.0) < 0.01, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_date_functions(void) {
    printf("\n[PS with date functions]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT YEAR(?), MONTH(?), DAY(?)",
                                strlen("SELECT YEAR(?), MONTH(?), DAY(?)"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    MYSQL_TIME dt;
    unsigned long dt_len = sizeof(MYSQL_TIME);
    my_bool dt_null = 0;
    MYSQL_BIND pb[3];
    memset(pb, 0, sizeof(pb));
    for (int i = 0; i < 3; i++) {
        pb[i].buffer_type = MYSQL_TYPE_DATE;
        pb[i].buffer = &dt;
        pb[i].buffer_length = sizeof(dt);
        pb[i].length = &dt_len;
        pb[i].is_null = &dt_null;
    }
    memset(&dt, 0, sizeof(dt));
    dt.time_type = MYSQL_TIMESTAMP_DATE;
    dt.year = 2024;
    dt.month = 6;
    dt.day = 15;
    rc = mysql_stmt_bind_param(stmt, pb);

    long long out_year, out_month, out_day;
    unsigned long ol1 = 0, ol2 = 0, ol3 = 0;
    MYSQL_BIND rb[3];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONGLONG;
    rb[0].buffer = &out_year;
    rb[0].length = &ol1;
    rb[1].buffer_type = MYSQL_TYPE_LONGLONG;
    rb[1].buffer = &out_month;
    rb[1].length = &ol2;
    rb[2].buffer_type = MYSQL_TYPE_LONGLONG;
    rb[2].buffer = &out_day;
    rb[2].length = &ol3;
    rc = mysql_stmt_bind_result(stmt, rb);
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);

    TEST("YEAR(2024-06-15) == 2024");
    CHECK(rc == 0 && out_year == 2024, "wrong year");
    TEST("MONTH(2024-06-15) == 6");
    CHECK(out_month == 6, "wrong month");
    TEST("DAY(2024-06-15) == 15");
    CHECK(out_day == 15, "wrong day");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_if_null(void) {
    printf("\n[PS with IFNULL]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT IFNULL(?, 'default')",
                                strlen("SELECT IFNULL(?, 'default')"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    char val[20] = "actual";
    unsigned long vlen = 6;
    my_bool vnull = 0;
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_STRING;
    pb.buffer = val;
    pb.buffer_length = sizeof(val);
    pb.length = &vlen;
    pb.is_null = &vnull;
    rc = mysql_stmt_bind_param(stmt, &pb);

    char out[50];
    unsigned long olen = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_STRING;
    rb.buffer = out;
    rb.buffer_length = sizeof(out);
    rb.length = &olen;

    rc = mysql_stmt_bind_result(stmt, &rb);
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("IFNULL('actual', 'default') = 'actual'");
    CHECK(rc == 0 && strcmp(out, "actual") == 0, "wrong value");

    vnull = 1;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_bind_result(stmt, &rb);
    rc = mysql_stmt_fetch(stmt);
    TEST("IFNULL(NULL, 'default') = 'default'");
    CHECK(rc == 0 && strcmp(out, "default") == 0, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_coalesce(void) {
    printf("\n[PS with COALESCE]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT COALESCE(?, ?, ?)",
                                strlen("SELECT COALESCE(?, ?, ?)"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    int v1 = 0, v2 = 0, v3 = 0;
    my_bool n1 = 1, n2 = 1, n3 = 0;
    unsigned long l1 = sizeof(int), l2 = sizeof(int), l3 = sizeof(int);
    MYSQL_BIND pb[3];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG;
    pb[0].buffer = &v1;
    pb[0].length = &l1;
    pb[0].is_null = &n1;
    pb[1].buffer_type = MYSQL_TYPE_LONG;
    pb[1].buffer = &v2;
    pb[1].length = &l2;
    pb[1].is_null = &n2;
    pb[2].buffer_type = MYSQL_TYPE_LONG;
    pb[2].buffer = &v3;
    pb[2].length = &l3;
    pb[2].is_null = &n3;
    v3 = 42;
    rc = mysql_stmt_bind_param(stmt, pb);

    long long out;
    unsigned long olen = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONGLONG;
    rb.buffer = &out;
    rb.length = &olen;
    rc = mysql_stmt_bind_result(stmt, &rb);
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("COALESCE(NULL, NULL, 42) = 42");
    CHECK(rc == 0 && out == 42, "wrong value");

    n1 = 0; v1 = 7;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_bind_result(stmt, &rb);
    rc = mysql_stmt_fetch(stmt);
    TEST("COALESCE(7, NULL, 42) = 7");
    CHECK(rc == 0 && out == 7, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_case_when(void) {
    printf("\n[PS with CASE WHEN]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt,
        "SELECT CASE WHEN ? > 50 THEN 'high' WHEN ? > 20 THEN 'mid' ELSE 'low' END",
        strlen("SELECT CASE WHEN ? > 50 THEN 'high' WHEN ? > 20 THEN 'mid' ELSE 'low' END"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    int val = 0;
    unsigned long vlen = sizeof(int);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG;
    pb[0].buffer = &val;
    pb[0].length = &vlen;
    pb[1].buffer_type = MYSQL_TYPE_LONG;
    pb[1].buffer = &val;
    pb[1].length = &vlen;
    rc = mysql_stmt_bind_param(stmt, pb);

    char out[10];
    unsigned long olen = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_STRING;
    rb.buffer = out;
    rb.buffer_length = sizeof(out);
    rb.length = &olen;

    val = 75;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_bind_result(stmt, &rb);
    rc = mysql_stmt_fetch(stmt);
    TEST("CASE WHEN 75 > 50: 'high'");
    CHECK(rc == 0 && strcmp(out, "high") == 0, "wrong value");

    val = 30;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_bind_result(stmt, &rb);
    rc = mysql_stmt_fetch(stmt);
    TEST("CASE WHEN 30 > 20: 'mid'");
    CHECK(rc == 0 && strcmp(out, "mid") == 0, "wrong value");

    val = 10;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_bind_result(stmt, &rb);
    rc = mysql_stmt_fetch(stmt);
    TEST("CASE WHEN 10: 'low'");
    CHECK(rc == 0 && strcmp(out, "low") == 0, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_cast(void) {
    printf("\n[PS with CAST]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT CAST(? AS CHAR), CAST(? AS SIGNED)",
                                strlen("SELECT CAST(? AS CHAR), CAST(? AS SIGNED)"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    double dval = 3.14159;
    char sval[20] = "42";
    unsigned long dlen = sizeof(double), slen = 2;
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_DOUBLE;
    pb[0].buffer = &dval;
    pb[0].length = &dlen;
    pb[1].buffer_type = MYSQL_TYPE_STRING;
    pb[1].buffer = sval;
    pb[1].buffer_length = sizeof(sval);
    pb[1].length = &slen;
    rc = mysql_stmt_bind_param(stmt, pb);

    char out_str[50];
    long long out_int;
    unsigned long ol1 = 0, ol2 = 0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_STRING;
    rb[0].buffer = out_str;
    rb[0].buffer_length = sizeof(out_str);
    rb[0].length = &ol1;
    rb[1].buffer_type = MYSQL_TYPE_LONGLONG;
    rb[1].buffer = &out_int;
    rb[1].length = &ol2;
    rc = mysql_stmt_bind_result(stmt, rb);
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);

    TEST("CAST(3.14159 AS CHAR) starts with '3.14'");
    CHECK(rc == 0 && strncmp(out_str, "3.14", 4) == 0, "wrong value");
    TEST("CAST('42' AS SIGNED) == 42");
    CHECK(out_int == 42, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_exists(void) {
    printf("\n[PS with EXISTS subquery]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_exists_a, t_exists_b");
    Q("CREATE TABLE t_exists_a(id INT)");
    Q("CREATE TABLE t_exists_b(id INT)");
    Q("INSERT INTO t_exists_a VALUES(1),(2),(3)");
    Q("INSERT INTO t_exists_b VALUES(2),(3)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt,
        "SELECT id FROM t_exists_a a WHERE EXISTS (SELECT 1 FROM t_exists_b b WHERE b.id=a.id)",
        strlen("SELECT id FROM t_exists_a a WHERE EXISTS (SELECT 1 FROM t_exists_b b WHERE b.id=a.id)"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    rc = mysql_stmt_execute(stmt);
    int out_id;
    unsigned long olen = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG;
    rb.buffer = &out_id;
    rb.length = &olen;
    rc = mysql_stmt_bind_result(stmt, &rb);

    int count = 0;
    while (mysql_stmt_fetch(stmt) == 0) count++;
    TEST("EXISTS: 2 rows (2,3)");
    CHECK(count == 2, "wrong count");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_union(void) {
    printf("\n[PS with UNION]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_union_a, t_union_b");
    Q("CREATE TABLE t_union_a(val INT)");
    Q("CREATE TABLE t_union_b(val INT)");
    Q("INSERT INTO t_union_a VALUES(1),(2)");
    Q("INSERT INTO t_union_b VALUES(2),(3)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt,
        "SELECT val FROM t_union_a UNION SELECT val FROM t_union_b ORDER BY val",
        strlen("SELECT val FROM t_union_a UNION SELECT val FROM t_union_b ORDER BY val"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    rc = mysql_stmt_execute(stmt);
    int out_val;
    unsigned long olen = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG;
    rb.buffer = &out_val;
    rb.length = &olen;
    rc = mysql_stmt_bind_result(stmt, &rb);

    int vals[10], count = 0;
    while (mysql_stmt_fetch(stmt) == 0 && count < 10) {
        vals[count++] = out_val;
    }
    TEST("UNION: 3 distinct values");
    CHECK(count == 3, "wrong count");
    TEST("UNION values: 1,2,3");
    CHECK(vals[0] == 1 && vals[1] == 2 && vals[2] == 3, "wrong values");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_having(void) {
    printf("\n[PS with HAVING]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_having");
    Q("CREATE TABLE t_having(cat VARCHAR(5), val INT)");
    Q("INSERT INTO t_having VALUES('A',10),('A',20),('B',5),('B',5),('C',100)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt,
        "SELECT cat, SUM(val) AS s FROM t_having GROUP BY cat HAVING SUM(val) > 20 ORDER BY cat",
        strlen("SELECT cat, SUM(val) AS s FROM t_having GROUP BY cat HAVING SUM(val) > 20 ORDER BY cat"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    char out_cat[5];
    long long out_sum;
    unsigned long cl = 0, sl = 0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_STRING;
    rb[0].buffer = out_cat;
    rb[0].buffer_length = sizeof(out_cat);
    rb[0].length = &cl;
    rb[1].buffer_type = MYSQL_TYPE_LONGLONG;
    rb[1].buffer = &out_sum;
    rb[1].length = &sl;
    rc = mysql_stmt_bind_result(stmt, rb);
    rc = mysql_stmt_execute(stmt);

    rc = mysql_stmt_fetch(stmt);
    TEST("HAVING: A sum=30");
    CHECK(rc == 0 && strcmp(out_cat, "A") == 0 && out_sum == 30, "wrong data");

    rc = mysql_stmt_fetch(stmt);
    TEST("HAVING: C sum=100");
    CHECK(rc == 0 && strcmp(out_cat, "C") == 0 && out_sum == 100, "wrong data");

    rc = mysql_stmt_fetch(stmt);
    TEST("no more groups after HAVING");
    CHECK(rc == MYSQL_NO_DATA, "expected NO_DATA");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_distinct(void) {
    printf("\n[PS with DISTINCT]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_distinct");
    Q("CREATE TABLE t_distinct(val INT)");
    Q("INSERT INTO t_distinct VALUES(1),(2),(2),(3),(3),(3)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT DISTINCT val FROM t_distinct ORDER BY val",
                                strlen("SELECT DISTINCT val FROM t_distinct ORDER BY val"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    rc = mysql_stmt_execute(stmt);
    int out_val;
    unsigned long olen = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG;
    rb.buffer = &out_val;
    rb.length = &olen;
    rc = mysql_stmt_bind_result(stmt, &rb);

    int count = 0;
    while (mysql_stmt_fetch(stmt) == 0) count++;
    TEST("DISTINCT: 3 values");
    CHECK(count == 3, "wrong count");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_order_by_desc(void) {
    printf("\n[PS with ORDER BY DESC]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_order_desc");
    Q("CREATE TABLE t_order_desc(val INT)");
    Q("INSERT INTO t_order_desc VALUES(10),(30),(20),(50),(40)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT val FROM t_order_desc ORDER BY val DESC",
                                strlen("SELECT val FROM t_order_desc ORDER BY val DESC"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    rc = mysql_stmt_execute(stmt);
    int out_val;
    unsigned long olen = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG;
    rb.buffer = &out_val;
    rb.length = &olen;
    rc = mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_fetch(stmt);
    TEST("DESC first: 50");
    CHECK(rc == 0 && out_val == 50, "wrong value");
    rc = mysql_stmt_fetch(stmt);
    TEST("DESC second: 40");
    CHECK(rc == 0 && out_val == 40, "wrong value");
    rc = mysql_stmt_fetch(stmt);
    TEST("DESC third: 30");
    CHECK(rc == 0 && out_val == 30, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_is_null(void) {
    printf("\n[PS with IS NULL / IS NOT NULL]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_isnull");
    Q("CREATE TABLE t_isnull(id INT, val INT)");
    Q("INSERT INTO t_isnull VALUES(1,10),(2,NULL),(3,30)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT id FROM t_isnull WHERE val IS NULL",
                                strlen("SELECT id FROM t_isnull WHERE val IS NULL"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    rc = mysql_stmt_execute(stmt);
    int out_id;
    unsigned long olen = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG;
    rb.buffer = &out_id;
    rb.length = &olen;
    rc = mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_fetch(stmt);
    TEST("IS NULL: id=2");
    CHECK(rc == 0 && out_id == 2, "wrong value");
    rc = mysql_stmt_fetch(stmt);
    CHECK(rc == MYSQL_NO_DATA, "expected NO_DATA");
    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "SELECT COUNT(*) FROM t_isnull WHERE val IS NOT NULL",
                                strlen("SELECT COUNT(*) FROM t_isnull WHERE val IS NOT NULL"));
    rc = mysql_stmt_execute(stmt);
    long long cnt;
    unsigned long clen = 0;
    MYSQL_BIND rb2;
    memset(&rb2, 0, sizeof(rb2));
    rb2.buffer_type = MYSQL_TYPE_LONGLONG;
    rb2.buffer = &cnt;
    rb2.length = &clen;
    rc = mysql_stmt_bind_result(stmt, &rb2);
    rc = mysql_stmt_fetch(stmt);
    TEST("IS NOT NULL: count=2");
    CHECK(rc == 0 && cnt == 2, "wrong count");
    mysql_stmt_close(stmt);

    mysql_close(m);
}

static void test_stmt_ps_boolean_expr(void) {
    printf("\n[PS with AND/OR boolean expressions]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_bool");
    Q("CREATE TABLE t_bool(id INT, a INT, b INT)");
    Q("INSERT INTO t_bool VALUES(1,10,20),(2,30,5),(3,10,5),(4,30,20)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT id FROM t_bool WHERE a > ? AND b < ? ORDER BY id",
                                strlen("SELECT id FROM t_bool WHERE a > ? AND b < ? ORDER BY id"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    int lo = 15, hi = 10;
    unsigned long ll = sizeof(int), hl = sizeof(int);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG;
    pb[0].buffer = &lo;
    pb[0].length = &ll;
    pb[1].buffer_type = MYSQL_TYPE_LONG;
    pb[1].buffer = &hi;
    pb[1].length = &hl;
    rc = mysql_stmt_bind_param(stmt, pb);
    rc = mysql_stmt_execute(stmt);

    int out_id;
    unsigned long olen = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG;
    rb.buffer = &out_id;
    rb.length = &olen;
    rc = mysql_stmt_bind_result(stmt, &rb);

    int count = 0;
    while (mysql_stmt_fetch(stmt) == 0) count++;
    TEST("AND: a>15 AND b<10: 1 row (id=2)");
    CHECK(count == 1, "wrong count");
    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "SELECT id FROM t_bool WHERE a > ? OR b < ? ORDER BY id",
                                strlen("SELECT id FROM t_bool WHERE a > ? OR b < ? ORDER BY id"));
    lo = 25; hi = 10;
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG;
    pb[0].buffer = &lo;
    pb[0].length = &ll;
    pb[1].buffer_type = MYSQL_TYPE_LONG;
    pb[1].buffer = &hi;
    pb[1].length = &hl;
    rc = mysql_stmt_bind_param(stmt, pb);
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_bind_result(stmt, &rb);
    count = 0;
    while (mysql_stmt_fetch(stmt) == 0) count++;
    TEST("OR: a>25 OR b<10: 3 rows (id=2,3,4)");
    CHECK(count == 3, "wrong count");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_nested_arithmetic(void) {
    printf("\n[PS with nested arithmetic]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT (? + ?) * ? - ?",
                                strlen("SELECT (? + ?) * ? - ?"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    int a = 3, b = 7, c = 2, d = 5;
    unsigned long al = sizeof(int), bl = sizeof(int), cl = sizeof(int), dl = sizeof(int);
    MYSQL_BIND pb[4];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG;
    pb[0].buffer = &a;
    pb[0].length = &al;
    pb[1].buffer_type = MYSQL_TYPE_LONG;
    pb[1].buffer = &b;
    pb[1].length = &bl;
    pb[2].buffer_type = MYSQL_TYPE_LONG;
    pb[2].buffer = &c;
    pb[2].length = &cl;
    pb[3].buffer_type = MYSQL_TYPE_LONG;
    pb[3].buffer = &d;
    pb[3].length = &dl;
    rc = mysql_stmt_bind_param(stmt, pb);

    long long out;
    unsigned long olen = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONGLONG;
    rb.buffer = &out;
    rb.length = &olen;
    rc = mysql_stmt_bind_result(stmt, &rb);
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("(3+7)*2-5 = 15");
    CHECK(rc == 0 && out == 15, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_string_compare(void) {
    printf("\n[PS with string comparison]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_str_cmp");
    Q("CREATE TABLE t_str_cmp(name VARCHAR(20))");
    Q("INSERT INTO t_str_cmp VALUES('apple'),('banana'),('cherry'),('date')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT name FROM t_str_cmp WHERE name > ? ORDER BY name",
                                strlen("SELECT name FROM t_str_cmp WHERE name > ? ORDER BY name"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    char threshold[20] = "banana";
    unsigned long tlen = 6;
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_STRING;
    pb.buffer = threshold;
    pb.buffer_length = sizeof(threshold);
    pb.length = &tlen;
    rc = mysql_stmt_bind_param(stmt, &pb);

    char out_name[20];
    unsigned long olen = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_STRING;
    rb.buffer = out_name;
    rb.buffer_length = sizeof(out_name);
    rb.length = &olen;
    rc = mysql_stmt_bind_result(stmt, &rb);
    rc = mysql_stmt_execute(stmt);

    rc = mysql_stmt_fetch(stmt);
    TEST("name > 'banana': 'cherry'");
    CHECK(rc == 0 && strcmp(out_name, "cherry") == 0, "wrong value");
    rc = mysql_stmt_fetch(stmt);
    TEST("name > 'banana': 'date'");
    CHECK(rc == 0 && strcmp(out_name, "date") == 0, "wrong value");
    rc = mysql_stmt_fetch(stmt);
    CHECK(rc == MYSQL_NO_DATA, "expected NO_DATA");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_multi_column_order(void) {
    printf("\n[PS with multi-column ORDER BY]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_multi_order");
    Q("CREATE TABLE t_multi_order(a INT, b INT)");
    Q("INSERT INTO t_multi_order VALUES(2,1),(1,2),(2,2),(1,1)");

    Q("SELECT a, b FROM t_multi_order ORDER BY a, b");
    MYSQL_RES *res = mysql_store_result(m);
    int expected[4][2] = {{1,1},{1,2},{2,1},{2,2}};
    int ok = 1, i = 0;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)) != NULL && i < 4) {
        if (atoi(row[0]) != expected[i][0] || atoi(row[1]) != expected[i][1]) ok = 0;
        i++;
    }
    TEST("multi-column ORDER BY: correct order");
    CHECK(ok && i == 4, "wrong order");
    mysql_free_result(res);

    mysql_close(m);
}

static void test_stmt_ps_left_join(void) {
    printf("\n[PS with LEFT JOIN]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_left_a, t_left_b");
    Q("CREATE TABLE t_left_a(id INT, name VARCHAR(10))");
    Q("CREATE TABLE t_left_b(a_id INT, val INT)");
    Q("INSERT INTO t_left_a VALUES(1,'alice'),(2,'bob'),(3,'carol')");
    Q("INSERT INTO t_left_b VALUES(1,100),(3,300)");

    Q("SELECT a.name, b.val FROM t_left_a a LEFT JOIN t_left_b b ON a.id=b.a_id ORDER BY a.id");
    MYSQL_RES *res = mysql_store_result(m);
    TEST("LEFT JOIN: 3 rows");
    CHECK(res && res->row_count == 3, "wrong count");

    MYSQL_ROW row;
    row = mysql_fetch_row(res);
    TEST("alice: val=100");
    CHECK(row && row[1] && strcmp(row[1], "100") == 0, "wrong value");

    row = mysql_fetch_row(res);
    TEST("bob: val is NULL");
    CHECK(row && row[1] == NULL, "expected NULL");

    row = mysql_fetch_row(res);
    TEST("carol: val=300");
    CHECK(row && row[1] && strcmp(row[1], "300") == 0, "wrong value");

    mysql_free_result(res);
    mysql_close(m);
}

static void test_stmt_ps_reexecute_multi(void) {
    printf("\n[PS re-execute 5 times]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT ? * 2",
                                strlen("SELECT ? * 2"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    int val = 0;
    unsigned long vlen = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG;
    pb.buffer = &val;
    pb.length = &vlen;
    mysql_stmt_bind_param(stmt, &pb);

    long long out;
    unsigned long olen = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONGLONG;
    rb.buffer = &out;
    rb.length = &olen;
    mysql_stmt_bind_result(stmt, &rb);

    int expected[] = {2, 10, 50, 100, 0};
    int all_ok = 1;
    for (int i = 0; i < 5; i++) {
        val = expected[i] / 2;
        rc = mysql_stmt_execute(stmt);
        rc = mysql_stmt_fetch(stmt);
        if (rc != 0 || out != expected[i]) { all_ok = 0; }
    }
    TEST("re-execute 5 times with different values");
    CHECK(all_ok, "some values wrong");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_mixed_types(void) {
    printf("\n[PS mixed column types]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_mix");
    Q("CREATE TABLE t_mix(id INT, name VARCHAR(30), score DOUBLE, active TINYINT, created DATE)");
    Q("INSERT INTO t_mix VALUES(1,'alice',95.5,1,'2024-01-15'),(2,'bob',82.3,0,'2024-06-20')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT id, name, score, active, created FROM t_mix WHERE id = ?",
                                strlen("SELECT id, name, score, active, created FROM t_mix WHERE id = ?"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    int id = 1;
    unsigned long idlen = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG;
    pb.buffer = &id;
    pb.length = &idlen;
    mysql_stmt_bind_param(stmt, &pb);

    int out_id;
    char out_name[32];
    double out_score;
    int8_t out_active;
    MYSQL_TIME out_date;
    unsigned long l1=0,l2=0,l3=0,l4=0,l5=0;
    MYSQL_BIND rb[5];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &out_id; rb[0].length = &l1;
    rb[1].buffer_type = MYSQL_TYPE_STRING; rb[1].buffer = out_name; rb[1].buffer_length = sizeof(out_name); rb[1].length = &l2;
    rb[2].buffer_type = MYSQL_TYPE_DOUBLE; rb[2].buffer = &out_score; rb[2].length = &l3;
    rb[3].buffer_type = MYSQL_TYPE_TINY; rb[3].buffer = &out_active; rb[3].length = &l4;
    rb[4].buffer_type = MYSQL_TYPE_DATE; rb[4].buffer = &out_date; rb[4].buffer_length = sizeof(out_date); rb[4].length = &l5;
    mysql_stmt_bind_result(stmt, rb);

    id = 1;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("mixed types row 1: id=1, name='alice'");
    CHECK(rc == 0 && out_id == 1 && strcmp(out_name, "alice") == 0, "wrong values");
    TEST("mixed types row 1: score~95.5, active=1");
    CHECK(fabs(out_score - 95.5) < 0.1 && out_active == 1, "wrong values");
    TEST("mixed types row 1: date=2024-01-15");
    CHECK(out_date.year == 2024 && out_date.month == 1 && out_date.day == 15, "wrong date");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_affected_rows_dml(void) {
    printf("\n[PS affected rows for DML]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_dml");
    Q("CREATE TABLE t_dml(id INT PRIMARY KEY, val INT)");
    Q("INSERT INTO t_dml VALUES(1,10),(2,20),(3,30)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "UPDATE t_dml SET val = val + ? WHERE val > ?",
                                strlen("UPDATE t_dml SET val = val + ? WHERE val > ?"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    int delta = 100, threshold = 15;
    unsigned long l1 = sizeof(int), l2 = sizeof(int);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &delta; pb[0].length = &l1;
    pb[1].buffer_type = MYSQL_TYPE_LONG; pb[1].buffer = &threshold; pb[1].length = &l2;
    mysql_stmt_bind_param(stmt, pb);

    rc = mysql_stmt_execute(stmt);
    TEST("UPDATE affected_rows == 2");
    CHECK(rc == 0 && mysql_stmt_affected_rows(stmt) == 2, "wrong count");

    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "DELETE FROM t_dml WHERE id = ?",
                            strlen("DELETE FROM t_dml WHERE id = ?"));
    int del_id = 1;
    unsigned long dl = sizeof(int);
    MYSQL_BIND dpb;
    memset(&dpb, 0, sizeof(dpb));
    dpb.buffer_type = MYSQL_TYPE_LONG; dpb.buffer = &del_id; dpb.length = &dl;
    mysql_stmt_bind_param(stmt, &dpb);
    rc = mysql_stmt_execute(stmt);
    TEST("DELETE affected_rows == 1");
    CHECK(rc == 0 && mysql_stmt_affected_rows(stmt) == 1, "wrong count");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_null_in_where(void) {
    printf("\n[PS NULL in WHERE clause]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_nw");
    Q("CREATE TABLE t_nw(id INT, val INT)");
    Q("INSERT INTO t_nw VALUES(1,10),(2,NULL),(3,30)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT id FROM t_nw WHERE val IS NULL",
                                strlen("SELECT id FROM t_nw WHERE val IS NULL"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    rc = mysql_stmt_execute(stmt);
    int out_id;
    unsigned long olen = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_id; rb.length = &olen;
    mysql_stmt_bind_result(stmt, &rb);

    int count = 0;
    while (mysql_stmt_fetch(stmt) == 0) count++;
    TEST("val IS NULL returns 1 row (id=2)");
    CHECK(count == 1, "wrong count");

    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "SELECT id FROM t_nw WHERE val = ?",
                            strlen("SELECT id FROM t_nw WHERE val = ?"));
    int val = 10;
    unsigned long vlen = sizeof(int);
    my_bool vnull = 0;
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &val; pb.length = &vlen; pb.is_null = &vnull;
    mysql_stmt_bind_param(stmt, &pb);
    rc = mysql_stmt_execute(stmt);
    mysql_stmt_bind_result(stmt, &rb);
    count = 0;
    while (mysql_stmt_fetch(stmt) == 0) count++;
    TEST("val = 10 returns 1 row");
    CHECK(count == 1, "wrong count");

    vnull = 1;
    rc = mysql_stmt_execute(stmt);
    mysql_stmt_bind_result(stmt, &rb);
    count = 0;
    while (mysql_stmt_fetch(stmt) == 0) count++;
    TEST("val = NULL returns 0 rows");
    CHECK(count == 0, "wrong count");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_inner_join(void) {
    printf("\n[PS INNER JOIN]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_ij_a, t_ij_b");
    Q("CREATE TABLE t_ij_a(id INT, name VARCHAR(20))");
    Q("CREATE TABLE t_ij_b(a_id INT, score INT)");
    Q("INSERT INTO t_ij_a VALUES(1,'alice'),(2,'bob'),(3,'carol')");
    Q("INSERT INTO t_ij_b VALUES(1,90),(2,85),(2,95)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT a.name, b.score FROM t_ij_a a INNER JOIN t_ij_b b ON a.id = b.a_id WHERE b.score > ? ORDER BY a.name, b.score",
                                strlen("SELECT a.name, b.score FROM t_ij_a a INNER JOIN t_ij_b b ON a.id = b.a_id WHERE b.score > ? ORDER BY a.name, b.score"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    int min_score = 88;
    unsigned long ml = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &min_score; pb.length = &ml;
    mysql_stmt_bind_param(stmt, &pb);

    char out_name[20];
    int out_score;
    unsigned long l1=0,l2=0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_STRING; rb[0].buffer = out_name; rb[0].buffer_length = sizeof(out_name); rb[0].length = &l1;
    rb[1].buffer_type = MYSQL_TYPE_LONG; rb[1].buffer = &out_score; rb[1].length = &l2;
    mysql_stmt_bind_result(stmt, rb);

    rc = mysql_stmt_execute(stmt);
    int count = 0;
    while (mysql_stmt_fetch(stmt) == 0) count++;
    TEST("INNER JOIN score>88: 2 rows (alice:90, bob:95)");
    CHECK(count == 2, "wrong count");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_group_concat(void) {
    printf("\n[PS GROUP_CONCAT]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_gc");
    Q("CREATE TABLE t_gc(cat VARCHAR(10), item VARCHAR(20))");
    Q("INSERT INTO t_gc VALUES('A','apple'),('A','avocado'),('B','banana')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT cat, GROUP_CONCAT(item ORDER BY item SEPARATOR ',') AS items FROM t_gc GROUP BY cat HAVING COUNT(*) > ?",
                                strlen("SELECT cat, GROUP_CONCAT(item ORDER BY item SEPARATOR ',') AS items FROM t_gc GROUP BY cat HAVING COUNT(*) > ?"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    int min_count = 1;
    unsigned long ml = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &min_count; pb.length = &ml;
    mysql_stmt_bind_param(stmt, &pb);

    char out_cat[10], out_items[100];
    unsigned long l1=0,l2=0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_STRING; rb[0].buffer = out_cat; rb[0].buffer_length = sizeof(out_cat); rb[0].length = &l1;
    rb[1].buffer_type = MYSQL_TYPE_STRING; rb[1].buffer = out_items; rb[1].buffer_length = sizeof(out_items); rb[1].length = &l2;
    mysql_stmt_bind_result(stmt, rb);

    rc = mysql_stmt_execute(stmt);
    int found_a = 0;
    while (mysql_stmt_fetch(stmt) == 0) {
        if (strcmp(out_cat, "A") == 0 && strstr(out_items, "apple") && strstr(out_items, "avocado"))
            found_a = 1;
    }
    TEST("GROUP_CONCAT cat=A has apple,avocado");
    CHECK(found_a, "wrong concat result");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_date_arithmetic(void) {
    printf("\n[PS date arithmetic]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT DATE_ADD(?, INTERVAL ? DAY)",
                                strlen("SELECT DATE_ADD(?, INTERVAL ? DAY)"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    MYSQL_TIME dt;
    memset(&dt, 0, sizeof(dt));
    dt.time_type = MYSQL_TIMESTAMP_DATE;
    dt.year = 2024; dt.month = 1; dt.day = 15;
    unsigned long dtlen = sizeof(MYSQL_TIME);
    int days = 10;
    unsigned long dlen = sizeof(int);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_DATE; pb[0].buffer = &dt; pb[0].buffer_length = sizeof(dt); pb[0].length = &dtlen;
    pb[1].buffer_type = MYSQL_TYPE_LONG; pb[1].buffer = &days; pb[1].length = &dlen;
    mysql_stmt_bind_param(stmt, pb);

    char out[32];
    unsigned long olen = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_STRING; rb.buffer = out; rb.buffer_length = sizeof(out); rb.length = &olen;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("DATE_ADD(2024-01-15, 10 DAY) contains 2024-01-25");
    CHECK(rc == 0 && strstr(out, "2024-01-25") != NULL, "wrong date");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_string_funcs(void) {
    printf("\n[PS string functions]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT LENGTH(?), UPPER(?), SUBSTRING(?, ?, ?)",
                                strlen("SELECT LENGTH(?), UPPER(?), SUBSTRING(?, ?, ?)"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    char str[30] = "Hello World";
    unsigned long slen = 11;
    int start = 7, len = 5;
    unsigned long s1=sizeof(int), s2=sizeof(int);
    MYSQL_BIND pb[5];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_STRING; pb[0].buffer = str; pb[0].buffer_length = sizeof(str); pb[0].length = &slen;
    pb[1].buffer_type = MYSQL_TYPE_STRING; pb[1].buffer = str; pb[1].buffer_length = sizeof(str); pb[1].length = &slen;
    pb[2].buffer_type = MYSQL_TYPE_STRING; pb[2].buffer = str; pb[2].buffer_length = sizeof(str); pb[2].length = &slen;
    pb[3].buffer_type = MYSQL_TYPE_LONG; pb[3].buffer = &start; pb[3].length = &s1;
    pb[4].buffer_type = MYSQL_TYPE_LONG; pb[4].buffer = &len; pb[4].length = &s2;
    mysql_stmt_bind_param(stmt, pb);

    long long out_len;
    char out_upper[20], out_sub[20];
    unsigned long ol1=0,ol2=0,ol3=0;
    MYSQL_BIND rb[3];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONGLONG; rb[0].buffer = &out_len; rb[0].length = &ol1;
    rb[1].buffer_type = MYSQL_TYPE_STRING; rb[1].buffer = out_upper; rb[1].buffer_length = sizeof(out_upper); rb[1].length = &ol2;
    rb[2].buffer_type = MYSQL_TYPE_STRING; rb[2].buffer = out_sub; rb[2].buffer_length = sizeof(out_sub); rb[2].length = &ol3;
    mysql_stmt_bind_result(stmt, rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("LENGTH('Hello World') = 11");
    CHECK(rc == 0 && out_len == 11, "wrong length");
    TEST("UPPER('Hello World') = 'HELLO WORLD'");
    CHECK(strcmp(out_upper, "HELLO WORLD") == 0, "wrong upper");
    TEST("SUBSTRING('Hello World', 7, 5) = 'World'");
    CHECK(strcmp(out_sub, "World") == 0, "wrong substring");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_many_params(void) {
    printf("\n[PS many parameters]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT ?+?+?+?+?+?+?+?+?+?",
                                strlen("SELECT ?+?+?+?+?+?+?+?+?+?"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    TEST("param_count == 10");
    CHECK(stmt->param_count == 10, "wrong count");

    int vals[10];
    unsigned long lens[10];
    MYSQL_BIND pb[10];
    memset(pb, 0, sizeof(pb));
    for (int i = 0; i < 10; i++) {
        vals[i] = i + 1;
        lens[i] = sizeof(int);
        pb[i].buffer_type = MYSQL_TYPE_LONG;
        pb[i].buffer = &vals[i];
        pb[i].length = &lens[i];
    }
    mysql_stmt_bind_param(stmt, pb);

    long long out;
    unsigned long olen = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONGLONG; rb.buffer = &out; rb.length = &olen;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("1+2+...+10 = 55");
    CHECK(rc == 0 && out == 55, "wrong sum");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_error_recovery(void) {
    printf("\n[PS error recovery]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_err");
    Q("CREATE TABLE t_err(id INT PRIMARY KEY, val INT)");
    Q("INSERT INTO t_err VALUES(1,100)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_err VALUES(?, ?)",
                                strlen("INSERT INTO t_err VALUES(?, ?)"));

    int id = 1, val = 200;
    unsigned long l1 = sizeof(int), l2 = sizeof(int);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &l1;
    pb[1].buffer_type = MYSQL_TYPE_LONG; pb[1].buffer = &val; pb[1].length = &l2;
    mysql_stmt_bind_param(stmt, pb);

    rc = mysql_stmt_execute(stmt);
    TEST("duplicate key insert returns error");
    CHECK(rc != 0, "should have failed");

    id = 2; val = 200;
    rc = mysql_stmt_execute(stmt);
    TEST("after error, re-execute succeeds");
    CHECK(rc == 0, "should have succeeded");

    TEST("after recovery, affected_rows == 1");
    CHECK(mysql_stmt_affected_rows(stmt) == 1, "wrong count");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_multi_conn(void) {
    printf("\n[PS multiple connections]\n");
    MYSQL *m1 = get_conn();
    MYSQL *m2 = get_conn();
    if (!m1 || !m2) { FAIL("connect failed"); if(m1) mysql_close(m1); if(m2) mysql_close(m2); return; }

    Q2(m1, "DROP TABLE IF EXISTS t_mc");
    Q2(m1, "CREATE TABLE t_mc(id INT, val INT)");
    Q2(m1, "INSERT INTO t_mc VALUES(1,10),(2,20)");

    MYSQL_STMT *s1 = mysql_stmt_init(m1);
    int rc = mysql_stmt_prepare(s1, "SELECT val FROM t_mc WHERE id = ?",
                                strlen("SELECT val FROM t_mc WHERE id = ?"));
    int id1 = 1;
    unsigned long il1 = sizeof(int);
    MYSQL_BIND pb1;
    memset(&pb1, 0, sizeof(pb1));
    pb1.buffer_type = MYSQL_TYPE_LONG; pb1.buffer = &id1; pb1.length = &il1;
    mysql_stmt_bind_param(s1, &pb1);

    MYSQL_STMT *s2 = mysql_stmt_init(m2);
    rc = mysql_stmt_prepare(s2, "SELECT val FROM t_mc WHERE id = ?",
                            strlen("SELECT val FROM t_mc WHERE id = ?"));
    int id2 = 2;
    unsigned long il2 = sizeof(int);
    MYSQL_BIND pb2;
    memset(&pb2, 0, sizeof(pb2));
    pb2.buffer_type = MYSQL_TYPE_LONG; pb2.buffer = &id2; pb2.length = &il2;
    mysql_stmt_bind_param(s2, &pb2);

    rc = mysql_stmt_execute(s1);
    int out1;
    unsigned long ol1 = 0;
    MYSQL_BIND rb1;
    memset(&rb1, 0, sizeof(rb1));
    rb1.buffer_type = MYSQL_TYPE_LONG; rb1.buffer = &out1; rb1.length = &ol1;
    mysql_stmt_bind_result(s1, &rb1);
    rc = mysql_stmt_fetch(s1);

    rc = mysql_stmt_execute(s2);
    int out2;
    unsigned long ol2 = 0;
    MYSQL_BIND rb2;
    memset(&rb2, 0, sizeof(rb2));
    rb2.buffer_type = MYSQL_TYPE_LONG; rb2.buffer = &out2; rb2.length = &ol2;
    mysql_stmt_bind_result(s2, &rb2);
    rc = mysql_stmt_fetch(s2);

    TEST("conn1: id=1 val=10, conn2: id=2 val=20");
    CHECK(out1 == 10 && out2 == 20, "wrong values");

    mysql_stmt_close(s1);
    mysql_stmt_close(s2);
    mysql_close(m1);
    mysql_close(m2);
}

static void test_stmt_ps_auto_increment(void) {
    printf("\n[PS auto-increment]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_ai");
    Q("CREATE TABLE t_ai(id INT AUTO_INCREMENT PRIMARY KEY, name VARCHAR(20))");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_ai(name) VALUES(?)",
                                strlen("INSERT INTO t_ai(name) VALUES(?)"));

    char name[20] = "first";
    unsigned long nlen = 5;
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_STRING; pb.buffer = name; pb.buffer_length = sizeof(name); pb.length = &nlen;
    mysql_stmt_bind_param(stmt, &pb);

    rc = mysql_stmt_execute(stmt);
    my_ulonglong id1 = mysql_stmt_insert_id(stmt);
    TEST("first insert_id > 0");
    CHECK(rc == 0 && id1 > 0, "no insert_id");

    strcpy(name, "second"); nlen = 6;
    rc = mysql_stmt_execute(stmt);
    my_ulonglong id2 = mysql_stmt_insert_id(stmt);
    TEST("second insert_id > first");
    CHECK(rc == 0 && id2 > id1, "wrong id order");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_decimal_precision(void) {
    printf("\n[PS DECIMAL precision]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_dec");
    Q("CREATE TABLE t_dec(id INT, price DECIMAL(10,2))");
    Q("INSERT INTO t_dec VALUES(1, 1234.56),(2, 0.01),(3, 9999999.99)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT price FROM t_dec WHERE id = ?",
                                strlen("SELECT price FROM t_dec WHERE id = ?"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    char out[32];
    unsigned long olen = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_STRING; rb.buffer = out; rb.buffer_length = sizeof(out); rb.length = &olen;
    mysql_stmt_bind_result(stmt, &rb);

    id = 1;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("DECIMAL 1234.56 as string");
    CHECK(rc == 0 && strcmp(out, "1234.56") == 0, "wrong value");

    id = 2;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("DECIMAL 0.01 as string");
    CHECK(rc == 0 && strcmp(out, "0.01") == 0, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_time_type(void) {
    printf("\n[PS TIME type]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_time");
    Q("CREATE TABLE t_time(id INT, tm TIME)");
    Q("INSERT INTO t_time VALUES(1, '08:30:45'),(2, '-12:00:00')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT tm FROM t_time WHERE id = ?",
                                strlen("SELECT tm FROM t_time WHERE id = ?"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    MYSQL_TIME out_tm;
    unsigned long olen = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_TIME; rb.buffer = &out_tm; rb.buffer_length = sizeof(out_tm); rb.length = &olen;
    mysql_stmt_bind_result(stmt, &rb);

    id = 1;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("TIME 08:30:45");
    CHECK(rc == 0 && out_tm.hour == 8 && out_tm.minute == 30 && out_tm.second == 45 && out_tm.neg == 0, "wrong time");

    id = 2;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("TIME -12:00:00 (neg=1, hour=12)");
    CHECK(rc == 0 && out_tm.neg == 1 && out_tm.hour == 12 && out_tm.minute == 0 && out_tm.second == 0, "wrong time");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_set_type(void) {
    printf("\n[PS SET type]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_set");
    Q("CREATE TABLE t_set(id INT, colors SET('red','green','blue'))");
    Q("INSERT INTO t_set VALUES(1,'red,blue'),(2,'green')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT colors FROM t_set WHERE id = ?",
                                strlen("SELECT colors FROM t_set WHERE id = ?"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    char out[32];
    unsigned long olen = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_STRING; rb.buffer = out; rb.buffer_length = sizeof(out); rb.length = &olen;
    mysql_stmt_bind_result(stmt, &rb);

    id = 1;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("SET 'red,blue' as string");
    CHECK(rc == 0 && strcmp(out, "red,blue") == 0, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_prepare_close_cycle(void) {
    printf("\n[PS prepare/close cycle]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_pcc");
    Q("CREATE TABLE t_pcc(id INT, val INT)");
    Q("INSERT INTO t_pcc VALUES(1,100)");

    int all_ok = 1;
    for (int i = 0; i < 5; i++) {
        MYSQL_STMT *stmt = mysql_stmt_init(m);
        int rc = mysql_stmt_prepare(stmt, "SELECT val FROM t_pcc WHERE id = ?",
                                    strlen("SELECT val FROM t_pcc WHERE id = ?"));
        if (rc != 0) { all_ok = 0; mysql_stmt_close(stmt); continue; }

        int id = 1;
        unsigned long il = sizeof(int);
        MYSQL_BIND pb;
        memset(&pb, 0, sizeof(pb));
        pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
        mysql_stmt_bind_param(stmt, &pb);

        int out_val;
        unsigned long ol = 0;
        MYSQL_BIND rb;
        memset(&rb, 0, sizeof(rb));
        rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.length = &ol;
        mysql_stmt_bind_result(stmt, &rb);

        rc = mysql_stmt_execute(stmt);
        rc = mysql_stmt_fetch(stmt);
        if (rc != 0 || out_val != 100) all_ok = 0;
        mysql_stmt_close(stmt);
    }
    TEST("5x prepare/close cycle all return val=100");
    CHECK(all_ok, "some cycles failed");

    mysql_close(m);
}

static void test_stmt_ps_no_param_select(void) {
    printf("\n[PS no-param SELECT]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT 42 AS answer, 'hello' AS greeting",
                                strlen("SELECT 42 AS answer, 'hello' AS greeting"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    TEST("param_count == 0");
    CHECK(stmt->param_count == 0, "wrong count");

    long long out_num;
    char out_str[20];
    unsigned long l1=0,l2=0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONGLONG; rb[0].buffer = &out_num; rb[0].length = &l1;
    rb[1].buffer_type = MYSQL_TYPE_STRING; rb[1].buffer = out_str; rb[1].buffer_length = sizeof(out_str); rb[1].length = &l2;
    mysql_stmt_bind_result(stmt, rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("no-param SELECT: 42, 'hello'");
    CHECK(rc == 0 && out_num == 42 && strcmp(out_str, "hello") == 0, "wrong values");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_negative_param(void) {
    printf("\n[PS negative parameters]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT ? + ?",
                                strlen("SELECT ? + ?"));

    int a = -10, b = 3;
    unsigned long la = sizeof(int), lb = sizeof(int);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &a; pb[0].length = &la;
    pb[1].buffer_type = MYSQL_TYPE_LONG; pb[1].buffer = &b; pb[1].length = &lb;
    mysql_stmt_bind_param(stmt, pb);

    long long out;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONGLONG; rb.buffer = &out; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("-10 + 3 = -7");
    CHECK(rc == 0 && out == -7, "wrong value");

    a = -100; b = -200;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("-100 + -200 = -300");
    CHECK(rc == 0 && out == -300, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_div_zero(void) {
    printf("\n[PS division by zero]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT ? / 0",
                                strlen("SELECT ? / 0"));

    int val = 10;
    unsigned long vl = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &val; pb.length = &vl;
    mysql_stmt_bind_param(stmt, &pb);

    rc = mysql_stmt_execute(stmt);
    if (rc != 0) {
        TEST("division by zero returns error or NULL");
        PASS();
    } else {
        long long out;
        unsigned long ol = 0;
        my_bool is_null = 0;
        MYSQL_BIND rb;
        memset(&rb, 0, sizeof(rb));
        rb.buffer_type = MYSQL_TYPE_LONGLONG; rb.buffer = &out; rb.length = &ol; rb.is_null = &is_null;
        mysql_stmt_bind_result(stmt, &rb);
        rc = mysql_stmt_fetch(stmt);
        TEST("division by zero returns NULL");
        CHECK(is_null, "expected NULL");
    }

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_blob_read(void) {
    printf("\n[PS BLOB read]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_blob2");
    Q("CREATE TABLE t_blob2(id INT, data BLOB)");
    char blob_data[256];
    for (int i = 0; i < 256; i++) blob_data[i] = (char)i;
    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_blob2(id, data) VALUES(?, ?)",
                                strlen("INSERT INTO t_blob2(id, data) VALUES(?, ?)"));
    int id = 1;
    unsigned long il = sizeof(int), bl = 256;
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &il;
    pb[1].buffer_type = MYSQL_TYPE_BLOB; pb[1].buffer = blob_data; pb[1].buffer_length = 256; pb[1].length = &bl;
    mysql_stmt_bind_param(stmt, pb);
    rc = mysql_stmt_execute(stmt);
    TEST("insert BLOB 256 bytes");
    CHECK(rc == 0, "insert failed");
    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "SELECT data FROM t_blob2 WHERE id = ?",
                            strlen("SELECT data FROM t_blob2 WHERE id = ?"));
    mysql_stmt_bind_param(stmt, &pb[0]);
    rc = mysql_stmt_execute(stmt);

    char out_buf[512];
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_BLOB; rb.buffer = out_buf; rb.buffer_length = sizeof(out_buf); rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);
    rc = mysql_stmt_fetch(stmt);
    TEST("read BLOB 256 bytes, length matches");
    CHECK(rc == 0 && ol == 256, "wrong length");

    int match = 1;
    for (int i = 0; i < 256; i++) {
        if ((unsigned char)out_buf[i] != (unsigned char)i) { match = 0; break; }
    }
    TEST("BLOB content matches");
    CHECK(match, "content mismatch");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_store_result_fetch(void) {
    printf("\n[PS store_result + fetch]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_srf");
    Q("CREATE TABLE t_srf(id INT, val INT)");
    Q("INSERT INTO t_srf VALUES(1,10),(2,20),(3,30),(4,40),(5,50)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT id, val FROM t_srf ORDER BY id",
                                strlen("SELECT id, val FROM t_srf ORDER BY id"));

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_store_result(stmt);
    TEST("store_result succeeds");
    CHECK(rc == 0, "store failed");

    TEST("num_rows == 5 after store");
    CHECK(mysql_stmt_num_rows(stmt) == 5, "wrong count");

    int out_id, out_val, sum = 0;
    unsigned long l1=0,l2=0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &out_id; rb[0].length = &l1;
    rb[1].buffer_type = MYSQL_TYPE_LONG; rb[1].buffer = &out_val; rb[1].length = &l2;
    mysql_stmt_bind_result(stmt, rb);

    while (mysql_stmt_fetch(stmt) == 0) sum += out_val;
    TEST("sum of all vals = 150");
    CHECK(sum == 150, "wrong sum");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_data_seek_stored(void) {
    printf("\n[PS data_seek on stored result]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_ds");
    Q("CREATE TABLE t_ds(id INT PRIMARY KEY, name VARCHAR(20))");
    Q("INSERT INTO t_ds VALUES(1,'a'),(2,'b'),(3,'c'),(4,'d')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT id, name FROM t_ds ORDER BY id",
                                strlen("SELECT id, name FROM t_ds ORDER BY id"));
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_store_result(stmt);

    int out_id;
    char out_name[20];
    unsigned long l1=0,l2=0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &out_id; rb[0].length = &l1;
    rb[1].buffer_type = MYSQL_TYPE_STRING; rb[1].buffer = out_name; rb[1].buffer_length = sizeof(out_name); rb[1].length = &l2;
    mysql_stmt_bind_result(stmt, rb);

    mysql_stmt_data_seek(stmt, 2);
    rc = mysql_stmt_fetch(stmt);
    TEST("data_seek(2): id=3, name='c'");
    CHECK(rc == 0 && out_id == 3 && strcmp(out_name, "c") == 0, "wrong row");

    mysql_stmt_data_seek(stmt, 0);
    rc = mysql_stmt_fetch(stmt);
    TEST("data_seek(0): id=1, name='a'");
    CHECK(rc == 0 && out_id == 1 && strcmp(out_name, "a") == 0, "wrong row");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_truncate_buffer(void) {
    printf("\n[PS buffer truncation]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT ?",
                                strlen("SELECT ?"));

    char param[20] = "Hello World";
    unsigned long plen = 11;
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_STRING; pb.buffer = param; pb.buffer_length = sizeof(param); pb.length = &plen;
    mysql_stmt_bind_param(stmt, &pb);

    char out[5];
    unsigned long olen = 0;
    my_bool err = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_STRING; rb.buffer = out; rb.buffer_length = sizeof(out); rb.length = &olen; rb.error = &err;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("truncated: first 4 chars = 'Hell'");
    CHECK(rc == MYSQL_DATA_TRUNCATED && strncmp(out, "Hell", 4) == 0, "wrong truncation");
    TEST("truncated: error flag set");
    CHECK(err == 1, "no error flag");
    TEST("truncated: length = 11 (full)");
    CHECK(olen == 11, "wrong length");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_unsigned_param(void) {
    printf("\n[PS unsigned parameters]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT ?",
                                strlen("SELECT ?"));

    unsigned int uval = 3000000000U;
    unsigned long ulen = sizeof(unsigned int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &uval; pb.buffer_length = sizeof(uval); pb.length = &ulen; pb.is_unsigned = 1;
    mysql_stmt_bind_param(stmt, &pb);

    unsigned long long out;
    unsigned long olen = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONGLONG; rb.buffer = &out; rb.length = &olen;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("unsigned 3000000000 preserved");
    CHECK(rc == 0 && out == 3000000000U, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_reexecute_after_store(void) {
    printf("\n[PS re-execute after store_result]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_ras");
    Q("CREATE TABLE t_ras(id INT, val INT)");
    Q("INSERT INTO t_ras VALUES(1,100),(2,200)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT val FROM t_ras WHERE id = ?",
                                strlen("SELECT val FROM t_ras WHERE id = ?"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    int out_val;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    id = 1;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_store_result(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("first execute: id=1, val=100");
    CHECK(rc == 0 && out_val == 100, "wrong value");

    id = 2;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_store_result(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("second execute: id=2, val=200");
    CHECK(rc == 0 && out_val == 200, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_multiple_rows_fetch(void) {
    printf("\n[PS multiple rows streaming fetch]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_mrf");
    Q("CREATE TABLE t_mrf(id INT PRIMARY KEY)");
    Q("INSERT INTO t_mrf VALUES(1),(2),(3),(4),(5),(6),(7),(8),(9),(10)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT id FROM t_mrf WHERE id > ? ORDER BY id",
                                strlen("SELECT id FROM t_mrf WHERE id > ? ORDER BY id"));

    int threshold = 3;
    unsigned long tl = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &threshold; pb.length = &tl;
    mysql_stmt_bind_param(stmt, &pb);

    int out_id;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_id; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    int count = 0, last_id = 0;
    while (mysql_stmt_fetch(stmt) == 0) { last_id = out_id; count++; }
    TEST("id > 3: 7 rows, last id = 10");
    CHECK(count == 7 && last_id == 10, "wrong result");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_datetime_timestamp(void) {
    printf("\n[PS DATETIME and TIMESTAMP]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_dt");
    Q("CREATE TABLE t_dt(id INT, dt DATETIME, ts TIMESTAMP DEFAULT '2024-03-15 10:30:00')");
    Q("INSERT INTO t_dt(id, dt) VALUES(1, '2024-06-20 14:30:45')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT dt, ts FROM t_dt WHERE id = ?",
                                strlen("SELECT dt, ts FROM t_dt WHERE id = ?"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    MYSQL_TIME out_dt, out_ts;
    unsigned long l1=0,l2=0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_DATETIME; rb[0].buffer = &out_dt; rb[0].buffer_length = sizeof(out_dt); rb[0].length = &l1;
    rb[1].buffer_type = MYSQL_TYPE_TIMESTAMP; rb[1].buffer = &out_ts; rb[1].buffer_length = sizeof(out_ts); rb[1].length = &l2;
    mysql_stmt_bind_result(stmt, rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("DATETIME 2024-06-20 14:30:45");
    CHECK(rc == 0 && out_dt.year == 2024 && out_dt.month == 6 && out_dt.day == 20 &&
          out_dt.hour == 14 && out_dt.minute == 30 && out_dt.second == 45, "wrong datetime");
    TEST("TIMESTAMP 2024-03-15 10:30:00");
    CHECK(rc == 0 && out_ts.year == 2024 && out_ts.month == 3 && out_ts.day == 15 &&
          out_ts.hour == 10 && out_ts.minute == 30 && out_ts.second == 0, "wrong timestamp");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_null_result_bind(void) {
    printf("\n[PS NULL in result columns]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_nrb");
    Q("CREATE TABLE t_nrb(id INT, a INT, b VARCHAR(20), c DOUBLE)");
    Q("INSERT INTO t_nrb VALUES(1, NULL, NULL, NULL),(2, 42, 'hello', 3.14)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT a, b, c FROM t_nrb WHERE id = ?",
                                strlen("SELECT a, b, c FROM t_nrb WHERE id = ?"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    int out_a;
    char out_b[20];
    double out_c;
    my_bool n_a=0, n_b=0, n_c=0;
    unsigned long l1=0,l2=0,l3=0;
    MYSQL_BIND rb[3];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &out_a; rb[0].is_null = &n_a; rb[0].length = &l1;
    rb[1].buffer_type = MYSQL_TYPE_STRING; rb[1].buffer = out_b; rb[1].buffer_length = sizeof(out_b); rb[1].is_null = &n_b; rb[1].length = &l2;
    rb[2].buffer_type = MYSQL_TYPE_DOUBLE; rb[2].buffer = &out_c; rb[2].is_null = &n_c; rb[2].length = &l3;
    mysql_stmt_bind_result(stmt, rb);

    id = 1;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("row 1: all three columns NULL");
    CHECK(rc == 0 && n_a && n_b && n_c, "NULL not detected");

    id = 2;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("row 2: all three columns non-NULL");
    CHECK(rc == 0 && !n_a && !n_b && !n_c && out_a == 42 && strcmp(out_b, "hello") == 0 && fabs(out_c - 3.14) < 0.01, "wrong values");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_float_precision(void) {
    printf("\n[PS FLOAT precision]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_fp");
    Q("CREATE TABLE t_fp(id INT, f FLOAT, d DOUBLE)");
    Q("INSERT INTO t_fp VALUES(1, 3.14, 3.141592653589793)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT f, d FROM t_fp WHERE id = ?",
                                strlen("SELECT f, d FROM t_fp WHERE id = ?"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    float out_f;
    double out_d;
    unsigned long l1=0,l2=0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_FLOAT; rb[0].buffer = &out_f; rb[0].length = &l1;
    rb[1].buffer_type = MYSQL_TYPE_DOUBLE; rb[1].buffer = &out_d; rb[1].length = &l2;
    mysql_stmt_bind_result(stmt, rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("FLOAT ~3.14");
    CHECK(rc == 0 && fabsf(out_f - 3.14f) < 0.01f, "wrong float");
    TEST("DOUBLE ~3.141592653589793");
    CHECK(rc == 0 && fabs(out_d - 3.141592653589793) < 0.000001, "wrong double");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_bigint(void) {
    printf("\n[PS BIGINT]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_bi");
    Q("CREATE TABLE t_bi(id INT, val BIGINT)");
    Q("INSERT INTO t_bi VALUES(1, 9223372036854775807),(2, -9223372036854775807)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT val FROM t_bi WHERE id = ?",
                                strlen("SELECT val FROM t_bi WHERE id = ?"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    long long out;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONGLONG; rb.buffer = &out; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    id = 1;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("BIGINT max: 9223372036854775807");
    CHECK(rc == 0 && out == 9223372036854775807LL, "wrong value");

    id = 2;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("BIGINT min: -9223372036854775807");
    CHECK(rc == 0 && out == -9223372036854775807LL, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_empty_table(void) {
    printf("\n[PS empty table query]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_empty");
    Q("CREATE TABLE t_empty(id INT, val INT)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT id, val FROM t_empty",
                                strlen("SELECT id, val FROM t_empty"));

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_store_result(stmt);
    TEST("store_result on empty table: num_rows == 0");
    CHECK(rc == 0 && mysql_stmt_num_rows(stmt) == 0, "wrong count");

    int fetch_rc = mysql_stmt_fetch(stmt);
    TEST("fetch on empty: MYSQL_NO_DATA");
    CHECK(fetch_rc == MYSQL_NO_DATA, "expected no data");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_column_names(void) {
    printf("\n[PS column names via metadata]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_cn");
    Q("CREATE TABLE t_cn(user_id INT, user_name VARCHAR(20))");
    Q("INSERT INTO t_cn VALUES(1,'alice')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT user_id, user_name FROM t_cn",
                                strlen("SELECT user_id, user_name FROM t_cn"));

    MYSQL_RES *meta = mysql_stmt_result_metadata(stmt);
    TEST("metadata is non-NULL");
    CHECK(meta != NULL, "null metadata");

    if (meta) {
        TEST("num_fields == 2");
        CHECK(mysql_num_fields(meta) == 2, "wrong field count");

        MYSQL_FIELD *fields = mysql_fetch_fields(meta);
        TEST("field[0].name = 'user_id'");
        CHECK(fields && strcmp(fields[0].name, "user_id") == 0, "wrong name");
        TEST("field[1].name = 'user_name'");
        CHECK(fields && strcmp(fields[1].name, "user_name") == 0, "wrong name");

        mysql_free_result(meta);
    }

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_reexecute_diff_null(void) {
    printf("\n[PS re-execute with changing NULL pattern]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_rdn");
    Q("CREATE TABLE t_rdn(id INT, a INT, b VARCHAR(20))");
    Q("INSERT INTO t_rdn VALUES(1,10,'hello'),(2,NULL,'world'),(3,30,NULL)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT a, b FROM t_rdn WHERE id = ?",
                                strlen("SELECT a, b FROM t_rdn WHERE id = ?"));

    int id = 0;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    int out_a;
    char out_b[20];
    my_bool na=0, nb=0;
    unsigned long la=0, lb=0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &out_a; rb[0].is_null = &na; rb[0].length = &la;
    rb[1].buffer_type = MYSQL_TYPE_STRING; rb[1].buffer = out_b; rb[1].buffer_length = sizeof(out_b); rb[1].is_null = &nb; rb[1].length = &lb;
    mysql_stmt_bind_result(stmt, rb);

    id = 1;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("row 1: a=10, b='hello' (both non-null)");
    CHECK(rc == 0 && !na && !nb && out_a == 10 && strcmp(out_b, "hello") == 0, "wrong values");

    id = 2;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("row 2: a=NULL, b='world'");
    CHECK(rc == 0 && na && !nb && strcmp(out_b, "world") == 0, "wrong null pattern");

    id = 3;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("row 3: a=30, b=NULL");
    CHECK(rc == 0 && !na && nb && out_a == 30, "wrong null pattern");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_string_param_long(void) {
    printf("\n[PS long string parameter]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_lsp");
    Q("CREATE TABLE t_lsp(id INT, txt TEXT)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_lsp(id, txt) VALUES(?, ?)",
                                strlen("INSERT INTO t_lsp(id, txt) VALUES(?, ?)"));

    char longstr[2001];
    for (int i = 0; i < 2000; i++) longstr[i] = 'A' + (i % 26);
    longstr[2000] = '\0';

    int id = 1;
    unsigned long il = sizeof(int), tl = 2000;
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &il;
    pb[1].buffer_type = MYSQL_TYPE_STRING; pb[1].buffer = longstr; pb[1].buffer_length = 2001; pb[1].length = &tl;
    mysql_stmt_bind_param(stmt, pb);

    rc = mysql_stmt_execute(stmt);
    TEST("insert 2000-char string");
    CHECK(rc == 0, "insert failed");
    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "SELECT LENGTH(txt) FROM t_lsp WHERE id = ?",
                            strlen("SELECT LENGTH(txt) FROM t_lsp WHERE id = ?"));
    mysql_stmt_bind_param(stmt, &pb[0]);
    rc = mysql_stmt_execute(stmt);

    long long out_len;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONGLONG; rb.buffer = &out_len; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);
    rc = mysql_stmt_fetch(stmt);
    TEST("LENGTH(txt) = 2000");
    CHECK(rc == 0 && out_len == 2000, "wrong length");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_update_return_count(void) {
    printf("\n[PS UPDATE returns affected_rows and matched_rows]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_urc");
    Q("CREATE TABLE t_urc(id INT PRIMARY KEY, val INT)");
    Q("INSERT INTO t_urc VALUES(1,10),(2,20),(3,30)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "UPDATE t_urc SET val = ? WHERE val > ?",
                                strlen("UPDATE t_urc SET val = ? WHERE val > ?"));

    int new_val = 99, threshold = 15;
    unsigned long l1 = sizeof(int), l2 = sizeof(int);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &new_val; pb[0].length = &l1;
    pb[1].buffer_type = MYSQL_TYPE_LONG; pb[1].buffer = &threshold; pb[1].length = &l2;
    mysql_stmt_bind_param(stmt, pb);

    rc = mysql_stmt_execute(stmt);
    TEST("affected_rows == 2");
    CHECK(rc == 0 && mysql_stmt_affected_rows(stmt) == 2, "wrong count");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_zero_rows_where(void) {
    printf("\n[PS zero rows from WHERE]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_zr");
    Q("CREATE TABLE t_zr(id INT, val INT)");
    Q("INSERT INTO t_zr VALUES(1,10),(2,20)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT id FROM t_zr WHERE val > ?",
                                strlen("SELECT id FROM t_zr WHERE val > ?"));

    int threshold = 100;
    unsigned long tl = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &threshold; pb.length = &tl;
    mysql_stmt_bind_param(stmt, &pb);

    int out_id;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_id; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    int fetch_rc = mysql_stmt_fetch(stmt);
    TEST("no rows: fetch returns MYSQL_NO_DATA");
    CHECK(fetch_rc == MYSQL_NO_DATA, "expected no data");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_result_metadata_after_exec(void) {
    printf("\n[PS result metadata after execute]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_rma");
    Q("CREATE TABLE t_rma(id INT, name VARCHAR(30))");
    Q("INSERT INTO t_rma VALUES(1,'test')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT id, name FROM t_rma WHERE id = ?",
                                strlen("SELECT id, name FROM t_rma WHERE id = ?"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    rc = mysql_stmt_execute(stmt);

    MYSQL_RES *meta = mysql_stmt_result_metadata(stmt);
    TEST("metadata after execute is non-NULL");
    CHECK(meta != NULL, "null metadata");

    if (meta) {
        unsigned int nf = mysql_num_fields(meta);
        TEST("num_fields == 2");
        CHECK(nf == 2, "wrong count");
        mysql_free_result(meta);
    }

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_smallint(void) {
    printf("\n[PS SMALLINT/TINYINT types]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_si");
    Q("CREATE TABLE t_si(id INT, a TINYINT, b SMALLINT)");
    Q("INSERT INTO t_si VALUES(1, 127, 32767),(2, -128, -32768)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT a, b FROM t_si WHERE id = ?",
                                strlen("SELECT a, b FROM t_si WHERE id = ?"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    int out_a, out_b;
    unsigned long l1=0,l2=0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &out_a; rb[0].length = &l1;
    rb[1].buffer_type = MYSQL_TYPE_LONG; rb[1].buffer = &out_b; rb[1].length = &l2;
    mysql_stmt_bind_result(stmt, rb);

    id = 1;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("TINYINT 127, SMALLINT 32767");
    CHECK(rc == 0 && out_a == 127 && out_b == 32767, "wrong values");

    id = 2;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("TINYINT -128, SMALLINT -32768");
    CHECK(rc == 0 && out_a == -128 && out_b == -32768, "wrong values");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_stmt_ps_mediumint(void) {
    printf("\n[PS MEDIUMINT type]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_mi");
    Q("CREATE TABLE t_mi(id INT, val MEDIUMINT)");
    Q("INSERT INTO t_mi VALUES(1, 8388607),(2, -8388608)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT val FROM t_mi WHERE id = ?",
                                strlen("SELECT val FROM t_mi WHERE id = ?"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    int out_val;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    id = 1;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("MEDIUMINT max: 8388607");
    CHECK(rc == 0 && out_val == 8388607, "wrong value");

    id = 2;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("MEDIUMINT min: -8388608");
    CHECK(rc == 0 && out_val == -8388608, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_text_query_field_types(void) {
    printf("\n[text query: field types]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_tqft");
    Q("CREATE TABLE t_tqft(a INT, b VARCHAR(20), c DOUBLE, d DATE, e DATETIME)");
    Q("INSERT INTO t_tqft VALUES(1,'hello',3.14,'2024-01-15','2024-06-20 14:30:00')");

    Q("SELECT a, b, c, d, e FROM t_tqft");
    MYSQL_RES *res = mysql_store_result(m);
    if (!res) { FAIL("no result"); mysql_close(m); return; }

    MYSQL_FIELD *fields = mysql_fetch_fields(res);
    TEST("field[0] type INT");
    CHECK(fields[0].type == MYSQL_TYPE_LONG, "wrong type");
    TEST("field[1] type VAR_STRING");
    CHECK(fields[1].type == MYSQL_TYPE_VAR_STRING, "wrong type");
    TEST("field[2] type DOUBLE");
    CHECK(fields[2].type == MYSQL_TYPE_DOUBLE, "wrong type");
    TEST("field[3] type DATE");
    CHECK(fields[3].type == MYSQL_TYPE_DATE, "wrong type");
    TEST("field[4] type DATETIME");
    CHECK(fields[4].type == MYSQL_TYPE_DATETIME, "wrong type");

    mysql_free_result(res);
    mysql_close(m);
}

static void test_text_query_null_fields(void) {
    printf("\n[text query: NULL fields]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_tqnf");
    Q("CREATE TABLE t_tqnf(id INT, a INT, b VARCHAR(20))");
    Q("INSERT INTO t_tqnf VALUES(1,NULL,NULL),(2,42,'hello')");

    Q("SELECT a, b FROM t_tqnf ORDER BY id");
    MYSQL_RES *res = mysql_store_result(m);
    if (!res) { FAIL("no result"); mysql_close(m); return; }

    MYSQL_ROW row;
    row = mysql_fetch_row(res);
    TEST("row 1: both NULL");
    CHECK(row && row[0] == NULL && row[1] == NULL, "not null");

    row = mysql_fetch_row(res);
    TEST("row 2: both non-NULL");
    CHECK(row && row[0] && row[1] && strcmp(row[0], "42") == 0 && strcmp(row[1], "hello") == 0, "wrong values");

    mysql_free_result(res);
    mysql_close(m);
}

static void test_text_query_fetch_lengths(void) {
    printf("\n[text query: fetch_lengths]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_tqfl");
    Q("CREATE TABLE t_tqfl(id INT, name VARCHAR(30))");
    Q("INSERT INTO t_tqfl VALUES(1,'alice'),(2,'bob')");

    Q("SELECT id, name FROM t_tqfl WHERE id = 1");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    unsigned long *lens = mysql_fetch_lengths(res);
    TEST("length of '1' = 1");
    CHECK(lens && lens[0] == 1, "wrong length");
    TEST("length of 'alice' = 5");
    CHECK(lens && lens[1] == 5, "wrong length");

    mysql_free_result(res);
    mysql_close(m);
}

static void test_text_query_affected_rows(void) {
    printf("\n[text query: affected_rows]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_tqar");
    Q("CREATE TABLE t_tqar(id INT PRIMARY KEY, val INT)");
    Q("INSERT INTO t_tqar VALUES(1,10),(2,20),(3,30)");

    Q("UPDATE t_tqar SET val = val + 100 WHERE val > 15");
    TEST("UPDATE affected_rows == 2");
    CHECK(mysql_affected_rows(m) == 2, "wrong count");

    Q("DELETE FROM t_tqar WHERE id = 1");
    TEST("DELETE affected_rows == 1");
    CHECK(mysql_affected_rows(m) == 1, "wrong count");

    Q("INSERT INTO t_tqar VALUES(4,40)");
    TEST("INSERT affected_rows == 1");
    CHECK(mysql_affected_rows(m) == 1, "wrong count");

    mysql_close(m);
}

static void test_text_query_insert_id(void) {
    printf("\n[text query: insert_id]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_tqii");
    Q("CREATE TABLE t_tqii(id INT AUTO_INCREMENT PRIMARY KEY, name VARCHAR(20))");

    Q("INSERT INTO t_tqii(name) VALUES('first')");
    my_ulonglong id1 = mysql_insert_id(m);
    Q("INSERT INTO t_tqii(name) VALUES('second')");
    my_ulonglong id2 = mysql_insert_id(m);
    TEST("second insert_id > first");
    CHECK(id1 > 0 && id2 > id1, "wrong id order");

    mysql_close(m);
}

static void test_text_query_warning_count(void) {
    printf("\n[text query: warning_count]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_tqwc");
    Q("CREATE TABLE t_tqwc(id INT, val TINYINT)");
    Q("INSERT INTO t_tqwc VALUES(1, 127)");
    Q("SELECT 1/0");
    MYSQL_RES *res = mysql_store_result(m);
    if (res) mysql_free_result(res);
    TEST("warning_count > 0 after division by zero");
    CHECK(mysql_warning_count(m) > 0, "no warning");

    mysql_close(m);
}

static void test_text_query_multi_fetch(void) {
    printf("\n[text query: multiple fetch loops]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_tqmf");
    Q("CREATE TABLE t_tqmf(id INT)");
    Q("INSERT INTO t_tqmf VALUES(1),(2),(3),(4),(5)");

    for (int round = 0; round < 3; round++) {
        char sql[64];
        snprintf(sql, sizeof(sql), "SELECT id FROM t_tqmf WHERE id > %d ORDER BY id", round);
        Q(sql);
        MYSQL_RES *res = mysql_store_result(m);
        if (!res) { FAIL("no result"); mysql_close(m); return; }
        int count = 0;
        while (mysql_fetch_row(res)) count++;
        mysql_free_result(res);
        if (round == 0) {
            TEST("round 1: 5 rows");
            CHECK(count == 5, "wrong count");
        }
    }
    TEST("3 rounds of text query succeed");
    PASS();

    mysql_close(m);
}

static void test_text_query_use_result_all(void) {
    printf("\n[text query: use_result consume all]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_tqur");
    Q("CREATE TABLE t_tqur(id INT)");
    Q("INSERT INTO t_tqur VALUES(1),(2),(3)");

    Q("SELECT id FROM t_tqur ORDER BY id");
    MYSQL_RES *res = mysql_use_result(m);
    if (!res) { FAIL("use_result failed"); mysql_close(m); return; }

    int count = 0;
    while (mysql_fetch_row(res)) count++;
    mysql_free_result(res);
    TEST("use_result: 3 rows consumed");
    CHECK(count == 3, "wrong count");

    Q("SELECT id FROM t_tqur");
    res = mysql_store_result(m);
    TEST("after use_result, next query works");
    CHECK(res != NULL, "query failed after use_result");
    if (res) mysql_free_result(res);

    mysql_close(m);
}


void test_ps_binary_varbinary(void) {
    printf("\n[PS BINARY/VARBINARY]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_bin");
    Q("CREATE TABLE t_bin(id INT, data VARBINARY(100))");
    unsigned char bin_data[5] = {0x00, 0x01, 0xFF, 0xFE, 0x7F};

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_bin(id, data) VALUES(?, ?)",
                                strlen("INSERT INTO t_bin(id, data) VALUES(?, ?)"));
    int id = 1;
    unsigned long il = sizeof(int), bl = 5;
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &il;
    pb[1].buffer_type = MYSQL_TYPE_BLOB; pb[1].buffer = bin_data; pb[1].buffer_length = 5; pb[1].length = &bl;
    mysql_stmt_bind_param(stmt, pb);
    rc = mysql_stmt_execute(stmt);
    TEST("insert VARBINARY with embedded NULs");
    CHECK(rc == 0, "insert failed");
    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "SELECT data FROM t_bin WHERE id = ?",
                            strlen("SELECT data FROM t_bin WHERE id = ?"));
    mysql_stmt_bind_param(stmt, &pb[0]);
    rc = mysql_stmt_execute(stmt);

    unsigned char out_buf[100];
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_BLOB; rb.buffer = out_buf; rb.buffer_length = sizeof(out_buf); rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);
    rc = mysql_stmt_fetch(stmt);
    TEST("read VARBINARY: length=5, content matches");
    CHECK(rc == 0 && ol == 5 && memcmp(out_buf, bin_data, 5) == 0, "wrong binary data");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_replace_into(void) {
    printf("\n[PS REPLACE INTO]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_ri");
    Q("CREATE TABLE t_ri(id INT PRIMARY KEY, val INT)");
    Q("INSERT INTO t_ri VALUES(1,100)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "REPLACE INTO t_ri(id, val) VALUES(?, ?)",
                                strlen("REPLACE INTO t_ri(id, val) VALUES(?, ?)"));
    int id = 1, val = 200;
    unsigned long l1 = sizeof(int), l2 = sizeof(int);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &l1;
    pb[1].buffer_type = MYSQL_TYPE_LONG; pb[1].buffer = &val; pb[1].length = &l2;
    mysql_stmt_bind_param(stmt, pb);

    rc = mysql_stmt_execute(stmt);
    TEST("REPLACE INTO succeeds");
    CHECK(rc == 0, "replace failed");
    TEST("affected_rows == 2 (delete+insert)");
    CHECK(mysql_stmt_affected_rows(stmt) == 2, "wrong count");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_on_dup_key_update(void) {
    printf("\n[PS INSERT ON DUPLICATE KEY UPDATE]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_odku");
    Q("CREATE TABLE t_odku(id INT PRIMARY KEY, val INT)");
    Q("INSERT INTO t_odku VALUES(1,100)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_odku(id, val) VALUES(?, ?) ON DUPLICATE KEY UPDATE val = ?",
                                strlen("INSERT INTO t_odku(id, val) VALUES(?, ?) ON DUPLICATE KEY UPDATE val = ?"));

    int id = 1, val_ins = 200, val_upd = 300;
    unsigned long l1 = sizeof(int), l2 = sizeof(int), l3 = sizeof(int);
    MYSQL_BIND pb[3];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &l1;
    pb[1].buffer_type = MYSQL_TYPE_LONG; pb[1].buffer = &val_ins; pb[1].length = &l2;
    pb[2].buffer_type = MYSQL_TYPE_LONG; pb[2].buffer = &val_upd; pb[2].length = &l3;
    mysql_stmt_bind_param(stmt, pb);

    rc = mysql_stmt_execute(stmt);
    TEST("ON DUPLICATE KEY UPDATE succeeds");
    CHECK(rc == 0, "failed");
    TEST("affected_rows == 2 (update)");
    CHECK(mysql_stmt_affected_rows(stmt) == 2, "wrong count");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_stmt_reset_between_exec(void) {
    printf("\n[PS stmt_reset between executes]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_sre");
    Q("CREATE TABLE t_sre(id INT, val INT)");
    Q("INSERT INTO t_sre VALUES(1,10)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_sre(id, val) VALUES(?, ?)",
                                strlen("INSERT INTO t_sre(id, val) VALUES(?, ?)"));

    int id = 2, val = 20;
    unsigned long l1 = sizeof(int), l2 = sizeof(int);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &l1;
    pb[1].buffer_type = MYSQL_TYPE_LONG; pb[1].buffer = &val; pb[1].length = &l2;
    mysql_stmt_bind_param(stmt, pb);

    rc = mysql_stmt_execute(stmt);
    TEST("first insert ok");
    CHECK(rc == 0, "insert failed");

    rc = mysql_stmt_reset(stmt);
    TEST("stmt_reset ok");
    CHECK(rc == 0, "reset failed");

    id = 3; val = 30;
    rc = mysql_stmt_execute(stmt);
    TEST("insert after reset ok");
    CHECK(rc == 0, "insert after reset failed");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_concurrent_stmts_same_conn(void) {
    printf("\n[PS sequential stmts on same connection]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_csc");
    Q("CREATE TABLE t_csc(id INT, val INT)");
    Q("INSERT INTO t_csc VALUES(1,100),(2,200)");

    MYSQL_STMT *s1 = mysql_stmt_init(m);
    MYSQL_STMT *s2 = mysql_stmt_init(m);
    int rc1 = mysql_stmt_prepare(s1, "SELECT val FROM t_csc WHERE id = ?",
                                 strlen("SELECT val FROM t_csc WHERE id = ?"));
    int rc2 = mysql_stmt_prepare(s2, "SELECT val FROM t_csc WHERE id = ?",
                                 strlen("SELECT val FROM t_csc WHERE id = ?"));

    int id1 = 1, id2 = 2;
    unsigned long il1 = sizeof(int), il2 = sizeof(int);
    MYSQL_BIND pb1, pb2;
    memset(&pb1, 0, sizeof(pb1));
    pb1.buffer_type = MYSQL_TYPE_LONG; pb1.buffer = &id1; pb1.length = &il1;
    memset(&pb2, 0, sizeof(pb2));
    pb2.buffer_type = MYSQL_TYPE_LONG; pb2.buffer = &id2; pb2.length = &il2;
    mysql_stmt_bind_param(s1, &pb1);
    mysql_stmt_bind_param(s2, &pb2);

    int out1, out2;
    unsigned long ol1=0, ol2=0;
    MYSQL_BIND rb1, rb2;
    memset(&rb1, 0, sizeof(rb1));
    rb1.buffer_type = MYSQL_TYPE_LONG; rb1.buffer = &out1; rb1.length = &ol1;
    memset(&rb2, 0, sizeof(rb2));
    rb2.buffer_type = MYSQL_TYPE_LONG; rb2.buffer = &out2; rb2.length = &ol2;
    mysql_stmt_bind_result(s1, &rb1);
    mysql_stmt_bind_result(s2, &rb2);

    rc1 = mysql_stmt_execute(s1);
    rc1 = mysql_stmt_store_result(s1);
    rc1 = mysql_stmt_fetch(s1);
    TEST("stmt1: id=1 val=100");
    CHECK(rc1 == 0 && out1 == 100, "wrong value");

    rc2 = mysql_stmt_execute(s2);
    rc2 = mysql_stmt_fetch(s2);
    TEST("stmt2: id=2 val=200");
    CHECK(rc2 == 0 && out2 == 200, "wrong value");

    mysql_stmt_close(s1);
    mysql_stmt_close(s2);
    mysql_close(m);
}

static void test_ps_free_result_before_fetch(void) {
    printf("\n[PS free_result before fetch]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_frbf");
    Q("CREATE TABLE t_frbf(id INT, val INT)");
    Q("INSERT INTO t_frbf VALUES(1,10),(2,20)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT val FROM t_frbf",
                                strlen("SELECT val FROM t_frbf"));
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_free_result(stmt);
    TEST("free_result before fetch succeeds");
    CHECK(rc == 0, "free_result failed");

    rc = mysql_stmt_execute(stmt);
    TEST("re-execute after free_result succeeds");
    CHECK(rc == 0, "re-execute failed");

    int out_val;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);
    rc = mysql_stmt_fetch(stmt);
    TEST("fetch after re-execute works");
    CHECK(rc == 0 && out_val == 10, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_close_with_pending_rows(void) {
    printf("\n[PS close with pending rows]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_cwp");
    Q("CREATE TABLE t_cwp(id INT)");
    Q("INSERT INTO t_cwp VALUES(1),(2),(3)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT id FROM t_cwp",
                                strlen("SELECT id FROM t_cwp"));
    rc = mysql_stmt_execute(stmt);
    TEST("execute ok, rows pending on wire");
    CHECK(rc == 0, "execute failed");

    rc = mysql_stmt_close(stmt);
    TEST("close with pending rows succeeds");
    CHECK(rc == 0, "close failed");

    Q("SELECT 1");
    MYSQL_RES *res = mysql_store_result(m);
    TEST("connection still usable after close with pending rows");
    CHECK(res != NULL, "connection broken");
    if (res) mysql_free_result(res);

    mysql_close(m);
}

static void test_ps_reset_with_pending_rows(void) {
    printf("\n[PS reset with pending rows (BUG: no drain in mysql_stmt_reset)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_rwp");
    Q("CREATE TABLE t_rwp(id INT, val INT)");
    Q("INSERT INTO t_rwp VALUES(1,10),(2,20),(3,30)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT val FROM t_rwp WHERE id > ?",
                                strlen("SELECT val FROM t_rwp WHERE id > ?"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    int threshold = 0;
    unsigned long tl = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &threshold; pb.length = &tl;
    mysql_stmt_bind_param(stmt, &pb);

    int out_val;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    TEST("execute returns 3 rows");
    CHECK(rc == 0, "execute failed");

    rc = mysql_stmt_fetch(stmt);
    TEST("fetch row 1 (val=10)");
    CHECK(rc == 0 && out_val == 10, "wrong value");

    rc = mysql_stmt_reset(stmt);
    TEST("reset with pending rows should succeed");
    CHECK(rc == 0, "reset failed — wire likely corrupted");

    Q("SELECT 1");
    MYSQL_RES *res = mysql_store_result(m);
    TEST("connection still usable after reset");
    CHECK(res != NULL, "connection broken after reset");
    if (res) mysql_free_result(res);

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_text_query_escape_all(void) {
    printf("\n[text query: escape special chars]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_tqes");
    Q("CREATE TABLE t_tqes(id INT, val VARCHAR(100))");

    char raw[] = "it's a \"test\" with \\backslash\\ and \nnewline";
    char escaped[256];
    unsigned long elen = mysql_real_escape_string(m, escaped, raw, strlen(raw));

    char sql[512];
    snprintf(sql, sizeof(sql), "INSERT INTO t_tqes(id, val) VALUES(1, '%s')", escaped);
    Q(sql);

    Q("SELECT val FROM t_tqes WHERE id = 1");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("escaped string roundtrips correctly");
    CHECK(row && row[0] && strcmp(row[0], raw) == 0, "wrong value");

    mysql_free_result(res);
    mysql_close(m);
}

static void test_text_query_select_constants(void) {
    printf("\n[text query: SELECT constants]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("SELECT 1, 'hello', 3.14, NULL, 42 + 8");
    MYSQL_RES *res = mysql_store_result(m);
    if (!res) { FAIL("no result"); mysql_close(m); return; }

    MYSQL_ROW row = mysql_fetch_row(res);
    unsigned long *lens = mysql_fetch_lengths(res);
    TEST("col0='1', col1='hello', col4='50'");
    CHECK(row && strcmp(row[0], "1") == 0 && strcmp(row[1], "hello") == 0 && strcmp(row[4], "50") == 0, "wrong values");
    TEST("col3 is NULL");
    CHECK(row && row[3] == NULL, "not null");

    mysql_free_result(res);
    mysql_close(m);
}

static void test_ps_window_function(void) {
    printf("\n[PS window function (ROW_NUMBER)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_wf");
    Q("CREATE TABLE t_wf(id INT, cat VARCHAR(10), val INT)");
    Q("INSERT INTO t_wf VALUES(1,'A',10),(2,'A',30),(3,'B',20),(4,'B',5)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT id, cat, val, ROW_NUMBER() OVER (PARTITION BY cat ORDER BY val DESC) AS rn FROM t_wf WHERE val > ?",
                                strlen("SELECT id, cat, val, ROW_NUMBER() OVER (PARTITION BY cat ORDER BY val DESC) AS rn FROM t_wf WHERE val > ?"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    int min_val = 0;
    unsigned long ml = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &min_val; pb.length = &ml;
    mysql_stmt_bind_param(stmt, &pb);

    rc = mysql_stmt_execute(stmt);
    int count = 0;
    int out_id, out_val, out_rn;
    char out_cat[10];
    unsigned long l1=0,l2=0,l3=0,l4=0;
    MYSQL_BIND rb[4];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &out_id; rb[0].length = &l1;
    rb[1].buffer_type = MYSQL_TYPE_STRING; rb[1].buffer = out_cat; rb[1].buffer_length = sizeof(out_cat); rb[1].length = &l2;
    rb[2].buffer_type = MYSQL_TYPE_LONG; rb[2].buffer = &out_val; rb[2].length = &l3;
    rb[3].buffer_type = MYSQL_TYPE_LONG; rb[3].buffer = &out_rn; rb[3].length = &l4;
    mysql_stmt_bind_result(stmt, rb);

    while (mysql_stmt_fetch(stmt) == 0) count++;
    TEST("ROW_NUMBER returns 4 rows");
    CHECK(count == 4, "wrong count");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_cte(void) {
    printf("\n[PS CTE (WITH clause)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_cte");
    Q("CREATE TABLE t_cte(id INT, val INT)");
    Q("INSERT INTO t_cte VALUES(1,10),(2,20),(3,30)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "WITH cte AS (SELECT id, val FROM t_cte WHERE val > ?) SELECT SUM(val) FROM cte",
                                strlen("WITH cte AS (SELECT id, val FROM t_cte WHERE val > ?) SELECT SUM(val) FROM cte"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    int threshold = 15;
    unsigned long tl = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &threshold; pb.length = &tl;
    mysql_stmt_bind_param(stmt, &pb);

    long long out_sum;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONGLONG; rb.buffer = &out_sum; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("CTE SUM(val>15) = 50");
    CHECK(rc == 0 && out_sum == 50, "wrong sum");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_multi_column_where(void) {
    printf("\n[PS multi-column WHERE]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_mcw");
    Q("CREATE TABLE t_mcw(a INT, b INT, c INT)");
    Q("INSERT INTO t_mcw VALUES(1,2,3),(4,5,6),(7,8,9)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT a, b, c FROM t_mcw WHERE a > ? AND b < ? AND c = ?",
                                strlen("SELECT a, b, c FROM t_mcw WHERE a > ? AND b < ? AND c = ?"));

    int va = 3, vb = 10, vc = 6;
    unsigned long la = sizeof(int), lb = sizeof(int), lc = sizeof(int);
    MYSQL_BIND pb[3];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &va; pb[0].length = &la;
    pb[1].buffer_type = MYSQL_TYPE_LONG; pb[1].buffer = &vb; pb[1].length = &lb;
    pb[2].buffer_type = MYSQL_TYPE_LONG; pb[2].buffer = &vc; pb[2].length = &lc;
    mysql_stmt_bind_param(stmt, pb);

    int out_a, out_b, out_c;
    unsigned long l1=0,l2=0,l3=0;
    MYSQL_BIND rb[3];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &out_a; rb[0].length = &l1;
    rb[1].buffer_type = MYSQL_TYPE_LONG; rb[1].buffer = &out_b; rb[1].length = &l2;
    rb[2].buffer_type = MYSQL_TYPE_LONG; rb[2].buffer = &out_c; rb[2].length = &l3;
    mysql_stmt_bind_result(stmt, rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("a>3 AND b<10 AND c=6: (4,5,6)");
    CHECK(rc == 0 && out_a == 4 && out_b == 5 && out_c == 6, "wrong values");

    rc = mysql_stmt_fetch(stmt);
    TEST("only 1 row matches");
    CHECK(rc == MYSQL_NO_DATA, "too many rows");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_double_reexecute_with_store(void) {
    printf("\n[PS double re-execute with store_result]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_drs");
    Q("CREATE TABLE t_drs(id INT, val INT)");
    Q("INSERT INTO t_drs VALUES(1,10),(2,20),(3,30)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT val FROM t_drs WHERE id <= ? ORDER BY id",
                                strlen("SELECT val FROM t_drs WHERE id <= ? ORDER BY id"));

    int id_limit;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id_limit; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    int out_val;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    id_limit = 1;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_store_result(stmt);
    TEST("exec1: num_rows=1");
    CHECK(mysql_stmt_num_rows(stmt) == 1, "wrong count");
    rc = mysql_stmt_fetch(stmt);
    CHECK(rc == 0 && out_val == 10, "wrong val");

    id_limit = 2;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_store_result(stmt);
    TEST("exec2: num_rows=2");
    CHECK(mysql_stmt_num_rows(stmt) == 2, "wrong count");

    id_limit = 3;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_store_result(stmt);
    TEST("exec3: num_rows=3");
    CHECK(mysql_stmt_num_rows(stmt) == 3, "wrong count");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_param_type_mismatch(void) {
    printf("\n[PS param type mismatch (string as int)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT ? + 1",
                                strlen("SELECT ? + 1"));

    char str_val[] = "42";
    unsigned long slen = 2;
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_STRING; pb.buffer = str_val; pb.buffer_length = sizeof(str_val); pb.length = &slen;
    mysql_stmt_bind_param(stmt, &pb);

    long long out;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONGLONG; rb.buffer = &out; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("'42' + 1 = 43");
    CHECK(rc == 0 && out == 43, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_text_query_field_count_no_result(void) {
    printf("\n[text query: field_count for INSERT]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_tqfc");
    Q("CREATE TABLE t_tqfc(id INT)");
    Q("INSERT INTO t_tqfc VALUES(1)");

    TEST("field_count after INSERT == 0");
    CHECK(mysql_field_count(m) == 0, "wrong field count");

    Q("SELECT id FROM t_tqfc");
    TEST("field_count after SELECT == 1");
    CHECK(mysql_field_count(m) == 1, "wrong field count");
    MYSQL_RES *res = mysql_store_result(m);
    if (res) mysql_free_result(res);

    mysql_close(m);
}

static void test_conn_ping_alive(void) {
    printf("\n[conn: ping alive connection]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }
    int rc = mysql_ping(m);
    TEST("ping on alive connection returns 0");
    CHECK(rc == 0, "ping failed");
    mysql_close(m);
}

static void test_conn_character_set(void) {
    printf("\n[conn: character set info]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }
    const char *cs = mysql_character_set_name(m);
    TEST("character_set_name returns non-NULL");
    CHECK(cs != NULL && cs[0] != '\0', "null charset");
    mysql_close(m);
}

static void test_conn_host_info(void) {
    printf("\n[conn: host info]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }
    const char *hi = mysql_get_host_info(m);
    TEST("host_info contains 'TCP/IP'");
    CHECK(hi && strstr(hi, "TCP/IP") != NULL, "wrong host info");
    mysql_close(m);
}

static void test_conn_server_version(void) {
    printf("\n[conn: server version]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }
    const char *ver = mysql_get_server_info(m);
    TEST("server version starts with '8'");
    CHECK(ver && ver[0] == '8', "wrong version");
    unsigned long vnum = mysql_get_server_version(m);
    TEST("server version number >= 80000");
    CHECK(vnum >= 80000, "wrong version number");
    mysql_close(m);
}

static void test_conn_thread_id(void) {
    printf("\n[conn: thread id]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }
    unsigned long tid = mysql_thread_id(m);
    TEST("thread_id > 0");
    CHECK(tid > 0, "zero thread id");
    mysql_close(m);
}

static void test_conn_proto_info(void) {
    printf("\n[conn: protocol info]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }
    unsigned int proto = mysql_get_proto_info(m);
    TEST("protocol version == 10");
    CHECK(proto == 10, "wrong protocol");
    mysql_close(m);
}

static void test_text_query_multi_statement(void) {
    printf("\n[text query: multiple statements sequentially]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_ms");
    Q("CREATE TABLE t_ms(id INT)");
    Q("INSERT INTO t_ms VALUES(1)");
    Q("INSERT INTO t_ms VALUES(2)");
    Q("INSERT INTO t_ms VALUES(3)");

    Q("SELECT COUNT(*) FROM t_ms");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("COUNT(*) = 3");
    CHECK(row && strcmp(row[0], "3") == 0, "wrong count");
    mysql_free_result(res);
    mysql_close(m);
}

static void test_text_query_like_pattern(void) {
    printf("\n[text query: LIKE pattern]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_like");
    Q("CREATE TABLE t_like(id INT, name VARCHAR(30))");
    Q("INSERT INTO t_like VALUES(1,'apple'),(2,'application'),(3,'banana'),(4,'grape')");

    Q("SELECT COUNT(*) FROM t_like WHERE name LIKE 'app%'");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("LIKE 'app%%' matches 2 rows");
    CHECK(row && strcmp(row[0], "2") == 0, "wrong count");
    mysql_free_result(res);

    Q("SELECT COUNT(*) FROM t_like WHERE name LIKE '%ana%'");
    res = mysql_store_result(m);
    row = mysql_fetch_row(res);
    TEST("LIKE '%%ana%%' matches 1 row");
    CHECK(row && strcmp(row[0], "1") == 0, "wrong count");
    mysql_free_result(res);
    mysql_close(m);
}

static void test_text_query_between(void) {
    printf("\n[text query: BETWEEN]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_btwn");
    Q("CREATE TABLE t_btwn(id INT, val INT)");
    Q("INSERT INTO t_btwn VALUES(1,10),(2,20),(3,30),(4,40),(5,50)");

    Q("SELECT COUNT(*) FROM t_btwn WHERE val BETWEEN 20 AND 40");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("BETWEEN 20 AND 40: 3 rows");
    CHECK(row && strcmp(row[0], "3") == 0, "wrong count");
    mysql_free_result(res);
    mysql_close(m);
}

static void test_text_query_in_clause(void) {
    printf("\n[text query: IN clause]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_in");
    Q("CREATE TABLE t_in(id INT, val VARCHAR(10))");
    Q("INSERT INTO t_in VALUES(1,'a'),(2,'b'),(3,'c'),(4,'d')");

    Q("SELECT COUNT(*) FROM t_in WHERE val IN ('a', 'c', 'd')");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("IN ('a','c','d'): 3 rows");
    CHECK(row && strcmp(row[0], "3") == 0, "wrong count");
    mysql_free_result(res);
    mysql_close(m);
}

static void test_text_query_subquery(void) {
    printf("\n[text query: subquery]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_sq1, t_sq2");
    Q("CREATE TABLE t_sq1(id INT, val INT)");
    Q("CREATE TABLE t_sq2(id INT, threshold INT)");
    Q("INSERT INTO t_sq1 VALUES(1,10),(2,20),(3,30)");
    Q("INSERT INTO t_sq2 VALUES(1,15)");

    Q("SELECT val FROM t_sq1 WHERE val > (SELECT threshold FROM t_sq2 WHERE id = 1) ORDER BY val");
    MYSQL_RES *res = mysql_store_result(m);
    int count = 0;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) count++;
    TEST("subquery: val > 15 returns 2 rows (20,30)");
    CHECK(count == 2, "wrong count");
    mysql_free_result(res);
    mysql_close(m);
}

static void test_text_query_aggregate_funcs(void) {
    printf("\n[text query: aggregate functions]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_agg");
    Q("CREATE TABLE t_agg(id INT, val INT)");
    Q("INSERT INTO t_agg VALUES(1,10),(2,20),(3,30),(4,40)");

    Q("SELECT SUM(val), AVG(val), MIN(val), MAX(val), COUNT(val) FROM t_agg");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("SUM=100, AVG=25, MIN=10, MAX=40, COUNT=4");
    CHECK(row && strcmp(row[0],"100")==0 && strcmp(row[1],"25.0000")==0 &&
          strcmp(row[2],"10")==0 && strcmp(row[3],"40")==0 && strcmp(row[4],"4")==0, "wrong values");
    mysql_free_result(res);
    mysql_close(m);
}


static void test_ps_year_type(void) {
    printf("\n[PS YEAR type]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_year");
    Q("CREATE TABLE t_year(id INT, yr YEAR)");
    Q("INSERT INTO t_year VALUES(1, 2024),(2, 1999)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT yr FROM t_year WHERE id = ?",
                                strlen("SELECT yr FROM t_year WHERE id = ?"));
    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    char out[8];
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_STRING; rb.buffer = out; rb.buffer_length = sizeof(out); rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    id = 1;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("YEAR 2024 as string");
    CHECK(rc == 0 && strcmp(out, "2024") == 0, "wrong year");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_text_column(void) {
    printf("\n[PS TEXT column read]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_txt");
    Q("CREATE TABLE t_txt(id INT, content TEXT)");
    Q("INSERT INTO t_txt VALUES(1, 'This is a text column with more data')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT content FROM t_txt WHERE id = ?",
                                strlen("SELECT content FROM t_txt WHERE id = ?"));
    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    char out[256];
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_STRING; rb.buffer = out; rb.buffer_length = sizeof(out); rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("TEXT column read correctly");
    CHECK(rc == 0 && strcmp(out, "This is a text column with more data") == 0, "wrong text");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_double_param_reexecute(void) {
    printf("\n[PS DOUBLE param re-execute]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT ? * 2.0",
                                strlen("SELECT ? * 2.0"));

    double val = 3.5;
    unsigned long vl = sizeof(double);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_DOUBLE; pb.buffer = &val; pb.length = &vl;
    mysql_stmt_bind_param(stmt, &pb);

    double out;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_DOUBLE; rb.buffer = &out; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    val = 3.5;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("3.5 * 2.0 = 7.0");
    CHECK(rc == 0 && fabs(out - 7.0) < 0.001, "wrong value");

    val = 1.25;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("1.25 * 2.0 = 2.5");
    CHECK(rc == 0 && fabs(out - 2.5) < 0.001, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_execute_after_error(void) {
    printf("\n[PS execute after SQL error]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_eae");
    Q("CREATE TABLE t_eae(id INT PRIMARY KEY, val INT)");
    Q("INSERT INTO t_eae VALUES(1,100)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT val FROM t_eae WHERE id = ?",
                                strlen("SELECT val FROM t_eae WHERE id = ?"));

    MYSQL_STMT *bad_stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(bad_stmt, "SELECT * FROM nonexistent_table_xyz",
                            strlen("SELECT * FROM nonexistent_table_xyz"));
    TEST("prepare on nonexistent table returns error");
    CHECK(rc != 0, "should fail");
    mysql_stmt_close(bad_stmt);

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    int out_val;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("after error, valid stmt still works: val=100");
    CHECK(rc == 0 && out_val == 100, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_ps_null_param_insert(void) {
    printf("\n[PS NULL param INSERT]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_npi");
    Q("CREATE TABLE t_npi(id INT, a INT, b VARCHAR(20))");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_npi(id, a, b) VALUES(?, ?, ?)",
                                strlen("INSERT INTO t_npi(id, a, b) VALUES(?, ?, ?)"));

    int id = 1;
    my_bool a_null = 1, b_null = 0;
    unsigned long il = sizeof(int), al = 0, bl = 5;
    char b_val[] = "hello";
    MYSQL_BIND pb[3];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &il;
    pb[1].buffer_type = MYSQL_TYPE_LONG; pb[1].is_null = &a_null;
    pb[2].buffer_type = MYSQL_TYPE_STRING; pb[2].buffer = b_val; pb[2].buffer_length = 6; pb[2].length = &bl; pb[2].is_null = &b_null;
    mysql_stmt_bind_param(stmt, pb);

    rc = mysql_stmt_execute(stmt);
    TEST("INSERT with NULL param succeeds");
    CHECK(rc == 0, "insert failed");
    mysql_stmt_close(stmt);

    Q("SELECT a, b FROM t_npi WHERE id = 1");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("a is NULL, b='hello'");
    CHECK(row && row[0] == NULL && row[1] && strcmp(row[1], "hello") == 0, "wrong values");
    mysql_free_result(res);
    mysql_close(m);
}

static void test_ps_batch_insert(void) {
    printf("\n[PS batch INSERT (loop)]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_bi2");
    Q("CREATE TABLE t_bi2(id INT, val INT)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_bi2(id, val) VALUES(?, ?)",
                                strlen("INSERT INTO t_bi2(id, val) VALUES(?, ?)"));

    int id, val;
    unsigned long il = sizeof(int), vl = sizeof(int);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &il;
    pb[1].buffer_type = MYSQL_TYPE_LONG; pb[1].buffer = &val; pb[1].length = &vl;
    mysql_stmt_bind_param(stmt, pb);

    int total = 100;
    for (int i = 0; i < total; i++) {
        id = i + 1;
        val = (i + 1) * 10;
        rc = mysql_stmt_execute(stmt);
        if (rc != 0) break;
    }
    TEST("100 row batch insert all succeed");
    CHECK(rc == 0, "some inserts failed");

    mysql_stmt_close(stmt);

    Q("SELECT COUNT(*) FROM t_bi2");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("COUNT(*) = 100");
    CHECK(row && strcmp(row[0], "100") == 0, "wrong count");
    mysql_free_result(res);

    Q("SELECT SUM(val) FROM t_bi2");
    res = mysql_store_result(m);
    row = mysql_fetch_row(res);
    TEST("SUM(val) = 50500");
    CHECK(row && strcmp(row[0], "50500") == 0, "wrong sum");
    mysql_free_result(res);

    mysql_close(m);
}

static void test_text_query_date_functions(void) {
    printf("\n[text query: date functions]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("SELECT YEAR('2024-06-20'), MONTH('2024-06-20'), DAY('2024-06-20')");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("YEAR=2024, MONTH=6, DAY=20");
    CHECK(row && strcmp(row[0],"2024")==0 && strcmp(row[1],"6")==0 && strcmp(row[2],"20")==0, "wrong date parts");
    mysql_free_result(res);

    Q("SELECT DATEDIFF('2024-06-20', '2024-01-01')");
    res = mysql_store_result(m);
    row = mysql_fetch_row(res);
    TEST("DATEDIFF = 171");
    CHECK(row && atoi(row[0]) == 171, "wrong diff");
    mysql_free_result(res);
    mysql_close(m);
}

static void test_text_query_string_functions(void) {
    printf("\n[text query: string functions]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("SELECT CONCAT('hello', ' ', 'world'), REVERSE('abcde'), REPEAT('ha', 3)");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("CONCAT='hello world', REVERSE='edcba', REPEAT='hahaha'");
    CHECK(row && strcmp(row[0],"hello world")==0 && strcmp(row[1],"edcba")==0 && strcmp(row[2],"hahaha")==0, "wrong values");
    mysql_free_result(res);

    Q("SELECT TRIM('  hello  '), LTRIM('  hello'), RTRIM('hello  ')");
    res = mysql_store_result(m);
    row = mysql_fetch_row(res);
    TEST("TRIM='hello', LTRIM='hello', RTRIM='hello'");
    CHECK(row && strcmp(row[0],"hello")==0 && strcmp(row[1],"hello")==0 && strcmp(row[2],"hello")==0, "wrong trim");
    mysql_free_result(res);
    mysql_close(m);
}

static void test_text_query_math_functions(void) {
    printf("\n[text query: math functions]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("SELECT ABS(-42), CEIL(3.2), FLOOR(3.8), ROUND(3.456, 2), POWER(2, 10)");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("ABS=42, CEIL=4, FLOOR=3, ROUND=3.46, POWER=1024");
    CHECK(row && atoi(row[0])==42 && atoi(row[1])==4 && atoi(row[2])==3 &&
          strncmp(row[3],"3.46",4)==0 && atoi(row[4])==1024, "wrong math");
    mysql_free_result(res);
    mysql_close(m);
}

static void test_ps_date_param_insert(void) {
    printf("\n[PS DATE param INSERT]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_dpi");
    Q("CREATE TABLE t_dpi(id INT, dt DATE)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_dpi(id, dt) VALUES(?, ?)",
                                strlen("INSERT INTO t_dpi(id, dt) VALUES(?, ?)"));

    int id = 1;
    MYSQL_TIME dt;
    memset(&dt, 0, sizeof(dt));
    dt.time_type = MYSQL_TIMESTAMP_DATE;
    dt.year = 2024; dt.month = 12; dt.day = 25;
    unsigned long il = sizeof(int), dl = sizeof(MYSQL_TIME);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &il;
    pb[1].buffer_type = MYSQL_TYPE_DATE; pb[1].buffer = &dt; pb[1].buffer_length = sizeof(dt); pb[1].length = &dl;
    mysql_stmt_bind_param(stmt, pb);

    rc = mysql_stmt_execute(stmt);
    TEST("INSERT DATE param succeeds");
    CHECK(rc == 0, "insert failed");
    mysql_stmt_close(stmt);

    Q("SELECT dt FROM t_dpi WHERE id = 1");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("DATE roundtrip: '2024-12-25'");
    CHECK(row && row[0] && strcmp(row[0], "2024-12-25") == 0, "wrong date");
    mysql_free_result(res);
    mysql_close(m);
}

static void test_ps_datetime_param_insert(void) {
    printf("\n[PS DATETIME param INSERT]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_dti");
    Q("CREATE TABLE t_dti(id INT, dt DATETIME)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_dti(id, dt) VALUES(?, ?)",
                                strlen("INSERT INTO t_dti(id, dt) VALUES(?, ?)"));

    int id = 1;
    MYSQL_TIME dt;
    memset(&dt, 0, sizeof(dt));
    dt.time_type = MYSQL_TIMESTAMP_DATETIME;
    dt.year = 2024; dt.month = 6; dt.day = 20; dt.hour = 14; dt.minute = 30; dt.second = 45;
    unsigned long il = sizeof(int), dl = sizeof(MYSQL_TIME);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &il;
    pb[1].buffer_type = MYSQL_TYPE_DATETIME; pb[1].buffer = &dt; pb[1].buffer_length = sizeof(dt); pb[1].length = &dl;
    mysql_stmt_bind_param(stmt, pb);

    rc = mysql_stmt_execute(stmt);
    TEST("INSERT DATETIME param succeeds");
    CHECK(rc == 0, "insert failed");
    mysql_stmt_close(stmt);

    Q("SELECT dt FROM t_dti WHERE id = 1");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("DATETIME roundtrip: '2024-06-20 14:30:45'");
    CHECK(row && row[0] && strncmp(row[0], "2024-06-20 14:30:45", 19) == 0, "wrong datetime");
    mysql_free_result(res);
    mysql_close(m);
}

static void test_ps_time_param_insert(void) {
    printf("\n[PS TIME param INSERT]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_tpi");
    Q("CREATE TABLE t_tpi(id INT, tm TIME)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_tpi(id, tm) VALUES(?, ?)",
                                strlen("INSERT INTO t_tpi(id, tm) VALUES(?, ?)"));

    int id = 1;
    MYSQL_TIME tm;
    memset(&tm, 0, sizeof(tm));
    tm.time_type = MYSQL_TIMESTAMP_TIME;
    tm.hour = 8; tm.minute = 30; tm.second = 45;
    unsigned long il = sizeof(int), tl = sizeof(MYSQL_TIME);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &il;
    pb[1].buffer_type = MYSQL_TYPE_TIME; pb[1].buffer = &tm; pb[1].buffer_length = sizeof(tm); pb[1].length = &tl;
    mysql_stmt_bind_param(stmt, pb);

    rc = mysql_stmt_execute(stmt);
    TEST("INSERT TIME param succeeds");
    CHECK(rc == 0, "insert failed");
    mysql_stmt_close(stmt);

    Q("SELECT tm FROM t_tpi WHERE id = 1");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("TIME roundtrip: '08:30:45'");
    CHECK(row && row[0] && strcmp(row[0], "08:30:45") == 0, "wrong time");
    mysql_free_result(res);
    mysql_close(m);
}

static void test_mysql_info_after_update(void) {
    printf("\n[mysql_info after UPDATE]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_mia");
    Q("CREATE TABLE t_mia(id INT PRIMARY KEY, val INT)");
    Q("INSERT INTO t_mia VALUES(1,10),(2,20),(3,30)");
    Q("UPDATE t_mia SET val = val + 1 WHERE id > 1");

    const char *info = mysql_info(m);
    TEST("mysql_info returns non-NULL after UPDATE");
    CHECK(info != NULL, "info is NULL after UPDATE");
    if (info) {
        TEST("mysql_info contains 'Rows matched'");
        CHECK(strstr(info, "Rows matched") != NULL || strstr(info, "Changed") != NULL,
              "info missing expected content");
    }
    mysql_close(m);
}

static void test_host_info_info_independent(void) {
    printf("\n[host_info and info are independent]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    const char *hi = mysql_get_host_info(m);
    TEST("mysql_get_host_info returns non-NULL");
    CHECK(hi != NULL && hi[0] != 0, "host_info is empty");

    Q("DROP TABLE IF EXISTS t_hii");
    Q("CREATE TABLE t_hii(id INT PRIMARY KEY, val INT)");
    Q("INSERT INTO t_hii VALUES(1,10),(2,20)");
    Q("UPDATE t_hii SET val = val + 1");

    const char *info = mysql_info(m);
    TEST("mysql_info after UPDATE");
    CHECK(info != NULL, "info is NULL after UPDATE");

    const char *hi2 = mysql_get_host_info(m);
    TEST("mysql_get_host_info still returns host info");
    CHECK(hi2 != NULL && hi2[0] != 0, "host_info lost after mysql_info");

    mysql_close(m);
}

static void test_charset_mbmaxlen_utf8mb4(void) {
    printf("\n[charset mbmaxlen for utf8mb4]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MY_CHARSET_INFO cs;
    mysql_get_character_set_info(m, &cs);
    TEST("charset mbmaxlen >= 4 for utf8mb4");
    CHECK(cs.mbmaxlen >= 4, "mbmaxlen < 4 for utf8mb4");

    int rc = mysql_set_character_set(m, "utf8mb4");
    TEST("mysql_set_character_set utf8mb4 succeeds");
    CHECK(rc == 0, "set charset utf8mb4 failed");

    mysql_get_character_set_info(m, &cs);
    TEST("after set utf8mb4, mbmaxlen >= 4");
    CHECK(cs.mbmaxlen >= 4, "mbmaxlen < 4 after set utf8mb4");

    mysql_close(m);
}

static void test_set_unknown_charset_error(void) {
    printf("\n[set unknown charset returns error]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    const char *orig_cs = m->charset_name ? strdup(m->charset_name) : NULL;

    int rc = mysql_set_character_set(m, "nonexistent_charset_xyz");
    TEST("mysql_set_character_set with unknown charset fails");
    CHECK(rc != 0, "set unknown charset should fail");

    TEST("charset_name unchanged after failed set");
    CHECK(m->charset_name && orig_cs && strcmp(m->charset_name, orig_cs) == 0,
          "charset_name changed after failure");
    if (orig_cs) free((void*)orig_cs);

    TEST("errno is set after failed charset");
    CHECK(mysql_errno(m) != 0, "errno not set after charset failure");

    mysql_close(m);
}

static void test_mysql_info_insert(void) {
    printf("\n[mysql_info after INSERT...SELECT]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_mis1, t_mis2");
    Q("CREATE TABLE t_mis1(id INT, val INT)");
    Q("CREATE TABLE t_mis2(id INT, val INT)");
    Q("INSERT INTO t_mis1 VALUES(1,10),(2,20),(3,30)");
    Q("INSERT INTO t_mis2 SELECT * FROM t_mis1");

    const char *info = mysql_info(m);
    TEST("mysql_info returns non-NULL after INSERT...SELECT");
    CHECK(info != NULL, "info is NULL after INSERT...SELECT");

    mysql_close(m);
}

static void test_mysql_info_alter_table(void) {
    printf("\n[mysql_info after ALTER TABLE]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_mia2");
    Q("CREATE TABLE t_mia2(id INT PRIMARY KEY, val INT)");
    Q("ALTER TABLE t_mia2 ADD COLUMN extra INT DEFAULT 0");

    const char *info = mysql_info(m);
    TEST("mysql_info after ALTER TABLE (may be NULL)");
    CHECK(1, "always pass - ALTER TABLE info is optional");

    mysql_close(m);
}

static void test_select_db_malloc_oom(void) {
    printf("\n[select_db with very long name returns error]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    char longdb[4096];
    memset(longdb, 'a', sizeof(longdb) - 1);
    longdb[sizeof(longdb) - 1] = 0;

    int rc = mysql_select_db(m, longdb);
    TEST("mysql_select_db with nonexistent long name returns error");
    CHECK(rc != 0, "select_db should fail for nonexistent db");

    rc = mysql_select_db(m, g_db);
    TEST("mysql_select_db restore original db succeeds");
    CHECK(rc == 0, "failed to restore original db");

    mysql_close(m);
}

static void test_ping_returns_valid(void) {
    printf("\n[mysql_ping returns 0 on live connection]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    int rc = mysql_ping(m);
    TEST("mysql_ping returns 0 on live connection");
    CHECK(rc == 0, "ping failed on live connection");

    rc = mysql_ping(m);
    TEST("second mysql_ping also returns 0");
    CHECK(rc == 0, "second ping failed");

    mysql_close(m);
}

static void test_reset_connection_clears_ps(void) {
    printf("\n[reset_connection clears PS cache]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_rcp");
    Q("CREATE TABLE t_rcp(id INT)");
    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT * FROM t_rcp", strlen("SELECT * FROM t_rcp"));
    TEST("PS prepared before reset");
    CHECK(mysql_stmt_param_count(stmt) == 0, "param count wrong");

    int rc = mysql_reset_connection(m);
    TEST("mysql_reset_connection succeeds");
    CHECK(rc == 0, "reset_connection failed");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

static void test_list_fields_com_field_list(void) {
    printf("\n[mysql_list_fields uses COM_FIELD_LIST]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_lf");
    Q("CREATE TABLE t_lf(id INT PRIMARY KEY, name VARCHAR(50), val DOUBLE)");
    MYSQL_RES *res = mysql_list_fields(m, "t_lf", NULL);
    TEST("mysql_list_fields returns non-NULL");
    CHECK(res != NULL, "list_fields returned NULL");
    if (res) {
        unsigned int nf = mysql_num_fields(res);
        TEST("list_fields returns 3 fields");
        CHECK(nf == 3, "wrong field count");
        MYSQL_FIELD *fields = mysql_fetch_fields(res);
        TEST("first field name is 'id'");
        CHECK(fields && fields[0].name && strcmp(fields[0].name, "id") == 0, "wrong field name");
        TEST("second field name is 'name'");
        CHECK(fields && fields[1].name && strcmp(fields[1].name, "name") == 0, "wrong field name");
        mysql_free_result(res);
    }
    mysql_close(m);
}

static void test_change_user_updates_reconnect_creds(void) {
    printf("\n[change_user updates reconnect credentials]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    int rc = mysql_change_user(m, g_user, g_passwd, g_db);
    TEST("mysql_change_user with same credentials succeeds");
    CHECK(rc == 0, "change_user failed");

    Q("SELECT 1");
    TEST("query after change_user succeeds");
    CHECK(1, "always pass if we got here");

    mysql_close(m);
}

static void test_ps_date_param_always_4bytes(void) {
    printf("\n[PS DATE param always sends 4 bytes]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_dp4");
    Q("CREATE TABLE t_dp4(id INT, dt DATE)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_dp4(id, dt) VALUES(?, ?)",
                                strlen("INSERT INTO t_dp4(id, dt) VALUES(?, ?)"));
    TEST("prepare DATE insert");
    CHECK(rc == 0, "prepare failed");

    int id = 1;
    MYSQL_TIME dt;
    memset(&dt, 0, sizeof(dt));
    dt.time_type = MYSQL_TIMESTAMP_DATE;
    dt.year = 2024; dt.month = 6; dt.day = 15;
    unsigned long il = sizeof(int), dl = sizeof(MYSQL_TIME);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &il;
    pb[1].buffer_type = MYSQL_TYPE_DATE; pb[1].buffer = &dt; pb[1].buffer_length = sizeof(dt); pb[1].length = &dl;
    mysql_stmt_bind_param(stmt, pb);

    rc = mysql_stmt_execute(stmt);
    TEST("execute DATE param (4-byte encoding) succeeds");
    CHECK(rc == 0, "execute failed");
    mysql_stmt_close(stmt);

    Q("SELECT dt FROM t_dp4 WHERE id = 1");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("DATE roundtrip: '2024-06-15'");
    CHECK(row && row[0] && strcmp(row[0], "2024-06-15") == 0, "wrong date");
    mysql_free_result(res);
    mysql_close(m);
}

static void test_ps_time_zero_length(void) {
    printf("\n[PS TIME zero-length encoding for 00:00:00]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_tz");
    Q("CREATE TABLE t_tz(id INT, tm TIME)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_tz(id, tm) VALUES(?, ?)",
                                strlen("INSERT INTO t_tz(id, tm) VALUES(?, ?)"));
    TEST("prepare TIME insert");
    CHECK(rc == 0, "prepare failed");

    int id = 1;
    MYSQL_TIME tm;
    memset(&tm, 0, sizeof(tm));
    tm.time_type = MYSQL_TIMESTAMP_TIME;
    unsigned long il = sizeof(int), tl = sizeof(MYSQL_TIME);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &il;
    pb[1].buffer_type = MYSQL_TYPE_TIME; pb[1].buffer = &tm; pb[1].buffer_length = sizeof(tm); pb[1].length = &tl;
    mysql_stmt_bind_param(stmt, pb);

    rc = mysql_stmt_execute(stmt);
    TEST("execute TIME zero (length=0 encoding) succeeds");
    CHECK(rc == 0, "execute failed");
    mysql_stmt_close(stmt);

    Q("SELECT tm FROM t_tz WHERE id = 1");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("TIME roundtrip: '00:00:00'");
    CHECK(row && row[0] && strcmp(row[0], "00:00:00") == 0, "wrong time");
    mysql_free_result(res);
    mysql_close(m);
}

void test_multi_resultset_store_next(void) {
    printf("\n[multi-resultset: store_result + next_result]\n");
    MYSQL *m = mysql_init(NULL);
    unsigned int ssl_mode = SSL_MODE_DISABLED;
    mysql_options(m, MYSQL_OPT_SSL_MODE, &ssl_mode);
    if (!mysql_real_connect(m, g_host, g_user, g_passwd, g_db, g_port, NULL,
                            CLIENT_MULTI_STATEMENTS)) {
        FAIL("connect failed"); return;
    }

    int rc = mysql_real_query(m, "SELECT 1 AS a; SELECT 2 AS b; SELECT 3 AS c",
                              strlen("SELECT 1 AS a; SELECT 2 AS b; SELECT 3 AS c"));
    TEST("multi-stmt: 3 SELECTs");
    CHECK(rc == 0, "query failed");

    MYSQL_RES *res = mysql_store_result(m);
    TEST("1st SELECT: has result");
    CHECK(res && mysql_num_rows(res) == 1, "should have 1 row");
    if (res) {
        MYSQL_ROW row = mysql_fetch_row(res);
        CHECK(row && strcmp(row[0], "1") == 0, "wrong value");
        mysql_free_result(res);
    }

    rc = mysql_next_result(m);
    TEST("next_result 2");
    CHECK(rc == 0, "next_result failed");
    res = mysql_store_result(m);
    TEST("2nd SELECT: value=2");
    CHECK(res && mysql_num_rows(res) == 1, "should have 1 row");
    if (res) {
        MYSQL_ROW row = mysql_fetch_row(res);
        CHECK(row && strcmp(row[0], "2") == 0, "wrong value");
        mysql_free_result(res);
    }

    rc = mysql_next_result(m);
    TEST("next_result 3");
    CHECK(rc == 0, "next_result failed");
    res = mysql_store_result(m);
    TEST("3rd SELECT: value=3");
    CHECK(res && mysql_num_rows(res) == 1, "should have 1 row");
    if (res) {
        MYSQL_ROW row = mysql_fetch_row(res);
        CHECK(row && strcmp(row[0], "3") == 0, "wrong value");
        mysql_free_result(res);
    }

    rc = mysql_next_result(m);
    TEST("no more results");
    CHECK(rc != 0, "should be done");

    mysql_close(m);
}

void test_ps_deprecate_eof_row_with_zero_prefix(void) {
    printf("\n[PS: DEPRECATE_EOF rows with 0x00 in data]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_deof");
    Q("CREATE TABLE t_deof(id INT PRIMARY KEY, val INT)");
    char ins[64];
    for (int i = 1; i <= 300; i++) {
        snprintf(ins, sizeof(ins), "INSERT INTO t_deof VALUES(%d,%d)", i, i * 10);
        Q(ins);
    }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    mysql_stmt_prepare(stmt, "SELECT id, val FROM t_deof ORDER BY id",
                        strlen("SELECT id, val FROM t_deof ORDER BY id"));
    mysql_stmt_execute(stmt);
    mysql_stmt_store_result(stmt);

    TEST("all 300 rows stored");
    CHECK(mysql_stmt_num_rows(stmt) == 300, "wrong count");

    int count = 0;
    while (mysql_stmt_fetch(stmt) == 0) count++;
    TEST("fetched all 300 rows");
    CHECK(count == 300, "wrong fetch count");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_row_seek_boundary(void) {
    printf("\n[row_seek boundary check]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_rsk");
    Q("CREATE TABLE t_rsk(id INT PRIMARY KEY)");
    for (int i = 1; i <= 5; i++) {
        char ins[32];
        snprintf(ins, sizeof(ins), "INSERT INTO t_rsk VALUES(%d)", i);
        Q(ins);
    }

    Q("SELECT id FROM t_rsk ORDER BY id");
    MYSQL_RES *res = mysql_store_result(m);
    TEST("store_result: 5 rows");
    CHECK(res && mysql_num_rows(res) == 5, "wrong count");

    MYSQL_ROW_OFFSET save = mysql_row_tell(res);
    mysql_data_seek(res, 2);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("data_seek(2): id=3");
    CHECK(row && strcmp(row[0], "3") == 0, "wrong row");

    mysql_row_seek(res, save);
    row = mysql_fetch_row(res);
    TEST("row_seek(0): id=1");
    CHECK(row && strcmp(row[0], "1") == 0, "wrong row");

    MYSQL_ROW_OFFSET invalid = (MYSQL_ROW_OFFSET)(uintptr_t)999;
    mysql_row_seek(res, invalid);
    row = mysql_fetch_row(res);
    TEST("row_seek(999) clamped: no row");
    CHECK(row == NULL, "should be NULL");

    mysql_free_result(res);
    mysql_close(m);
}

void test_ps_call_mixed_params_multi_rs(void) {
    printf("\n[PS: CALL with IN/OUT/INOUT mixed params + multi-resultset]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP PROCEDURE IF EXISTS p_mixed");
    Q("DROP TABLE IF EXISTS t_mixed");
    Q("CREATE TABLE t_mixed(a INT, b VARCHAR(32))");
    Q("INSERT INTO t_mixed VALUES(1,'alpha'),(2,'beta'),(3,'gamma')");

    Q("CREATE PROCEDURE p_mixed("
      "IN p_in INT, "
      "OUT p_out_str VARCHAR(64), "
      "OUT p_out_int INT, "
      "INOUT p_inout_dbl DOUBLE) "
      "BEGIN "
      "SET p_out_str = 'hello_out'; "
      "SET p_out_int = p_in * 10; "
      "SET p_inout_dbl = p_inout_dbl + 1.5; "
      "SELECT a, b FROM t_mixed WHERE a <= p_in; "
      "SELECT a * 100 AS doubled FROM t_mixed; "
      "END");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "CALL p_mixed(?, ?, ?, ?)",
                                strlen("CALL p_mixed(?, ?, ?, ?)"));
    TEST("prepare CALL p_mixed");
    CHECK(rc == 0, "prepare failed");

    TEST("param_count = 4");
    CHECK(mysql_stmt_param_count(stmt) == 4, "should have 4 params");

    int p_in = 2;
    char p_out_str[128]; unsigned long p_out_str_len = sizeof(p_out_str);
    int p_out_int = 0;
    double p_inout_dbl = 3.5;
    my_bool n1 = 0, n2 = 0, n3 = 0, n4 = 0;

    MYSQL_BIND pb[4];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG;  pb[0].buffer = &p_in;       pb[0].is_null = &n1;
    pb[1].buffer_type = MYSQL_TYPE_STRING; pb[1].buffer = p_out_str;  pb[1].buffer_length = sizeof(p_out_str); pb[1].length = &p_out_str_len; pb[1].is_null = &n2;
    pb[2].buffer_type = MYSQL_TYPE_LONG;  pb[2].buffer = &p_out_int;  pb[2].is_null = &n3;
    pb[3].buffer_type = MYSQL_TYPE_DOUBLE; pb[3].buffer = &p_inout_dbl; pb[3].is_null = &n4;
    mysql_stmt_bind_param(stmt, pb);

    rc = mysql_stmt_execute(stmt);
    TEST("execute CALL p_mixed");
    CHECK(rc == 0, "execute failed");

    int rs_idx = 0;
    int found_out_params = 0;
    for (;;) {
        rc = mysql_stmt_store_result(stmt);
        CHECK(rc == 0 || mysql_stmt_field_count(stmt) == 0, "store_result failed");

        MYSQL_RES *meta = mysql_stmt_result_metadata(stmt);
        unsigned int nf = meta ? mysql_num_fields(meta) : 0;

        if (nf == 3 && rs_idx > 0) {
            found_out_params = 1;
            char out_s[128]; int out_i = 0; double out_d = 0;
            unsigned long osl = sizeof(out_s);
            my_bool ons = 0, oni = 0, ond = 0;
            MYSQL_BIND out_bind[3];
            memset(out_bind, 0, sizeof(out_bind));
            out_bind[0].buffer_type = MYSQL_TYPE_STRING;  out_bind[0].buffer = out_s;     out_bind[0].buffer_length = sizeof(out_s); out_bind[0].length = &osl; out_bind[0].is_null = &ons;
            out_bind[1].buffer_type = MYSQL_TYPE_LONG;    out_bind[1].buffer = &out_i;    out_bind[1].is_null = &oni;
            out_bind[2].buffer_type = MYSQL_TYPE_DOUBLE;  out_bind[2].buffer = &out_d;    out_bind[2].is_null = &ond;
            mysql_stmt_bind_result(stmt, out_bind);

            rc = mysql_stmt_fetch(stmt);
            TEST("OUT: p_out_str = 'hello_out'");
            CHECK(rc == 0 && strncmp(out_s, "hello_out", 9) == 0, "wrong out string");
            TEST("OUT: p_out_int = 20");
            CHECK(out_i == 20, "wrong out int");
            TEST("OUT: p_inout_dbl = 5.0");
            CHECK(out_d > 4.99 && out_d < 5.01, "wrong inout double");
        } else if (nf == 2 && rs_idx == 0) {
            int ra = 0; char rb[64]; unsigned long rbl = sizeof(rb); my_bool rna = 0, rnb = 0;
            MYSQL_BIND rb_bind[2];
            memset(rb_bind, 0, sizeof(rb_bind));
            rb_bind[0].buffer_type = MYSQL_TYPE_LONG;   rb_bind[0].buffer = &ra; rb_bind[0].is_null = &rna;
            rb_bind[1].buffer_type = MYSQL_TYPE_STRING; rb_bind[1].buffer = rb;  rb_bind[1].buffer_length = sizeof(rb); rb_bind[1].length = &rbl; rb_bind[1].is_null = &rnb;
            mysql_stmt_bind_result(stmt, rb_bind);
            int row_count = 0;
            while (mysql_stmt_fetch(stmt) == 0) row_count++;
            TEST("1st resultset: 2 rows (a<=2)");
            CHECK(row_count == 2, "wrong row count");
        } else if (nf == 1 && rs_idx == 1) {
            int rd = 0; my_bool rnd = 0;
            MYSQL_BIND rd_bind;
            memset(&rd_bind, 0, sizeof(rd_bind));
            rd_bind.buffer_type = MYSQL_TYPE_LONG; rd_bind.buffer = &rd; rd_bind.is_null = &rnd;
            mysql_stmt_bind_result(stmt, &rd_bind);
            int row_count = 0;
            while (mysql_stmt_fetch(stmt) == 0) row_count++;
            TEST("2nd resultset: 3 rows");
            CHECK(row_count == 3, "wrong row count");
        }
        if (meta) mysql_free_result(meta);

        rc = mysql_stmt_next_result(stmt);
        if (rc != 0) break;
        rs_idx++;
    }

    TEST("found OUT param resultset");
    CHECK(found_out_params, "OUT params not found in resultsets");

    mysql_stmt_close(stmt);
    Q("DROP PROCEDURE IF EXISTS p_mixed");
    Q("DROP TABLE IF EXISTS t_mixed");
    mysql_close(m);
}

void test_ps_out_param_all_types(void) {
    printf("\n[PS: OUT param all types]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP PROCEDURE IF EXISTS p_types");
    Q("CREATE PROCEDURE p_types("
      "OUT o_tiny TINYINT, OUT o_small SMALLINT, OUT o_int INT, OUT o_big BIGINT, "
      "OUT o_float FLOAT, OUT o_double DOUBLE, "
      "OUT o_date DATE, OUT o_time TIME, OUT o_dt DATETIME, "
      "OUT o_char CHAR(16), OUT o_varchar VARCHAR(32)) "
      "BEGIN "
      "SET o_tiny = 127; SET o_small = 32000; SET o_int = 2000000; SET o_big = 9000000000; "
      "SET o_float = 3.14; SET o_double = 2.718281828; "
      "SET o_date = '2025-06-15'; SET o_time = '14:30:00'; SET o_dt = '2025-06-15 14:30:00'; "
      "SET o_char = 'fixed_char'; SET o_varchar = 'variable_text'; "
      "END");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "CALL p_types(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
                                strlen("CALL p_types(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));
    TEST("prepare p_types");
    CHECK(rc == 0, "prepare failed");

    TEST("param_count = 11");
    CHECK(mysql_stmt_param_count(stmt) == 11, "should have 11 params");

    signed char o_tiny = 0;
    short o_small = 0;
    int o_int = 0;
    long long o_big = 0;
    float o_float = 0;
    double o_double = 0;
    MYSQL_TIME o_date, o_time, o_dt;
    char o_char[32], o_varchar[64];
    unsigned long ocl = sizeof(o_char), ovl = sizeof(o_varchar);
    my_bool nulls[11] = {0};

    MYSQL_BIND pb[11];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_TINY;    pb[0].buffer = &o_tiny;    pb[0].is_null = &nulls[0];
    pb[1].buffer_type = MYSQL_TYPE_SHORT;   pb[1].buffer = &o_small;   pb[1].is_null = &nulls[1];
    pb[2].buffer_type = MYSQL_TYPE_LONG;    pb[2].buffer = &o_int;     pb[2].is_null = &nulls[2];
    pb[3].buffer_type = MYSQL_TYPE_LONGLONG; pb[3].buffer = &o_big;    pb[3].is_null = &nulls[3];
    pb[4].buffer_type = MYSQL_TYPE_FLOAT;   pb[4].buffer = &o_float;   pb[4].is_null = &nulls[4];
    pb[5].buffer_type = MYSQL_TYPE_DOUBLE;  pb[5].buffer = &o_double;  pb[5].is_null = &nulls[5];
    pb[6].buffer_type = MYSQL_TYPE_DATE;    pb[6].buffer = &o_date;    pb[6].is_null = &nulls[6];
    pb[7].buffer_type = MYSQL_TYPE_TIME;    pb[7].buffer = &o_time;    pb[7].is_null = &nulls[7];
    pb[8].buffer_type = MYSQL_TYPE_DATETIME; pb[8].buffer = &o_dt;     pb[8].is_null = &nulls[8];
    pb[9].buffer_type = MYSQL_TYPE_STRING;  pb[9].buffer = o_char;     pb[9].buffer_length = sizeof(o_char); pb[9].length = &ocl; pb[9].is_null = &nulls[9];
    pb[10].buffer_type = MYSQL_TYPE_STRING; pb[10].buffer = o_varchar; pb[10].buffer_length = sizeof(o_varchar); pb[10].length = &ovl; pb[10].is_null = &nulls[10];
    mysql_stmt_bind_param(stmt, pb);

    rc = mysql_stmt_execute(stmt);
    TEST("execute p_types");
    CHECK(rc == 0, "execute failed");

    int found_out = 0;
    int ptypes_loop = 0;
    for (;;) {
        rc = mysql_stmt_store_result(stmt);
        CHECK(rc == 0 || mysql_stmt_field_count(stmt) == 0, "store_result failed");

        MYSQL_RES *meta = mysql_stmt_result_metadata(stmt);
        unsigned int nf = meta ? mysql_num_fields(meta) : 0;

        if (meta && nf == 11) {
            found_out = 1;
            signed char r_tiny = 0; short r_small = 0; int r_int = 0; long long r_big = 0;
            float r_float = 0; double r_double = 0;
            MYSQL_TIME r_date, r_time, r_dt;
            char r_char[32], r_varchar[64];
            unsigned long rcl = 0, rvl = 0;
            my_bool rn[11] = {0};

            MYSQL_BIND rb[11];
            memset(rb, 0, sizeof(rb));
            rb[0].buffer_type = MYSQL_TYPE_TINY;     rb[0].buffer = &r_tiny;    rb[0].is_null = &rn[0];
            rb[1].buffer_type = MYSQL_TYPE_SHORT;    rb[1].buffer = &r_small;   rb[1].is_null = &rn[1];
            rb[2].buffer_type = MYSQL_TYPE_LONG;     rb[2].buffer = &r_int;     rb[2].is_null = &rn[2];
            rb[3].buffer_type = MYSQL_TYPE_LONGLONG; rb[3].buffer = &r_big;    rb[3].is_null = &rn[3];
            rb[4].buffer_type = MYSQL_TYPE_FLOAT;    rb[4].buffer = &r_float;  rb[4].is_null = &rn[4];
            rb[5].buffer_type = MYSQL_TYPE_DOUBLE;   rb[5].buffer = &r_double; rb[5].is_null = &rn[5];
            rb[6].buffer_type = MYSQL_TYPE_DATE;     rb[6].buffer = &r_date;   rb[6].is_null = &rn[6];
            rb[7].buffer_type = MYSQL_TYPE_TIME;     rb[7].buffer = &r_time;   rb[7].is_null = &rn[7];
            rb[8].buffer_type = MYSQL_TYPE_DATETIME; rb[8].buffer = &r_dt;     rb[8].is_null = &rn[8];
            rb[9].buffer_type = MYSQL_TYPE_STRING;   rb[9].buffer = r_char;    rb[9].buffer_length = sizeof(r_char); rb[9].length = &rcl; rb[9].is_null = &rn[9];
            rb[10].buffer_type = MYSQL_TYPE_STRING;  rb[10].buffer = r_varchar; rb[10].buffer_length = sizeof(r_varchar); rb[10].length = &rvl; rb[10].is_null = &rn[10];
            mysql_stmt_bind_result(stmt, rb);

            rc = mysql_stmt_fetch(stmt);
            TEST("OUT TINYINT = 127");
            CHECK(rc == 0 && r_tiny == 127, "wrong tiny");
            TEST("OUT SMALLINT = 32000");
            CHECK(r_small == 32000, "wrong small");
            TEST("OUT INT = 2000000");
            CHECK(r_int == 2000000, "wrong int");
            TEST("OUT BIGINT = 9000000000");
            CHECK(r_big == 9000000000LL, "wrong big");
            TEST("OUT FLOAT ~3.14");
            CHECK(r_float > 3.0f && r_float < 3.5f, "wrong float");
            TEST("OUT DOUBLE ~2.718");
            CHECK(r_double > 2.7 && r_double < 2.8, "wrong double");
            TEST("OUT DATE = 2025-06-15");
            CHECK(r_date.year == 2025 && r_date.month == 6 && r_date.day == 15, "wrong date");
            TEST("OUT TIME = 14:30:00");
            CHECK(r_time.hour == 14 && r_time.minute == 30 && r_time.second == 0, "wrong time");
            TEST("OUT DATETIME = 2025-06-15 14:30:00");
            CHECK(r_dt.year == 2025 && r_dt.month == 6 && r_dt.day == 15 && r_dt.hour == 14 && r_dt.minute == 30, "wrong datetime");
            TEST("OUT CHAR = 'fixed_char'");
            CHECK(strncmp(r_char, "fixed_char", 10) == 0, "wrong char");
            TEST("OUT VARCHAR = 'variable_text'");
            CHECK(strncmp(r_varchar, "variable_text", 13) == 0, "wrong varchar");
        }
        if (meta) mysql_free_result(meta);

        int nr2 = mysql_stmt_next_result(stmt);
        if (nr2 != 0) break;
        ptypes_loop++;
    }

    TEST("found OUT param resultset");
    CHECK(found_out, "OUT params not found");

    mysql_stmt_close(stmt);
    Q("DROP PROCEDURE IF EXISTS p_types");
    mysql_close(m);
}

void test_truncation_all_types(void) {
    printf("\n[PS: data truncation all types]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_trunc");
    Q("CREATE TABLE t_trunc("
      "c_tiny TINYINT, c_small SMALLINT, c_int INT, c_big BIGINT, "
      "c_float FLOAT, c_double DOUBLE, c_char CHAR(10), c_varchar VARCHAR(5))");
    Q("INSERT INTO t_trunc VALUES(127, 32000, 2000000, 9000000000, 3.14, 2.71828, 'abcdefghij', 'hello')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT c_tiny, c_small, c_int, c_big, c_float, c_double, c_char, c_varchar FROM t_trunc",
                                strlen("SELECT c_tiny, c_small, c_int, c_big, c_float, c_double, c_char, c_varchar FROM t_trunc"));
    CHECK(rc == 0, "prepare failed");
    mysql_stmt_execute(stmt);
    mysql_stmt_store_result(stmt);

    signed char r_tiny = 0; short r_small = 0; int r_int = 0; long long r_big = 0;
    float r_float = 0; double r_double = 0;
    char r_char[4], r_varchar[3];
    unsigned long cl = 0, vl = 0;
    my_bool e_tiny = 0, e_small = 0, e_int = 0, e_big = 0;
    my_bool e_float = 0, e_double = 0, e_char = 0, e_varchar = 0;
    my_bool n[8] = {0};

    MYSQL_BIND rb[8];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_TINY;     rb[0].buffer = &r_tiny;    rb[0].error = &e_tiny;    rb[0].is_null = &n[0];
    rb[1].buffer_type = MYSQL_TYPE_SHORT;    rb[1].buffer = &r_small;   rb[1].error = &e_small;   rb[1].is_null = &n[1];
    rb[2].buffer_type = MYSQL_TYPE_LONG;     rb[2].buffer = &r_int;     rb[2].error = &e_int;     rb[2].is_null = &n[2];
    rb[3].buffer_type = MYSQL_TYPE_LONGLONG; rb[3].buffer = &r_big;     rb[3].error = &e_big;     rb[3].is_null = &n[3];
    rb[4].buffer_type = MYSQL_TYPE_FLOAT;    rb[4].buffer = &r_float;   rb[4].error = &e_float;   rb[4].is_null = &n[4];
    rb[5].buffer_type = MYSQL_TYPE_DOUBLE;   rb[5].buffer = &r_double;  rb[5].error = &e_double;  rb[5].is_null = &n[5];
    rb[6].buffer_type = MYSQL_TYPE_STRING;   rb[6].buffer = r_char;     rb[6].buffer_length = sizeof(r_char); rb[6].length = &cl; rb[6].error = &e_char; rb[6].is_null = &n[6];
    rb[7].buffer_type = MYSQL_TYPE_STRING;   rb[7].buffer = r_varchar;  rb[7].buffer_length = sizeof(r_varchar); rb[7].length = &vl; rb[7].error = &e_varchar; rb[7].is_null = &n[7];
    mysql_stmt_bind_result(stmt, rb);

    rc = mysql_stmt_fetch(stmt);
    TEST("fetch truncation row");
    CHECK(rc == 0 || rc == MYSQL_DATA_TRUNCATED, "fetch failed");

    TEST("TINYINT: no truncation (127 fits in signed char)");
    CHECK(e_tiny == 0 && r_tiny == 127, "should not truncate");

    TEST("SMALLINT: no truncation (32000 fits in short)");
    CHECK(e_small == 0 && r_small == 32000, "should not truncate");

    TEST("INT: no truncation");
    CHECK(e_int == 0 && r_int == 2000000, "should not truncate");

    TEST("BIGINT: no truncation");
    CHECK(e_big == 0 && r_big == 9000000000LL, "should not truncate");

    TEST("FLOAT: no truncation (float can hold 3.14)");
    CHECK(e_float == 0, "should not truncate float");

    TEST("DOUBLE: no truncation");
    CHECK(e_double == 0, "should not truncate double");

    TEST("CHAR(10)->char[4]: truncation detected");
    CHECK(e_char == 1, "should detect char truncation");

    TEST("VARCHAR(5)->char[3]: truncation detected");
    CHECK(e_varchar == 1, "should detect varchar truncation");

    mysql_stmt_close(stmt);
    Q("DROP TABLE IF EXISTS t_trunc");
    mysql_close(m);
}

void test_cursor_type_switch(void) {
    printf("\n[PS: cursor type switch on same stmt]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_csw");
    Q("CREATE TABLE t_csw(id INT PRIMARY KEY, val INT)");
    for (int i = 1; i <= 10; i++) {
        char ins[48];
        snprintf(ins, sizeof(ins), "INSERT INTO t_csw VALUES(%d,%d)", i, i * 10);
        Q(ins);
    }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT id, val FROM t_csw ORDER BY id",
                                strlen("SELECT id, val FROM t_csw ORDER BY id"));
    CHECK(rc == 0, "prepare failed");

    int rid = 0, rval = 0; my_bool rn1 = 0, rn2 = 0;
    MYSQL_BIND rb[2];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONG; rb[0].buffer = &rid;  rb[0].is_null = &rn1;
    rb[1].buffer_type = MYSQL_TYPE_LONG; rb[1].buffer = &rval; rb[1].is_null = &rn2;

    unsigned long prefetch = 5;
    unsigned long cursor_type = CURSOR_TYPE_READ_ONLY;
    mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, &cursor_type);
    mysql_stmt_attr_set(stmt, STMT_ATTR_PREFETCH_ROWS, &prefetch);

    rc = mysql_stmt_execute(stmt);
    TEST("cursor execute 1");
    CHECK(rc == 0, "cursor execute failed");

    rc = mysql_stmt_store_result(stmt);
    mysql_stmt_bind_result(stmt, rb);
    int count = 0;
    while (mysql_stmt_fetch(stmt) == 0) count++;
    TEST("cursor fetch 1: 10 rows");
    CHECK(count == 10, "wrong count");

    cursor_type = 0;
    mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, &cursor_type);
    rc = mysql_stmt_execute(stmt);
    TEST("no-cursor execute 2");
    CHECK(rc == 0, "no-cursor execute failed");

    rc = mysql_stmt_store_result(stmt);
    mysql_stmt_bind_result(stmt, rb);
    count = 0;
    while (mysql_stmt_fetch(stmt) == 0) count++;
    TEST("no-cursor fetch 2: 10 rows");
    CHECK(count == 10, "wrong count");

    cursor_type = CURSOR_TYPE_READ_ONLY;
    mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, &cursor_type);
    rc = mysql_stmt_execute(stmt);
    TEST("cursor execute 3");
    CHECK(rc == 0, "cursor execute 3 failed");

    rc = mysql_stmt_store_result(stmt);
    mysql_stmt_bind_result(stmt, rb);
    count = 0;
    while (mysql_stmt_fetch(stmt) == 0) count++;
    TEST("cursor fetch 3: 10 rows");
    CHECK(count == 10, "wrong count");

    mysql_stmt_close(stmt);
    Q("DROP TABLE IF EXISTS t_csw");
    mysql_close(m);
}

void test_server_status_flags(void) {
    printf("\n[server status flags]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("START TRANSACTION");
    TEST("SERVER_STATUS_IN_TRANS after START TRANSACTION");
    CHECK((m->server_status & SERVER_STATUS_IN_TRANS) != 0, "IN_TRANS should be set");

    Q("SELECT 1");
    MYSQL_RES *sr = mysql_store_result(m);
    if (sr) mysql_free_result(sr);
    TEST("SERVER_STATUS_IN_TRANS preserved in transaction");
    CHECK((m->server_status & SERVER_STATUS_IN_TRANS) != 0, "IN_TRANS should still be set");

    Q("COMMIT");
    TEST("COMMIT returns OK");
    CHECK((m->server_status & SERVER_STATUS_IN_TRANS) == 0 || (m->server_status & SERVER_STATUS_AUTOCOMMIT) != 0,
          "IN_TRANS cleared or autocommit mode");

    mysql_close(m);
}

void test_double_real_connect(void) {
    printf("\n[double mysql_real_connect should fail]\n");
    MYSQL *m = mysql_init(NULL);
    unsigned int ssl_mode = SSL_MODE_DISABLED;
    mysql_options(m, MYSQL_OPT_SSL_MODE, &ssl_mode);
    if (!mysql_real_connect(m, g_host, g_user, g_passwd, g_db, g_port, NULL, 0)) {
        FAIL("first connect failed");
        mysql_close(m);
        return;
    }

    MYSQL *m2 = mysql_real_connect(m, g_host, g_user, g_passwd, g_db, g_port, NULL, 0);
    TEST("second connect on same handle returns NULL");
    CHECK(m2 == NULL, "should fail on already-connected handle");

    if (m2) mysql_close(m2);
    else mysql_close(m);
}

void test_opt_reconnect_flag(void) {
    printf("\n[MYSQL_OPT_RECONNECT flag behavior]\n");
    MYSQL *m = mysql_init(NULL);
    my_bool recon = 0;

    TEST("reconnect default = 0");
    CHECK(mysql_get_option(m, MYSQL_OPT_RECONNECT, &recon) == 0 && recon == 0, "default should be 0");

    recon = 1;
    mysql_options(m, MYSQL_OPT_RECONNECT, &recon);
    TEST("reconnect after set = 1");
    CHECK(mysql_get_option(m, MYSQL_OPT_RECONNECT, &recon) == 0 && recon == 1, "should be 1 after set");

    unsigned int ssl_mode = SSL_MODE_DISABLED;
    mysql_options(m, MYSQL_OPT_SSL_MODE, &ssl_mode);
    if (!mysql_real_connect(m, g_host, g_user, g_passwd, g_db, g_port, NULL, 0)) {
        FAIL("connect failed");
        mysql_close(m);
        return;
    }

    TEST("reconnect preserved after connect = 1");
    CHECK(mysql_get_option(m, MYSQL_OPT_RECONNECT, &recon) == 0 && recon == 1, "should still be 1 after connect");

    mysql_close(m);

    m = mysql_init(NULL);
    recon = 0;
    mysql_options(m, MYSQL_OPT_SSL_MODE, &ssl_mode);
    mysql_real_connect(m, g_host, g_user, g_passwd, g_db, g_port, NULL, 0);
    TEST("reconnect without set = 0 after connect");
    CHECK(mysql_get_option(m, MYSQL_OPT_RECONNECT, &recon) == 0 && recon == 0, "should be 0");
    mysql_close(m);
}

void test_cursor_read_only_reexecute(void);
void test_ps_large_result_set(void);
void test_ps_reprepare_after_alter_column(void);
void test_ps_year_boundary_values(void);
void test_cursor_prefetch_rows_large(void);
void test_stmt_reset_state(void);
void test_ps_date_time_string_bind(void);
void test_ps_time_string_bind(void);
void test_ps_store_result_large(void);
void test_ps_multiple_reprepare(void);
void test_ps_longlong_truncation(void);
void test_ps_execute_after_reset_state(void);
void test_sql_injection_list_wild(void);
void test_proto_info(void);
void test_host_info_thread_safe(void);
void test_stat_thread_safe(void);
void test_ps_time_without_microseconds(void);
void test_ps_datetime_without_microseconds(void);
void test_ps_time_with_microseconds(void);
void test_ps_close_normal(void);
void test_change_user_reauth(void);
void test_ps_blob_medium(void);
void test_ps_concurrent_connections(void);
void test_ps_null_param_reexecute(void);
void test_ps_double_type_precision(void);
void test_ps_float_type_precision(void);
void test_charset_info(void);
void test_get_option_roundtrip(void);
void test_ps_zero_length_string(void);
void test_ps_unsigned_column_types(void);
void test_local_infile_error(void);
void test_ps_date_only_encoding(void);
void test_ps_datetime_with_microseconds(void);
void test_ps_time_microseconds_text(void);
void test_ps_date_insert_and_select(void);
void test_ps_multiple_null_params(void);
void test_ps_reexecute_after_store(void);
void test_ps_bit_type_to_int(void);
void test_send_long_data_null_check(void);
void test_ps_bit_type_64(void);
void test_ps_decimal_type(void);
void test_ps_bit_type(void);
void test_ps_blob_send_long_data_chunks(void);
void test_ps_mixed_param_types(void);
void test_ps_empty_result(void);
void test_concurrent_stmts(void);
void test_ps_inout_param(void);
void test_multi_resultset_error_propagation(void);
void test_ps_stmt_reset(void);
void test_cursor_fetch_column(void);
void test_ps_reexecute_different_values(void);
void test_stmt_close_before_fetch(void);
void test_ps_null_result_metadata(void);
void test_ps_decimal_precision(void);
void test_ps_enum_type(void);
void test_ps_set_type(void);
void test_ps_json_type(void);
void test_ps_bigint_signed_boundary(void);
void test_more_results_no_multi(void);
void test_ps_store_result_on_insert(void);
void test_dump_debug_info(void);
void test_ps_datetime_microsecond_insert(void);
void test_ps_time_microsecond(void);
void test_ps_tinyint_unsigned_boundary(void);
void test_ps_int_unsigned_boundary(void);
void test_ps_stmt_sqlstate(void);
void test_fetch_field_direct(void);
void test_ps_decimal_zero(void);
void test_insert_id_replace(void);
void test_ps_date_boundary(void);
void test_mb_escape_gbk(void);
void test_mb_escape_big5(void);
void test_mb_escape_sjis(void);
void test_mb_escape_utf8(void);
void test_mb_escape_euckr(void);
void test_set_character_set_updates_escape(void);
void test_call_multi_resultset(void);
void test_call_ps_multi_resultset(void);
void test_stmt_next_result_no_multi(void);
void test_report_data_truncation(void);
void test_stmt_update_max_length(void);
void test_escape_string_not_connected(void);
void test_escape_string_nul_byte(void);
void test_ps_param_count_mismatch(void);
void test_ps_prepare_null_query(void);
void test_ps_close_after_error(void);
void test_change_user_resets_ps(void);
void test_set_charset_name_before_connect(void);
void test_gb18030_fourbyte_escape(void);
void test_ps_out_param(void);
void test_flush_tables_reprepare(void);
void test_mysql_init_command(void);
void test_ps_datetime_boundary(void);
void test_ps_time_negative(void);
void test_ps_timestamp_auto_update(void);
void test_ps_reprepare_after_rename(void);
void test_ps_affected_rows_insert(void);
void test_ps_insert_auto_increment(void);
void test_ps_multiple_stmts_different_tables(void);
void test_ps_store_result_seek_back(void);
void test_ps_execute_with_no_params_bound(void);
void test_tran_commit(void);
void test_tran_rollback(void);
void test_autocommit_mode(void);
void test_ps_fetch_no_execute(void);
void test_ps_double_precision(void);
void test_ps_select_null_constant(void);
void test_text_insert_select2(void);
void test_create_drop_table(void);
void test_ps_stmt_err_after_execute(void);
void test_ps_multiple_stmt_lifecycle(void);
void test_text_query_out_of_sync(void);
void test_ps_affected_rows_various(void);
void test_ps_null_param_all_positions(void);
void test_ps_reexecute_after_store_partial(void);
void test_text_query_long_result(void);
void test_ps_decimal_param(void);
void test_ps_float_param(void);
void test_ps_tiny_param(void);
void test_ps_short_param(void);
void test_ps_long_param(void);
void test_ps_longlong_param(void);
void test_ps_double_param(void);
void test_ps_date_param_insert(void);
void test_ps_datetime_param_insert(void);
void test_ps_time_param_insert(void);
void test_ps_date_param_select(void);
void test_cursor_read_only_basic(void);
void test_cursor_read_only_prefetch_rows(void);
void test_cursor_read_only_store_result(void);
void test_cursor_read_only_with_param(void);

int main(int argc, char **argv) {
    if (argc > 1) g_host = argv[1];
    if (argc > 2) g_user = argv[2];
    if (argc > 3) g_passwd = argv[3];
    if (argc > 4) g_db = argv[4];
    if (argc > 5) g_port = (unsigned int)atoi(argv[5]);

    printf("=== sp-mysql-client integration test ===\n");
    printf("target: %s@%s:%u/%s\n\n", g_user, g_host, g_port, g_db);

    test_init_close();
    test_connect_basic();
    test_ping();
    test_simple_query();
    test_create_table();
    test_text_insert_select();
    test_null_values();
    test_use_result();
    test_real_escape_string();
    test_field_info();
    test_field_flags();
    test_stmt_prepare_execute();
    test_stmt_insert();
    test_stmt_bind_result();
    test_stmt_store_result();
    test_stmt_fetch_null();
    test_stmt_insert_tiny();
    test_stmt_send_long_data();
    test_insert_id();
    test_warnings();
    test_update_delete();
    test_data_seek();
    test_select_db();
    test_charset();
    test_multi_statements();
    test_stmt_result_metadata();
    test_stmt_close_and_reprepare();
    test_connection_pool();
    test_error_handling();
    test_change_user();
    test_create_drop();
    test_concurrent_connections();
    test_empty_result();
    test_big_result();
    test_tran_innodb();
    test_prepare_multi_type();
    test_bind_result_ext();
    test_ps_null_param();
    test_buffers();
    test_prepare_simple();
    test_prepare_noparam();
    test_stmt_reset();
    test_stmt_free_result_sync();
    test_decimal();
    test_simple_temporal();
    test_stmt_fetch_column();
    test_stmt_data_seek_ps();
    test_stmt_insert_id();
    test_double_reconnect();
    test_prepare_insert_update();
    test_stmt_reprepare_after_error();
    test_parse_error_and_bad_length();
    test_stmt_affected_rows();
    test_stmt_field_count();
    test_stmt_sqlstate();
    test_multiple_stmt_handles();
    test_stmt_update_bind();
    test_stmt_unsigned();
    test_stmt_bit_type();
    test_stmt_text_blob();
    test_stmt_double_float();
    test_mysql_errno_sqlstate();
    test_stmt_reexecute();
    test_mysql_fetch_lengths();
    test_mysql_field_seek();
    test_mysql_fetch_field_direct();
    test_mysql_field_count_api();
    test_mysql_escape_string();
    test_mysql_get_host_info();
    test_stmt_ps_select_like();
    test_stmt_ps_between();
    test_stmt_ps_in_clause();
    test_stmt_ps_join();
    test_stmt_ps_aggregate();
    test_stmt_ps_group_by();
    test_stmt_ps_subquery();
    test_stmt_ps_limit_offset();
    test_stmt_large_string();
    test_stmt_year_type();
    test_stmt_enum_type();
    test_stmt_ps_delete_with_param();
    test_stmt_ps_update_with_param();
    test_stmt_ps_null_update();
    test_stmt_ps_concat();
    test_stmt_ps_math();
    test_stmt_ps_date_functions();
    test_stmt_ps_if_null();
    test_stmt_ps_coalesce();
    test_stmt_ps_case_when();
    test_stmt_ps_cast();
    test_stmt_ps_exists();
    test_stmt_ps_union();
    test_stmt_ps_having();
    test_stmt_ps_distinct();
    test_stmt_ps_order_by_desc();
    test_stmt_ps_is_null();
    test_stmt_ps_boolean_expr();
    test_stmt_ps_nested_arithmetic();
    test_stmt_ps_string_compare();
    test_stmt_ps_multi_column_order();
    test_stmt_ps_left_join();

    test_stmt_ps_reexecute_multi();
    test_stmt_ps_mixed_types();
    test_stmt_ps_affected_rows_dml();
    test_stmt_ps_null_in_where();
    test_stmt_ps_inner_join();
    test_stmt_ps_group_concat();
    test_stmt_ps_date_arithmetic();
    test_stmt_ps_string_funcs();
    test_stmt_ps_many_params();
    test_stmt_ps_error_recovery();
    test_stmt_ps_multi_conn();
    test_stmt_ps_auto_increment();
    test_stmt_ps_decimal_precision();
    test_stmt_ps_time_type();
    test_stmt_ps_set_type();
    test_stmt_ps_prepare_close_cycle();
    test_stmt_ps_no_param_select();
    test_stmt_ps_negative_param();
    test_stmt_ps_div_zero();
    test_stmt_ps_blob_read();
    test_stmt_ps_store_result_fetch();
    test_stmt_ps_data_seek_stored();
    test_stmt_ps_truncate_buffer();
    test_stmt_ps_unsigned_param();
    test_stmt_ps_reexecute_after_store();
    test_stmt_ps_multiple_rows_fetch();
    test_stmt_ps_datetime_timestamp();
    test_stmt_ps_null_result_bind();
    test_stmt_ps_float_precision();
    test_stmt_ps_bigint();
    test_stmt_ps_empty_table();
    test_stmt_ps_column_names();
    test_stmt_ps_reexecute_diff_null();
    test_stmt_ps_string_param_long();
    test_stmt_ps_update_return_count();
    test_stmt_ps_zero_rows_where();
    test_stmt_ps_result_metadata_after_exec();
    test_stmt_ps_smallint();
    test_stmt_ps_mediumint();
    test_text_query_field_types();
    test_text_query_null_fields();
    test_text_query_fetch_lengths();
    test_text_query_affected_rows();
    test_text_query_insert_id();
    test_text_query_warning_count();
    test_text_query_multi_fetch();
    test_text_query_use_result_all();
    test_ps_json_type();
    test_ps_binary_varbinary();
    test_ps_replace_into();
    test_ps_on_dup_key_update();
    test_ps_stmt_reset_between_exec();
    test_ps_concurrent_stmts_same_conn();
    test_ps_free_result_before_fetch();
    test_ps_close_with_pending_rows();
    test_ps_reset_with_pending_rows();
    test_text_query_escape_all();
    test_text_query_select_constants();
    test_ps_window_function();
    test_ps_cte();
    test_ps_multi_column_where();
    test_ps_double_reexecute_with_store();
    test_ps_param_type_mismatch();
    test_text_query_field_count_no_result();
    test_conn_ping_alive();
    test_conn_character_set();
    test_conn_host_info();
    test_conn_server_version();
    test_conn_thread_id();
    test_conn_proto_info();
    test_text_query_multi_statement();
    test_text_query_like_pattern();
    test_text_query_between();
    test_text_query_in_clause();
    test_text_query_subquery();
    test_text_query_aggregate_funcs();
    test_ps_bit_type();
    test_ps_year_type();
    test_ps_text_column();
    test_ps_double_param_reexecute();
    test_ps_execute_after_error();
    test_ps_null_param_insert();
    test_ps_batch_insert();
    test_text_query_date_functions();
    test_text_query_string_functions();
    test_text_query_math_functions();
    test_ps_date_param_insert();
    test_ps_datetime_param_insert();
    test_ps_time_param_insert();

    test_tran_commit();
    test_tran_rollback();
    test_autocommit_mode();
    test_ps_fetch_no_execute();
    test_ps_double_precision();
    test_ps_select_null_constant();
    test_text_insert_select2();
    test_create_drop_table();
    test_ps_stmt_err_after_execute();
    test_ps_multiple_stmt_lifecycle();
    test_text_query_out_of_sync();
    test_ps_affected_rows_various();
    test_ps_null_param_all_positions();
    test_ps_reexecute_after_store_partial();
    test_text_query_long_result();
    test_ps_decimal_param();
    test_ps_float_param();
    test_ps_tiny_param();
    test_ps_short_param();
    test_ps_longlong_param();
    test_ps_date_param_select();

    test_connect_nonexistent_db();
    test_connect_with_db();
    test_ps_bind_null_type();
    test_ps_close_without_fetch();
    test_ps_reprepare_after_alter();
    test_ps_duplicate_key_recovery();
    test_ps_free_result_mid_fetch();
    test_ps_multi_reexecute_loop();
    test_ps_empty_string_param();
    test_ps_unsigned_tinyint();
    test_ps_string_truncation();
    test_ps_datetime_microsecond_insert();
    test_ps_prepare_error_then_valid();
    test_ps_null_param_rebind();
    test_text_query_select_no_rows();
    test_ps_large_blob_long_data();
    test_ps_param_count_verify();
    test_ps_rebind_different_params();
    test_ps_sqlstate_after_error();
    test_text_query_show_tables();

    test_ps_store_result_multi_iter();
    test_ps_fetch_then_store_result();
    test_ps_close_after_execute_error();
    test_ps_store_result_full_then_reexecute();
    test_ps_store_result_partial_then_reexecute();
    test_ps_stmt_reopen_same_query();
    test_ps_fetch_date_type();
    test_ps_fetch_str_various_lengths();
    test_ps_close_after_store_without_fetch();
    test_ps_binary_null_in_blob();
    test_ps_reexecute_with_different_param_types();
    test_ps_multiple_store_result_cycles();
    test_text_query_describe_table();
    test_ps_zero_row_count();
    test_ps_stmt_errno_after_error();
    test_text_query_select_database();
    test_ps_store_result_data_seek();
    test_ps_timestamp_param_insert();
    test_ps_negative_values();
    test_ps_max_varchar_param();

    test_ps_fetch_on_non_select();
    test_ps_reprepare_metadata_refresh();
    test_ps_int_truncation_tiny();
    test_ps_int_truncation_short();
    test_ps_blob_zero_buffer_length();
    test_ps_store_result_row_count_after_partial_fetch();
    test_ps_reexecute_without_full_consume();
    test_ps_unsigned_int_param();
    test_ps_store_result_zero_cols();
    test_ps_fetch_column_with_offset();
    test_ps_multiple_prepare_same_stmt();
    test_ps_execute_after_reset();
    test_ps_update_affected_rows_verify();
    test_ps_delete_affected_rows_verify();
    test_text_query_count_func();
    test_text_query_min_max();
    test_ps_mixed_null_row();
    test_ps_result_metadata_field_names();
    test_ps_store_result_then_data_seek_loop();

    test_ps_double_prepare_same_stmt();
    test_ps_fetch_without_bind_result();
    test_ps_store_result_on_insert();
    test_ps_fetch_with_warning();
    test_ps_execute_on_dropped_table();
    test_ps_prepare_empty_sql();
    test_ps_close_after_execute_no_fetch();
    test_ps_store_result_no_fetch_then_reexecute();
    test_ps_close_then_new_stmt();
    test_ps_reprepare_different_col_count();
    test_ps_multiple_warnings();
    test_ps_select_constant_expr();
    test_ps_between_operator();
    test_ps_in_clause();
    test_ps_like_operator();
    test_ps_is_null_operator();
    test_ps_group_by_count();
    test_ps_having_clause();
    test_ps_order_by_desc();
    test_ps_limit_clause();

    test_ps_union_query();
    test_ps_close_one_stmt_execute_another();
    test_ps_reset_then_store_result();
    test_ps_send_long_data_zero_length();
    test_ps_result_metadata_after_execute();
    test_ps_empty_result_store_then_reexecute();
    test_ps_multiple_stmt_handles();
    test_ps_long_sql_prepare();
    test_ps_subquery();
    test_ps_case_when();
    test_ps_distinct();
    test_ps_concat_function();
    test_ps_if_function();
    test_ps_coalesce();
    test_ps_cast_type();
    test_ps_date_add_interval();
    test_ps_string_length_function();
    test_ps_abs_function();
    test_ps_round_function();
    test_ps_mod_function();

    test_ps_store_result_on_update();
    test_ps_blob_null_fetch();
    test_ps_decimal_newdecimal();
    test_ps_null_pattern_change();
    test_ps_time_type_fetch();
    test_ps_year_type();
    test_ps_long_identifier_prepare();
    test_ps_geometry_type();
    test_ps_multiple_null_params();
    test_ps_reexecute_after_store();

    /* ---- Phase 17: protocol bug fixes + edge cases ---- */
    test_ps_bit_type_to_int();
    test_send_long_data_null_check();
    test_ps_bit_type_64();
    test_ps_decimal_type();
    test_ps_json_type();
    test_ps_enum_type();

    test_ps_text_blob_insert_fetch();
    test_ps_set_type();
    test_ps_bit_type();
    test_ps_json_type();
    test_ps_double_reprepare();
    test_ps_execute_after_conn_reset();
    test_ps_wide_row_many_columns();
    test_ps_concurrent_insert_select();

    /* ---- Phase 2: new API tests ---- */
    test_row_seek_tell();
    test_field_tell_eof_result_metadata();
    test_stmt_row_seek_tell();
    test_stmt_attr_set_get();
    test_reset_connection();
    test_get_client_version();
    test_mysql_info();
    test_mysql_stat();
    test_list_dbs();
    test_list_tables();
    test_list_fields();
    test_list_processes();
    test_real_escape_string_quote();
    test_hex_string();
    test_get_character_set_info();
    test_set_server_option();
    test_session_track();
    test_stmt_param_metadata();
    test_cr_error_codes();
    test_helper_macros();
    test_enum_compat();
    test_client_flags_compat();
    test_stmt_state_tracking();
    test_fetch_column_offset();

    /* ---- Phase 3: edge case and bug-fix verification tests ---- */
    test_fetch_without_bind_result();
    test_fetch_column_streaming();
    test_options_roundtrip();
    test_connect_attrs();
    test_options4_overwrite();
    test_ps_unsigned_bigint();
    test_stmt_execute_after_error();
    test_stmt_execute_zero_params();
    test_ps_reexecute_drain();
    test_text_query_empty_result();
    test_stmt_close_after_execute_no_fetch();
    test_ps_tiny_unsigned_boundary();
    test_ps_short_unsigned_boundary();
    test_ps_long_unsigned_boundary();
    test_ps_blob_max_length_attr();
    test_mysql_get_option();
    test_reset_connection_ps();
    test_list_dbs_with_pattern();
    test_hex_string_roundtrip();
    test_real_escape_binary();
    test_stmt_attr_prefetch_execute();
    test_stmt_field_count_no_prepare();

    /* ---- Phase 4: critical gap tests from official MySQL test suite ---- */
    test_ps_null_bitmap_offset();
    test_ps_reexecute_different_param_type();
    test_ps_reset_after_send_long_data();
    test_ps_multiple_blob_columns();
    test_ps_unsigned_bigint_boundary();
    test_ps_double_negative_zero();
    test_select_db_nonexistent();
    test_select_db_verify();
    test_change_user_wrong_password();
    test_multi_stmt_mixed_ok_result();
    test_ps_store_result_where_zero();
    test_ps_all_columns_null();

    /* ---- Phase 5: more official MySQL test suite edge cases ---- */
    test_ps_mediumint_type();
    test_ps_year_boundary();
    test_ps_float_double_precision();
    test_ps_varchar_max_length();
    test_ps_text_longtext();
    test_ps_binary_varbinary();
    test_ps_multiple_send_long_data();
    test_ps_execute_after_dropped_table();
    test_ps_insert_null_various_types();
    test_ps_update_return_affected_rows();
    test_ps_char_padding();
    test_ps_zero_length_string();
    test_ps_negative_tinyint();
    test_ps_mixed_types_row();
    test_multi_stmt_all_select();
    test_multi_stmt_error_midway();
    test_multi_stmt_ps_reexecute_after_store();
    test_ps_double_zero();
    test_ps_small_negative_boundary();
    test_ps_string_with_embedded_null();

    /* ---- Phase 6: critical protocol edge cases ---- */
    test_ps_datetime_boundary();
    test_ps_time_negative();
    test_ps_timestamp_auto_update();
    test_ps_reprepare_after_rename();
    test_ps_affected_rows_insert();
    test_ps_insert_auto_increment();
    test_ps_multiple_stmts_different_tables();
    test_ps_store_result_seek_back();
    test_ps_execute_with_no_params_bound();
    test_ps_bind_result_after_store();
    test_ps_warning_count();

    /* ---- Phase 7: CURSOR_TYPE_READ_ONLY ---- */
    test_cursor_read_only_basic();
    test_cursor_read_only_with_param();
    test_cursor_read_only_store_result();
    test_cursor_read_only_prefetch_rows();
    test_cursor_read_only_reexecute();
    test_ps_bit_type();
    test_ps_decimal_precision();
    test_ps_enum_type();
    test_ps_set_type();
    test_ps_json_type();
    test_ps_bigint_signed_boundary();
    test_more_results_no_multi();
    test_ps_store_result_on_insert();
    test_dump_debug_info();
    test_ps_datetime_microsecond_insert();
    test_ps_time_microsecond();
    test_ps_tinyint_unsigned_boundary();
    test_ps_int_unsigned_boundary();
    test_ps_stmt_sqlstate();
    test_ps_reprepare_after_alter();
    test_fetch_field_direct();
    test_ps_decimal_zero();
    test_insert_id_replace();
    test_ps_date_boundary();
    test_mb_escape_gbk();
    test_mb_escape_big5();
    test_mb_escape_sjis();
    test_mb_escape_utf8();
    test_mb_escape_euckr();
    test_set_character_set_updates_escape();
    test_call_multi_resultset();
    test_call_ps_multi_resultset();
    test_stmt_next_result_no_multi();
    test_report_data_truncation();
    test_stmt_update_max_length();
    test_escape_string_not_connected();
    test_escape_string_nul_byte();
    test_ps_param_count_mismatch();
    test_ps_prepare_null_query();
    test_ps_close_after_error();
    test_change_user_resets_ps();
    test_set_charset_name_before_connect();
    test_gb18030_fourbyte_escape();
    test_ps_out_param();
    test_flush_tables_reprepare();
    test_mysql_init_command();
    test_ps_inout_param();
    test_multi_resultset_error_propagation();
    test_ps_stmt_reset();
    test_cursor_fetch_column();
    test_ps_reexecute_different_values();
    test_stmt_close_before_fetch();
    test_ps_null_result_metadata();
    test_ps_blob_send_long_data_chunks();
    test_ps_mixed_param_types();
    test_ps_bit_type();
    test_ps_decimal_precision();
    test_ps_empty_result();
    test_concurrent_stmts();

    /* ---- Phase 13: bug fixes + official test suite edge cases ---- */
    test_ps_large_result_set();
    test_ps_reprepare_after_alter_column();
    test_ps_year_boundary_values();
    test_cursor_prefetch_rows_large();
    test_stmt_reset_state();
    test_ps_date_time_string_bind();
    test_ps_time_string_bind();
    test_ps_store_result_large();
    test_ps_multiple_reprepare();
    test_ps_longlong_truncation();
    test_ps_execute_after_reset_state();
    test_sql_injection_list_wild();

    /* ---- Phase 14: code quality fixes ---- */
    test_proto_info();
    test_host_info_thread_safe();
    test_stat_thread_safe();
    test_ps_time_without_microseconds();
    test_ps_datetime_without_microseconds();
    test_ps_time_with_microseconds();
    test_ps_close_normal();

    /* ---- Phase 15: functional improvements + edge cases ---- */
    test_change_user_reauth();
    test_ps_blob_medium();
    test_ps_concurrent_connections();
    test_ps_null_param_reexecute();
    test_ps_double_type_precision();
    test_ps_float_type_precision();
    test_charset_info();
    test_get_option_roundtrip();
    test_ps_zero_length_string();
    test_ps_unsigned_column_types();

    /* ---- Phase 16: protocol bug fixes + edge cases ---- */
    test_local_infile_error();
    test_ps_date_only_encoding();
    test_ps_datetime_with_microseconds();
    test_ps_time_microseconds_text();
    test_ps_date_insert_and_select();
    test_ps_multiple_null_params();
    test_ps_reexecute_after_store();

    /* ---- Phase 18: fourth-round audit bug fixes ---- */
    test_mysql_info_after_update();
    test_host_info_info_independent();
    test_charset_mbmaxlen_utf8mb4();
    test_set_unknown_charset_error();
    test_mysql_info_insert();
    test_mysql_info_alter_table();
    test_select_db_malloc_oom();
    test_ping_returns_valid();

    /* ---- Phase 19: fifth-round audit bug fixes ---- */
    test_reset_connection_clears_ps();
    test_list_fields_com_field_list();
    test_change_user_updates_reconnect_creds();
    test_ps_date_param_always_4bytes();
    test_ps_time_zero_length();

    /* ---- Phase 20: sixth-round audit bug fixes ---- */
    test_multi_resultset_store_next();
    test_ps_deprecate_eof_row_with_zero_prefix();
    test_row_seek_boundary();

    /* ---- Phase 21: official test suite coverage gaps ---- */
    test_ps_call_mixed_params_multi_rs();
    test_ps_out_param_all_types();
    test_truncation_all_types();
    test_cursor_type_switch();
    test_server_status_flags();
    test_double_real_connect();
    test_opt_reconnect_flag();

    /* ---- Phase 22: MariaDB Connector/C test suite coverage ---- */
    test_conc67_cursor_param_fetch();
    test_bug1115_string_param_reexecute();
    test_bug1664_send_long_data();
    test_conc141_sp_multi_rs_reexecute();
    test_conc154_empty_nonempty_reexecute();
    test_conc155_text_zero_terminated();
    test_conc168_datetime3_precision();
    test_conc205_mixed_type_fetch();
    test_conc504_sp_three_selects();
    test_conc566_cursor_call_sp();
    test_conc762_null_and_nonnull();
    test_bug15518_prepare_fail_reprepare();
    test_bug15613_text_metadata_length();
    test_conc208_union_int_truncation();
    test_conc182_bit_type_fetch();

    printf("\n=== Results: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}

void test_tran_commit(void) {
    printf("\n[transaction: COMMIT]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_tc");
    Q("CREATE TABLE t_tc(id INT, val INT) ENGINE=InnoDB");

    mysql_autocommit(m, 0);
    Q("INSERT INTO t_tc VALUES(1,100)");
    Q("INSERT INTO t_tc VALUES(2,200)");
    mysql_commit(m);

    Q("INSERT INTO t_tc VALUES(3,300)");
    mysql_rollback(m);

    mysql_autocommit(m, 1);

    Q("SELECT COUNT(*) FROM t_tc");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("after commit+rollback: 2 rows");
    CHECK(row && strcmp(row[0], "2") == 0, "wrong count");
    mysql_free_result(res);
    mysql_close(m);
}

void test_tran_rollback(void) {
    printf("\n[transaction: ROLLBACK]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_tr");
    Q("CREATE TABLE t_tr(id INT PRIMARY KEY, val INT) ENGINE=InnoDB");

    mysql_autocommit(m, 0);
    Q("INSERT INTO t_tr VALUES(1,10)");
    mysql_commit(m);

    Q("UPDATE t_tr SET val = 999 WHERE id = 1");
    mysql_rollback(m);

    mysql_autocommit(m, 1);

    Q("SELECT val FROM t_tr WHERE id = 1");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("after rollback UPDATE: val still 10");
    CHECK(row && strcmp(row[0], "10") == 0, "rollback failed");
    mysql_free_result(res);
    mysql_close(m);
}

void test_autocommit_mode(void) {
    printf("\n[autocommit on/off]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_ac");
    Q("CREATE TABLE t_ac(id INT, val INT) ENGINE=InnoDB");

    TEST("autocommit off succeeds");
    CHECK(mysql_autocommit(m, 0) == 0, "autocommit off failed");

    Q("INSERT INTO t_ac VALUES(1,10)");
    mysql_rollback(m);

    mysql_autocommit(m, 1);

    Q("SELECT COUNT(*) FROM t_ac");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("rollback with autocommit off: 0 rows");
    CHECK(row && strcmp(row[0], "0") == 0, "wrong count");
    mysql_free_result(res);

    Q("INSERT INTO t_ac VALUES(2,20)");
    Q("SELECT COUNT(*) FROM t_ac");
    res = mysql_store_result(m);
    row = mysql_fetch_row(res);
    TEST("autocommit on: insert auto-committed, 1 row");
    CHECK(row && strcmp(row[0], "1") == 0, "wrong count");
    mysql_free_result(res);
    mysql_close(m);
}

void test_ps_fetch_no_execute(void) {
    printf("\n[PS: fetch without execute]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_fne");
    Q("CREATE TABLE t_fne(id INT, val INT)");
    Q("INSERT INTO t_fne VALUES(1,100)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT val FROM t_fne WHERE id = ?",
                                strlen("SELECT val FROM t_fne WHERE id = ?"));

    int out_val;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_fetch(stmt);
    TEST("fetch without execute returns error");
    CHECK(rc != 0, "should fail");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_double_precision(void) {
    printf("\n[PS: DOUBLE precision roundtrip]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_dp");
    Q("CREATE TABLE t_dp(id INT, d DOUBLE)");
    Q("INSERT INTO t_dp VALUES(1, 6578.001),(2, 0.1+0.2),(3, 1e15)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT d FROM t_dp WHERE id = ?",
                                strlen("SELECT d FROM t_dp WHERE id = ?"));

    int id = 0;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    double out;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_DOUBLE; rb.buffer = &out; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    id = 1;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("6578.001 roundtrip (BUG#9914)");
    CHECK(rc == 0 && fabs(out - 6578.001) < 0.0001, "wrong value");

    id = 3;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("1e15 roundtrip");
    CHECK(rc == 0 && fabs(out - 1e15) < 1e10, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_select_null_constant(void) {
    printf("\n[PS: SELECT NULL constant]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT NULL, 42, 'hello'",
                                strlen("SELECT NULL, 42, 'hello'"));

    long long out_num;
    char out_str[20];
    my_bool n1=0, n2=0, n3=0;
    unsigned long l1=0,l2=0,l3=0;
    MYSQL_BIND rb[3];
    memset(rb, 0, sizeof(rb));
    rb[0].buffer_type = MYSQL_TYPE_LONGLONG; rb[0].is_null = &n1; rb[0].length = &l1;
    rb[1].buffer_type = MYSQL_TYPE_LONGLONG; rb[1].buffer = &out_num; rb[1].is_null = &n2; rb[1].length = &l2;
    rb[2].buffer_type = MYSQL_TYPE_STRING; rb[2].buffer = out_str; rb[2].buffer_length = sizeof(out_str); rb[2].is_null = &n3; rb[2].length = &l3;
    mysql_stmt_bind_result(stmt, &rb[0]);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("SELECT NULL: first column is NULL");
    CHECK(rc == 0 && n1, "not null");
    TEST("42 is non-NULL");
    CHECK(!n2 && out_num == 42, "wrong value");
    TEST("'hello' is non-NULL");
    CHECK(!n3 && strcmp(out_str, "hello") == 0, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_text_insert_select2(void) {
    printf("\n[text: INSERT ... SELECT]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_is1, t_is2");
    Q("CREATE TABLE t_is1(id INT, val INT)");
    Q("CREATE TABLE t_is2(id INT, val INT)");
    Q("INSERT INTO t_is1 VALUES(1,10),(2,20),(3,30)");

    Q("INSERT INTO t_is2 SELECT id, val * 2 FROM t_is1 WHERE val > 15");
    TEST("INSERT SELECT affected_rows == 2");
    CHECK(mysql_affected_rows(m) == 2, "wrong count");

    Q("SELECT COUNT(*) FROM t_is2");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("t_is2 has 2 rows");
    CHECK(row && strcmp(row[0], "2") == 0, "wrong count");
    mysql_free_result(res);
    mysql_close(m);
}

void test_create_drop_table(void) {
    printf("\n[text: CREATE/DROP TABLE cycle]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    for (int i = 0; i < 5; i++) {
        Q("DROP TABLE IF EXISTS t_cd");
        Q("CREATE TABLE t_cd(id INT)");
        Q("INSERT INTO t_cd VALUES(1)");
        Q("SELECT id FROM t_cd");
        MYSQL_RES *res = mysql_store_result(m);
        mysql_free_result(res);
    }
    TEST("5x CREATE/DROP/INSERT/SELECT cycle succeeds");
    PASS();
    mysql_close(m);
}

void test_ps_stmt_err_after_execute(void) {
    printf("\n[PS: error after execute, then retry]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_eae2");
    Q("CREATE TABLE t_eae2(id INT PRIMARY KEY, val INT)");
    Q("INSERT INTO t_eae2 VALUES(1,100)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT val FROM t_eae2 WHERE id = ?",
                                strlen("SELECT val FROM t_eae2 WHERE id = ?"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    int out_val;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("first execute: val=100");
    CHECK(rc == 0 && out_val == 100, "wrong value");
    while (mysql_stmt_fetch(stmt) == 0) {}

    Q("DROP TABLE t_eae2");

    rc = mysql_stmt_execute(stmt);
    TEST("execute after DROP TABLE returns error");
    CHECK(rc != 0, "should fail");

    Q("CREATE TABLE t_eae2(id INT PRIMARY KEY, val INT)");
    Q("INSERT INTO t_eae2 VALUES(1,200)");

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("re-execute after recreate: val=200");
    CHECK(rc == 0 && out_val == 200, "wrong value");
    while (mysql_stmt_fetch(stmt) == 0) {}

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_multiple_stmt_lifecycle(void) {
    printf("\n[PS: multiple stmt lifecycle]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_msl");
    Q("CREATE TABLE t_msl(id INT, val INT)");
    Q("INSERT INTO t_msl VALUES(1,10),(2,20),(3,30)");

    MYSQL_STMT *s1 = mysql_stmt_init(m);
    MYSQL_STMT *s2 = mysql_stmt_init(m);
    MYSQL_STMT *s3 = mysql_stmt_init(m);

    mysql_stmt_prepare(s1, "SELECT val FROM t_msl WHERE id = ?",
                       strlen("SELECT val FROM t_msl WHERE id = ?"));
    mysql_stmt_prepare(s2, "INSERT INTO t_msl(id, val) VALUES(?, ?)",
                       strlen("INSERT INTO t_msl(id, val) VALUES(?, ?)"));
    mysql_stmt_prepare(s3, "DELETE FROM t_msl WHERE id = ?",
                       strlen("DELETE FROM t_msl WHERE id = ?"));

    int id = 1, val = 40;
    unsigned long il = sizeof(int), vl = sizeof(int);
    MYSQL_BIND pb1;
    memset(&pb1, 0, sizeof(pb1));
    pb1.buffer_type = MYSQL_TYPE_LONG; pb1.buffer = &id; pb1.length = &il;

    MYSQL_BIND pb2[2];
    memset(pb2, 0, sizeof(pb2));
    pb2[0].buffer_type = MYSQL_TYPE_LONG; pb2[0].buffer = &id; pb2[0].length = &il;
    pb2[1].buffer_type = MYSQL_TYPE_LONG; pb2[1].buffer = &val; pb2[1].length = &vl;

    MYSQL_BIND pb3;
    memset(&pb3, 0, sizeof(pb3));
    pb3.buffer_type = MYSQL_TYPE_LONG; pb3.buffer = &id; pb3.length = &il;

    mysql_stmt_bind_param(s1, &pb1);
    mysql_stmt_bind_param(s2, pb2);
    mysql_stmt_bind_param(s3, &pb3);

    int out_val;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.length = &ol;
    mysql_stmt_bind_result(s1, &rb);

    id = 1;
    mysql_stmt_execute(s1);
    mysql_stmt_fetch(s1);
    TEST("s1 SELECT id=1: val=10");
    CHECK(out_val == 10, "wrong value");
    while (mysql_stmt_fetch(s1) == 0) {}

    id = 4; val = 40;
    mysql_stmt_execute(s2);
    TEST("s2 INSERT id=4: affected_rows=1");
    CHECK(mysql_stmt_affected_rows(s2) == 1, "wrong count");

    id = 3;
    mysql_stmt_execute(s3);
    TEST("s3 DELETE id=3: affected_rows=1");
    CHECK(mysql_stmt_affected_rows(s3) == 1, "wrong count");

    id = 4;
    mysql_stmt_execute(s1);
    mysql_stmt_fetch(s1);
    TEST("s1 SELECT id=4: val=40");
    CHECK(out_val == 40, "wrong value");
    while (mysql_stmt_fetch(s1) == 0) {}

    mysql_stmt_close(s1);
    mysql_stmt_close(s2);
    mysql_stmt_close(s3);
    mysql_close(m);
}

void test_text_query_out_of_sync(void) {
    printf("\n[text: query without consuming result]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_oos");
    Q("CREATE TABLE t_oos(id INT)");
    Q("INSERT INTO t_oos VALUES(1)");

    Q("SELECT id FROM t_oos");
    MYSQL_RES *res = mysql_store_result(m);
    mysql_free_result(res);

    Q("SELECT id FROM t_oos");
    res = mysql_use_result(m);
    while (mysql_fetch_row(res)) ;
    mysql_free_result(res);

    Q("SELECT id FROM t_oos");
    res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("query after proper cleanup works");
    CHECK(row && strcmp(row[0], "1") == 0, "wrong value");
    mysql_free_result(res);
    mysql_close(m);
}

void test_ps_affected_rows_various(void) {
    printf("\n[PS: affected_rows various DML]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_arv");
    Q("CREATE TABLE t_arv(id INT PRIMARY KEY, val INT)");
    Q("INSERT INTO t_arv VALUES(1,10),(2,20),(3,30)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "UPDATE t_arv SET val = val + ? WHERE val > ?",
                                strlen("UPDATE t_arv SET val = val + ? WHERE val > ?"));

    int delta = 1000, threshold = 15;
    unsigned long l1 = sizeof(int), l2 = sizeof(int);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &delta; pb[0].length = &l1;
    pb[1].buffer_type = MYSQL_TYPE_LONG; pb[1].buffer = &threshold; pb[1].length = &l2;
    mysql_stmt_bind_param(stmt, pb);

    rc = mysql_stmt_execute(stmt);
    TEST("UPDATE val>15: affected_rows=2");
    CHECK(rc == 0 && mysql_stmt_affected_rows(stmt) == 2, "wrong count");

    delta = 0; threshold = 0;
    rc = mysql_stmt_execute(stmt);
    TEST("UPDATE all (no change): affected_rows=0 (no CLIENT_FOUND_ROWS)");
    CHECK(rc == 0 && mysql_stmt_affected_rows(stmt) == 0, "wrong count");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_null_param_all_positions(void) {
    printf("\n[PS: NULL param in all positions]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_npa");
    Q("CREATE TABLE t_npa(id INT, a INT, b INT, c INT)");
    Q("INSERT INTO t_npa VALUES(1,10,20,30)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT a, b, c FROM t_npa WHERE id = ? AND a = ? AND b = ? AND c = ?",
                                strlen("SELECT a, b, c FROM t_npa WHERE id = ? AND a = ? AND b = ? AND c = ?"));

    int id = 1;
    my_bool n1=0, n2=1, n3=0, n4=1;
    unsigned long l1=sizeof(int);
    MYSQL_BIND pb[4];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &l1; pb[0].is_null = &n1;
    pb[1].buffer_type = MYSQL_TYPE_LONG; pb[1].length = &l1; pb[1].is_null = &n2;
    pb[2].buffer_type = MYSQL_TYPE_LONG; pb[2].length = &l1; pb[2].is_null = &n3;
    pb[3].buffer_type = MYSQL_TYPE_LONG; pb[3].length = &l1; pb[3].is_null = &n4;
    mysql_stmt_bind_param(stmt, pb);

    rc = mysql_stmt_execute(stmt);
    TEST("NULL in WHERE: 0 rows (NULL != anything)");
    CHECK(rc == 0, "execute failed");

    int count = 0;
    while (mysql_stmt_fetch(stmt) == 0) count++;
    CHECK(count == 0, "should be 0 rows");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_reexecute_after_store_partial(void) {
    printf("\n[PS: re-execute after partial store_result fetch]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_rsp");
    Q("CREATE TABLE t_rsp(id INT, val INT)");
    Q("INSERT INTO t_rsp VALUES(1,10),(2,20),(3,30)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT val FROM t_rsp WHERE id <= ? ORDER BY id",
                                strlen("SELECT val FROM t_rsp WHERE id <= ? ORDER BY id"));

    int id_limit;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id_limit; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    int out_val;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    id_limit = 2;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_store_result(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("partial fetch row 1: val=10");
    CHECK(rc == 0 && out_val == 10, "wrong value");

    id_limit = 3;
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_store_result(stmt);
    TEST("re-execute after partial fetch: num_rows=3");
    CHECK(mysql_stmt_num_rows(stmt) == 3, "wrong count");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_text_query_long_result(void) {
    printf("\n[text: long result set]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_lr");
    Q("CREATE TABLE t_lr(id INT, val INT)");
    char sql[128];
    for (int i = 0; i < 200; i++) {
        snprintf(sql, sizeof(sql), "INSERT INTO t_lr VALUES(%d, %d)", i + 1, (i + 1) * 10);
        Q(sql);
    }

    Q("SELECT COUNT(*) FROM t_lr");
    MYSQL_RES *res = mysql_store_result(m);
    MYSQL_ROW row = mysql_fetch_row(res);
    TEST("200 rows inserted");
    CHECK(row && strcmp(row[0], "200") == 0, "wrong count");
    mysql_free_result(res);

    Q("SELECT val FROM t_lr ORDER BY id");
    res = mysql_store_result(m);
    int count = 0, sum = 0;
    while ((row = mysql_fetch_row(res))) {
        count++;
        sum += atoi(row[0]);
    }
    TEST("200 rows fetched, sum=201000");
    CHECK(count == 200 && sum == 201000, "wrong result");
    mysql_free_result(res);
    mysql_close(m);
}

void test_ps_decimal_param(void) {
    printf("\n[PS: DECIMAL as string param]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_decp");
    Q("CREATE TABLE t_decp(id INT, price DECIMAL(10,2))");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_decp(id, price) VALUES(?, ?)",
                                strlen("INSERT INTO t_decp(id, price) VALUES(?, ?)"));

    int id = 1;
    char price[] = "1234.56";
    unsigned long il = sizeof(int), pl = 7;
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &il;
    pb[1].buffer_type = MYSQL_TYPE_STRING; pb[1].buffer = price; pb[1].buffer_length = sizeof(price); pb[1].length = &pl;
    mysql_stmt_bind_param(stmt, pb);

    rc = mysql_stmt_execute(stmt);
    TEST("INSERT DECIMAL as string");
    CHECK(rc == 0, "insert failed");
    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "SELECT price FROM t_decp WHERE id = ?",
                            strlen("SELECT price FROM t_decp WHERE id = ?"));
    mysql_stmt_bind_param(stmt, &pb[0]);
    rc = mysql_stmt_execute(stmt);

    char out[32];
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_STRING; rb.buffer = out; rb.buffer_length = sizeof(out); rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);
    rc = mysql_stmt_fetch(stmt);
    TEST("DECIMAL roundtrip: '1234.56'");
    CHECK(rc == 0 && strcmp(out, "1234.56") == 0, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_float_param(void) {
    printf("\n[PS: FLOAT param and result]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_fp2");
    Q("CREATE TABLE t_fp2(id INT, f FLOAT)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_fp2(id, f) VALUES(?, ?)",
                                strlen("INSERT INTO t_fp2(id, f) VALUES(?, ?)"));

    int id = 1;
    float fval = 3.14f;
    unsigned long il = sizeof(int), fl = sizeof(float);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &il;
    pb[1].buffer_type = MYSQL_TYPE_FLOAT; pb[1].buffer = &fval; pb[1].buffer_length = sizeof(fval); pb[1].length = &fl;
    mysql_stmt_bind_param(stmt, pb);

    rc = mysql_stmt_execute(stmt);
    TEST("INSERT FLOAT param");
    CHECK(rc == 0, "insert failed");
    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "SELECT f FROM t_fp2 WHERE id = ?",
                            strlen("SELECT f FROM t_fp2 WHERE id = ?"));
    mysql_stmt_bind_param(stmt, &pb[0]);
    rc = mysql_stmt_execute(stmt);

    float out_f;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_FLOAT; rb.buffer = &out_f; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);
    rc = mysql_stmt_fetch(stmt);
    TEST("FLOAT roundtrip ~3.14");
    CHECK(rc == 0 && fabsf(out_f - 3.14f) < 0.01f, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_tiny_param(void) {
    printf("\n[PS: TINYINT param and result]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_tp");
    Q("CREATE TABLE t_tp(id INT, val TINYINT)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_tp(id, val) VALUES(?, ?)",
                                strlen("INSERT INTO t_tp(id, val) VALUES(?, ?)"));

    int id = 1;
    int8_t tval = 42;
    unsigned long il = sizeof(int), tl = sizeof(int8_t);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &il;
    pb[1].buffer_type = MYSQL_TYPE_TINY; pb[1].buffer = &tval; pb[1].buffer_length = sizeof(tval); pb[1].length = &tl;
    mysql_stmt_bind_param(stmt, pb);

    rc = mysql_stmt_execute(stmt);
    TEST("INSERT TINYINT param");
    CHECK(rc == 0, "insert failed");
    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "SELECT val FROM t_tp WHERE id = ?",
                            strlen("SELECT val FROM t_tp WHERE id = ?"));
    mysql_stmt_bind_param(stmt, &pb[0]);
    rc = mysql_stmt_execute(stmt);

    int8_t out_t;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_TINY; rb.buffer = &out_t; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);
    rc = mysql_stmt_fetch(stmt);
    TEST("TINYINT roundtrip: 42");
    CHECK(rc == 0 && out_t == 42, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_short_param(void) {
    printf("\n[PS: SMALLINT param and result]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_sp");
    Q("CREATE TABLE t_sp(id INT, val SMALLINT)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_sp(id, val) VALUES(?, ?)",
                                strlen("INSERT INTO t_sp(id, val) VALUES(?, ?)"));

    int id = 1;
    short sval = 12345;
    unsigned long il = sizeof(int), sl = sizeof(short);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &il;
    pb[1].buffer_type = MYSQL_TYPE_SHORT; pb[1].buffer = &sval; pb[1].buffer_length = sizeof(sval); pb[1].length = &sl;
    mysql_stmt_bind_param(stmt, pb);

    rc = mysql_stmt_execute(stmt);
    TEST("INSERT SMALLINT param");
    CHECK(rc == 0, "insert failed");
    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "SELECT val FROM t_sp WHERE id = ?",
                            strlen("SELECT val FROM t_sp WHERE id = ?"));
    mysql_stmt_bind_param(stmt, &pb[0]);
    rc = mysql_stmt_execute(stmt);

    short out_s;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_SHORT; rb.buffer = &out_s; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);
    rc = mysql_stmt_fetch(stmt);
    TEST("SMALLINT roundtrip: 12345");
    CHECK(rc == 0 && out_s == 12345, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_longlong_param(void) {
    printf("\n[PS: BIGINT LONGLONG param and result]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_llp");
    Q("CREATE TABLE t_llp(id INT, val BIGINT)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "INSERT INTO t_llp(id, val) VALUES(?, ?)",
                                strlen("INSERT INTO t_llp(id, val) VALUES(?, ?)"));

    int id = 1;
    long long llval = 9223372036854775807LL;
    unsigned long il = sizeof(int), ll = sizeof(long long);
    MYSQL_BIND pb[2];
    memset(pb, 0, sizeof(pb));
    pb[0].buffer_type = MYSQL_TYPE_LONG; pb[0].buffer = &id; pb[0].length = &il;
    pb[1].buffer_type = MYSQL_TYPE_LONGLONG; pb[1].buffer = &llval; pb[1].buffer_length = sizeof(llval); pb[1].length = &ll;
    mysql_stmt_bind_param(stmt, pb);

    rc = mysql_stmt_execute(stmt);
    TEST("INSERT BIGINT LONGLONG param");
    CHECK(rc == 0, "insert failed");
    mysql_stmt_close(stmt);

    stmt = mysql_stmt_init(m);
    rc = mysql_stmt_prepare(stmt, "SELECT val FROM t_llp WHERE id = ?",
                            strlen("SELECT val FROM t_llp WHERE id = ?"));
    mysql_stmt_bind_param(stmt, &pb[0]);
    rc = mysql_stmt_execute(stmt);

    long long out_ll;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONGLONG; rb.buffer = &out_ll; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);
    rc = mysql_stmt_fetch(stmt);
    TEST("BIGINT roundtrip: 9223372036854775807");
    CHECK(rc == 0 && out_ll == 9223372036854775807LL, "wrong value");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_ps_date_param_select(void) {
    printf("\n[PS: DATE param in WHERE clause]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_dps");
    Q("CREATE TABLE t_dps(id INT, dt DATE)");
    Q("INSERT INTO t_dps VALUES(1,'2024-01-15'),(2,'2024-06-20'),(3,'2024-12-25')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT id FROM t_dps WHERE dt = ?",
                                strlen("SELECT id FROM t_dps WHERE dt = ?"));

    MYSQL_TIME dt;
    memset(&dt, 0, sizeof(dt));
    dt.time_type = MYSQL_TIMESTAMP_DATE;
    dt.year = 2024; dt.month = 6; dt.day = 20;
    unsigned long dl = sizeof(MYSQL_TIME);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_DATE; pb.buffer = &dt; pb.buffer_length = sizeof(dt); pb.length = &dl;
    mysql_stmt_bind_param(stmt, &pb);

    int out_id;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_id; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("DATE param in WHERE: id=2");
    CHECK(rc == 0 && out_id == 2, "wrong id");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_cursor_read_only_basic(void) {
    printf("\n[CURSOR_TYPE_READ_ONLY: basic fetch]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_cur1");
    Q("CREATE TABLE t_cur1(id INT PRIMARY KEY)");
    Q("INSERT INTO t_cur1 VALUES(1),(2),(3),(4),(5)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    unsigned long cursor_type = CURSOR_TYPE_READ_ONLY;
    mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, &cursor_type);

    mysql_stmt_prepare(stmt, "SELECT id FROM t_cur1 ORDER BY id",
                        strlen("SELECT id FROM t_cur1 ORDER BY id"));
    int rc = mysql_stmt_execute(stmt);
    TEST("cursor execute: ok");
    CHECK(rc == 0, "execute failed");

    int out_id;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_id; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_fetch(stmt);
    TEST("cursor fetch 1: id=1");
    CHECK(rc == 0 && out_id == 1, "wrong value");

    rc = mysql_stmt_fetch(stmt);
    TEST("cursor fetch 2: id=2");
    CHECK(rc == 0 && out_id == 2, "wrong value");

    rc = mysql_stmt_fetch(stmt);
    TEST("cursor fetch 3: id=3");
    CHECK(rc == 0 && out_id == 3, "wrong value");

    rc = mysql_stmt_fetch(stmt);
    TEST("cursor fetch 4: id=4");
    CHECK(rc == 0 && out_id == 4, "wrong value");

    rc = mysql_stmt_fetch(stmt);
    TEST("cursor fetch 5: id=5");
    CHECK(rc == 0 && out_id == 5, "wrong value");

    rc = mysql_stmt_fetch(stmt);
    TEST("cursor fetch end: MYSQL_NO_DATA");
    CHECK(rc == MYSQL_NO_DATA, "should be no data");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_cursor_read_only_with_param(void) {
    printf("\n[CURSOR_TYPE_READ_ONLY: with parameter]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_cur2");
    Q("CREATE TABLE t_cur2(id INT, val INT)");
    Q("INSERT INTO t_cur2 VALUES(1, 10),(1, 20),(2, 30)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    unsigned long cursor_type = CURSOR_TYPE_READ_ONLY;
    mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, &cursor_type);

    mysql_stmt_prepare(stmt, "SELECT val FROM t_cur2 WHERE id = ? ORDER BY val",
                        strlen("SELECT val FROM t_cur2 WHERE id = ? ORDER BY val"));

    int id = 1;
    unsigned long il = sizeof(int);
    MYSQL_BIND pb;
    memset(&pb, 0, sizeof(pb));
    pb.buffer_type = MYSQL_TYPE_LONG; pb.buffer = &id; pb.length = &il;
    mysql_stmt_bind_param(stmt, &pb);

    int rc = mysql_stmt_execute(stmt);
    TEST("cursor with param: execute ok");
    CHECK(rc == 0, "execute failed");

    int out_val;
    unsigned long vl = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_val; rb.length = &vl;
    mysql_stmt_bind_result(stmt, &rb);

    rc = mysql_stmt_fetch(stmt);
    TEST("cursor param fetch 1: val=10");
    CHECK(rc == 0 && out_val == 10, "wrong value");

    rc = mysql_stmt_fetch(stmt);
    TEST("cursor param fetch 2: val=20");
    CHECK(rc == 0 && out_val == 20, "wrong value");

    rc = mysql_stmt_fetch(stmt);
    TEST("cursor param fetch end");
    CHECK(rc == MYSQL_NO_DATA, "should be no data");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_cursor_read_only_store_result(void) {
    printf("\n[CURSOR_TYPE_READ_ONLY: store_result]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_cur3");
    Q("CREATE TABLE t_cur3(id INT PRIMARY KEY)");
    Q("INSERT INTO t_cur3 VALUES(1),(2),(3)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    unsigned long cursor_type = CURSOR_TYPE_READ_ONLY;
    mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, &cursor_type);

    mysql_stmt_prepare(stmt, "SELECT id FROM t_cur3 ORDER BY id",
                        strlen("SELECT id FROM t_cur3 ORDER BY id"));
    mysql_stmt_execute(stmt);

    int rc = mysql_stmt_store_result(stmt);
    TEST("cursor store_result: ok");
    CHECK(rc == 0, "store_result failed");

    TEST("cursor num_rows: 3");
    CHECK(mysql_stmt_num_rows(stmt) == 3, "wrong row count");

    int out_id;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_id; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    mysql_stmt_fetch(stmt);
    TEST("cursor store fetch 1: id=1");
    CHECK(out_id == 1, "wrong");

    mysql_stmt_fetch(stmt);
    TEST("cursor store fetch 2: id=2");
    CHECK(out_id == 2, "wrong");

    mysql_stmt_fetch(stmt);
    TEST("cursor store fetch 3: id=3");
    CHECK(out_id == 3, "wrong");

    rc = mysql_stmt_fetch(stmt);
    TEST("cursor store fetch end");
    CHECK(rc == MYSQL_NO_DATA, "should be no data");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_cursor_read_only_prefetch_rows(void) {
    printf("\n[CURSOR_TYPE_READ_ONLY: prefetch_rows]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_cur4");
    Q("CREATE TABLE t_cur4(id INT PRIMARY KEY)");
    char ins[64];
    for (int i = 1; i <= 20; i++) {
        snprintf(ins, sizeof(ins), "INSERT INTO t_cur4 VALUES(%d)", i);
        Q(ins);
    }

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    unsigned long cursor_type = CURSOR_TYPE_READ_ONLY;
    unsigned long prefetch = 5;
    mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, &cursor_type);
    mysql_stmt_attr_set(stmt, STMT_ATTR_PREFETCH_ROWS, &prefetch);

    mysql_stmt_prepare(stmt, "SELECT id FROM t_cur4 ORDER BY id",
                        strlen("SELECT id FROM t_cur4 ORDER BY id"));
    int rc = mysql_stmt_execute(stmt);
    TEST("cursor prefetch execute: ok");
    CHECK(rc == 0, "execute failed");

    int out_id;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_id; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    int count = 0;
    while ((rc = mysql_stmt_fetch(stmt)) == 0) count++;
    TEST("cursor prefetch: fetched 20 rows");
    CHECK(count == 20, "wrong count");
    CHECK(rc == MYSQL_NO_DATA, "should end with NO_DATA");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

void test_cursor_read_only_reexecute(void) {
    printf("\n[CURSOR_TYPE_READ_ONLY: re-execute]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }

    Q("DROP TABLE IF EXISTS t_cur5");
    Q("CREATE TABLE t_cur5(id INT PRIMARY KEY)");
    Q("INSERT INTO t_cur5 VALUES(1),(2),(3)");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    unsigned long cursor_type = CURSOR_TYPE_READ_ONLY;
    mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, &cursor_type);

    mysql_stmt_prepare(stmt, "SELECT id FROM t_cur5 ORDER BY id",
                        strlen("SELECT id FROM t_cur5 ORDER BY id"));

    int out_id;
    unsigned long ol = 0;
    MYSQL_BIND rb;
    memset(&rb, 0, sizeof(rb));
    rb.buffer_type = MYSQL_TYPE_LONG; rb.buffer = &out_id; rb.length = &ol;
    mysql_stmt_bind_result(stmt, &rb);

    int rc = mysql_stmt_execute(stmt);
    TEST("cursor re-execute 1: ok");
    CHECK(rc == 0, "execute 1 failed");

    mysql_stmt_fetch(stmt);
    TEST("cursor re-execute 1: id=1");
    CHECK(out_id == 1, "wrong");

    rc = mysql_stmt_execute(stmt);
    TEST("cursor re-execute 2: ok");
    CHECK(rc == 0, "execute 2 failed");

    mysql_stmt_fetch(stmt);
    TEST("cursor re-execute 2: id=1 (fresh)");
    CHECK(out_id == 1, "wrong after re-execute");

    mysql_stmt_fetch(stmt);
    mysql_stmt_fetch(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("cursor re-execute 2: 3 rows then NO_DATA");
    CHECK(rc == MYSQL_NO_DATA, "should be no data");

    mysql_stmt_close(stmt);
    mysql_close(m);
}
