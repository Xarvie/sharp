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

#define TEST(msg) do { printf("  TEST %-50s ", msg); fflush(stdout); } while(0)
#define CHECK(cond, msg) do { if (cond) { printf("OK\n"); g_pass++; } else { printf("FAIL (%s)\n", msg); g_fail++; } } while(0)

static void test_prepare_multi_type(void) {
    printf("\n[prepare multi-type]\n");
    MYSQL *m = get_conn();
    if (!m) { printf("FAIL connect\n"); return; }

    mysql_real_query(m, "DROP TABLE IF EXISTS t_prep_multi", strlen("DROP TABLE IF EXISTS t_prep_multi"));
    mysql_real_query(m, "CREATE TABLE t_prep_multi(col1 TINYINT, col2 VARCHAR(15), col3 INT, col4 SMALLINT, col5 BIGINT, col6 FLOAT, col7 DOUBLE)",
                     strlen("CREATE TABLE t_prep_multi(col1 TINYINT, col2 VARCHAR(15), col3 INT, col4 SMALLINT, col5 BIGINT, col6 FLOAT, col7 DOUBLE)"));

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    const char *sql = "INSERT INTO t_prep_multi VALUES(?,?,?,?,?,?,?)";
    int rc = mysql_stmt_prepare(stmt, sql, strlen(sql));
    if (rc != 0) { printf("FAIL prepare\n"); mysql_stmt_close(stmt); mysql_close(m); return; }

    TEST("param_count == 7");
    CHECK(stmt->param_count == 7, "wrong count");

    char tiny_data;
    short s_data;
    int i_data;
    long long b_data;
    float f_data;
    double d_data;
    char szData[20];
    unsigned long length[7];
    my_bool is_null[7];

    MYSQL_BIND my_bind[7];
    memset(my_bind, 0, sizeof(my_bind));
    for (int i = 0; i < 7; i++) {
        my_bind[i].length = &length[i];
        my_bind[i].is_null = &is_null[i];
    }
    my_bind[0].buffer_type = MYSQL_TYPE_TINY;
    my_bind[0].buffer = &tiny_data;
    my_bind[1].buffer_type = MYSQL_TYPE_STRING;
    my_bind[1].buffer = szData;
    my_bind[1].buffer_length = sizeof(szData);
    my_bind[2].buffer_type = MYSQL_TYPE_LONG;
    my_bind[2].buffer = &i_data;
    my_bind[3].buffer_type = MYSQL_TYPE_SHORT;
    my_bind[3].buffer = &s_data;
    my_bind[4].buffer_type = MYSQL_TYPE_LONGLONG;
    my_bind[4].buffer = &b_data;
    my_bind[5].buffer_type = MYSQL_TYPE_FLOAT;
    my_bind[5].buffer = &f_data;
    my_bind[6].buffer_type = MYSQL_TYPE_DOUBLE;
    my_bind[6].buffer = &d_data;

    rc = mysql_stmt_bind_param(stmt, my_bind);

    for (int i = 0; i < 10; i++) {
        tiny_data = (char)(i + 'a');
        sprintf(szData, "row%d", i);
        i_data = i * 100;
        s_data = (short)(i * 200);
        b_data = i * 300;
        f_data = (float)(i * 1.5);
        d_data = i * 1.55;
        rc = mysql_stmt_execute(stmt);
        if (rc != 0) { printf("FAIL execute %d: %s\n", i, mysql_stmt_error(stmt)); break; }
    }

    TEST("inserted 10 rows via multi-type PS");
    CHECK(rc == 0, "execute failed");

    mysql_stmt_close(stmt);

    rc = mysql_real_query(m, "SELECT * FROM t_prep_multi", strlen("SELECT * FROM t_prep_multi"));
    MYSQL_RES *res = mysql_store_result(m);
    TEST("10 rows in table");
    CHECK(mysql_num_rows(res) == 10, "wrong row count");
    mysql_free_result(res);
    mysql_close(m);
}

static void test_bind_result_ext(void) {
    printf("\n[bind result ext]\n");
    MYSQL *m = get_conn();
    if (!m) { printf("FAIL connect\n"); return; }

    mysql_real_query(m, "DROP TABLE IF EXISTS t_bind_ext", strlen("DROP TABLE IF EXISTS t_bind_ext"));
    mysql_real_query(m, "CREATE TABLE t_bind_ext(c1 TINYINT, c2 SMALLINT, c3 INT, c4 BIGINT, c5 FLOAT, c6 DOUBLE, c7 VARBINARY(10), c8 VARCHAR(50))",
                     strlen("CREATE TABLE t_bind_ext(c1 TINYINT, c2 SMALLINT, c3 INT, c4 BIGINT, c5 FLOAT, c6 DOUBLE, c7 VARBINARY(10), c8 VARCHAR(50))"));
    mysql_real_query(m, "INSERT INTO t_bind_ext VALUES(19, 2999, 3999, 4999999, 2345.6, 5678.89563, 'venu', 'mysql')",
                     strlen("INSERT INTO t_bind_ext VALUES(19, 2999, 3999, 4999999, 2345.6, 5678.89563, 'venu', 'mysql')"));

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT * FROM t_bind_ext", strlen("SELECT * FROM t_bind_ext"));
    if (rc != 0) { printf("FAIL prepare: %s\n", mysql_stmt_error(stmt)); return; }

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
    TEST("tiny");
    CHECK(t_data == 19, "wrong tiny value");
    TEST("short");
    CHECK(s_data == 2999, "wrong short value");
    TEST("int");
    CHECK(i_data == 3999, "wrong int value");
    TEST("bigint");
    CHECK(b_data == 4999999, "wrong bigint value");
    TEST("string");
    CHECK(strcmp(szData, "venu") == 0, "wrong string");
    TEST("string length");
    CHECK(length[6] == 4, "wrong string length");
    TEST("varchar");
    CHECK(strcmp(bData, "mysql") == 0, "wrong varchar");
    TEST("varchar length");
    CHECK(length[7] == 5, "wrong varchar length");

    TEST("no more rows");
    rc = mysql_stmt_fetch(stmt);
    CHECK(rc == MYSQL_NO_DATA, "expected NO_DATA");

    mysql_stmt_close(stmt);
    mysql_close(m);
}

int main(void) {
    printf("=== two-test: prepare_multi_type + bind_result_ext ===\n\n");
    test_prepare_multi_type();
    test_bind_result_ext();
    printf("\npass=%d fail=%d\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
