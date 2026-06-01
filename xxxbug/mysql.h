/*
 * sp-mysql-client - MySQL/MariaDB client library for Sharp ecosystem.
 *
 * This header re-creates the public C API interface (function names and
 * types) of the well-known MySQL client API so that existing source code
 * can be recompiled against it. It is source/API compatible, NOT binary/ABI
 * compatible with Oracle's libmysqlclient (struct layouts differ).
 *
 * No GPL/LGPL source was used. See LICENSE.
 */
#ifndef MITMYSQL_MYSQL_H
#define MITMYSQL_MYSQL_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- scalar typedefs (match the conventional API) ---------------------- */
typedef char          my_bool;
typedef unsigned long long my_ulonglong;
typedef unsigned int  MYSQL_FIELD_OFFSET;

/* ---- field types (values match MySQL 8.0 enum_field_types) ------------- */
enum enum_field_types {
    MYSQL_TYPE_DECIMAL     = 0,
    MYSQL_TYPE_TINY        = 1,
    MYSQL_TYPE_SHORT       = 2,
    MYSQL_TYPE_LONG        = 3,
    MYSQL_TYPE_FLOAT       = 4,
    MYSQL_TYPE_DOUBLE      = 5,
    MYSQL_TYPE_NULL        = 6,
    MYSQL_TYPE_TIMESTAMP   = 7,
    MYSQL_TYPE_LONGLONG    = 8,
    MYSQL_TYPE_INT24       = 9,
    MYSQL_TYPE_DATE        = 10,
    MYSQL_TYPE_TIME        = 11,
    MYSQL_TYPE_DATETIME    = 12,
    MYSQL_TYPE_YEAR        = 13,
    MYSQL_TYPE_NEWDATE     = 14,
    MYSQL_TYPE_VARCHAR     = 15,
    MYSQL_TYPE_BIT         = 16,
    MYSQL_TYPE_TIMESTAMP2  = 17,
    MYSQL_TYPE_DATETIME2   = 18,
    MYSQL_TYPE_TIME2       = 19,
    MYSQL_TYPE_TYPED_ARRAY = 20,
    MYSQL_TYPE_INVALID     = 243,
    MYSQL_TYPE_BOOL        = 244,
    MYSQL_TYPE_JSON        = 245,
    MYSQL_TYPE_NEWDECIMAL  = 246,
    MYSQL_TYPE_ENUM        = 247,
    MYSQL_TYPE_SET         = 248,
    MYSQL_TYPE_TINY_BLOB   = 249,
    MYSQL_TYPE_MEDIUM_BLOB = 250,
    MYSQL_TYPE_LONG_BLOB   = 251,
    MYSQL_TYPE_BLOB        = 252,
    MYSQL_TYPE_VAR_STRING  = 253,
    MYSQL_TYPE_STRING      = 254,
    MYSQL_TYPE_GEOMETRY    = 255
};

/* ---- column flags (values match MySQL 8.0 mysql_com.h) ----------------- */
#define NOT_NULL_FLAG           1
#define PRI_KEY_FLAG            2
#define UNIQUE_KEY_FLAG         4
#define MULTIPLE_KEY_FLAG       8
#define BLOB_FLAG              16
#define UNSIGNED_FLAG          32
#define ZEROFILL_FLAG          64
#define BINARY_FLAG           128
#define ENUM_FLAG             256
#define AUTO_INCREMENT_FLAG   512
#define TIMESTAMP_FLAG       1024
#define SET_FLAG              2048
#define NO_DEFAULT_VALUE_FLAG 4096
#define ON_UPDATE_NOW_FLAG    8192
#define NUM_FLAG             32768
#define PART_KEY_FLAG        16384
#define GROUP_FLAG           32768
#define UNIQUE_FLAG          65536
#define BINCMP_FLAG         131072

/* ---- helper macros for field flags/types ------------------------------- */
#define IS_PRI_KEY(n)   ((n) & PRI_KEY_FLAG)
#define IS_NOT_NULL(n)  ((n) & NOT_NULL_FLAG)
#define IS_BLOB(n)      ((n) & BLOB_FLAG)
#define IS_NUM(t) \
    (((t) <= MYSQL_TYPE_INT24 && (t) != MYSQL_TYPE_TIMESTAMP) || \
     (t) == MYSQL_TYPE_YEAR || (t) == MYSQL_TYPE_NEWDECIMAL)
#define IS_LONGDATA(t) ((t) >= MYSQL_TYPE_TINY_BLOB && (t) <= MYSQL_TYPE_STRING)

/* ---- TLS modes (match libmysqlclient's enum mysql_ssl_mode) ------------ */
enum mysql_ssl_mode {
    SSL_MODE_DISABLED = 1,
    SSL_MODE_PREFERRED,
    SSL_MODE_REQUIRED,
    SSL_MODE_VERIFY_CA,
    SSL_MODE_VERIFY_IDENTITY
};

enum mysql_ssl_fips_mode {
    SSL_FIPS_MODE_OFF    = 0,
    SSL_FIPS_MODE_ON     = 1,
    SSL_FIPS_MODE_STRICT = 2
};

/* ---- mysql_options() option ids (values match MySQL 8.0) -------------- */
enum mysql_option {
    MYSQL_OPT_CONNECT_TIMEOUT,
    MYSQL_OPT_COMPRESS,
    MYSQL_OPT_NAMED_PIPE,
    MYSQL_INIT_COMMAND,
    MYSQL_READ_DEFAULT_FILE,
    MYSQL_READ_DEFAULT_GROUP,
    MYSQL_SET_CHARSET_DIR,
    MYSQL_SET_CHARSET_NAME,
    MYSQL_OPT_LOCAL_INFILE,
    MYSQL_OPT_PROTOCOL,
    MYSQL_SHARED_MEMORY_BASE_NAME,
    MYSQL_OPT_READ_TIMEOUT,
    MYSQL_OPT_WRITE_TIMEOUT,
    MYSQL_OPT_USE_RESULT,
    MYSQL_REPORT_DATA_TRUNCATION,
    MYSQL_OPT_RECONNECT,
    MYSQL_PLUGIN_DIR,
    MYSQL_DEFAULT_AUTH,
    MYSQL_OPT_BIND,
    MYSQL_OPT_SSL_KEY,
    MYSQL_OPT_SSL_CERT,
    MYSQL_OPT_SSL_CA,
    MYSQL_OPT_SSL_CAPATH,
    MYSQL_OPT_SSL_CIPHER,
    MYSQL_OPT_SSL_CRL,
    MYSQL_OPT_SSL_CRLPATH,
    MYSQL_OPT_CONNECT_ATTR_RESET,
    MYSQL_OPT_CONNECT_ATTR_ADD,
    MYSQL_OPT_CONNECT_ATTR_DELETE,
    MYSQL_SERVER_PUBLIC_KEY,
    MYSQL_ENABLE_CLEARTEXT_PLUGIN,
    MYSQL_OPT_CAN_HANDLE_EXPIRED_PASSWORDS,
    MYSQL_OPT_MAX_ALLOWED_PACKET,
    MYSQL_OPT_NET_BUFFER_LENGTH,
    MYSQL_OPT_TLS_VERSION,
    MYSQL_OPT_SSL_MODE,
    MYSQL_OPT_GET_SERVER_PUBLIC_KEY,
    MYSQL_OPT_RETRY_COUNT,
    MYSQL_OPT_OPTIONAL_RESULTSET_METADATA,
    MYSQL_OPT_SSL_FIPS_MODE,
    MYSQL_OPT_TLS_CIPHERSUITES,
    MYSQL_OPT_COMPRESSION_ALGORITHMS,
    MYSQL_OPT_ZSTD_COMPRESSION_LEVEL,
    MYSQL_OPT_LOAD_DATA_LOCAL_DIR,
    MYSQL_OPT_USER_PASSWORD,
    MYSQL_OPT_SSL_SESSION_DATA
};

/* ---- connection status ------------------------------------------------- */
enum mysql_status {
    MYSQL_STATUS_READY,
    MYSQL_STATUS_GET_RESULT,
    MYSQL_STATUS_USE_RESULT,
    MYSQL_STATUS_STATEMENT_GET_RESULT
};

enum mysql_protocol_type {
    MYSQL_PROTOCOL_DEFAULT,
    MYSQL_PROTOCOL_TCP,
    MYSQL_PROTOCOL_SOCKET,
    MYSQL_PROTOCOL_PIPE,
    MYSQL_PROTOCOL_MEMORY
};

/* ---- result set metadata ----------------------------------------------- */
enum enum_resultset_metadata {
    RESULTSET_METADATA_NONE = 0,
    RESULTSET_METADATA_FULL = 1
};

/* ---- cursor types for COM_STMT_EXECUTE --------------------------------- */
enum enum_cursor_type {
    CURSOR_TYPE_NO_CURSOR = 0,
    CURSOR_TYPE_READ_ONLY = 1,
    CURSOR_TYPE_FOR_UPDATE = 2,
    CURSOR_TYPE_SCROLLABLE = 4,
    PARAMETER_COUNT_AVAILABLE = 8
};

/* ---- set server option ------------------------------------------------- */
enum enum_mysql_set_option {
    MYSQL_OPTION_MULTI_STATEMENTS_ON,
    MYSQL_OPTION_MULTI_STATEMENTS_OFF
};

/* ---- session state tracking -------------------------------------------- */
enum enum_session_state_type {
    SESSION_TRACK_SYSTEM_VARIABLES,
    SESSION_TRACK_SCHEMA,
    SESSION_TRACK_STATE_CHANGE,
    SESSION_TRACK_GTIDS,
    SESSION_TRACK_TRANSACTION_CHARACTERISTICS,
    SESSION_TRACK_TRANSACTION_STATE
};

/* ---- server status flags ----------------------------------------------- */
enum SERVER_STATUS_flags_enum {
    SERVER_STATUS_IN_TRANS             = 1,
    SERVER_STATUS_AUTOCOMMIT           = 2,
    SERVER_MORE_RESULTS_EXISTS         = 8,
    SERVER_QUERY_NO_GOOD_INDEX_USED    = 16,
    SERVER_QUERY_NO_INDEX_USED         = 32,
    SERVER_STATUS_CURSOR_EXISTS        = 64,
    SERVER_STATUS_LAST_ROW_SENT        = 128,
    SERVER_STATUS_DB_DROPPED           = 256,
    SERVER_STATUS_NO_BACKSLASH_ESCAPES = 512,
    SERVER_STATUS_METADATA_CHANGED     = 1024,
    SERVER_QUERY_WAS_SLOW              = 2048,
    SERVER_PS_OUT_PARAMS               = 4096,
    SERVER_STATUS_IN_TRANS_READONLY    = 8192,
    SERVER_SESSION_STATE_CHANGED       = (1U << 14)
};

/* ---- character set info ------------------------------------------------ */
typedef struct {
    unsigned int number;
    unsigned int state;
    const char  *csname;
    const char  *name;
    const char  *comment;
    const char  *dir;
    unsigned int mbminlen;
    unsigned int mbmaxlen;
} MY_CHARSET_INFO;

/* ---- field metadata ---------------------------------------------------- */
typedef struct st_mysql_field {
    char *name;
    char *org_name;
    char *table;
    char *org_table;
    char *db;
    char *catalog;
    char *def;
    unsigned long length;
    unsigned long max_length;
    unsigned int  name_length;
    unsigned int  org_name_length;
    unsigned int  table_length;
    unsigned int  org_table_length;
    unsigned int  db_length;
    unsigned int  catalog_length;
    unsigned int  def_length;
    unsigned int  flags;
    unsigned int  decimals;
    unsigned int  charsetnr;
    enum enum_field_types type;
    void *extension;
} MYSQL_FIELD;

typedef char **MYSQL_ROW;

#define MYSQL_COUNT_ERROR (~(my_ulonglong)0)

/* ---- row linked list (for row_seek/tell) ------------------------------- */
typedef struct st_mysql_rows {
    struct st_mysql_rows *next;
    MYSQL_ROW data;
    unsigned long length;
} MYSQL_ROWS;

typedef MYSQL_ROWS *MYSQL_ROW_OFFSET;

/* ---- buffered result data ---------------------------------------------- */
typedef struct st_mysql_data {
    MYSQL_ROWS *data;
    void       *alloc;
    my_ulonglong rows;
    unsigned int  fields;
} MYSQL_DATA;

/* ---- result set -------------------------------------------------------- */
typedef struct st_mysql_res {
    my_ulonglong     row_count;
    MYSQL_FIELD     *fields;
    MYSQL_DATA      *data;
    MYSQL_ROWS      *data_cursor;
    unsigned long   *lengths;
    struct st_mysql *handle;
    MYSQL_ROW        row;
    MYSQL_ROW        current_row;
    void            *field_alloc;
    unsigned int     field_count, current_field;
    my_bool          eof;
    my_bool          unbuffered_fetch_cancelled;
    enum enum_resultset_metadata metadata;
    void            *extension;
    /* --- our internal fields (not in official layout) --- */
    MYSQL_ROW     *store_data;
    unsigned long **store_lengths;
    my_ulonglong   store_cursor;
    unsigned long *cur_lengths;
    int            streaming;
    MYSQL_ROW      cur_row;
    unsigned long *cur_row_lens;
} MYSQL_RES;

/* ---- connection handle ------------------------------------------------- */
typedef struct st_mysql {
    int          fd;
    char        *host;
    char        *user;
    char        *db;
    unsigned int port;
    unsigned long client_flag;
    unsigned long server_capabilities;
    unsigned int server_status;
    unsigned int warning_count;
    unsigned long thread_id;
    char        *server_version;
    unsigned int  protocol_version;
    my_ulonglong affected_rows;
    my_ulonglong insert_id;
    unsigned int field_count;
    unsigned int last_errno;
    char         last_error[512];
    char         sqlstate[6];
    unsigned char seq;
    unsigned int  connect_timeout;
    unsigned int  read_timeout;
    unsigned int  write_timeout;
    char         *charset_name;
    void         *st;
    int           multi_status;
    void         *stmt_cache;
    char         *info;
    char         *host_info;
    enum mysql_status status;
} MYSQL;

/* ---- CLIENT capability flags (values match MySQL 8.0) ------------------ */
#define CLIENT_LONG_PASSWORD                  0x00000001UL
#define CLIENT_FOUND_ROWS                     0x00000002UL
#define CLIENT_LONG_FLAG                      0x00000004UL
#define CLIENT_CONNECT_WITH_DB                0x00000008UL
#define CLIENT_NO_SCHEMA                      0x00000010UL
#define CLIENT_COMPRESS                       0x00000020UL
#define CLIENT_ODBC                           0x00000040UL
#define CLIENT_LOCAL_FILES                    0x00000080UL
#define CLIENT_IGNORE_SPACE                   0x00000100UL
#define CLIENT_PROTOCOL_41                    0x00000200UL
#define CLIENT_INTERACTIVE                    0x00000400UL
#define CLIENT_SSL                            0x00000800UL
#define CLIENT_IGNORE_SIGPIPE                 0x00001000UL
#define CLIENT_TRANSACTIONS                   0x00002000UL
#define CLIENT_RESERVED                       0x00004000UL
#define CLIENT_SECURE_CONNECTION              0x00008000UL
#define CLIENT_MULTI_STATEMENTS               0x00010000UL
#define CLIENT_MULTI_RESULTS                  0x00020000UL
#define CLIENT_PS_MULTI_RESULTS               0x00040000UL
#define CLIENT_PLUGIN_AUTH                    0x00080000UL
#define CLIENT_CONNECT_ATTRS                  0x00100000UL
#define CLIENT_PLUGIN_AUTH_LENENC_CLIENT_DATA 0x00200000UL
#define CLIENT_CAN_HANDLE_EXPIRED_PASSWORDS   0x00400000UL
#define CLIENT_SESSION_TRACK                  0x00800000UL
#define CLIENT_DEPRECATE_EOF                  0x01000000UL
#define CLIENT_OPTIONAL_RESULTSET_METADATA    0x02000000UL
#define CLIENT_ZSTD_COMPRESSION_ALGORITHM     0x04000000UL
#define CLIENT_QUERY_ATTRIBUTES               0x08000000UL
#define MULTI_FACTOR_AUTHENTICATION           0x10000000UL
#define CLIENT_CAPABILITY_EXTENSION           0x20000000UL
#define CLIENT_SSL_VERIFY_SERVER_CERT         0x40000000UL
#define CLIENT_REMEMBER_OPTIONS               0x80000000UL

#define CLIENT_ZSTD_COMPRESSION CLIENT_ZSTD_COMPRESSION_ALGORITHM

/* ---- timestamp types --------------------------------------------------- */
enum enum_mysql_timestamp_type {
    MYSQL_TIMESTAMP_NONE      = -2,
    MYSQL_TIMESTAMP_ERROR     = -1,
    MYSQL_TIMESTAMP_DATE      = 0,
    MYSQL_TIMESTAMP_DATETIME  = 1,
    MYSQL_TIMESTAMP_TIME      = 2,
    MYSQL_TIMESTAMP_DATETIME_TZ = 3
};

typedef struct st_mysql_time {
    unsigned int  year, month, day, hour, minute, second;
    unsigned long second_part;
    my_bool       neg;
    enum enum_mysql_timestamp_type time_type;
    int           time_zone_displacement;
} MYSQL_TIME;

/* ---- statement attribute types ----------------------------------------- */
enum enum_stmt_attr_type {
    STMT_ATTR_UPDATE_MAX_LENGTH,
    STMT_ATTR_CURSOR_TYPE,
    STMT_ATTR_PREFETCH_ROWS
};

/* ---- statement state --------------------------------------------------- */
enum enum_mysql_stmt_state {
    MYSQL_STMT_INIT_DONE    = 1,
    MYSQL_STMT_PREPARE_DONE,
    MYSQL_STMT_EXECUTE_DONE,
    MYSQL_STMT_FETCH_DONE
};

/* ---- fetch return codes ------------------------------------------------ */
#define MYSQL_NO_DATA        100
#define MYSQL_DATA_TRUNCATED 101

/* ---- error constants --------------------------------------------------- */
#define WARN_DATA_TRUNCATED  1265
#define ER_WARN_DATA_TRUNCATED WARN_DATA_TRUNCATED

/* ---- CLIENT_ net constants --------------------------------------------- */
#define CLIENT_NET_RETRY_COUNT  1
#define CLIENT_NET_READ_TIMEOUT (365 * 24 * 3600)
#define CLIENT_NET_WRITE_TIMEOUT (365 * 24 * 3600)

/* ---- local infile ------------------------------------------------------ */
#define LOCAL_INFILE_ERROR_LEN 512

/* ---- bind structure (matches official layout order) --------------------- */
typedef struct st_mysql_bind {
    unsigned long *length;
    my_bool       *is_null;
    void          *buffer;
    my_bool       *error;
    unsigned char *row_ptr;
    void         (*store_param_func)(void *net, struct st_mysql_bind *param);
    void         (*fetch_result)(struct st_mysql_bind *, MYSQL_FIELD *, unsigned char **row);
    void         (*skip_result)(struct st_mysql_bind *, MYSQL_FIELD *, unsigned char **row);
    unsigned long  buffer_length;
    unsigned long  offset;
    unsigned long  length_value;
    unsigned int   param_number;
    unsigned int   pack_length;
    enum enum_field_types buffer_type;
    my_bool        error_value;
    my_bool        is_unsigned;
    my_bool        long_data_used;
    my_bool        is_null_value;
    void          *extension;
} MYSQL_BIND;

/* ---- statement handle -------------------------------------------------- */
typedef struct st_mysql_stmt {
    void          *mem_root;
    void          *list_prev;
    void          *list_next;
    MYSQL         *mysql;
    MYSQL_BIND    *params;
    MYSQL_BIND    *bind;
    MYSQL_FIELD   *fields;
    MYSQL_DATA     result;
    MYSQL_ROWS    *data_cursor;
    void         (*read_row_func)(void);
    my_ulonglong   affected_rows;
    my_ulonglong   insert_id;
    unsigned long  stmt_id;
    unsigned long  flags;
    unsigned long  prefetch_rows;
    unsigned int   server_status;
    unsigned int   last_errno;
    unsigned int   param_count;
    unsigned int   field_count;
    enum enum_mysql_stmt_state state;
    char           last_error[512];
    char           sqlstate[6];
    my_bool        send_types_to_server;
    my_bool        bind_param_done;
    unsigned char  bind_result_done;
    my_bool        unbuffered_fetch_cancelled;
    my_bool        update_max_length;
    void          *extension;
    /* --- our internal fields (not in official layout) --- */
    MYSQL_FIELD  *param_fields;
    MYSQL_BIND   *bind_params;
    MYSQL_BIND   *bind_results;
    unsigned int   bind_result_count;
    my_ulonglong   row_count;
    unsigned int   warning_count;
    void          *internal_result;
} MYSQL_STMT;

/* ---- CR_ client error codes (values match MySQL 8.0 errmsg.h) --------- */
#define CR_MIN_ERROR                2000
#define CR_MAX_ERROR                2999
#define CLIENT_ERRMAP               2
#define CR_ERROR_FIRST              2000
#define CR_UNKNOWN_ERROR            2000
#define CR_SOCKET_CREATE_ERROR      2001
#define CR_CONNECTION_ERROR         2002
#define CR_CONN_HOST_ERROR          2003
#define CR_IPSOCK_ERROR             2004
#define CR_UNKNOWN_HOST             2005
#define CR_SERVER_GONE_ERROR        2006
#define CR_VERSION_ERROR            2007
#define CR_OUT_OF_MEMORY            2008
#define CR_WRONG_HOST_INFO          2009
#define CR_LOCALHOST_CONNECTION     2010
#define CR_TCP_CONNECTION           2011
#define CR_SERVER_HANDSHAKE_ERR     2012
#define CR_SERVER_LOST              2013
#define CR_COMMANDS_OUT_OF_SYNC     2014
#define CR_NAMEDPIPE_CONNECTION     2015
#define CR_NAMEDPIPEWAIT_ERROR      2016
#define CR_NAMEDPIPEOPEN_ERROR      2017
#define CR_NAMEDPIPESETSTATE_ERROR  2018
#define CR_CANT_READ_CHARSET        2019
#define CR_NET_PACKET_TOO_LARGE     2020
#define CR_EMBEDDED_CONNECTION      2021
#define CR_PROBE_REPLICA_STATUS     2022
#define CR_PROBE_REPLICA_HOSTS      2023
#define CR_PROBE_REPLICA_CONNECT    2024
#define CR_PROBE_SOURCE_CONNECT     2025
#define CR_SSL_CONNECTION_ERROR     2026
#define CR_MALFORMED_PACKET         2027
#define CR_WRONG_LICENSE            2028
#define CR_NULL_POINTER             2029
#define CR_NO_PREPARE_STMT          2030
#define CR_PARAMS_NOT_BOUND         2031
#define CR_DATA_TRUNCATED           2032
#define CR_NO_PARAMETERS_EXISTS     2033
#define CR_INVALID_PARAMETER_NO     2034
#define CR_INVALID_BUFFER_USE       2035
#define CR_UNSUPPORTED_PARAM_TYPE   2036
#define CR_SHARED_MEMORY_CONNECTION                      2037
#define CR_SHARED_MEMORY_CONNECT_REQUEST_ERROR           2038
#define CR_SHARED_MEMORY_CONNECT_ANSWER_ERROR            2039
#define CR_SHARED_MEMORY_CONNECT_FILE_MAP_ERROR          2040
#define CR_SHARED_MEMORY_CONNECT_MAP_ERROR               2041
#define CR_SHARED_MEMORY_FILE_MAP_ERROR                  2042
#define CR_SHARED_MEMORY_MAP_ERROR                       2043
#define CR_SHARED_MEMORY_EVENT_ERROR                     2044
#define CR_SHARED_MEMORY_CONNECT_ABANDONED_ERROR         2045
#define CR_SHARED_MEMORY_CONNECT_SET_ERROR               2046
#define CR_CONN_UNKNOW_PROTOCOL     2047
#define CR_INVALID_CONN_HANDLE      2048
#define CR_UNUSED_1                 2049
#define CR_FETCH_CANCELED           2050
#define CR_NO_DATA                  2051
#define CR_NO_STMT_METADATA         2052
#define CR_NO_RESULT_SET            2053
#define CR_NOT_IMPLEMENTED          2054
#define CR_SERVER_LOST_EXTENDED     2055
#define CR_STMT_CLOSED              2056
#define CR_NEW_STMT_METADATA        2057
#define CR_ALREADY_CONNECTED        2058
#define CR_AUTH_PLUGIN_CANNOT_LOAD  2059
#define CR_DUPLICATE_CONNECTION_ATTR 2060
#define CR_AUTH_PLUGIN_ERR          2061
#define CR_INSECURE_API_ERR         2062
#define CR_FILE_NAME_TOO_LONG       2063
#define CR_SSL_FIPS_MODE_ERR        2064
#define CR_DEPRECATED_COMPRESSION_NOT_SUPPORTED 2065
#define CR_COMPRESSION_WRONGLY_CONFIGURED       2066
#define CR_KERBEROS_USER_NOT_FOUND              2067
#define CR_LOAD_DATA_LOCAL_INFILE_REJECTED      2068
#define CR_LOAD_DATA_LOCAL_INFILE_REALPATH_FAIL 2069
#define CR_DNS_SRV_LOOKUP_FAILED               2070
#define CR_MANDATORY_TRACKER_NOT_FOUND          2071
#define CR_INVALID_FACTOR_NO                    2072
#define CR_CANT_GET_SESSION_DATA                2073
#define CR_INVALID_CLIENT_CHARSET               2074
#define CR_ERROR_LAST                           2074

/* ---- backward compatibility aliases ------------------------------------ */
#define FIELD_TYPE_DECIMAL    MYSQL_TYPE_DECIMAL
#define FIELD_TYPE_TINY       MYSQL_TYPE_TINY
#define FIELD_TYPE_SHORT      MYSQL_TYPE_SHORT
#define FIELD_TYPE_LONG       MYSQL_TYPE_LONG
#define FIELD_TYPE_FLOAT      MYSQL_TYPE_FLOAT
#define FIELD_TYPE_DOUBLE     MYSQL_TYPE_DOUBLE
#define FIELD_TYPE_NULL       MYSQL_TYPE_NULL
#define FIELD_TYPE_TIMESTAMP  MYSQL_TYPE_TIMESTAMP
#define FIELD_TYPE_LONGLONG   MYSQL_TYPE_LONGLONG
#define FIELD_TYPE_INT24      MYSQL_TYPE_INT24
#define FIELD_TYPE_DATE       MYSQL_TYPE_DATE
#define FIELD_TYPE_TIME       MYSQL_TYPE_TIME
#define FIELD_TYPE_DATETIME   MYSQL_TYPE_DATETIME
#define FIELD_TYPE_YEAR       MYSQL_TYPE_YEAR
#define FIELD_TYPE_NEWDATE    MYSQL_TYPE_NEWDATE
#define FIELD_TYPE_ENUM       MYSQL_TYPE_ENUM
#define FIELD_TYPE_SET        MYSQL_TYPE_SET
#define FIELD_TYPE_TINY_BLOB  MYSQL_TYPE_TINY_BLOB
#define FIELD_TYPE_MEDIUM_BLOB MYSQL_TYPE_MEDIUM_BLOB
#define FIELD_TYPE_LONG_BLOB  MYSQL_TYPE_LONG_BLOB
#define FIELD_TYPE_BLOB       MYSQL_TYPE_BLOB
#define FIELD_TYPE_VAR_STRING MYSQL_TYPE_VAR_STRING
#define FIELD_TYPE_STRING     MYSQL_TYPE_STRING
#define FIELD_TYPE_CHAR       MYSQL_TYPE_TINY
#define FIELD_TYPE_INTERVAL   MYSQL_TYPE_ENUM
#define FIELD_TYPE_GEOMETRY   MYSQL_TYPE_GEOMETRY
#define FIELD_TYPE_BIT        MYSQL_TYPE_BIT
#define FIELD_TYPE_NEWDECIMAL MYSQL_TYPE_NEWDECIMAL
#define FIELD_TYPE_JSON       MYSQL_TYPE_JSON
#define CLIENT_MULTI_QUERIES  CLIENT_MULTI_STATEMENTS
#define mysql_reload(mysql)   mysql_refresh((mysql), REFRESH_GRANT)
#define REFRESH_GRANT         1
#define REFRESH_LOG           2
#define REFRESH_TABLES        4
#define REFRESH_HOSTS         8
#define REFRESH_STATUS        16
#define REFRESH_THREADS       32
#define REFRESH_REPLICA       64
#define REFRESH_SLAVE         REFRESH_REPLICA
#define REFRESH_MASTER        128
#define HAVE_MYSQL_REAL_CONNECT 1

/* ---- API --------------------------------------------------------------- */
MYSQL        *mysql_init(MYSQL *mysql);
int           mysql_options(MYSQL *mysql, enum mysql_option option, const void *arg);
int           mysql_options4(MYSQL *mysql, enum mysql_option option,
                             const void *arg1, const void *arg2);
int           mysql_get_option(MYSQL *mysql, enum mysql_option option,
                               const void *arg);

int           mysql_ssl_set(MYSQL *mysql, const char *key, const char *cert,
                            const char *ca, const char *capath, const char *cipher);
const char   *mysql_get_ssl_cipher(MYSQL *mysql);
MYSQL        *mysql_real_connect(MYSQL *mysql, const char *host, const char *user,
                                 const char *passwd, const char *db,
                                 unsigned int port, const char *unix_socket,
                                 unsigned long client_flag);
void          mysql_close(MYSQL *mysql);

int           mysql_query(MYSQL *mysql, const char *stmt_str);
int           mysql_send_query(MYSQL *mysql, const char *q, unsigned long length);
int           mysql_real_query(MYSQL *mysql, const char *stmt_str, unsigned long length);
my_bool       mysql_read_query_result(MYSQL *mysql);

MYSQL_RES    *mysql_store_result(MYSQL *mysql);
MYSQL_RES    *mysql_use_result(MYSQL *mysql);
void          mysql_free_result(MYSQL_RES *result);

MYSQL_ROW     mysql_fetch_row(MYSQL_RES *result);
unsigned long*mysql_fetch_lengths(MYSQL_RES *result);
void          mysql_data_seek(MYSQL_RES *result, my_ulonglong offset);
MYSQL_ROW_OFFSET mysql_row_seek(MYSQL_RES *result, MYSQL_ROW_OFFSET offset);
MYSQL_ROW_OFFSET mysql_row_tell(MYSQL_RES *result);

unsigned int  mysql_num_fields(MYSQL_RES *result);
my_ulonglong  mysql_num_rows(MYSQL_RES *result);
unsigned int  mysql_field_count(MYSQL *mysql);

MYSQL_FIELD  *mysql_fetch_field(MYSQL_RES *result);
MYSQL_FIELD  *mysql_fetch_fields(MYSQL_RES *result);
MYSQL_FIELD  *mysql_fetch_field_direct(MYSQL_RES *result, unsigned int fieldnr);
void          mysql_field_seek(MYSQL_RES *result, MYSQL_FIELD_OFFSET offset);
MYSQL_FIELD_OFFSET mysql_field_tell(MYSQL_RES *result);
my_bool       mysql_eof(MYSQL_RES *result);
enum enum_resultset_metadata mysql_result_metadata(MYSQL_RES *result);

my_ulonglong  mysql_affected_rows(MYSQL *mysql);
my_ulonglong  mysql_insert_id(MYSQL *mysql);
unsigned int  mysql_warning_count(MYSQL *mysql);

const char   *mysql_error(MYSQL *mysql);
unsigned int  mysql_errno(MYSQL *mysql);
const char   *mysql_sqlstate(MYSQL *mysql);
const char   *mysql_info(MYSQL *mysql);

int           mysql_ping(MYSQL *mysql);
int           mysql_change_user(MYSQL *mysql, const char *user,
                                const char *passwd, const char *db);
int           mysql_next_result(MYSQL *mysql);
int           mysql_more_results(MYSQL *mysql);
int           mysql_select_db(MYSQL *mysql, const char *db);
int           mysql_set_character_set(MYSQL *mysql, const char *csname);
void          mysql_get_character_set_info(MYSQL *mysql, MY_CHARSET_INFO *charset);

unsigned long mysql_escape_string(char *to, const char *from, unsigned long length);
unsigned long mysql_real_escape_string(MYSQL *mysql, char *to,
                                        const char *from, unsigned long length);
unsigned long mysql_real_escape_string_quote(MYSQL *mysql, char *to,
                                              const char *from,
                                              unsigned long length, char quote);
unsigned long mysql_hex_string(char *to, const char *from, unsigned long length);

const char   *mysql_get_server_info(MYSQL *mysql);
const char   *mysql_get_host_info(MYSQL *mysql);
const char   *mysql_get_client_info(void);
unsigned long mysql_get_client_version(void);
unsigned long mysql_get_server_version(MYSQL *mysql);
unsigned long mysql_thread_id(MYSQL *mysql);
const char   *mysql_character_set_name(MYSQL *mysql);
unsigned int  mysql_get_proto_info(MYSQL *mysql);
int           mysql_autocommit(MYSQL *mysql, my_bool mode);
int           mysql_commit(MYSQL *mysql);
int           mysql_rollback(MYSQL *mysql);

int           mysql_reset_connection(MYSQL *mysql);
int           mysql_set_server_option(MYSQL *mysql, enum enum_mysql_set_option option);
int           mysql_kill(MYSQL *mysql, unsigned long pid);
int           mysql_shutdown(MYSQL *mysql, int shutdown_level);
int           mysql_refresh(MYSQL *mysql, unsigned int refresh_options);
int           mysql_dump_debug_info(MYSQL *mysql);
const char   *mysql_stat(MYSQL *mysql);

MYSQL_RES    *mysql_list_dbs(MYSQL *mysql, const char *wild);
MYSQL_RES    *mysql_list_tables(MYSQL *mysql, const char *wild);
MYSQL_RES    *mysql_list_fields(MYSQL *mysql, const char *table, const char *wild);
MYSQL_RES    *mysql_list_processes(MYSQL *mysql);

int           mysql_session_track_get_first(MYSQL *mysql,
                                             enum enum_session_state_type type,
                                             const char **data, size_t *length);
int           mysql_session_track_get_next(MYSQL *mysql,
                                            enum enum_session_state_type type,
                                            const char **data, size_t *length);

/* ---- prepared statements (binary protocol) ----------------------------- */
MYSQL_STMT   *mysql_stmt_init(MYSQL *mysql);
int           mysql_stmt_prepare(MYSQL_STMT *stmt, const char *stmt_str, unsigned long length);
int           mysql_stmt_execute(MYSQL_STMT *stmt);
int           mysql_stmt_bind_param(MYSQL_STMT *stmt, MYSQL_BIND *bnd);
int           mysql_stmt_bind_result(MYSQL_STMT *stmt, MYSQL_BIND *bnd);
int           mysql_stmt_store_result(MYSQL_STMT *stmt);
int           mysql_stmt_fetch(MYSQL_STMT *stmt);
int           mysql_stmt_fetch_column(MYSQL_STMT *stmt, MYSQL_BIND *bnd,
                                      unsigned int column, unsigned long offset);
my_bool       mysql_stmt_send_long_data(MYSQL_STMT *stmt, unsigned int param_number,
                                        const char *data, unsigned long length);
int           mysql_stmt_reset(MYSQL_STMT *stmt);
int           mysql_stmt_free_result(MYSQL_STMT *stmt);
int           mysql_stmt_close(MYSQL_STMT *stmt);
void          mysql_stmt_data_seek(MYSQL_STMT *stmt, my_ulonglong offset);
MYSQL_ROW_OFFSET mysql_stmt_row_seek(MYSQL_STMT *stmt, MYSQL_ROW_OFFSET offset);
MYSQL_ROW_OFFSET mysql_stmt_row_tell(MYSQL_STMT *stmt);

unsigned long mysql_stmt_param_count(MYSQL_STMT *stmt);
unsigned int  mysql_stmt_field_count(MYSQL_STMT *stmt);
my_ulonglong  mysql_stmt_affected_rows(MYSQL_STMT *stmt);
my_ulonglong  mysql_stmt_insert_id(MYSQL_STMT *stmt);
my_ulonglong  mysql_stmt_num_rows(MYSQL_STMT *stmt);
unsigned int  mysql_stmt_warning_count(MYSQL_STMT *stmt);
MYSQL_RES    *mysql_stmt_result_metadata(MYSQL_STMT *stmt);
MYSQL_RES    *mysql_stmt_param_metadata(MYSQL_STMT *stmt);

my_bool       mysql_stmt_attr_set(MYSQL_STMT *stmt,
                                  enum enum_stmt_attr_type attr_type,
                                  const void *attr);
my_bool       mysql_stmt_attr_get(MYSQL_STMT *stmt,
                                  enum enum_stmt_attr_type attr_type,
                                  void *attr);

const char   *mysql_stmt_error(MYSQL_STMT *stmt);
unsigned int  mysql_stmt_errno(MYSQL_STMT *stmt);
const char   *mysql_stmt_sqlstate(MYSQL_STMT *stmt);
int           mysql_stmt_next_result(MYSQL_STMT *stmt);

unsigned int  mysql_thread_safe(void);
void          mysql_debug(const char *debug);

#ifdef __cplusplus
}
#endif

#endif /* MITMYSQL_MYSQL_H */
