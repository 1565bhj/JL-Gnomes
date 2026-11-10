# BLE 广播触发 OTA：第五步联调

本步将第四步重组得到的完整 URL 接入 HTTP OTA。设备收到完整、合法的 URL 后，会先停止 BLE 被动扫描，再异步启动 HTTP 下载；HTTP OTA 使用蓝牙广播中携带的 URL，不再强制使用固定地址。

## 调试目标

- 收齐 URL 分片后，确认设备输出 `OTA_HTTP_START_REQUEST`。
- 确认 BLE 扫描先停止，再输出 `OTA_HTTP_START`，避免扫描回调和 HTTP OTA 同时操作 BLE 控制器。
- 确认 HTTP OTA 日志中的 URL 与手机广播中的 URL 完全一致。
- 确认 HTTP 下载、固件头校验、固件数据长度和 CRC 校验仍按原流程执行。
- 确认同一 URL 的重复广播不会重复创建 HTTP OTA 任务。
- 确认 WiFi 未完成 DHCP、仍处于配网或 HTTP OTA 已忙时，不会误启动新的 OTA 任务。

## 当前实现范围

代码位置：

- `apps/common/ble/le_net_cfg.c`：URL 收齐后请求停止扫描，并在扫描停止后触发 HTTP OTA。
- `apps/common/update/http_update.c`：增加按 URL 启动的异步接口，并复制调用方传入的 URL。
- `include_lib/update/http_update.h`：声明 `http_ota_init_with_url()`。

接口行为：

```c
int http_ota_init_with_url(const char *url);
```

该接口只复制 URL 并创建 HTTP OTA 任务，不保存 BLE 接收缓存的指针。URL 为空、超过 127 字节、系统处于生产测试/闹钟唤醒状态，或已有 HTTP OTA 任务运行时，会拒绝启动并打印原因。

## 正常日志

收到最后一个分片后，应看到类似日志：

```text
[BLE_OTA][... ms] OTA_URL_READY len=34 url=http://8.163.15.180/update-ota.ufw
[BLE_OTA][... ms] OTA_HTTP_START_REQUEST url=http://8.163.15.180/update-ota.ufw
[BLE_OTA][... ms] SCAN_STOP reports=... matched=...
[BLE_OTA][... ms] SCAN_EXIT
[BLE_OTA][... ms] OTA_HTTP_START url=http://8.163.15.180/update-ota.ufw
-> HTTP OTA : http://8.163.15.180/update-ota.ufw
```

随后应继续出现 HTTP 下载进度，例如：

```text
OTA len:..., upload : ..., total:... (...%)
```

下载结束后，只有在固件头、固件数据长度、数据 CRC 和文件关闭均成功时，才允许出现：

```text
OTA write file OK
```

## 异常日志

```text
[BLE_OTA][... ms] OTA_HTTP_START_FAILED ret=...
```

表示扫描已经停止，但 HTTP OTA 任务没有成功创建。重点检查系统状态、HTTP OTA 是否已经运行，以及线程创建资源。

```text
HTTP OTA: URL too long (...)
HTTP OTA: start rejected, task is busy or system is unavailable
HTTP OTA: create task failed
```

分别表示 URL 超长、系统条件不满足或线程创建失败。

如果只看到 `OTA_URL_READY` 而没有 `OTA_HTTP_START_REQUEST`，说明 URL 重组后的合法性检查或重复 URL 判断没有通过；如果看到 `OTA_HTTP_START_REQUEST` 但没有 `SCAN_EXIT`，应检查 BLE 扫描停止命令返回值。

## 硬件联调步骤

1. 编译、烧录并确认设备已完成 WiFi DHCP；不要在 BLE 配网过程中开始本步。
2. 双击进入 BLE 广播接收模式，确认出现 `SCAN_START`。
3. 手机按第四步协议轮播发送：

   ```text
   FF FF OTA 00 02 11 <17字节 URL 前半段> CRC8
   FF FF OTA 01 02 11 <剩余 URL> CRC8
   ```

   实际广播中还要保留 BLE AD Type 和长度字段。具体分片长度以第四步联调已经验证的手机广播包为准。
4. 确认设备先输出 `OTA_URL_READY`、`OTA_HTTP_START_REQUEST`，然后停止扫描并输出 `OTA_HTTP_START`。
5. 确认 `-> HTTP OTA` 后面的 URL 为：

   ```text
   http://8.163.15.180/update-ota.ufw
   ```

6. 观察 HTTP 下载日志，确认进度持续增加；若服务端不返回总长度，应只打印已下载字节数，不应出现除零异常。
7. 用同一组分片重复广播，确认不再次出现 `OTA_HTTP_START`，也不重新创建 HTTP OTA 任务。
8. 在 HTTP OTA 下载期间再次发送 URL，确认出现任务忙或重复处理日志，不应同时启动第二个 HTTP OTA。
9. 分别测试错误 URL、超长 URL、错误 CRC 和乱序分片，确认都不会进入 HTTP OTA。

## 验收标准

完整 URL 能从 BLE 广播可靠传递到 HTTP OTA；扫描与 HTTP OTA 按“先停止扫描、后启动下载”的顺序执行；HTTP 日志中的 URL 正确；重复或非法广播不会重复启动；下载完成仍必须通过固件合法性和数据校验，不能仅凭 HTTP 请求结束判定升级成功。
