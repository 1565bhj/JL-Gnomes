# omni 设备语音互聊客户端（对外接口）使用说明

> 本文档基于 `websocket_sxy_platform.c` 中 omni 设备语音互聊客户端代码整理，只列出对外接口和可重定义的弱符号回调。
>
> 协议：AI 多模态协议 V1.3（28 字节 SEND 帧 / 12 字节 RESP 帧，大端，无逐片 ACK）
>
> 服务器地址：`wss://<host>/multimodal/omni/<MAC>-<UUID>-<批次号>`

------

## 一、对外 API（应用层可直接调用）

### 1. `qyai_omni_client_create`

**函数原型**

c

```
int qyai_omni_client_create(void);
```



**功能**

创建 omni 互聊客户端并连接服务器。录音发送/接收消息前调用。

**内部行为**

1. 检查网络是否就绪；
2. 创建 OS 同步对象（互斥锁，只创建一次）；
3. 分配发送帧缓存 `tx_buf`（28 字节帧头 + 一个分片）；
4. 分配 PCM 发送 pipe 缓存；
5. 打开 pipe；
6. 若当前状态为 `WBSK_INI`，启动 omni 主任务线程。

**返回值**

| 返回值 | 说明                                             |
| ------ | ------------------------------------------------ |
| `0`    | 成功                                             |
| `-1`   | 失败（网络未就绪 / 内存分配失败 / 线程创建失败） |

**调用说明**

- 已在线/正在连接时重复调用直接返回成功。
- 调用前需确保网络已连接（`sys_connect_net_success()` 为真）。

------

### 2. `qyai_omni_client_close`

**函数原型**

c

```
int qyai_omni_client_close(void);
```



**功能**

关闭 omni 互聊客户端连接并退出任务。

**内部行为**

1. 若当前在线，设置 socket 超时并请求关闭 websocket；
2. 向 omni 任务发送 `QYAI_OMNI_MSG_KILL` 消息。

**返回值**

| 返回值 | 说明                 |
| ------ | -------------------- |
| `0`    | 成功                 |
| `-1`   | 失败（任务未创建等） |

------

### 3. `qyai_omni_check_online`

**函数原型**

c

```
int qyai_omni_check_online(void);
```



**功能**

获取 omni 通道在线状态。

**返回值**

| 返回值 | 说明 |
| ------ | ---- |
| `1`    | 在线 |
| `0`    | 离线 |

------

### 4. `qyai_omni_send_pcm_push`

**函数原型**

c

```
int qyai_omni_send_pcm_push(char *buf, int len);
```



**功能**

录音过程中持续推送 PCM 数据。内部经 pipe 缓存后由 omni 任务分帧上行，不阻塞采集线程。

**参数**

| 参数  | 说明           |
| ----- | -------------- |
| `buf` | PCM 数据缓冲区 |
| `len` | PCM 数据长度   |

**音频格式要求**

- 16K 采样率
- 16bit
- 单声道

**返回值**

| 返回值 | 说明                                                         |
| ------ | ------------------------------------------------------------ |
| `0`    | 成功（包括服务端已判定说话结束、主动丢弃的情况）             |
| `-1`   | 失败（参数错误 / pipe 未初始化 / 离线 / pipe 满 / 入队失败） |

**调用说明**

- 服务端已下发 `RESP_STATUS_SPEAK_END` 后，本函数直接返回 `0`，不再上行。
- pipe 满时丢弃本次数据并返回 `-1`。
- 本路 PCM 流由 omni 任务实际发出第一帧时才开启 `sending_pcm`。

------

### 5. `qyai_omni_send_pcm_end`

**函数原型**

c

```
int qyai_omni_send_pcm_end(void);
```



**功能**

停止录音：发完残余 PCM 后补发空帧 EOF（`len=0`，`fin=1`）。

**服务端行为**

服务端收到 EOF 后，将本次完整音频上传 OSS（转 WAV），并把链接下发给 App。

**返回值**

| 返回值 | 说明                    |
| ------ | ----------------------- |
| `0`    | 成功                    |
| `-1`   | 失败（离线 / 入队失败） |

------

### 6. `qyai_omni_send_json`

**函数原型**

c

```
int qyai_omni_send_json(char *json);
```



**功能**

发送 JSON 控制消息（`dataType=2`，单条独立逻辑消息）。

**参数**

| 参数   | 说明                  |
| ------ | --------------------- |
| `json` | JSON 字符串，不能为空 |

**返回值**

| 返回值 | 说明                               |
| ------ | ---------------------------------- |
| `0`    | 成功                               |
| `-1`   | 失败（参数错误 / 离线 / 发送失败） |

**说明**

- 新 `ssrc`，`seq` 从 0 开始，超过 15KB 自动分片，末片 `fin=1`。

------

### 7. `qyai_omni_send_text`

**函数原型**

c

```
int qyai_omni_send_text(char *txt);
```



**功能**

发送纯文本消息（`dataType=1`，单条独立逻辑消息）。

**参数**

| 参数  | 说明                 |
| ----- | -------------------- |
| `txt` | 文本字符串，不能为空 |

**返回值**

| 返回值 | 说明                               |
| ------ | ---------------------------------- |
| `0`    | 成功                               |
| `-1`   | 失败（参数错误 / 离线 / 发送失败） |

**说明**

- 新 `ssrc`，`seq` 从 0 开始，超过 15KB 自动分片，末片 `fin=1`。

------

### 8. `qyai_omni_send_jpg`

**函数原型**

c

```
int qyai_omni_send_jpg(char *buf, int len);
```



**功能**

发送 JPEG 图片数据（`dataType=4`），超过 15KB 自动分片。

**参数**

| 参数  | 说明                 |
| ----- | -------------------- |
| `buf` | JPEG 数据缓冲区      |
| `len` | 数据长度，必须 `> 0` |

**返回值**

| 返回值 | 说明                               |
| ------ | ---------------------------------- |
| `0`    | 成功                               |
| `-1`   | 失败（参数错误 / 离线 / 发送失败） |

**说明**

- 新 `ssrc`，`seq` 从 0 开始，超过 15KB 自动分片，末片 `fin=1`。

------

## 二、应用层可重定义的弱符号回调

以下回调均为 `__attribute__((weak))`，应用层可自行重定义以接管处理逻辑。

### 1. `qyai_omni_online_callback`

**函数原型**

c

```
void qyai_omni_online_callback(char online);
```



**功能**

omni 通道上线/下线通知。

**参数**

| 参数     | 说明               |
| -------- | ------------------ |
| `online` | `1` 上线，`0` 下线 |

**默认行为**

打印 `-> omni online` / `-> omni offline`。

------

### 2. `qyai_omni_url_recv_callback`

**函数原型**

c

```
int qyai_omni_url_recv_callback(char *url, int len);
```



**功能**

收到 App 语音消息的 OSS 链接（WAV 格式），应用层下载并播放。

**参数**

| 参数  | 说明                   |
| ----- | ---------------------- |
| `url` | WAV 音频链接           |
| `len` | 字符串长度（含结束符） |

**默认行为**

打印 `omni url : %s`，返回 `0`。

------

### 3. `qyai_omni_json_recv_callback`

**函数原型**

c

```
int qyai_omni_json_recv_callback(char *json, int len);
```



**功能**

收到完整 JSON 消息（在解析 `"url"` 字段之前回调）。

**参数**

| 参数   | 说明                       |
| ------ | -------------------------- |
| `json` | JSON 字符串（已补 `'\0'`） |
| `len`  | 长度（含结束符）           |

**返回值**

| 返回值 | 说明                                |
| ------ | ----------------------------------- |
| `0`    | 继续走默认解析（code/url）          |
| 非 `0` | 应用层已自行处理，不再解析 url/code |

**默认行为**

返回 `0`。

------

### 4. `qyai_omni_text_recv_callback`

**函数原型**

c

```
int qyai_omni_text_recv_callback(char *buf, int len);
```



**功能**

收到纯文本消息（`dataType=1`，已补字符串结束符）。

**参数**

| 参数  | 说明             |
| ----- | ---------------- |
| `buf` | 文本缓冲区       |
| `len` | 长度（含结束符） |

**默认行为**

打印 `omni text : %s`，返回 `0`。

------

### 5. `qyai_omni_jpg_recv_callback`

**函数原型**

c

```
int qyai_omni_jpg_recv_callback(char *buf, int len);
```



**功能**

收到 JPEG/PNG 图片数据（`dataType=4/7`）。

**参数**

| 参数  | 说明           |
| ----- | -------------- |
| `buf` | 图片数据缓冲区 |
| `len` | 数据长度       |

**默认行为**

返回 `0`。

------

### 6. `qyai_omni_audio_recv_callback`

**函数原型**

c

```
int qyai_omni_audio_recv_callback(char type, char *buf, int len);
```



**功能**

收到音频数据（`dataType=0:PCM / 3:ogg-opus / 11:mp3`）。

**参数**

| 参数   | 说明           |
| ------ | -------------- |
| `type` | 音频数据类型   |
| `buf`  | 音频数据缓冲区 |
| `len`  | 数据长度       |

**默认行为**

返回 `0`。

**说明**

- 按 V1.3 互聊协议，下行音频统一走 OSS 链接，此回调仅作兼容保留。

------

### 7. `qyai_omni_error_callback`

**函数原型**

c

```
int qyai_omni_error_callback(int code, char *message);
```



**功能**

收到服务端错误通知（`code` 非 200，如 400 未绑定 / 403 封禁 / 413 过大）。

**参数**

| 参数      | 说明                    |
| --------- | ----------------------- |
| `code`    | 错误码                  |
| `message` | 错误描述，可能为 `NULL` |

**默认行为**

打印 `omni server err : code=%d msg=%s`，返回 `0`。

------

## 三、典型调用流程

c

```
/* 1. 创建 omni 客户端（需网络已就绪） */
if (qyai_omni_client_create() != 0) {
    /* 处理失败 */
    return;
}

/* 2. 等待上线（通过 qyai_omni_online_callback 通知） */
while (!qyai_omni_check_online()) {
    os_time_dly(10);
}

/* 3. 录音过程中持续推送 PCM */
qyai_omni_send_pcm_push(pcm_buf, pcm_len);

/* 4. 停止录音 */
qyai_omni_send_pcm_end();

/* 5. 发送文本 / JSON / 图片 */
qyai_omni_send_text("hello");
qyai_omni_send_json("{\"key\":\"value\"}");
qyai_omni_send_jpg(jpg_buf, jpg_len);

/* 6. 关闭客户端 */
qyai_omni_client_close();
```



------

## 四、应用层回调重定义示例

c

```
/* 上线/下线通知 */
void qyai_omni_online_callback(char online)
{
    if (online) {
        /* 上线处理 */
    } else {
        /* 下线处理 */
    }
}

/* 收到 App 语音 OSS 链接 */
int qyai_omni_url_recv_callback(char *url, int len)
{
    /* 下载并播放 WAV / mp3*/
    return 0;
}

/* 收到服务端错误 */
int qyai_omni_error_callback(int code, char *message)
{
    /* 错误处理 */
    return 0;
}
```



------

## 五、注意事项汇总

1. `qyai_omni_client_create()` 调用前需确保网络已连接。
2. `qyai_omni_send_pcm_push()` 不阻塞采集线程，内部经 pipe 缓存后由 omni 任务分帧上行。
3. PCM 格式要求：**16K 采样率、16bit、单声道**。
4. 服务端下发 `RESP_STATUS_SPEAK_END` 后，`qyai_omni_send_pcm_push()` 直接返回 `0`，不再上行。
5. `qyai_omni_send_pcm_end()` 必须在录音结束后调用，否则服务端无法收到 EOF，不会触发 OSS 上传。
6. 上行文本 / JSON / 图片均为独立逻辑消息（新 `ssrc`、`seq=0`、末片 `fin=1`），超过 15KB 自动分片。
7. 下行音频统一走 OSS 链接，`qyai_omni_audio_recv_callback` 仅作兼容保留。
8. 所有弱符号回调均可在应用层重定义，默认实现只打印日志或返回 0。
