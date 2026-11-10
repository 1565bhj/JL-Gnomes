 # BLE 广播触发 OTA：第三步联调

本步目标是识别专用 OTA 广播包。设备仍然只扫描和打印，不缓存 URL，不解析 URL，不启动 HTTP 下载，也不写入固件。

## 调试目标

在第二步已经能够接收广播的基础上，确认设备只接受格式完整、分片字段合法且 CRC 校验通过的专用 OTA 广播包，拒绝普通广播和错误 OTA 包，避免后续误触发 URL 接收或 OTA 升级。

## 调试内容

- 验证专用包的 Company ID、`OTA` 标识、分片序号、总分片数和负载长度。
- 验证 AD Structure 长度检查，确认广播数据被截断、长度不一致或字段越界时不会被识别。
- 验证 CRC-8/ATM 计算范围、初始值和多项式，确认修改任意协议字段或 CRC 后都能被拒绝。
- 验证合法包输出 `OTA_ADV_RX` 和原始包日志，错误包输出对应的 `OTA_ADV_REJECT reason`。
- 验证旧的第二步测试包不会被误认为第三步合法 OTA 包。
- 验证扫描停止、WiFi 断开和重新进入 BLE 配网后，不再处理专用 OTA 广播。
- 本步不保存 URL Payload、不进行分片重组、不触发 HTTP 下载和固件写入。

调试通过标准：合法包能够稳定识别；长度、序号或 CRC 任一错误都被拒绝；普通广播不会产生合法 OTA 包日志；配网和 WiFi 联网流程不受影响。

## 本步协议

专用数据放在 BLE Advertising Data 的 Manufacturer Specific Data（AD type `FF`）中。采用 Legacy 广播，完整广播数据建议包含 Flags 和 Manufacturer Data：

```text
02 01 06
1B FF
FF FF 4F 54 41 QQ TT LL [payload] CC
```

其中：

| 字段 | 长度 | 说明 |
| --- | ---: | --- |
| `FF FF` | 2 | Company ID，当前开发标识 `0xFFFF`，低字节在前 |
| `4F 54 41` | 3 | ASCII `OTA`，专用包标识 |
| `QQ` | 1 | 当前分片序号，从 0 开始 |
| `TT` | 1 | 分片总数，不能为 0；`QQ` 必须小于 `TT` |
| `LL` | 1 | 当前负载长度，最大 17 字节 |
| `[payload]` | `LL` | 本步只打印，不解析其内容 |
| `CC` | 1 | CRC-8/ATM，计算范围为 Company ID 到 payload 末尾，不包含 `CC` |

CRC-8 参数：多项式 `0x07`，初始值 `0x00`，输入和输出均不反射。

Manufacturer Data 的 AD 长度字节必须根据实际负载长度填写：

```text
AD length = 1（AD type）+ 9（固定字段和 CRC）+ LL
```

当 `LL=0` 时，AD length 为 `0x0A`；当 `LL=17` 时，AD length 为 `0x1B`。完整广播中的 `1B FF` 只适用于 `LL=17` 的情况。

## 示例数据

下面示例为一个空负载、第 0/1 片的 OTA 专用包。其 Manufacturer Data CRC 为 `94`：

```text
0A FF FF FF 4F 54 41 00 01 00 94
```

含义为：

```text
Company ID=FFFF
Magic=OTA
Seq=00
Total=01
PayloadLen=00
CRC=94
```

测试时也可以先使用 `LL=0` 验证识别链路；后续 URL 分片阶段再填入负载并重新计算 CRC 和 AD length。

## 代码处理流程

`apps/common/ble/le_net_cfg.c` 中的 `ble_ota_scan_report()` 收到广播报告后，调用 `ble_ota_scan_ota_packet()`：

1. 遍历广播中的 AD Structure，并检查每个字段长度是否越界。
2. 只查找 AD type 为 `0xFF` 的 Manufacturer Data。
3. 匹配 Company ID `0xFFFF` 和 ASCII `OTA`。
4. 校验分片总数和分片序号。
5. 校验负载长度不超过 17 字节，且与实际 Manufacturer Data 长度一致。
6. 使用 CRC-8/ATM 校验包内容。
7. 全部通过后，增加 `ota_scan_matches` 并打印 `OTA_ADV_RX`。

当前只识别和打印包，不会保存 `payload`，也不会根据 URL 触发升级。

## 日志说明

识别成功：

```text
[BLE_OTA][... ms] OTA_ADV_RX seq=0/1 payload=0 crc=94 rssi=-45 addr=... matched=1
[BLE_OTA][... ms] OTA_ADV_RAW size=...: ...
```

识别失败时，代码会按约 1 秒限流打印：

```text
[BLE_OTA][... ms] OTA_ADV_REJECT reason=-2
```

原因值：

| 值 | 原因 |
| ---: | --- |
| `-1` | AD 字段格式或长度错误 |
| `-2` | 协议版本不支持 |
| `-3` | 包类型不支持 |
| `-4` | 分片字段或负载长度错误 |
| `-5` | CRC 校验失败 |

普通广播或只有旧格式的测试广播不会输出 `OTA_ADV_RX`，因为它们没有通过专用协议的完整校验。

## 硬件联调步骤

1. 编译、烧录并保存串口日志；确认 WiFi 联网后不会自动出现 `SCAN_START`，在 `ai_speaker` 中双击 `KEY_OK` 后再确认出现 `SCAN_START`。
2. 使用 Android nRF Connect Advertiser 或其他 BLE 广播工具发送上面的专用 Manufacturer Data。
3. 连续广播至少 10 秒，确认出现 `OTA_ADV_RX seq=0/1`。
4. 修改 `payload_len` 或 `total`，应出现 `OTA_ADV_REJECT reason=-2`，且不出现 `OTA_ADV_RX`。
5. 修改 CRC，应出现 `OTA_ADV_REJECT reason=-3`，且不出现 `OTA_ADV_RX`。
6. 修改 `seq` 为大于等于 `total`，应出现 `OTA_ADV_REJECT reason=-2`。
7. 重新进入 BLE 配网或断开 WiFi，确认 OTA 扫描停止，期间不再处理专用广播；退出配网或恢复联网后，必须再次双击 `KEY_OK` 才能重新扫描。

本步验收标准：只有完整、合法、CRC 正确的专用 OTA 广播包被识别；错误包被拒绝；正常配网和 WiFi 联网不受影响。
