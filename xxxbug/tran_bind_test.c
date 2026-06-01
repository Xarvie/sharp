#include "mysql.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *g_host = "127.0.0.1";
static const char *g_user = "sp_test";
static const char *g_passwd = "sp_test_pass";
static const char *g_db = "sp_test";
static unsigned int g_port = 3306;
static int g_pass = 0, g_fail = 0;

static MYSQL *get_conn(void) {
    MYSQL *m = mysql_init(NULL);
    if (!m) return NULL;
    unsigned int ssl_mode = SSL_MODE_DISABLED;
    mysql_options(m, MYSQL_OPT_SSL_MODE, &ssl_mode);
    m = mysql_real_connect(m, g_host, g_user, g_passwd, g_db, g_port, NULL, 0);
    return m;
}

#define Q(sql) mysql_real_query(m, (sql), (unsigned long)strlen(sql))
#define TEST(msg) do { printf("  TEST %-50s ", msg); fflush(stdout); } while(0)
#define CHECK(cond, msg) do { if (cond) { printf("OK\n"); g_pass++; } else { printf("FAIL (%s)\n", msg); g_fail++; } } while(0)
#define FAIL(msg) do { printf("FAIL (%s)\n", msg); g_fail++; } while(0)

static void test_tran_innodb(void) {
    printf("\n[transaction]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }
    Q("DROP TABLE IF EXISTS t_tran");
    Q("CREATE TABLE t_tran(col1 INT, col2 VARCHAR(20)) ENGINE=InnoDB");
    Q("INSERT INTO t_tran VALUES(10, 'venu')");
    mysql_autocommit(m, 0);
    Q("INSERT INTO t_tran VALUES(20, 'test')");
    mysql_commit(m);
    Q("INSERT INTO t_tran VALUES(30, 'rollback')");
    mysql_rollback(m);
    mysql_autocommit(m, 1);
    Q("SELECT * FROM t_tran");
    MYSQL_RES *res = mysql_store_result(m);
    TEST("after commit+rollback: 1 row remains");
    CHECK(mysql_num_rows(res) == 1, "wrong row count");
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row) {
        TEST("remaining row: col1=10, col2='venu'");
        CHECK(atoi(row[0]) == 10 && strcmp(row[1], "venu") == 0, "wrong data");
    }
    mysql_free_result(res);
    mysql_close(m);
}

static void test_bind_result_ext(void) {
    printf("\n[bind result ext]\n");
    MYSQL *m = get_conn();
    if (!m) { FAIL("connect failed"); return; }
    Q("DROP TABLE IF EXISTS t_bind_ext");
    Q("CREATE TABLE t_bind_ext(c1 TINYINT, c2 SMALLINT, c3 INT, c4 BIGINT, c5 FLOAT, c6 DOUBLE, c7 VARBINARY(10), c8 VARCHAR(50))");
    Q("INSERT INTO t_bind_ext VALUES(19, 2999, 3999, 4999999, 2345.6, 5678.89563, 'venu', 'mysql')");

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT * FROM t_bind_ext", strlen("SELECT * FROM t_bind_ext"));
    if (rc != 0) { FAIL(mysql_stmt_error(stmt)); mysql_stmt_close(stmt); mysql_close(m); return; }

    char t_data; short s_data; int i_data; long long b_data; float f_data; double d_data;
    char szData[20], bData[20]; unsigned long length[8]; my_bool is_null[8];
    MYSQL_BIND my_bind[8]; memset(my_bind, 0, sizeof(my_bind));
    for (int i = 0; i < 8; i++) { my_bind[i].length = &length[i]; my_bind[i].is_null = &is_null[i]; }
    my_bind[0].buffer_type = MYSQL_TYPE_TINY; my_bind[0].buffer = &t_data;
    my_bind[1].buffer_type = MYSQL_TYPE_SHORT; my_bind[1].buffer = &s_data;
    my_bind[2].buffer_type = MYSQL_TYPE_LONG; my_bind[2].buffer = &i_data;
    my_bind[3].buffer_type = MYSQL_TYPE_LONGLONG; my_bind[3].buffer = &b_data;
    my_bind[4].buffer_type = MYSQL_TYPE_FLOAT; my_bind[4].buffer = &f_data;
    my_bind[5].buffer_type = MYSQL_TYPE_DOUBLE; my_bind[5].buffer = &d_data;
    my_bind[6].buffer_type = MYSQL_TYPE_STRING; my_bind[6].buffer = szData; my_bind[6].buffer_length = sizeof(szData);
    my_bind[7].buffer_type = MYSQL_TYPE_STRING; my_bind[7].buffer = bData; my_bind[7].buffer_length = sizeof(bData);

    rc = mysql_stmt_bind_result(stmt, my_bind);
    rc = mysql_stmt_execute(stmt);
    rc = mysql_stmt_fetch(stmt);
    TEST("tiny"); CHECK(t_data == 19, "wrong value");
    TEST("short"); CHECK(s_data == 2999, "wrong value");
    TEST("int"); CHECK(i_data == 3999, "wrong value");
    TEST("bigint"); CHECK(b_data == 4999999, "wrong value");
    TEST("string"); CHECK(strcmp(szData, "venu") == 0, "wrong value");
    TEST("string length"); CHECK(length[6] == 4, "wrong length");
    TEST("varchar"); CHECK(strncmp(bData, "mysql", 5) == 0, "wrong value");
    TEST("varchar length"); CHECK(length[7] == 5, "wrong length");

    TEST("no more rows");
    rc = mysql_stmt_fetch(stmt);
    CHECK(rc == MYSQL_NO_DATA, "expected NO_DATA");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

int main(void) {
    printf("=== tran + bind_result_ext ===\n\n");
    test_tran_innodb();
    test_bind_result_ext();
    printf("\npass=%d fail=%d\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
