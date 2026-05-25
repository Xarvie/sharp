## Sharp 语言项目概览

### 1. Sharp 是什么
Sharp(.sp) 是 C 的超集语言。语法兼容 C，但额外增加 `.sp` 扩展（模块、泛型等特性）。
所有合法 C 代码都是合法 Sharp 代码。

### 2. 项目目录结构

```
sharp/
├── sharpc/              # 编译器核心（纯 C）
├── sharp-fe/            # 前端（词法/语法分析）
├── sharp-test/          # 测试用例（.sp 文件）
├── net/                 # 网络库（socket/poller/ring/tcp/udp/timer）
├── sharp-pkg/
│   ├── libgit2/         # libgit2 + 内置 mbedTLS 2.28.9（自包含 Git 支持）
│   ├── lua/             # Lua 5.4 源码（spkg 的 Lua VM 源，不要提交）
│   └── spkg/            # 包管理器（C + Lua，cmake 构建）
│       ├── main.c           # CLI 入口，Lua VM，嵌入脚本
│       ├── native.c         # 11 个 C 函数暴露给 Lua
│       ├── spkg.h           # 共享声明
│       ├── embed_lua.cmake  # 把 Lua 脚本嵌入为 C 数组
│       ├── CMakeLists.txt   # spkg 的 cmake 构建
│       ├── lua/             # Lua 5.4 子集（.c/.h + CMakeLists.txt）
│       └── scripts/         # 5 个 Lua 脚本（嵌入到 spkg 二进制）
│           ├── spkg_init.lua    # 命令分发 + Lua 表序列化
│           ├── spkg_build.lua   # 编译流程：sharpc -c → zig cc link
│           ├── spkg_fetch.lua   # git clone 拉取依赖
│           ├── spkg_resolve.lua # 依赖 URL 解析（config.spkm + lock）
│           └── spkg_lock.lua    # Sharp.lock 读写
└── build/               # cmake 输出目录（sharpc 在此）
```

### 3. sharpc 编译器

- 纯 C 实现，cmake 构建
- **强依赖 zig**：内部调用 `zig cc` 编译和链接
- 用法：`sharpc -c src/main.sp -o build/main.o` 然后 `zig cc build/main.o -o app`
- 编译产物：`.sp → zig cc → .o → zig cc → exe`
- 构建：`cmake -B build && cmake --build build`，输出 `build/sharpc`

### 4. spkg 包管理器

**设计决策**：
- **纯 C + Lua**（不是 Sharp 自举），因为 libgit2 依赖 cmake，sharpc 无法被 cmake 调用
- **Lua 脚本嵌入到 C 二进制**：build 时通过 embed_lua.cmake 把 `.lua` 转成 C 数组，编译进 spkg
- **不用 JSON**：`Sharp.lua` 返回 Lua table，用 `dofile()` 读取。JSON 解析器已删除
- **依赖拉取用系统 git**：`git clone` 到 `spkg_packages/`。libgit2 已编译好但尚未集成到 spkg

**命令**：
```
spkg init       # 生成 Sharp.lua
spkg build      # fetch + 编译 + 链接
spkg run        # build + 运行
spkg add <name> # 添加依赖
spkg update     # 更新依赖（未实现）
spkg clean      # 删除 build/ 和 spkg_packages/
spkg help       # 帮助
```

**项目清单格式 Sharp.lua**：
```lua
return {
  name = "my-project",
  version = "0.1.0",
  type = "exe",           -- exe | staticlib
  src = { "src/**/*.sp" },-- 支持 ** 递归 glob
  deps = { sharp_net = "*" },
  link = {}
}
```

**依赖解析流程**：
1. 检查 `Sharp.lock`（本地缓存）
2. 读取 `~/.sharp/config.spkm`（源映射）
3. → `git clone <url> → spkg_packages/<name>/`
4. 写入 `Sharp.lock`

**源配置 `~/.sharp/config.spkm`**：
```lua
return {
  source = {
    sharp_net = "git@gitee.com:xarvie/sharp-net.git"
  }
}
```

**构建流程**：
1. `spkg_fetch.fetch_deps()` — git clone 依赖
2. `spkg.glob("src/**/*.sp")` — 收集源文件（POSIX glob 不支持 **，降级到 find 命令）
3. `sharpc -c src/main.sp -o build/c/src_main.o` — 编译
4. `zig cc build/c/*.o -o build/<name>` — 链接
5. spkg 通过 `/proc/self/exe` 自动定位 sharpc，也可设 `SHARPC` 环境变量

### 5. net 网络库

- 位置：`/root/code/sharp/net/`
- 已推送到 gitee：`git@gitee.com:xarvie/sharp-net.git`
- 当前是纯 C 代码（`.c/.h`），`Sharp.lua` 标记为 `type = "staticlib"`
- 包含：socket, poller, ring(io_uring), tcp, udp, timer, pipe, ringbuf, bufp, wepoll

### 6. 构建 spkg

```bash
cd /root/code/sharp/sharp-pkg/spkg
rm -rf build && cmake -B build && cmake --build build
# 输出: build/spkg
```

构建依赖：cmake, zig, libgit2（已在 `../libgit2/build/` 预编译）

### 7. 当前状态 & 待做

已完成：
- spkg init/build/run/add/clean 全部可用
- 依赖 fetch（git clone）从 gitee 真实拉取验证通过
- ** glob 递归文件查找
- Lua 格式清单（Sharp.lua / Sharp.lock / config.spkm）

未完成：
- `spkg update` 命令未实现
- 依赖的自动编译（fetch 后只拉取，不编译 dep 的 .c/.sp）
- libgit2 集成（已编译但未在 spkg 中使用，目前走系统 git）
- 没有 `spkg publish` 或版本管理

### 8. 关键设计规则

- **sharpc 是有 zig**：spkg 构建依赖 zig（sharpc 内部调用 zig cc）
- **net 是扩展库**：它独立于 sharpc，有自己的 Sharp.lua 清单
- **不要修改 sharp-pkg/libgit2/ 的 cmake**：mbedTLS 是内置 2.28.9，已配置好
- **不要提交 sharp-pkg/lua/**：这是全量 Lua 5.4 源码（有 .gitignore 保护）
- **提交只包含 sharp-pkg/spkg/lua/**：spkg 用的 Lua 子集

### 9. 语言设计原则

- **显式 `this`**：方法通过显式 `this` 参数接收 receiver（如 `bool Str.eq(this, Str other)`），方法体内字段访问必须显式写 `this->field`。不支持隐式 `this` 自动前缀。关联函数（函数体内不使用 `this`）不接收 receiver 参数。