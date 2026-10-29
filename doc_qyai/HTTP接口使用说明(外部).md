# 轻语 AI（QY-AI）HTTP 接口使用说明（外部）

> 适用模块：`http_chunck_stream`（framework 层，http_chunck_stream是支持分块传输的http协议）
>
> 本文档面向应用开发，只描述**应用层可调用的对外接口**：函数原型、参数、返回值、回调与调用示例。**内部实现不对外暴露**（如服务器地址、内部静态函数、请求体组装方式、缓冲管理等均不在本文档范围内）。

---

## 一、模块分层与调用关系

```
应用层 (apps/、lib/system/ 等)
        │  调用
        ▼
framework 层 (http)   ← 本文档描述这一层
        │  调用
        ▼
底层网络 (lwip socket / mbedtls)
```

- framework 层对上提供两类接口：
  1. **基础传输接口**：直接发起 HTTP/HTTPS 请求，回调返回原始数据。
  2. **业务接口**：在基础传输之上封装好的业务能力（绑定、智能体、内容点播、TTS、天气、上报等），上层只需传入业务参数与回调。
- 业务接口内部会自动完成设备标识（MAC/UUID）拼装、请求方法与路径选择等，**上层无需关心**。

---

## 二、通用约定

### 2.1 头文件

公共头文件位于 `include_lib/net/http_chunck_stream/`：

| 头文件 | 说明 |
| --- | --- |
| `http_all_api.h` | 部分业务接口（内容列表、智能体、TTS、天气、用户信息、图片等）的声明 |

> **注意**：仍有相当一部分接口**未在公共头文件中声明**，应用层使用前需自行 `extern` 声明，例如：
>
> ```c
> extern int http_wifi_device_bind_app(char *phone);
> extern int http_device_unbind_app(void);
> extern int http_tts_play_wait(void);
> ```
>
> 下文各接口的原型即为其**准确签名**，可直接照抄 `extern`。

### 2.2 统一数据回调（5 参数）

绝大多数底层接口与同步业务接口使用统一回调：

```c
int (*callback)(void *http, char *buf, int len, int payloadlen, char fin);
```

| 参数 | 含义 |
| --- | --- |
| `http` | 本次请求的 HTTP 句柄，失败时为 `NULL` |
| `buf` | 本次收到的数据缓冲区，失败时为 `NULL` |
| `len` | 本次收到的数据长度；**失败时底层传 `-1`** |
| `payloadlen` | 负载总长度（非分块时等于 `content_length`） |
| `fin` | 结束标志，`1` 表示最后一帧（成功结束或失败结束时均为 `1`） |

> **失败通知约定**：请求失败时，底层会以 `callback(NULL, NULL, -1, 0, 1)` 通知上层，应用可在回调中统一按 `buf == NULL || len <= 0` 判定失败。

### 2.3 常见简化回调

部分业务接口对回调做了简化，常见形态如下：

| 回调原型 | 出现位置 | 说明 |
| --- | --- | --- |
| `void (*cb)(char *buf, int len)` | 播客、教育课程 task 版 | 仅返回数据与长度 |
| `void (*cb)(int err_code, char *buf)` | 绑定/解绑、上报类 | `err_code` 为业务返回码（`200` 表示成功），`buf` 为消息 |
| `void (*cb)(int code, char *msg, char *data)` | SIM 卡绑定 | `code` 业务码、`msg` 消息、`data` 数据段 |
| `void (*cb)(char *buf, int role_idx)` | 智能体设置 | `role_idx > 0` 为智能体 ID，`-1` 表示失败 |
| `void (*cb)(char *data)` | MQTT 连接信息 | `data` 为服务器返回的原始 JSON 文本 |
| `void (*cb)(void *user_info)` | 用户信息 task 版 | `user_info` 为 `QYAI_USER_INFO *` |

### 2.4 返回值约定

- **同步接口**：请求完成后返回。`> 0` 表示成功（一般为接收到的数据长度）；`0` 表示失败/无数据；负值表示错误（见错误码）。
- **task（异步）接口**：通过 `thread_fork` 另起线程执行，立即返回线程创建结果，**成功一般为 `0`**；具体业务结果通过回调返回。部分 task 接口在创建线程失败时，会先以失败回调通知再返回错误码。
- **业务码**：服务器返回 JSON 中的 `errorCode` 字段，`200` 为操作成功（`PROCESS_CODE_SUCCESS`），`100` 为未定义错误（`ERROR_CODE_UNDEFINE`）。

### 2.5 同步版与 task（异步）版的区别

| | 同步版 | task（异步）版 |
| --- | --- | --- |
| 执行方式 | 阻塞调用者线程直至请求结束 | `thread_fork` 另起线程执行 |
| 返回值 | 数据长度 / 错误码 | 线程创建结果（`0` 成功） |
| 回调 | 结果在本次调用内回调 | 结果在线程内异步回调 |
| 命名 | 无 `_task` 后缀 | 一般以 `_task` 结尾 |

- **同步版是“一段流程”**：调用后**阻塞**当前线程，直到请求结束，数据/错误码在同一次调用中直接拿到。因此不要放在 LVGL、协议栈等对实时性敏感的线程里调用，否则会卡住界面/其它流程。
- **task 版只是“发起”**：立即返回线程创建结果，真正的结果**要等回调函数传回来**。调用返回时还没有数据，必须在回调里处理；若要“等结果”，可在回调中 `os_sem_post`、调用处 `os_sem_pend`。

> 命名并非绝对（如智能体同时提供 `http_ai_intell_set_id` 与 `http_ai_intell_set_id_task`），使用前请以本文档原型为准。

### 2.6 前置条件

- 调用业务接口前，应用层应保证设备已联网。部分接口内部会做联网检查，未联网时直接返回 `-1`。
- 同步接口若需在业务代码中“等待结果”，通常做法是：回调中 `os_sem_post`，调用处 `os_sem_pend` 等待（见第四章示例）。

联网判断示例：

```c
/* 主要接口：sys_connect_net_success()（联网判断） */
extern int sys_connect_net_success(void);   /* 系统联网状态：已联网返回非 0，否则返回 0 */

void fetch_something(void)
{
    /* 调用任何 http 业务接口前，先用它确认联网；未联网直接返回，避免无谓的等待/超时 */
    if (!sys_connect_net_success()) {
        printf("[net] 未联网，跳过 http 请求\n");
        return;
    }
    http_qyai_user_info_get(user_info_cb);  /* 已联网，再真正发起请求 */
}
```

> 也可用 `http_ip_connect_get()` 做服务器连通性检测（见第十三章）。

---

## 三、基础传输接口

### 3.1 GET 请求

```c
int http_chunck_stream_client_socket_request_thread(
        HTTP_CHUNCK_STREAM_INFO *HTTP,
        char *url,
        int (*buf_callback)(void *http, char *buf, int len, int payloadlen, char fin),
        char *static_buf, int static_buf_size);
```

- **作用**：以 GET 方式请求 `url`，通过回调返回数据。
- **参数**：
  - `HTTP`：句柄。传 `NULL` 由内部创建（独立缓冲区）；也可传外部静态实例。
  - `url`：完整请求地址。
  - `buf_callback`：数据回调，见 2.2。
  - `static_buf` / `static_buf_size`：外部提供的接收缓冲区及其大小；传 `NULL, 0` 则由内部管理。
- **返回值**：成功返回接收到的数据长度（`> 0`）；失败返回负错误码。

### 3.2 POST 请求（请求体方式）

```c
int http_chunck_stream_client_socket_post_body_thread(
        HTTP_CHUNCK_STREAM_INFO *HTTP,
        char *url, char *body_dat,
        int (*buf_callback)(void *http, char *buf, int len, int payloadlen, char fin),
        char *static_buf, int static_buf_size);
```

- **作用**：以 POST 方式请求 `url`，请求体为 `body_dat`（JSON）。
- **参数**：`body_dat` 为请求体字符串；其余同 3.1。
- **返回值**：同 3.1。

> `http_chunck_stream_client_socket_post_thread`（参数拼接在 URL 后）已标记为**逐步废弃**，新代码请使用 `post_body_thread`。

### 3.3 错误码

定义于 `http_chunck_stream_api.h`：

| 宏 | 值 | 含义 |
| --- | --- | --- |
| `HTTP_OK` | 0 | 成功 |
| `HTTP_ERR` | -1 | 通用错误 |
| `HTTP_ERR_NO_MEM` | -2 | 内存不足 |
| `HTTP_RECV_ERR` | -3 | 接收错误 |
| `HTTP_TIMEOUT` | -4 | 超时 |
| `HTTP_RANGE` | -5 | Range 错误 |
| `HTTP_STOP` | -6 | 已停止 |
| `HTTP_REQ_NO_FILE` | -6 | 请求的文件不存在 |

---

## 四、设备解绑

> 绑定成功后请将用户账户信息持久化（写入 Flash），解绑时需要用到该账户。

### 4.1 解除设备绑定

```c
int http_device_unbind_app(void);                 // 同步封装
int http_ai_device_unbind(void (*set_callback)(int err_code, char *buf));   // 直接请求
```

- **返回值**：`http_device_unbind_app` 返回 `-1` 未联网 / `-2` 超时 / 其他为底层结果；`http_ai_device_unbind` 未联网返回 `-1`，其他为底层结果。
- **回调**：`void (*set_callback)(int err_code, char *buf)`。

### 4.2 绑定 / 解绑（异步 task 版）

```c
int http_wifi_device_unbind_app_task(void);
```

- **返回值**：未联网返回 `-1`；任务已在进行或线程创建成功返回 `0`。

### 4.3 查询绑定状态（获取用户信息去判断）
详见  5.1、5.2。



- **返回值**：已成功绑定返回非 0，未绑定返回 0。

---

## 五、账号 / 用户信息

### 5.1 获取账号信息（同步，原始数据回调）

```c
int http_qyai_user_info_get(void (*callback)(void *http, char *buf, int len, int payloadlen, char fin));
```

- **回调**：5 参数统一回调；传 `NULL` 时使用内部默认解析回调。
- **返回值**：返回当前智能体 ID（内部解析后的 `works_id`）。

### 5.2 获取账号信息（异步 task，结构体回调）

```c
int http_qyai_user_info_get_task_create(void (*callback)(void *user_info));
```

- **回调**：`void (*callback)(void *user_info)`，参数为解析好的 `QYAI_USER_INFO *`。
- **返回值**：`0`。

`QYAI_USER_INFO` 结构（`http_chunck_stream_api.h`）：

```c
typedef struct user_info {
    void *user_nickname;
    void *user_account;
    void *user_phone;
    int   user_id;
    void *register_ip;
    void *esim_active_url;
    int   msg_code;
    int   works_id;
    int   timbre_id;
    void *works_name;
    void *callback_priv;
    void (*set_callback)(void *http, char *buf, int len, int payloadlen, char fin);
} QYAI_USER_INFO;
```

字段说明（数据来自服务端响应 `data` 节点）：

| 字段 | 说明 |
| --- | --- |
| `user_id` | 用户数字 ID |
| `user_account` | 账号（如 `QY_sf7J6n9A`） |
| `user_phone` | **手机号**（账号已绑定时有值） |
| `user_nickname` | 昵称 |
| `esim_active_url` | eSIM 激活地址（有则返回） |
| `works_id` | 当前智能体 ID |
| `works_name` | 当前智能体名称 |
| `timbre_id` | 当前音色 ID |
| `msg_code` / `register_ip` / `callback_priv` / `set_callback` | 内部保留字段，当前未填充 |

> **是否绑定**：服务端响应里 `code == 1` 表示设备**未授权（未绑定）**，此时 `message` 为提示信息、`data` 通常为空；能取到 `data`（例如 `user_phone` 有手机号）即表示账号**已绑定**。


**参考用法**（`lib/system/lvgl_v8/custom/screen_*`）：

```c
/* 主要接口：http_qyai_user_info_get() */
/* 回调：5 参数统一回调，收到的是原始 JSON，需自行解析；关键字段 data.mac_info.works_id */
static void user_info_cb(void *http, char *buf, int len, int payloadlen, char fin)
{
    if (!buf || len <= 0 || !fin) {          /* fin==0：还有后续分片，可先累积 */
        return;
    }
    char *json = malloc(len + 1);            /* 自行拷贝并补结束符后再解析 */
    memcpy(json, buf, len);
    json[len] = '\0';

    cJSON *root = cJSON_Parse(json);
    cJSON *data = cJSON_GetObjectItem(root, "data");
    cJSON *mac  = cJSON_GetObjectItem(data, "mac_info");
    cJSON *wid  = cJSON_GetObjectItem(mac, "works_id");   /* 字符串，atoi 转 int */
    if (cJSON_IsString(wid)) {
        s_works_id = atoi(wid->valuestring);              /* 记录账号当前智能体 ID */
    }
    cJSON_Delete(root);
    free(json);
}

/* 网络请求放独立线程，不能阻塞 LVGL / 主线程；未联网直接返回 */
if (sys_connect_net_success()) {
    thread_fork("user_info", 20, 2048, 0, 0, worker, NULL);  // worker 内调 http_qyai_user_info_get(user_info_cb)
}
```

> **要点**
> - 回调收到原始 JSON（`buf`/`len`），需自行拷贝补 `'\0'` 后再解析；`fin==0` 表示分片未结束。
> - 关键字段：`data.mac_info.works_id`（字符串 → `atoi`）。
> - 必须在独立线程里发起，调用前用 `sys_connect_net_success()` 判联网。
> - 想省去手动解析，可用 §5.2 的 `http_qyai_user_info_get_task_create`（回调直接给 `QYAI_USER_INFO *`）。

### 5.4 用户账户信息本地读写（需应用层实现）

```c
int user_data_account_info_opt(u8 opt_code, char *account, int len);
```

- **作用**：绑定后写入、解绑前读取用户账户（framework 层为空实现，**必须由应用层覆盖**）。
- **约定**：`opt_code == 0` 写入，`opt_code == 1` 读取。

---

## 六、智能体设置

```c
int http_ai_intell_set_str(char *str, void (*set_callback)(char *buf, int role_idx));   // 按名称设置
int http_ai_intell_set_id(int id_index, void (*set_callback)(char *buf, int role_idx)); // 按 ID 设置
int http_ai_intell_set_id_task(int id_index, void (*set_callback)(char *buf, int role_idx));
int http_ai_intell_set_success_status(void);                                            // 查询设置状态
```

- **回调**：`void (*set_callback)(char *buf, int role_idx)`
  - `role_idx > 0`：遍历到的智能体 ID（成功时也为该 ID）；
  - `role_idx == 0`：设置成功；
  - `role_idx < 0`：设置失败（如未绑定账号或网络异常）。
- **返回值**：成功发起返回 `0`（或请求结果）；未联网 / 线程创建失败返回 `-1`。
- **`http_ai_intell_set_success_status` 返回值**：`> 0` 设置成功并返回 ID；`-1` 设置失败；`-2` 未设置。

> **建议**：先按名称设置以获取完整列表与 ID，再按 ID 设置，速度可明显加快。

参考用法：

```c
/* 主要接口：http_ai_intell_set_str() / http_ai_intell_set_id() */
static void app_dev_intell_set_cb(char *buf, int role_idx)
{
    if (role_idx == 0) {          // 设置成功
        os_sem_post(&web_sync_sem);
    } else if (role_idx > 0) {    // 遍历智能体列表
        printf("id = %d, name: %s\n", role_idx, buf);
    } else {                      // 设置失败
        printf("set intelligent failed\n");
    }
}

http_ai_intell_set_str("奶龙", app_dev_intell_set_cb);   // 按名称
http_ai_intell_set_id(2254, app_dev_intell_set_cb);      // 按 ID
```


## 七、内容 / 课程 / 播客 / 汉字笔顺

### 7.1 内容列表与详情

```c
// 内容分类列表
int http_ai_get_content_list(int page, char *buf, int buf_size,
        void (*set_callback)(void *http, char *buf, int len, int payloadlen, char fin));
int http_ai_get_content_list_task(int page, char *buf, int buf_size,
        void (*set_callback)(void *http, char *buf, int len, int payloadlen, char fin));

// 某分类下的内容列表
int http_ai_get_content_data(int class_id, char *keyword, int page, char *buf, int buf_size,
        void (*set_callback)(void *http, char *buf, int len, int payloadlen, char fin));
int http_ai_get_content_data_task(int class_id, char *keyword, int page, char *buf, int buf_size,
        void (*set_callback)(void *http, char *buf, int len, int payloadlen, char fin));

// 内容详情
int http_ai_get_content_details_data(int id, int page, char *buf, int buf_size,
        void (*set_callback)(void *http, char *buf, int len, int payloadlen, char fin));
int http_ai_get_content_details_data_task(int id, int page, char *buf, int buf_size,
        void (*set_callback)(void *http, char *buf, int len, int payloadlen, char fin));

// 内容详情 V2
int http_ai_get_content_v2_details_data_task(int id, int page, char *buf, int buf_size,
        void (*set_callback)(void *http, char *buf, int len, int payloadlen, char fin));
```

- **参数**：
  - `page`：页码；
  - `class_id`：分类 ID；
  - `keyword`：搜索关键词（如 `"乌鸦喝水"`）；
  - `id`：内容 ID；
  - `buf` / `buf_size`：接收缓冲区及其大小；
  - `set_callback`：5 参数统一回调。
- **返回值**：同步版返回接收数据长度或负错误码；task 版返回线程创建结果；未联网 / 内存不足返回 `-1`。
- **回调失败**：`set_callback(NULL, NULL, -1, 0, 1)`。

**参考示例（取自 `lib/system/lvgl_v8/custom/screen_oral_common_api.c` 口语跟读页）**

该页面的典型做法是：**传入自备的 `buf` 缓冲区、并把回调置 `NULL`**——接口会把数据写入 `buf` 并以返回值为数据长度，拿到后自行用 cJSON 解析。网络请求放在独立线程里做，解析完成后用 `lv_async_call` 回主线程刷新 UI，避免阻塞 LVGL。

（1）拉取某分类下的内容列表（带分页），函数：http_ai_get_content_data()：

```c
/* 主要接口：http_ai_get_content_data() */
#define ORAL_BUF_SIZE (64 * 1024)

static void oral_list_fetch_worker(void *serial_opaque)
{
    int cid  = oral_get_class_id();     // 分类 ID：由页面注入
    int page = s_oral_current_page;     // 页码

    char *buf = malloc(ORAL_BUF_SIZE);
    if (!buf) {
        return;
    }
    memset(buf, 0, ORAL_BUF_SIZE);

    // keyword 传 NULL；回调传 NULL：数据直接写入 buf，返回值即数据长度
    int ret = http_ai_get_content_data(cid, NULL, page, buf, ORAL_BUF_SIZE, NULL);
    if (ret <= 0) {                     // ret <= 0：网络失败 / 内存不足 / 未联网
        printf("[口语列表] class_id=%d page=%d 拉取失败 ret=%d\n", cid, page, ret);
        free(buf);
        return;
    }

    int end = (ret < ORAL_BUF_SIZE) ? ret : (ORAL_BUF_SIZE - 1);
    buf[end] = '\0';                    // 自行补结束符，按 ret 截断防止越界

    cJSON *root = cJSON_Parse(buf);
    free(buf);
    if (!root) {
        return;                         // JSON 解析失败
    }

    cJSON *data = cJSON_GetObjectItem(root, "data");
    /* 列表数组字段名可能是 data / lists / list，按服务端返回兼容 */
    cJSON *list = cJSON_IsArray(data) ? data : cJSON_GetObjectItem(data, "list");

    if (list) {
        int n = cJSON_GetArraySize(list);
        for (int i = 0; i < n; i++) {
            cJSON *item  = cJSON_GetArrayItem(list, i);
            cJSON *id    = cJSON_GetObjectItem(item, "id");
            cJSON *title = cJSON_GetObjectItem(item, "title");
            if (!title) {
                title = cJSON_GetObjectItem(item, "name");
            }
            /* ... 保存 id / 标题到本地列表，供页面渲染 ... */
        }
    }
    cJSON_Delete(root);

    lv_async_call(oral_apply_list_fetch_cb, NULL);   // 回主线程刷新 UI
}

// 调用入口：起独立线程触发拉取
thread_fork("oral_list_fetch", 15, 2 * 1024, 0, 0,
            oral_list_fetch_worker, (void *)(uintptr_t)serial);
```

（2）拉取某条内容的详情：

```c
/* 主要接口：http_ai_get_content_details_data() */
#define ORAL_DETAIL_BUF_SIZE (32 * 1024)

static void oral_detail_fetch_worker(void *serial_opaque)
{
    int content_id = s_oral_detail_content_id;   // 内容 ID

    char *buf = malloc(ORAL_DETAIL_BUF_SIZE);
    if (!buf) {
        return;
    }
    memset(buf, 0, ORAL_DETAIL_BUF_SIZE);

    // page 传 0；同样使用 NULL 回调 + 自备缓冲区
    int ret = http_ai_get_content_details_data(content_id, 0,
                                               buf, ORAL_DETAIL_BUF_SIZE, NULL);
    if (ret <= 0) {
        printf("[口语详情] content_id=%d 拉取失败 ret=%d\n", content_id, ret);
        free(buf);
        return;
    }

    int end = (ret < ORAL_DETAIL_BUF_SIZE) ? ret : (ORAL_DETAIL_BUF_SIZE - 1);
    buf[end] = '\0';

    cJSON *root = cJSON_Parse(buf);
    free(buf);
    if (!root) {
        return;
    }

    cJSON *data         = cJSON_GetObjectItem(root, "data");
    cJSON *content_text = data ? cJSON_GetObjectItem(data, "content_text") : NULL; // 正文
    cJSON *translation  = data ? cJSON_GetObjectItem(data, "translation")  : NULL; // 译文
    cJSON *title        = data ? cJSON_GetObjectItem(data, "title")        : NULL; // 标题
    cJSON *author       = data ? cJSON_GetObjectItem(data, "author")       : NULL; // 作者

    if (cJSON_IsString(content_text) && content_text->valuestring) {
        strncpy(s_oral_detail_text, content_text->valuestring, sizeof(s_oral_detail_text) - 1);
        s_oral_detail_text[sizeof(s_oral_detail_text) - 1] = '\0';
    }
    /* ... 其余字段（translation / title / author）同理按需拷贝 ... */
    cJSON_Delete(root);

    lv_async_call(oral_detail_apply_fetch_cb, NULL);   // 回主线程刷新 UI
}

// 调用入口：起独立线程触发详情拉取
thread_fork("oral_detail_fetch", 15, 2 * 1024, 0, 0,
            oral_detail_fetch_worker, (void *)(uintptr_t)serial);
```

> **要点**：
> - **回调传 `NULL` + 自备 `buf`**：数据写入 `buf`，返回值为数据长度，`<= 0` 视为失败。适合“一次性拿全量数据再自行解析”的场景；若数据较大或想边收边处理，则改用回调方式。
> - `buf` 需自行补 `'\0'`，并按返回值截断长度（`end = min(ret, buf_size - 1)`），防止越界。
> - 网络请求应放在独立线程（`thread_fork`）；解析完成后用 `lv_async_call` 回主线程操作 LVGL 控件，避免跨线程刷 UI。
> - 列表/详情字段名以服务端返回为准（示例兼容了 `title/name`、`data/lists/list` 等写法）。

### 7.2 可变参数内容接口

```c
int http_ai_get_content_v2_details_data_task_parms(int argc, ...);
```

- **用法**（GET）：第 1 个可变参数为回调函数（5 参数统一回调），其后为若干查询参数串（内部以 `&` 拼到 URL 后面），如 `"id=124"`、`"page=10"`。
- **`argc`**：可变参数总个数 = 1 个回调 + N 个参数串。

```c
/* 主要接口：http_ai_get_content_v2_details_data_task_parms()
 * 作用：按可变参数拼装 GET 请求，拉取「内容详情 V2」，数据通过回调返回。
 * 约定：第 1 个可变参数是回调，后面每个参数串写成 "key=value"，库会用 '&' 拼到 URL 上。 */

/* ---- 1. 先拿到 id：它来自「内容列表」接口（见 7.1）的返回 ----
 * 列表 JSON 每一项都有一个 id 字段，详情接口就是靠这个 id 定位内容的；
 * 该字段有的服务端返回数字、有的返回字符串，这里统一转成 int。 */
static int content_id_from_json(cJSON *item)
{
    cJSON *id = cJSON_GetObjectItem(item, "id");   /* 取出该项的 id 字段 */
    if (cJSON_IsNumber(id)) {                      /* 数字型（如 124）：直接取值 */
        return (int)id->valuedouble;
    }
    if (cJSON_IsString(id) && id->valuestring) {   /* 字符串型（如 "124"）：atoi 转换 */
        return atoi(id->valuestring);
    }
    return 0;                                      /* 取不到 id，返回 0（表示无详情） */
}

/* 列表接口（http_ai_get_content_data，见 7.1）的回调：
 * 从返回的 JSON 里解析出每一项的 id 和标题，保存到本地，供详情接口 / 列表 UI 使用。 */
static void on_content_list(void *http, char *buf, int len, int payloadlen, char fin)
{
    if (!buf || len <= 0 || !fin) {
        return;                       /* fin==0 表示数据还没收完；buf 为空表示本次失败 */
    }
    cJSON *root = cJSON_Parse(buf);   /* buf 是原始 JSON 文本，需自行解析 */
    cJSON *data = cJSON_GetObjectItem(root, "data");
    cJSON *list = cJSON_IsArray(data) ? data : cJSON_GetObjectItem(data, "list");  /* 列表数组字段名兼容 data / list */
    if (list && cJSON_GetArraySize(list) > 0) {
        cJSON *item = cJSON_GetArrayItem(list, 0);      /* 这里以第 0 项为例 */
        s_detail_id  = content_id_from_json(item);      /* ← 记下 id，详情接口要用它 */
        cJSON *title = cJSON_GetObjectItem(item, "title");
        if (!title) {
            title = cJSON_GetObjectItem(item, "name");  /* 标题字段名兼容 title / name */
        }
        /* ... 保存标题，供详情页 / 列表显示 ... */
    }
    cJSON_Delete(root);               /* 解析完释放；valuestring 会一并释放，需先拷走再用 */
}

/* ---- 2. 用上面拿到的 id + 页码拼成参数串，请求详情 ----
 * 参数串必须写成 "key=value"，库会以 '&' 拼到 URL 后面。 */
static void fetch_content_detail(void)
{
    char id_parm[32];
    char page_parm[32];

    snprintf(id_parm,   sizeof(id_parm),   "id=%d",   s_detail_id);  /* id：来自内容列表，定位要看的这条内容 */
    snprintf(page_parm, sizeof(page_parm), "page=%d", 0);            /* page：详情页码，一般从 0 开始 */

    /* argc 的可变参数个数 = 1 个回调 + N 个参数串；这里 1 + 2 = 3 */
    http_ai_get_content_v2_details_data_task_parms(3, content_detail_cb, id_parm, page_parm);
}
```

### 7.3 教育课程表

```c
int http_get_edu_course_list(char *buf, int buf_size,
        void (*set_callback)(void *http, char *buf, int len, int payloadlen, char fin));
int http_get_edu_course_list_task(void (*set_callback)(char *buf, int len));   // 异步，简化回调

int http_get_edu_course_details(char *buf, int buf_size,
        void (*set_callback)(void *http, char *buf, int len, int payloadlen, char fin));
int http_get_edu_course_details_task(void (*set_callback)(char *buf, int len)); // 异步，简化回调
```

- **说明**：同步版回调为 5 参数；task 版回调为简化 2 参数 `void (*set_callback)(char *buf, int len)`。
- **返回值**：同步版返回数据长度或负错误码；task 版返回线程创建结果。定制APP！！！！！！

### 7.4 播客

```c
int http_ai_get_podcast_list(void (*set_callback)(char *buf, int len));
int http_ai_get_podcast_data(char *class_id, char *keyword, char *page_idx, char *per_page,
        void (*set_callback)(char *buf, int len));
```

- **参数**：`class_id` 分类 ID；`keyword` 关键词；`page_idx` 页码；`per_page` 每页数量。
- **回调**：`void (*set_callback)(char *buf, int len)`。
- **返回值**：失败（回调为空 / 内存不足）返回 `-1`，其他为请求结果。

### 7.5 汉字动态笔顺（GIF）

```c
int http_ai_ch_char_get(int argc, ...);
```

- **用法**（GET）：第 1 个可变参数为回调函数，其后为查询参数串：

```c
http_ai_ch_char_get(3, http_ch_char_get_cb, "char=狗", "size=300");
```

参考用法（来自 `apps/wifi_story_machine/chatbot/audio/ai_speaker.c`）：

```c
/* 主要接口：http_ai_ch_char_get() */
char font[32];
char img_size[32];
snprintf(font, sizeof(font), "char=%s", "狗");
snprintf(img_size, sizeof(img_size), "size=%d", 300);
http_ai_ch_char_get(3, http_ch_char_get_cb, font, img_size);
```

---

## 八、图片 / 拍照

### 8.1 获取图片 URL 列表

```c
int http_ai_get_img_url(int page, int Page_size,
        void (*set_callback)(void *http, char *buf, int len, int payloadlen, char fin));
```

- **参数**：`page` 页码；`Page_size` 每页数量。
- **返回值**：未联网 / 内存不足返回 `-1`，其他为请求结果。
- **回调**：5 参数统一回调，`buf` 为图片列表 JSON。

### 8.2 下载图片

```c
int http_ai_download_img(char *req_url,
        void (*set_callback)(void *http, char *buf, int len, int payloadlen, char fin));
```

- **参数**：`req_url` 为**完整**图片地址（域名 + 路径 + 参数）。
- **回调**：`buf` 为图片二进制数据，可直接写入文件系统。

示例：

```c
/* 主要接口：http_ai_download_img() */
int http_ai_download_img_app(void)
{
    char *url = "https://<图片域名>/xxx/xxxx.jpg";
    return http_ai_download_img(url, ai_dev_download_url_cb);
}

// 回调中将数据写到 SD 卡
static int ai_dev_download_url_cb(void *http, char *buf, int len, int payloadlen, char fin)
{
    if (len > 0) {
        FILE *fp = fopen("img/download.jpg", "wb");
        if (fp) { fwrite(buf, 1, len, fp); fclose(fp); }
    }
    return 0;
}
```

### 8.3 拍照套餐信息

```c
int http_ai_get_photo_grap(void (*callback_funs)(void *http, char *buf, int len, int payloadlen, char fin));
```

- **作用**：GET 获取拍照套餐信息。

```c
/* 主要接口：http_ai_get_photo_grap() */
http_ai_get_photo_grap(http_get_photo_grap_cb);！！！！！！！！！！！！！！！！！！！！
```

---

## 九、TTS（语音合成播放）

### 9.1 单条文本合成与播放

```c
int http_tts_request(char *utf8_str, int utf8_str_size);
int http_tts_reply(char *utf8_str, int utf8_str_size);
int http_tts_play_wait(void);          // 等待播放完成，超时返回 -1
int http_tts_is_playing(void);         // 返回是否正在播放
```

参考用法：

```c
/* 主要接口：http_tts_request() / http_tts_play_wait() */
http_tts_request("你好", strlen("你好") + 1);
http_tts_play_wait();
```

### 9.2 按类型播放提示音

```c
int http_tts_reply_type(int type);
```

- **`type` 取值**：`1` 唤醒、`2` 10 秒无对话、`3` 退下、`4` 没听清、`5` 好的、`6` 关机。

### 9.3 多句合成（长文本）

多句合成需按以下步骤处理（文本量大必须分句）：

```c
/* 主要接口：http_tts_play_buf_callback_reg() / http_tts_request() / http_tts_text_buf_free_size() */
extern void http_tts_play_buf_callback_reg(int (*cb)(char *text, char *buf, int len, int payloadlen, char fin));
extern void http_tts_play_no_wait_set(char no_wait_flag);
extern int  http_tts_text_buf_free_size(void);
extern void http_tts_text_buf_clean(void);

// 1、注册 buf 回调、清空文本缓存、设置不等待
http_tts_play_buf_callback_reg(tts_buf_callback_reg);
http_tts_text_buf_clean();
http_tts_play_no_wait_set(1);

// 2、分句后逐句调用，调用前先用 http_tts_text_buf_free_size() 检查剩余空间
if (http_tts_text_buf_free_size() > 256) {
    http_tts_request(sentence, strlen(sentence) + 1);
}
```

- **`http_tts_play_buf_callback_reg`** 注册的回调为 `int (*cb)(char *text, char *buf, int len, int payloadlen, char fin)`：`text` 为对应文本，`buf` 为音频数据（最后一帧为 `NULL`），`payloadlen` 在最后一帧为最后一个索引，`fin` 为最后一帧标志。
- **`http_tts_text_buf_free_size`**：返回文本循环缓冲剩余空间。

**完整参考示例（来自 `lib/system/lvgl_v8/custom/screen_content.c` 长文本朗读）**

一次“注册回调 → 清缓存 → 不等待 → 分句逐条 request”的完整流程，重点演示 `free_size` 与 `request` 的重试写法：

```c
/* 主要接口：http_tts_play_buf_callback_reg() / http_tts_request() / http_tts_text_buf_free_size() */
/* 音频 / 文本回调：一边收一边推进播放缓冲 */
int tts_buf_callback_reg(char *text, char *buf, int len, int payloadlen, char fin)
{
    if (music_play_alarm_ring()) {
        return 0;
    }
    if (buf) {
        music_buf_play_accept(1);
        if (text) {
            txt_buf_play_push(text, strlen(text) + 1, tts_buf_count, 0);  /* 文本与音频按 index 对齐 */
        }
        music_buf_play_push(buf, len, tts_buf_count++, 0);
        return len;
    } else if (payloadlen) {                     /* 最后空帧，payloadlen 为最后一个 index */
        music_buf_play_end_index(payloadlen);
    }
    return 0;
}

/* 起播：注册回调 + 清文本缓存 + 不等待 */
http_tts_play_buf_callback_reg(tts_buf_callback_reg);
tts_buf_cnt_clean();
http_tts_text_buf_clean();
http_tts_play_no_wait_set(1);

/* 逐句合成：调用 http_tts_request 前先确认文本循环缓冲空间够用，并做有限重试 */
static void send_one_segment(const char *start, int byte_len)
{
    if (byte_len <= 0) {
        return;
    }
    char *seg = malloc(byte_len + 1);
    if (!seg) {
        return;
    }
    memcpy(seg, start, byte_len);
    seg[byte_len] = '\0';

    int retry = 200;    /* 等剩余空间 >= 本次长度 + 余量 */
    while (http_tts_text_buf_free_size() < (byte_len + 64) && --retry) {
        if (s_tts_abort) { free(seg); return; }
        os_time_dly(1);
    }
    retry = 10;         /* 单句写入最多重试 10 次 */
    while (http_tts_request(seg, byte_len + 1) < 0 && --retry) {
        if (s_tts_abort) { free(seg); return; }
        os_time_dly(1);
    }
    free(seg);
}
```

> **要点**：
> - 分句建议以句末标点（`。！？\n`）为切分点，长句再按逗号 / 固定字节数二次切分；**单次写入建议不超过 2048 字节**。
> - `http_tts_request` 的长度传 `strlen(str) + 1`（含结束符）。
> - 长文本会持续写入文本循环缓冲，**务必先用 `http_tts_text_buf_free_size()` 判断空间**，不足则等待重试，否则会丢句。
> - 切换内容前先 `http_tts_play_stop_force()` + `http_tts_text_buf_clean()` 清理，避免旧数据残留在 `cycbuf` 里，导致新一段一写就“满 + 重试”。

### 9.4 播放控制

```c
int http_tts_play_request_stop(void);   // 请求停止
int http_tts_play_stop_force(void);     // 强制停止
int http_tts_loop(int enable);          // 使能/关闭循环
void http_tts_play_buf_static_set(char *buf, int len);  // 设置播放静态缓冲
```

参考用法（来自 `apps/wifi_story_machine/iflytek/ifly_wake_demo.c`）：唤醒 / 对话打断时，先强制停止 TTS，再停止管道音乐。

```c
/* 主要接口：http_tts_play_stop_force() / http_pipe_music_play_request_stop() */
http_tts_play_stop_force();
http_pipe_music_play_request_stop(NULL);
```

---

## 十、天气

```c
// 天气预报3天
int qyai_weather_forecast_get(char *utf8, int utf8_size, char *buf, int buf_size);

// 天气预报3天
int http_weather_forecast_get(char *utf8, int utf8_size,
        int (*cb)(void *http, char *buf, int len, int payloadlen, char fin));
// 实时天气
int http_weather_now_get(char *utf8, int utf8_size,
        int (*cb)(void *http, char *buf, int len, int payloadlen, char fin));
```

- **参数**：`utf8` 城市名（UTF-8），可传 `NULL` 使用默认；`utf8_size` 城市名长度。
- **返回值**：`> 0` 成功；`0` 失败；`-1` 未绑定账号。

示例：

```c
/* 主要接口：qyai_weather_forecast_get() */
char buf[1024];
int ret = qyai_weather_forecast_get(NULL, 0, buf, sizeof(buf));
if (ret > 0) {
    // buf 中为天气 JSON
} else if (ret == -1) {
    printf("未绑定账号\n");
}
```

回调方式示例（来自 `lib/system/lvgl_v8/custom/screen_main.c`）：实时天气 `http_weather_now_get` 用回调接收 JSON，再异步刷 UI。

```c
/* 主要接口：http_weather_now_get() */
static int weather_data_cb(void *http, char *buf, int len, int payloadlen, char fin)
{
    (void)http; (void)payloadlen; (void)fin;
    int ret = 0;
    if (buf && len > 0) {
        char *data_buf = malloc(len + 1);
        if (!data_buf) {
            return ret;
        }
        memset(data_buf, 0, len + 1);
        memcpy(data_buf, buf, len);
        /* 回 LVGL 线程刷 UI；失败返回非 0，由回调自行释放 */
        ret = lv_async_call(update_weather_ui_from_json, data_buf) == LV_RES_OK ? 0 : -1;
        if (ret) {
            free(data_buf);
        }
    }
    get_weather_ok = 0;
    return ret;
}

int fetch_weather_data(void)
{
    if (!lv_demo_system_net_connect_success() || get_weather_ok) {
        return -1;
    }
    get_weather_ok = 1;
    int result = http_weather_now_get(NULL, 0, weather_data_cb);   /* 城市 / 长度传 NULL、0 用默认 */
    if (result <= 0) {
        printf("[天气] result=%d（-1 表示未绑定账号）\n", result);
        get_weather_ok = 0;
    }
    return result;
}
```

字段解析（实时天气，`screen_main.c`）：`results[0].location.name`（城市）、`results[0].now.text`（天气）、`now.temperature`（温度）、`now.humidity`（湿度）、`now.wind_direction`（风向）、`now.wind_speed`（风速）。

> **两种调用契约**：
> - `qyai_weather_forecast_get(utf8, utf8_size, buf, buf_size)`：**同步**，数据写入自备 `buf`，返回长度，`> 0` 成功。
> - `http_weather_now_get(utf8, utf8_size, cb)` / `http_weather_forecast_get(utf8, utf8_size, cb)`：**回调**方式，在 `cb` 内处理 JSON。
> - 两者 `result == -1` 均表示**未绑定账号**。

---

## 十一、音乐播放

### 11.1 网络音乐播放

```c
int http_music_play_request(char *url, int buf_size);
int http_music_play_request_stop(void);
int http_music_play_set_puase(char pause, int timeout_sec);   // pause: 1 暂停 / 0 恢复
int http_music_play_get_puase(void);
int http_music_play_get_stop(void);
int http_music_play_set_volume(int volume);
int http_music_play_set_volume_step(int step);
void http_music_play_recv_buf_static(char *buf, int len);
void http_music_play_end_callback(int (*cb)(int event, int time));
```

- **`http_music_play_request` 的 `buf_size`**：**内部接收缓冲区（用来接收网络音频数据）的大小**（字节），由库内部使用，**不是音频文件长度**；调用方按可用内存给一个合理值即可（如 8K / 32K / 128K）。
- **`http_music_play_recv_buf_static`**：改用外部提供的内存作为接收缓冲区（`buf` 首地址、`len` 大小），与 `http_music_play_request` 的 `buf_size` 二选一。
- **`http_music_play_end_callback`**：注册播放结束回调，回调原型 `int (*cb)(int event, int time)`。

参考用法（来自 `apps/wifi_story_machine/chatbot/audio/net_music.c`）：

```c
/* 主要接口：http_music_play_request() / http_music_play_end_callback() */

/* 注册播放结束回调（event / time 由库回传） */
http_music_play_end_callback(dec_server_event_handler);

/* buf_size 为内部接收缓冲区大小，按可用内存给值，不必等于歌曲大小 */
int http_buf_size = 32 * 1024;      /* 小内存用 8K，大 SDRAM 可用 128K */
int ret = http_music_play_request(url, http_buf_size);

/* 播放控制 */
http_music_play_set_volume(volume);
http_music_play_request_stop();     /* 停止播放 */
```

### 11.2 管道音乐播放（多路句柄模型）

```c
void *http_pipe_music_play_request_start(char *url);          // 返回句柄，失败返回 NULL
int   http_pipe_music_play_request_stop(void *handler);       // handler 为 NULL 时停止全部
int   http_pipe_music_play_loop(void *handler, int loop_cnt); // 设置循环次数
int   http_pipe_music_play_stop_status(void *handler);        // 查询停止状态
int   http_pipe_music_play_set_volume(int volume);
int   http_pipe_music_play_set_volume_step(int step);
```

参考用法（来自 `apps/wifi_story_machine/chatbot/audio/ai_speaker.c`）：

```c
/* 主要接口：http_pipe_music_play_request_stop() / http_pipe_music_play_set_volume() / http_pipe_music_play_set_volume_step() */
// 停止管道音乐（handler 传 NULL 表示停止全部）
http_pipe_music_play_request_stop(NULL);

// 与其它播放通路一起统一设置音量 / 步进调节
int ret = music_buf_play_set_volume(volume) && music_play_set_volume(volume)
          && net_music_set_dec_volume(volume) && http_pipe_music_play_set_volume(volume);

http_pipe_music_play_set_volume_step(VOLUME_STEP);
```

> **注意**：应用层亦存在以无参形式声明与调用的写法，例如 `extern int http_pipe_music_play_stop_status(void);` → `http_pipe_music_play_stop_status();`（查询全局是否已停止播放）。使用时请与库内原型 `(void *handler)` 对齐。

---

## 十二、设备数据上报

### 12.1 喝水量上报 / 查询

```c
int http_ai_dev_water_dat_post(int water_intake, int year, int month, int day,
        char hour, char min, char sec, int water_tmp,
        void (*set_callback)(int err_code, char *buf));

int http_ai_dev_water_dat_get(void (*set_callback)(int err_code, char *buf));
```

- **参数**：`water_intake` 喝水量（ml）；`year/month/day/hour/min/sec` 饮水时间（**年月日必须与当天一致**，否则查询结果为 0）；`water_tmp` 水温。
- **回调**：`void (*set_callback)(int err_code, char *buf)`。
- **返回值**：未联网返回 `-1`，其他为请求结果。

### 12.2 闹钟信息上报

```c
int http_ai_dev_alarm_dat_post(char *reminder_time, char *reminder_text,
        void (*set_callback)(int err_code, char *buf));
int http_ai_dev_alarm_update_to_server(char *alarm_table,
        void (*set_callback)(int err_code, char *buf));
```

- **参数**：`reminder_time` 提醒时间；`reminder_text` 提醒文本；`alarm_table` 为闹钟表 JSON 片段（如 `"alarm":[]` 表示清空）。

### 12.3 设备异常上报

```c
int http_ai_dev_err_info_report(const char *reason, const char *trace_id,
        void (*set_callback)(int err_code, char *buf));
int http_ai_dev_err_info_report_task(const char *reason, const char *cur_trace_id);
int http_ai_dev_err_info_report_one_task(const char *reason, const char *cur_trace_id);
```

- **参数**：`reason` 异常原因；`trace_id` 追踪 ID。
- **说明**：`_task` 版为异步上报；`_one_task` 版保证同一时刻仅有一个上报任务在执行，适合频繁调用场景。

---

---

## 十三、时间与连通性

```c
int64_t http_ntp_utc_time_get(void);   // 获取网络时间（秒）；失败返回 0
int     http_ip_connect_get(void);     // 服务器连通性检测；失败返回 -1
```

---

## 十四、综合参考示例

以下为应用层“同步等待回调”的典型组织方式（取自 demo）：

```c
/* 主要接口：http_ai_device_unbind() */
static OS_SEM web_sync_sem;
#define char account[32];
    user_data_account_info_opt(1, account, sizeof(account));
    SYNC_TIME_OUT (300)

// 回调中释放信号量
static void ai_dev_unbind_set_cb(int err_code, char *msg)
{
    printf("err_code:%d, message:%s\n", err_code, msg);
    os_sem_post(&web_sync_sem);
}

// 调用处等待
int http_ai_device_unbind_app(void)
{
    char account[32];
    user_data_account_info_opt(1, account, sizeof(account));
    int ret = http_ai_device_unbind(ai_dev_unbind_set_cb);
    if (ret < 0) {
        printf("http_ai_device_unbind error\n");
    } else if (OS_TIMEOUT == os_sem_pend(&web_sync_sem, SYNC_TIME_OUT)) {
        printf("sem waitTime OUT\n");
    }
    return ret;
}
```

> 初始化时需先 `os_sem_create(&web_sync_sem, 0);`。

---

## 十五、注意事项

1. **接口声明**：未出现在 `http_all_api.h` 中的接口，应用层使用前需自行 `extern` 声明。
2. **回调线程上下文**：task（异步）接口的业务回调运行在 framework 创建的线程中，回调内避免长时间阻塞。
3. **数据生命周期**：统一回调中的 `buf` 只在当次回调内有效，如需保留请自行拷贝。
4. **设备标识**：业务接口会自动拼接设备 MAC / UUID 等标识，上层无需手动传入。
5. **重试**：底层对超时等场景存在自动重试（最多 3 次），上层无需重复处理。
6. **服务器环境切换**：正式 / 测试 / 灰度环境的切换由系统统一控制，业务代码无需关心具体地址。

## 十六、音频暂停接口
/**
 * @brief 停止网络音乐播放链路，完整关闭音乐、TTS、websocket相关资源
 * @note 【调用顺序很关键】：先停止播放输出 → 强制停止TTS → 停止音乐请求流 → 释放缓冲区内存
 *       → 请求关闭websocket连接 → 等待播放硬件/缓冲区完全停止，再清理计数。
 *       顺序不能随意调换：
 *       1.先停止播放器输出，避免还在往音频驱动送数据
 *       2.强制停止TTS，防止TTS与音乐混音冲突
 *       3.停止http音乐拉流请求，不再接收网络音频数据
 *       4.释放websocket缓存buffer，释放音乐播放环形buffer，避免内存泄漏
 *       5.发起websocket关闭请求，通知底层断开连接
 *       6.music_buf_play_stop_waite()阻塞等待播放流程真正结束，【必须等待完毕后才能播放后续提示音】
 *       7.最后清空网络音乐序号计数，状态复位
 */
```c
/* 主要接口：http_tts_play_stop_force() / http_pipe_music_play_request_stop() */
void aisp_all_pause(char stop)
{
    int to = 100;
    if (stop) {
        ifly_send_pcm = SEND_PCM_PAUSE;
        sys_vad_clear(0);
    } else {
        ifly_send_pcm = SEND_PCM_INI;
        sys_vad_clear(1);
    }
    keyword_start = 0;
    sys_vad_active = 0;

    union audio_req req = {0};
    if (server_handle) {
        req.enc.cmd = AUDIO_ENC_GET_STATUS;
        server_request(server_handle, AUDIO_REQ_ENC, &req);
        int status = req.enc.status;
        memset(&req, 0, sizeof(req));
        if (status == AUDIO_ENC_START && stop) {
            req.enc.cmd = AUDIO_ENC_PAUSE;
            server_request(server_handle, AUDIO_REQ_ENC, &req);
#if CONFIG_AISP_AEC_USE_LINEIN
            if (linein_server_handle) {
                server_request(linein_server_handle, AUDIO_REQ_ENC, &req);
            }
#endif
            printf("-> AUDIO_ENC_PAUSE \n");
        } else if (status == AUDIO_ENC_PAUSE && stop == 0) {
            req.enc.cmd = AUDIO_ENC_START;
            server_request(server_handle, AUDIO_REQ_ENC, &req);
#if CONFIG_AISP_AEC_USE_LINEIN
            if (linein_server_handle) {
                server_request(linein_server_handle, AUDIO_REQ_ENC, &req);
            }
#endif
            printf("-> AUDIO_ENC_START \n");
        }
    }
    music_buf_play_set_stop();
    http_tts_play_stop_force();
    http_pipe_music_play_request_stop(NULL);
    websockets_free_lbuf_buf();
    music_buf_play_free_lbuf();
    websockets_close_request(1);
    music_buf_play_stop_waite();//等待关闭完成再播放提示音
    net_music_num_clear();
    while (!music_buf_play_stop_status() && --to) {
        os_time_dly(1);
    }
}