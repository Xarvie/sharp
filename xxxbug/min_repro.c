/*
 * min_repro.c — 最小复现程序
 * 用法: 分别用 spkg 和手动 zig cc 编译，对比行为
 *
 * spkg 编译（有问题）:
 *   spkg build --rebuild
 *   ./build/test_integration/test_integration
 *
 * 手动 zig cc 编译（正常）:
 *   zig cc -c -O0 -Iinclude -Isrc -Ispkg_packages/mbedtls2/include \
 *          -Ispkg_packages/zlib -Ispkg_packages/zstd/lib -D_GNU_SOURCE \
 *          min_repro.c -o /tmp/min_repro.o
 *   zig cc /tmp/min_repro.o build/sp-mysql-client/libsp-mysql-client.a \
 *          build/mbedtls/libmbedtls.a build/zlib/libzlib.a \
 *          build/zstd/libzstd.a -lpthread -o /tmp/min_repro
 *   /tmp/min_repro
 */

#include "mysql.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    MYSQL *m = mysql_init(NULL);
    if (!m) { fprintf(stderr, "mysql_init failed\n"); return 1; }

    unsigned int ssl_mode = SSL_MODE_DISABLED;
    mysql_options(m, MYSQL_OPT_SSL_MODE, &ssl_mode);

    m = mysql_real_connect(m, "127.0.0.1", "sp_test", "sp_test_pass",
                           "sp_test", 3306, NULL, 0);
    if (!m) { fprintf(stderr, "connect failed: %s\n", mysql_error(NULL)); return 1; }

    mysql_real_query(m, "DROP TABLE IF EXISTS t_min_repro", strlen("DROP TABLE IF EXISTS t_min_repro"));
    mysql_real_query(m, "CREATE TABLE t_min_repro(c1 TINYINT, c2 SMALLINT, c3 INT, c4 BIGINT, c5 FLOAT, c6 DOUBLE, c7 VARBINARY(10), c8 VARCHAR(50))",
                     strlen("CREATE TABLE t_min_repro(c1 TINYINT, c2 SMALLINT, c3 INT, c4 BIGINT, c5 FLOAT, c6 DOUBLE, c7 VARBINARY(10), c8 VARCHAR(50))"));
    mysql_real_query(m, "INSERT INTO t_min_repro VALUES(19, 2999, 3999, 4999999, 2345.6, 5678.89563, 'venu', 'mysql')",
                     strlen("INSERT INTO t_min_repro VALUES(19, 2999, 3999, 4999999, 2345.6, 5678.89563, 'venu', 'mysql')"));

    MYSQL_STMT *stmt = mysql_stmt_init(m);
    int rc = mysql_stmt_prepare(stmt, "SELECT * FROM t_min_repro",
                                strlen("SELECT * FROM t_min_repro"));
    if (rc != 0) { fprintf(stderr, "prepare failed: %s\n", mysql_stmt_error(stmt)); return 1; }

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

    fprintf(stderr, "DEBUG: about to call first fetch...\n");
    rc = mysql_stmt_fetch(stmt);
    fprintf(stderr, "DEBUG: first fetch returned %d\n", rc);

    if (rc == 0) {
        fprintf(stderr, "DEBUG: t_data=%d, s_data=%d, i_data=%d, b_data=%lld\n",
                (int)t_data, (int)s_data, i_data, b_data);
    }

    fprintf(stderr, "DEBUG: about to call second fetch...\n");
    rc = mysql_stmt_fetch(stmt);
    fprintf(stderr, "DEBUG: second fetch returned %d (expected MYSQL_NO_DATA=%d)\n",
            rc, MYSQL_NO_DATA);

    mysql_stmt_close(stmt);
    mysql_close(m);
    return 0;
}
