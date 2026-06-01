# sp-mysql-client test_integration 卡住问题

## 重要结论

**这个问题不是 sharpc 的 BUG。** 经过详细调查，问题出在 sp-mysql-client 库本身：
某个测试函数在运行后破坏了 MySQL 连接状态，导致后续的 `test_bind_result_ext` 第二次
`mysql_stmt_fetch()` 卡住。GCC 编译的完整测试套件也会卡住，证明这是库的 bug。

## sharpc 已知的 BUG

虽然卡住问题不是 sharpc 导致的，但调查过程中发现了 sharpc 的以下问题：

### BUG 1: 旧版 sharpc 编译 mysql_client.c 时 double free 崩溃

**复现**:
```bash
cd /root/code/sp-mysql-client
/root/code/sharp-pkg/sharpc/bin/sharpc --optimize Debug -c -O0 \
  -Iinclude -Isrc -Ispkg_packages/mbedtls2/include \
  -Ispkg_packages/zlib -Ispkg_packages/zstd/lib \
  -D_GNU_SOURCE src/mysql_client.c -o /tmp/test.o
# 输出: free(): double free detected in tcache 2
# 但 .o 文件已经生成了
```

**说明**: 旧版 sharpc（`/root/code/sharp-pkg/sharpc/bin/sharpc`，MD5: 49dd694c4b7044befa87aa0f9011255c）
在编译 mysql_client.c 后清理内存时 double free 崩溃。但 .o 文件在崩溃前已经正确写入。

新版 sharpc（`/root/code/sharp/build/sharpc`，MD5: 15c9a5f37e182e84ca1dc02f42008d67）
**没有这个问题**，编译 mysql_client.c 正常完成。

### BUG 2: sharpc 的代码生成器（C → C 转换）

sharpc 编译 `.c` 文件时，不是简单透传给 zig cc，而是：
1. 用自己的前端（预处理器 + 解析器 + 语义分析）处理源码
2. 用代码生成器 `cg_generate()` 重新生成 C 代码
3. 把生成的 C 代码写入临时 `.i` 文件
4. 然后调用 zig cc 编译这个 `.i` 文件

**验证**: sharpc 编译 mysql_client.c 后，生成的中间 C 代码有 19892 行（原始 3000 行）。
代码生成器展开了宏、添加了 `#line` 指令、自动生成了前向声明等。

**但结构体布局验证通过**: sharpc 和 zig cc 对 `MYSQL_BIND`、`MYSQL_STMT` 等结构体的
sizeof 和 offsetof 完全一致。

## 实际问题：sp-mysql-client 库的 bug

### 症状

完整的 test_integration 在 `test_bind_result_ext` 的第二次 `mysql_stmt_fetch()` 时卡住。
但单独运行 `test_bind_result_ext` 是正常的。

### 证明不是 sharpc 的问题

| 编译方式 | 结果 |
|---------|------|
| GCC 编译全部源码 | **也卡住** |
| zig cc 编译全部源码 | **也卡住** |
| sharpc 编译全部源码 | **也卡住** |
| zig cc 编译 isolated_test（只有 test_bind_result_ext） | 正常 |
| sharpc 编译 isolated_test | 正常 |

所有编译器编译的完整测试套件都卡住，说明问题在库代码本身。

### 根因分析

`test_bind_result_ext` 之前的某个测试函数破坏了 MySQL 连接状态。
第二次 `mysql_stmt_fetch()` 调用 `stmt_read_row` → `read_compressed_packet` → `io_read`
时阻塞，因为客户端在等待服务器发送数据，但服务器已经发完了所有数据。

GDB 调试发现第二次 fetch 时：
- `stored=0` — 不是 stored 模式
- `cursor_open=0` — 不是 cursor 模式
- `wire_eof=0` — **没有收到 EOF**

这意味着第一次 fetch 后，OK/EOF 包没有被正确消费，导致协议状态不同步。

### 可能的原因

1. `test_prepare_multi_type` 中使用非 stored 模式 fetch 10 行，
   最后一次 fetch 返回 `MYSQL_NO_DATA` 时可能没有正确设置连接状态
2. `test_tran_innodb` 中使用了 `mysql_autocommit`、`mysql_commit`、`mysql_rollback`，
   这些操作可能改变了连接的协议状态
3. 某个测试函数没有正确关闭 statement handle，导致连接状态残留

### 下一步

需要用二分法找到具体是哪个测试函数导致了问题，然后修复库代码。

## 文件清单

| 文件 | 说明 |
|------|------|
| `mysql_client.c` | 核心库实现 |
| `mysql.h` | 公共 API 头文件 |
| `test_integration.c` | 集成测试 |
| `config.spkg` | spkg 构建配置 |
| `sha.h` / `sha.c` | SHA 认证 |
| `transport.h` / `transport.c` | 网络传输层 |
| `mysql_pool.h` / `mysql_pool.c` | 连接池 |
| `spkg_build.lua` | spkg 构建脚本 |
| `spkg_main.c` / `spkg.h` / `spkg_native.c` | spkg 源码 |
| `isolated_test.c` | 最小复现程序（单独运行正常） |
| `build-artifacts/` | spkg 编译的 .o、.a 和可执行文件 |
