# BLE 广播触发 OTA：第四步联调

本步实现专用 OTA 广播分片的接收、按序缓存和 URL 重组。收到全部分片后只打印完整 URL，暂不调用 HTTP OTA，也不写入固件。

进入/退出接收模式的入口为 `ai_speaker` App 中 `KEY_OK` 双击，对应现有按键动作 `KEY_DOUBLE_RECV_BLE`。第一次双击进入，扫描中再次双击退出。WiFi 联网不会自动启动 BLE OTA 扫描。

## 调试目标

- 收到 `seq=0/total` 后建立一轮 URL 接收。
- 只接受同一轮中按序到达的分片，拒绝缺片、乱序、总分片数变化和超长 URL。
- 收齐分片后输出 `OTA_URL_READY`，确认 URL 内容完整。
- 手机持续重复广播时，同一 URL 只作为重复包提示，不重复触发后续动作。
- 断网、重新进入 BLE 配网或扫描停止时清空当前缓存。
- 双击前提不满足时拒绝本次请求，后续条件恢复也不会自动进入，必须重新双击。
- 扫描中双击退出时，由扫描管理任务执行控制器停止命令，避免按键线程直接操作 BLE 控制器。

## 当前实现范围

代码位置：`apps/common/ble/le_net_cfg.c`。

- URL 缓存上限：64 字节。
- 分片数量上限：4 个。
- 分片接收超时：10 秒。
- 只处理已经通过第三步格式和 CRC 校验的分片。
- 拼接完成后要求 URL 以 `http://` 或 `https://` 开头，且内容为可打印 ASCII。
- 本步不会调用 `http_update.c`，不会下载或写入固件。

## 正常日志

双击后应先出现：

```text
[BLE_OTA][... ms] SCAN_REQUEST source=key_double_click
[BLE_OTA][... ms] SCAN_START passive interval=100ms window=30ms
```

扫描中再次双击应出现：

```text
[BLE_OTA][... ms] SCAN_EXIT_REQUEST source=key_double_click
[BLE_OTA][... ms] SCAN_EXIT
```

两个分片依次收到后应出现：

```text
[BLE_OTA][... ms] OTA_ADV_RX seq=0/2 payload=17 ...
[BLE_OTA][... ms] OTA_ADV_RX seq=1/2 payload=17 ...
[BLE_OTA][... ms] OTA_URL_READY len=34 url=http://8.163.15.180/update-ota.ufw
```

其中 URL 为：

```text
http://8.163.15.180/update-ota.ufw
```

如果手机重复发送已经完成的同一组分片，会看到：

```text
[BLE_OTA][... ms] OTA_URL_DUPLICATE len=34
```

这表示重复广播被识别，不表示再次启动升级。

## 异常日志

如果 WiFi 尚未完成 DHCP、仍处于 BLE 配网、存在 BLE 连接或配网广播尚未退出，会出现：

```text
[BLE_OTA][... ms] SCAN_REQUEST_REJECT prerequisites
```

本次请求会被清除，条件满足后需要重新双击。

```text
[BLE_OTA][... ms] OTA_URL_DROP reason=-1
```

分片为空或总分片数超过上限。

```text
[BLE_OTA][... ms] OTA_URL_DROP reason=-2
```

没有先收到 `seq=0`、分片乱序，或当前分片的 `total` 与首包不一致。

```text
[BLE_OTA][... ms] OTA_URL_DROP reason=-3
```

拼接后的 URL 超过 128 字节。

```text
[BLE_OTA][... ms] OTA_URL_DROP reason=-4
```

拼接结果不是 HTTP/HTTPS URL，或包含不可打印字符。

## 硬件联调步骤

1. 编译、烧录并确认 WiFi 联网，但只确认联网日志，不应自动出现 `SCAN_START`。
2. 在 `ai_speaker` App 中双击 `KEY_OK`，确认出现 `SCAN_REQUEST` 和 `SCAN_START`。
3. 手机按轮播方式发送 `seq=0/2` 和 `seq=1/2`，每个分片重复发送，设备无需同时接收两个包。
4. 确认两个 `OTA_ADV_RX` 日志都出现，并确认出现一条 `OTA_URL_READY`。
5. 暂停 `seq=0`、只发送 `seq=1`，应出现 `OTA_URL_DROP reason=-2`，不应输出 `OTA_URL_READY`。
6. 先发送 `seq=0`，再发送错误 `total` 或跳过序号，应丢弃本轮缓存。
7. 两个分片之间间隔超过 10 秒后重新发送，应不能使用超时前的旧缓存拼接。
8. 扫描过程中再次双击 `KEY_OK`，确认出现 `SCAN_EXIT_REQUEST` 和 `SCAN_EXIT`，之后不再接收广播。
9. 让设备断网或重新进入 BLE 配网，确认扫描停止后不会输出旧 URL。

## 验收标准

连续收到两个合法分片时，设备能准确输出完整 URL；异常顺序和超时不会生成 URL；重复广播不会重复产生有效 URL；WiFi 和 BLE 配网流程不受影响。本步完成后再进入下一步，将 `OTA_URL_READY` 接入 HTTP OTA 启动入口。
