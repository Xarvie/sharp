# Sharp webgui 标准库 — API 参考

> 语言定位: 纯 C11 语义，纯手动内存管理，零开销
> 设计理念: 只提供正交的基础设施，不做业务框架
> 版本: 1.0.0
> 日期: 2025-05-22

---

## 一、设计哲学

### 1.1 核心原则

**克制。** 标准库只提供通信、同步、渲染的基础设施，不做以下事情：

| 不做 | 原因 | 留给谁 |
|------|------|--------|
| 数据绑定 | 监听变量地址是框架行为 | 社区库 |
| 事件冒泡 | 复杂遍历无必要 | 业务代码自行处理 |
| 主题系统 | CSS 已经足够 | 前端生态 |
| 组件校验 | JSON 字符串透传最轻 | sp 语言本身的类型系统 |
| 渲染器抽象 | 99% 用户只需要换 HTML | 内部解耦，不暴露 API |

### 1.2 类比

webgui 不是 Qt（大包大揽），而是 **Linux 的 epoll + write()**：
- epoll 只管"有事件了通知你"，不管你怎么处理
- write() 只管"把数据发出去"，不管数据格式

webgui 只管两件事：
1. **把 UI 树的状态同步到浏览器**
2. **把浏览器的事件派发给 C 代码**

其余的一切，交给用户。

---

## 二、架构

### 2.1 整体模型

```
C 程序
  ├── Arena 内存池 (app 级，wg_app_destroy 时一键释放)
  ├── UI 节点树 (用户创建/修改)
  ├── vDom diff 引擎 (自动对比新旧树)
  ├── WebSocket 服务器 (通信)
  └── 事件派发器 (扁平路由)
         ↕ WebSocket (JSON)
      浏览器
         └── HTML/CSS/JS (可替换文件)
```

### 2.2 关键改进 vs 现有版本

| 改进项 | 现有 | 新版 |
|--------|------|------|
| 内存管理 | 手工 free | Arena 自动管理 |
| 渲染方式 | 全量推送 | vDom diff 增量 |
| HTML 页面 | 硬编码 | 可替换文件 |
| 组件类型 | 9 种写死 | 可注册扩展 |
| 渲染批次 | 每次 setter 都广播 | 每帧统一 diff 推送 |

---

## 三、内存管理：Arena 模型

### 3.1 设计

```c
struct wg_app {
    arena_t  arena;      // 应用级内存池
    ...
};
```

所有通过 `wg_button()`, `wg_text()`, `wg_create()` 创建的节点，内存都分配在 `app->arena` 中。

### 3.2 决策：一个 app 一个 Arena

**不需要多页面多 Arena。** 理由：
- 简单，用户不需要理解页面概念
- sp 是手动内存管理语言，需要中途释放时用 `wg_remove + wg_free` 即可
- 页面概念应由社区库封装，不在标准库中引入

### 3.3 用户行为

```c
wg_app_t *app = wg_app_create(root);
// 所有后续 wg_button/wg_text 等创建，自动分配在 app 的 arena 中

wg_app_destroy(app);
// 一键释放所有节点内存，用户不需要 free() 任何单个节点
```

### 3.4 边界情况

- **`wg_free(n)`** — 仅适用于**未挂载**的节点（即未调用 `wg_append` 的节点），释放回 arena
- **`wg_remove(parent, child)`** — 从树中摘除节点，但不释放内存
- **`wg_clear_children(parent)`** — 摘除所有子节点，不释放内存

### 3.5 Arena 实现要点

- 简单 bump allocator：指针偏移分配，O(1)
- 不需要支持单个释放（`wg_free` 只释放未挂载节点，挂载节点跟随 app 生命周期）
- 初始预留 64KB，按需扩展

---

## 四、渲染：虚拟 DOM Diff

### 4.1 工作流程

```
用户代码调用 wg_set_text(btn, "hello")
  → 更新本地节点树 (O(1))
  → 标记 dirty (不需要立刻推送)

事件循环末尾 / wg_render(app) 被调用
  → 对比 current_tree 和 sent_tree
  → 生成 JSON patch
  → 只推送变化的部分
  → 更新 sent_tree
```

### 4.2 决策：自动 render + 手动可选

**默认自动：** 每次事件回调返回后、定时器触发后，自动调用 `wg_render(app)`。用户无感。

**手动可选：** 用户可调用 `wg_render(app)` 强制刷新（例如在 `wg_post` 回调中、或需要立即推送时）。

**不提供 disable 自动 render 的 API。** 理由：
- 99% 的场景不需要
- 需要手动控制的用户可以直接修改 sent_tree 跳过 diff
- 避免增加 API 复杂度

### 4.3 Patch 格式

```json
{
  "type": "patch",
  "rev": 42,
  "ops": [
    {"op": "update", "id": "btn1", "prop": "text", "value": "hello"},
    {"op": "insert", "parent": "list", "index": 0, "node": {"id": "item3", "kind": "text", "text": "new"}},
    {"op": "remove", "id": "item1"},
    {"op": "attr", "id": "img1", "key": "src", "value": "logo.png"}
  ]
}
```

### 4.4 决策：diff 深度 — 包含树结构

diff 不仅对比叶子节点（文本/属性），也对比树结构（insert/remove）。

**理由：**
- insert/remove 是最常见的 UI 操作之一（列表增删）
- 只 diff 叶子会导致全量推送退化
- diff 树结构是 O(N)，代价可接受

### 4.5 决策：多客户端 sent_tree 独立维护

每个 `wg_client_t` 维护自己的 `sent_tree`。

**理由：**
- 不同客户端可能在不同时间连接，sent 状态不一致
- 新客户端连上时需要全量推送，旧客户端只需要增量
- 内存开销可控（每个 sent_tree 只是节点指针 + 属性快照，不复制节点本身）

---

## 五、事件：扁平派发

### 5.1 事件类型

内置事件：`click`, `input`, `change`, `submit`

自定义事件：用户可通过 `wg_emit` 触发任意事件名。

### 5.2 派发模型

```
前端发送: {"type": "event", "id": "btn", "event": "click", "value": ""}
后端接收: 查找 (id="btn", event="click") 的回调 → 直接调用
```

**无冒泡、无捕获。** 事件精准路由到注册的回调函数。

用户需要冒泡语义时，在回调中手动 `wg_emit(parent_id, event)`。

### 5.3 表单数据

`submit` 事件携带 `form_data`（JSON object），通过 `wg_form_get(ev, key)` 或 `wg_form_iter(ev, cursor, &key, &value)` 访问。

---

## 六、组件系统：轻量注册

### 6.1 自定义组件注册

```c
/* 组件操作集 — 可选，不需要就传 NULL */
typedef struct wg_component_ops {
    void (*on_destroy)(wg_node_t *n);  /* 节点销毁时，释放自定义资源 */
} wg_component_ops_t;

/* 注册新类型 */
int wg_register_type(const char *type_name, const wg_component_ops_t *ops);

/* 创建自定义组件 */
wg_node_t *wg_create(const char *type_name, const char *id);
```

### 6.2 决策：on_create 不需要

理由：`wg_create` 返回后用户可以立即执行初始化逻辑，不需要单独回调。

### 6.3 属性系统

自定义组件的属性通过 `wg_set_attr(n, key, value)` 设置，全为字符串透传。组件自己在回调中解析属性。

**不做类型校验。** 前端传什么，后端收到什么。

### 6.4 决策：组件属性不支持二进制

理由：
- WebSocket 消息以 JSON 为主，二进制需要额外协议支持
- 99% 的组件属性是字符串/数字/布尔，字符串足够
- 需要传输二进制的场景（如图片上传），通过文件路径字符串引用

---

## 七、前端资源：可替换

### 7.1 默认行为

使用内置 HTML（当前 `webgui_html.sph` 的内容）。

### 7.2 自定义前端

```c
/* 从文件加载 */
int wg_set_html_file(wg_app_t *app, const char *path);

/* 从字符串加载 */
int wg_set_html(wg_app_t *app, const char *html);
```

### 7.3 前端职责

HTML 文件包含：
- 页面结构
- CSS 样式/主题
- JS 通信逻辑（WebSocket 收发、DOM 操作）

用户可完全替换，也可以：
- 用 React/Vue 重写前端
- 引入 ECharts 做图表
- 自定义主题包

后端 C 代码**完全不需要改**。

---

## 八、生命周期

### 8.1 只保留 on_destroy

```c
typedef void (*wg_lifecycle_fn)(wg_node_t *n, void *ud);

int wg_on_destroy(wg_node_t *n, wg_lifecycle_fn fn, void *ud);
```

### 8.2 决策理由

| 生命周期 | 是否保留 | 理由 |
|----------|----------|------|
| on_create | 否 | `wg_create` 返回后可直接初始化 |
| on_update | 否 | diff 引擎内部处理，不需要暴露 |
| on_destroy | **是** | 自定义组件需要释放非 arena 管理的资源 |
| on_mount | 否 | 前端事件触发时后端已有回调，不需要额外生命周期 |

---

## 九、完整 API 列表

### 9.1 节点构造（11 个）

```c
wg_node_t *wg_container (const char *id);
wg_node_t *wg_text      (const char *id, const char *text);
wg_node_t *wg_heading   (const char *id, int level, const char *text);
wg_node_t *wg_button    (const char *id, const char *text);
wg_node_t *wg_input     (const char *id);
wg_node_t *wg_checkbox  (const char *id, const char *label);
wg_node_t *wg_select    (const char *id, const char *const *options, size_t n);
wg_node_t *wg_image     (const char *id, const char *src);
wg_node_t *wg_separator (const char *id);
wg_node_t *wg_form      (const char *id);
wg_node_t *wg_create    (const char *type_name, const char *id);  /* 自定义组件 */
```

### 9.2 节点操作（9 个）

```c
int  wg_append  (wg_node_t *parent, wg_node_t *child);
int  wg_append_n(wg_node_t *parent, wg_node_t *const *children, size_t n);
int  wg_insert  (wg_node_t *parent, wg_node_t *child, int index);
int  wg_remove  (wg_node_t *parent, wg_node_t *child);
void wg_remove_self(wg_node_t *n);
int  wg_clear_children(wg_node_t *parent);
void wg_free    (wg_node_t *n);           /* 仅未挂载节点 */
wg_node_t *wg_find(wg_app_t *app, const char *id);
const char *wg_get_id(const wg_node_t *n);
```

### 9.3 属性读写（10 个）

```c
int  wg_set_text    (wg_node_t *n, const char *text);
int  wg_set_value   (wg_node_t *n, const char *value);
int  wg_set_disabled(wg_node_t *n, int disabled);
int  wg_set_visible (wg_node_t *n, int visible);
int  wg_set_attr    (wg_node_t *n, const char *key, const char *value);

const char *wg_get_text    (const wg_node_t *n);
const char *wg_get_value   (const wg_node_t *n);
int         wg_get_disabled(const wg_node_t *n);
int         wg_get_visible (const wg_node_t *n);
const char *wg_get_attr    (const wg_node_t *n, const char *key);
```

### 9.4 渲染（3 个）

```c
int  wg_render        (wg_app_t *app);              /* 手动触发 diff + 推送 */
int  wg_set_html_file (wg_app_t *app, const char *path);
int  wg_set_html      (wg_app_t *app, const char *html);
```

### 9.5 事件（7 个）

```c
int wg_on      (wg_app_t *app, const char *id, const char *event,
                wg_callback_fn cb, void *userdata);
int wg_off     (wg_app_t *app, const char *id, const char *event);
int wg_emit    (wg_app_t *app, const char *id, const char *event,
                const char *value);

const char *wg_form_get  (const wg_event_t *ev, const char *key);
int         wg_form_iter (const wg_event_t *ev, size_t *cursor,
                          const char **key, const char **value);

int wg_register_type(const char *type_name, const wg_component_ops_t *ops);
```

### 9.6 生命周期（1 个）

```c
int wg_on_destroy(wg_node_t *n, wg_lifecycle_fn fn, void *ud);
```

### 9.7 应用（8 个）

```c
wg_app_t *wg_app_create (wg_node_t *root);
void      wg_app_destroy(wg_app_t *app);

int  wg_run (wg_app_t *app);
void wg_stop(wg_app_t *app);

int wg_bind_tcp(wg_app_t *app, const char *host, uint16_t port);

int wg_set_title(wg_app_t *app, const char *title);
int wg_set_icon (wg_app_t *app, const char *url);
int wg_set_debug(wg_app_t *app, int enabled);
```

### 9.8 定时器（2 个）

```c
int wg_set_timer  (wg_app_t *app, int delay_ms, int repeat,
                   wg_timer_fn fn, void *userdata);
int wg_clear_timer(wg_app_t *app, int timer_id);
```

### 9.9 跨线程（1 个）

```c
int wg_post(wg_app_t *app, wg_post_fn fn, void *userdata);
```

### 9.10 错误（2 个）

```c
int         wg_errno(void);
const char *wg_strerror(int err);
```

### 9.11 类型定义

```c
typedef struct wg_event {
    wg_app_t   *app;
    const char *id;
    const char *event;
    const char *value;
    void       *userdata;
} wg_event_t;

typedef void (*wg_callback_fn)(const wg_event_t *ev);

typedef void (*wg_timer_fn)(wg_app_t *app, void *userdata);

typedef void (*wg_post_fn)(wg_app_t *app, void *userdata);

typedef void (*wg_lifecycle_fn)(wg_node_t *n, void *ud);

typedef struct wg_component_ops {
    void (*on_destroy)(wg_node_t *n);
} wg_component_ops_t;
```

---

## 十、API 统计

| 分类 | 数量 |
|------|------|
| 节点构造 | 11 |
| 节点操作 | 9 |
| 属性读写 | 10 |
| 渲染 | 3 |
| 事件 | 7 |
| 生命周期 | 1 |
| 应用 | 8 |
| 定时器 | 2 |
| 跨线程 | 1 |
| 错误 | 2 |
| **函数合计** | **54** |
| 类型定义 | 5 |

> 其中 30 个是现有 API（不变），新增 24 个。

---

## 十一、设计决策记录

### 11.1 为什么用 Arena 而不是 GC/ARC？

sp 是手动内存管理语言。Arena 是手动管理的一种形式（批量管理 vs 逐个管理），不违背语言哲学。GC/ARC 会引入运行时开销和不确定性。

### 11.2 为什么一个 app 一个 Arena？

多页面多 Arena 引入了不必要的概念复杂度。sp 是手动内存管理语言，需要中途释放时用 `wg_remove + wg_free` 即可。页面概念应由社区库封装。

### 11.3 为什么保留 Virtual DOM？

WebSocket 带宽和前端重绘是硬瓶颈。即使 sp 是零开销语言，网络传输的开销不会消失。vDOM diff 是在 C 层做的 O(N) 操作，换取网络传输的 O(diff) 输出，是性价比最高的优化。

### 11.4 为什么自动 render？

手动 batch 需要用户记住调用，容易遗漏。自动 render 让用户像修改普通 C 结构体一样修改 UI，无需关心渲染时机。

### 11.5 为什么不做数据绑定？

监听变量内存地址的变化需要轮询（性能差）或 hook 内存写入（需要编译器支持）。两者都不符合 sp 的零开销哲学。显式 `wg_set_text` 是最佳方案。

### 11.6 为什么不做事件冒泡？

冒泡是 DOM 浏览器的历史包袱。sp 的事件模型应该更简单：注册什么，触发什么。用户需要冒泡语义时，可以在回调中手动 `wg_emit(parent_id, event)`。

### 11.7 为什么不暴露渲染器 API？

渲染器抽象是内部解耦手段，不是用户 API。99% 的用户只需要换 HTML。过早暴露渲染器接口会锁定渲染器协议、增加用户学习成本、引入不必要的抽象层。

### 11.8 为什么 diff 包含树结构？

insert/remove 是最常见的 UI 操作。只 diff 叶子会导致全量推送退化。diff 树结构是 O(N)，代价可接受。

### 11.9 为什么多客户端 sent_tree 独立？

不同客户端可能在不同时间连接，sent 状态不一致。新客户端连上时需要全量推送，旧客户端只需要增量。内存开销可控（只存属性快照，不复制节点）。

### 11.10 为什么只保留 on_destroy 生命周期？

on_create 可在 `wg_create` 后直接执行。on_update 由 diff 引擎处理。on_mount 已有事件回调覆盖。on_destroy 是唯一必需的——自定义组件需要释放非 arena 管理的资源。

### 11.11 为什么组件属性不支持二进制？

WebSocket 消息以 JSON 为主，二进制需要额外协议。99% 的组件属性是字符串/数字/布尔。需要传输二进制的场景通过文件路径字符串引用。

---

## 十二、协议规范

### 12.1 前端 → 后端

**事件消息：**
```json
{"type": "event", "id": "btn", "event": "click", "value": ""}
```

**表单提交：**
```json
{"type": "event", "id": "form1", "event": "submit", "value": "",
 "form_data": {"name": "test", "age": "25"}}
```

### 12.2 后端 → 前端

**初始渲染（新连接）：**
```json
{"type": "render", "title": "App", "tree": {...}}
```

**增量 Patch：**
```json
{"type": "patch", "rev": 42, "ops": [
  {"op": "update", "id": "btn1", "prop": "text", "value": "hello"}
]}
```

**元数据更新：**
```json
{"type": "meta", "title": "New Title"}
```

### 12.3 初始渲染协议

新客户端连接时，后端发送完整 `render` 消息。后续只发送 `patch`。

每个客户端的 `rev` 从 0 开始递增。`patch` 中的 `rev` 与客户端同步，客户端可据此检测丢包。
