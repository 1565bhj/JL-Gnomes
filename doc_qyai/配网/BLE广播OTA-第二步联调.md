# BLE 广播触发 OTA：第二步联调

本步验证“WiFi 应用联网、BLE 配网退出后，接收手机广播”。设备不建立 BLE 连接，收到测试广播只记录日志。本步不解析 URL，不触发下载或写固件。第三步启用专用 OTA 包校验后，本文中的旧测试格式仅用于回顾第二步，不能再作为第三步的合法 OTA 包。

## 调试目标

确认 WiFi 已联网且 BLE 配网完全退出后，设备能够稳定进入 BLE 被动扫描并收到手机广播，同时确认扫描不会与 BLE 配网、BLE 连接或断网流程冲突。本步只验证“能扫描、能接收、能打印”，不验证 OTA 协议和升级功能。

## 调试内容

- 验证蓝牙初始化后扫描管理定时器创建，以及 `ai_speaker` 中 `KEY_OK` 双击请求后的扫描启动条件是否全部满足。
- 验证被动扫描参数：间隔 100ms、窗口 30ms、关闭重复包过滤，不建立 BLE 连接。
- 使用 Android nRF Connect Advertiser 发送第二步测试广播，检查 `GAP_EVENT_ADVERTISING_REPORT`、`TEST_ADV_RX`、`ADV_RX` 和 `SCAN_STATS` 日志。
- 验证广播放在 Advertising Data 而不是 Scan Response，并确认第二步测试标记能够被识别。
- 测试断网、重新进入配网、配网成功退出三种切换场景，确认出现 `SCAN_STOP`；配网完成后不会自动恢复扫描，需在 `ai_speaker` 中再次双击 `KEY_OK`。
- 本步收到广播后只打印和统计，不解析 URL、不缓存分片、不启动 HTTP 下载、不写入固件；WiFi 联网后不会自动扫描，必须在 `ai_speaker` 中双击 `KEY_OK`。

调试通过标准：联网且退出配网后，双击 `KEY_OK` 才能开始接收测试广播；断网或进入配网后停止接收；配网功能正常；恢复前提后再次双击才能重新接收广播。

## 本步代码

| 文件 | 作用 |
| --- | --- |
| `apps/wifi_story_machine/include/app_config.h` | `CONFIG_BLE_OTA_SCAN_ENABLE=1` 开启本步功能，改为 0 可关闭 |
| `apps/common/ble/include/ble_ota_scan.h` | 蓝牙就绪、WiFi 应用联网状态和双击触发接口；其他 BLE profile 下为空实现 |
| `apps/common/ble/le_net_cfg.c` | 复用配网模块 HCI 回调，管理扫描启停、广播过滤和配网延迟退出 |
| `apps/wifi_story_machine/bt_ble/bt_music.c` | 蓝牙初始化完成后创建扫描管理定时器 |
| `apps/wifi_story_machine/chatbot/audio/ai_speaker.c` | 应用联网时允许扫描，应用断网时撤销许可 |
| `apps/wifi_story_machine/wifi_app_task.c` | WiFi 底层断开、DHCP 超时即撤销许可 |
| `apps/common/net/config_network.c` | 重新进入配网即撤销许可 |

当前工程使用 `DEF_BLE_DEMO_NET_CFG`，控制器配置已经包含 `LE_SCAN`。不需要打开 `BT_NET_CENTRAL_EN`，也不需要新增 C 文件到工程。

## 扫描条件与切换

双击请求后，同时满足以下条件才提交扫描开启命令：

1. 蓝牙初始化完成，扫描管理定时器创建成功。
2. 应用收到 WiFi 的 `NET_EVENT_CONNECTED`，且 WiFi 当前仍有 IP；LTE 联网不算。
3. 配网标志清零，没有 BLE 连接，没有等待中的配网结果。
4. 配网广播控制标志已关闭，软件记录的广播启用状态已清除。
5. 配网成功后的 1000ms 延迟退出任务已经执行或被新一轮配网取消；即使定时器申请失败，也会保持扫描关闭。

保留原有 500ms 配网停止和 1000ms BLE 退出延时。新增对延迟退出任务的跟踪，重新配网时取消旧任务；已经排队的旧回调通过轮次检查忽略，避免关闭新一轮配网。

管理定时器每 200ms 检查一次。正常断网后在下一次检查停止扫描；命令提交失败时约每秒重试。重新开启配网广播前同步提交停止扫描命令；若停止失败，先重试，成功后再开启配网广播。

扫描使用被动模式，周期 100ms、窗口 30ms，重复包过滤关闭。扫描和广播启停接口返回成功表示 SDK 接受命令，不等于已经观察到控制器完成事件；能持续收到测试广播才是本步接收成功的证据。

## 手机测试包

推荐用支持 BLE 广播的 Android 手机和 nRF Connect 的 Advertiser 功能。手机不需要与设备配对或连接，但需要打开蓝牙并授予应用所需权限。iPhone 常见广播工具受系统 API 限制，通常不能发送这里定义的自定义 Manufacturer Data；本轮建议先用 Android 或另一块 BLE 开发板验证。

创建 Legacy 广播，优先使用不可连接类型，间隔设为 100～200ms。将 Manufacturer Specific Data 放在 **Advertising Data**，不要放在 Scan Response（设备被动扫描，不主动请求扫描响应）。不同版本工具的字段名称可能略有不同。

Manufacturer Data 分开填写时：

| 字段 | 内容 |
| --- | --- |
| Company ID | `0xFFFF`，仅作为本轮开发测试标识 |
| Data（十六进制） | `4F 54 41 01` |

`4F 54 41` 是 ASCII `OTA`，最后的 `01` 是测试序号，可改为 `02`、`03`。若工具已经单独填写 Company ID，Data 中不要再次填写 `FF FF`。

若工具要求输入 Manufacturer Data 完整内容（包括 Company ID，但不含 AD 类型和长度），填写：

```text
FF FF 4F 54 41 01
```

若工具支持输入完整原始广播数据，可以使用下面的 12 字节：

```text
02 01 06 07 FF FF FF 4F 54 41 01
```

其中 `02 01 06` 是 Flags；`08 FF` 是后一个 AD 字段的长度和 Manufacturer 类型；其后的 `FF FF` 才是 Company ID。这是临时测试格式，后续 URL 分包协议另行定义。

## 硬件联调顺序

先使用现有 CodeBlocks 工程编译、烧录，并从上电开始保存串口日志。本次软件检查不执行烧录。

1. **保存 WiFi 后重启**：保持路由器可用。先确认 `SCAN_INIT timer=` 后面的值非 0，再等待 WiFi 联网完成；此时不应自动出现 `SCAN_START`。
2. **双击进入扫描**：在 `ai_speaker` 中双击 `KEY_OK`，应出现 `SCAN_REQUEST` 和 `SCAN_START`。启动手机广播，手机放在设备旁边，连续发送至少 10 秒。应出现 `TEST_ADV_RX seq=1`，以及 `SCAN_STATS` 的 `matched` 持续增长。将序号改成 `02` 后重启手机广播，确认收到 `seq=2`。
3. **手机重新配网**：保持手机测试广播发送，让设备进入原有配网流程。应先出现 `SCAN_STOP`，随后手机仍可正常发现配网设备、连接并配网。成功退出配网后，再次双击 `KEY_OK` 才应出现 `SCAN_START`，再收到 `TEST_ADV_RX`。
4. **断网恢复**：关闭路由器，应出现 `SCAN_STOP`，断网期间不再输出测试包接收日志。恢复路由器并重新满足联网前提后，仍需再次双击 `KEY_OK` 才启动扫描。如果设备自动进入配网，按原流程完成它。
5. **无 WiFi 上电**：关闭路由器后重启设备。允许出现 `SCAN_INIT`，但不应出现 `SCAN_START` 或 `TEST_ADV_RX`。

示例日志（时间、地址、计数因环境而异）：

```text
[BLE_OTA][... ms] SCAN_INIT timer=...
[NET][... ms] APP_CONNECTED net_ch=0 cfg=0
[BLE_OTA][... ms] SCAN_REQUEST source=key_double_click
[BLE_OTA][... ms] SCAN_START passive interval=100ms window=30ms
[BLE_OTA][... ms] TEST_ADV_RX seq=1 rssi=-45 addr=... type=1 matched=1
[BLE_OTA][... ms] SCAN_STATS reports=80 matched=20
[BLE_OTA][... ms] SCAN_STOP reports=100 matched=28
```

`TEST_ADV_RX` 首包立即打印，后续最多每秒打印一次；`SCAN_STATS` 每 5 秒输出一次。计数是本轮扫描会话累计值，重新启动扫描后清零。重复包过滤关闭，因此 `matched` 增长不代表收到了不同命令。

## 定位与回传

| 现象 | 优先检查 |
| --- | --- |
| 没有 `SCAN_INIT` | 是否烧录了新固件、蓝牙是否完成初始化、功能宏及当前 BLE profile |
| `SCAN_INIT timer=0` | 扫描管理定时器申请失败 |
| 双击后没有 `SCAN_START` | 是否为 WiFi、WiFi 是否已完成 DHCP、配网是否结束、BLE 是否断开、延迟退出是否执行、是否有 `SCAN_REQUEST_REJECT` 或 `SCAN_START_FAILED` |
| `SCAN_START_FAILED` / `SCAN_STOP_FAILED` | 保存返回值和前后日志，检查协议栈状态及是否有其他模块占用扫描 |
| `reports` 增长，`matched=0` | 测试数据、Company ID、AD 类型、广播是否误放在 Scan Response、是否启用了扩展广播 |
| `reports=0` | 手机是否真的开始广播、蓝牙权限、距离、手机广播能力；可用另一台手机扫描确认广播存在 |
| 收到测试包但没有升级 | 本步预期行为；URL 接收与升级触发在后续步骤实现 |

请回传双击进入扫描、重新配网、断网恢复三组完整串口日志，并说明手机型号、应用名称和实际发包数据。日志中的 WiFi 密码与账户信息请先删除。本步验收目标是正常配网不受影响，且只有在满足联网前提并双击后才能进入扫描、收到指定测试广播。
