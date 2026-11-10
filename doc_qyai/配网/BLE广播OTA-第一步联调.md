# BLE 广播触发 OTA：第一步联调

本步增加 WiFi 与 BLE 配网时序日志，用硬件确认后续切入广播扫描的时机。
本步未开启 BLE 扫描，未增加广播协议，也未改变 OTA 触发方式和配网关闭时序。

## 调试目标

确认设备在三种关键状态下的时序和边界：首次 BLE 配网、保存 WiFi 后自动联网、WiFi 断开及恢复。重点确认 WiFi 应用联网成功与 BLE 配网完全退出之间的先后关系，为第二步启动 BLE 被动扫描确定可靠门槛，且不能影响原有配网流程。

## 调试内容

- 增加并核对 WiFi 获取 IP、应用联网/断网、BLE 连接断开、配网成功通知和 BLE 广播启停相关日志。
- 区分“获取 IP 成功”“应用收到联网事件”和“BLE 配网退出完成”，避免仅凭其中一个日志就启动后续扫描。
- 验证配网成功后的 500ms 停止路径和 1000ms 延迟退出路径，确认 BLE 连接断开完成后不会意外重新开启配网广播。
- 使用手机首次配网、保存 WiFi 重启、断网恢复、再次配网四组场景采集完整串口日志。
- 本步只观察和记录，不启动 BLE OTA 扫描，不解析广播包，不触发 OTA 下载。

调试通过标准：正常配网仍可完成；WiFi 联网、配网成功通知、BLE 断开和配网广播关闭顺序明确；断网和再次配网时没有异常的 BLE 模式切换。

## 已确认的时序与日志精简

已核对手机正常配网和上电自动联网两组硬件日志：

- 手机配网：获取 IP → 应用联网 → 提交配网成功通知（返回 0）→ 500ms 退出路径请求断开 → BLE 断开完成。随后 1000ms 退出路径再次提交关闭广播命令。
- 自动联网：使用保存的 WiFi 获取 IP，经服务器连通性检查后应用收到联网事件；本次日志未进入 BLE 配网流程。

当前已删除第一步用于确认请求、定时器、事件转发等细节的临时日志。保留以下日志用于后续广播扫描联调：

| 前缀 | 标记 | 含义 |
| --- | --- | --- |
| `[NET]` | `WIFI_IP_READY` | 获取 IP 成功，附 IPv4 地址 |
| `[NET]` | `APP_CONNECTED` / `APP_DISCONNECTED` | 应用联网 / 断网，附网络通道和配网状态 |
| `[BLE]` | `BLE_DISCONNECTED` | BLE 断开完成，附原因、配网状态和广播控制标志 |
| `[BLE]` | `BLE_ADV_CMD_FAILED` | 广播启停命令提交失败时打印返回值 |
| `[BLE]` | `BLE_NOTIFY_SUCCESS_FAILED` | 配网成功通知提交失败时打印返回值 |

原有 500ms / 1000ms 退出时序保持不变。通知提交成功不等于手机已收到；`adv_ctrl=0` 是软件控制标志，不等于控制器停止广播完成事件。
后续扫描实验需同时满足 WiFi 应用联网和配网 BLE 已退出，不能只检查获取 IP 或退出函数返回。
其余断网恢复、再次配网场景仍需硬件核对。以下表格和验证顺序保留为第一步详细日志采集时的历史参考，已删除的临时标记不会在当前版本输出。

## 原始日志约定（历史参考）

串口搜索 `[OTA_STAGE1]`；`[12345 ms]` 是设备运行时的毫秒时间戳。
`net_ch=0` 表示 WiFi，`net_ch=1` 表示 LTE，分析时确认当前走的是 WiFi。
`cfg=1` 表示仍在配网流程；`complete=1` 表示 BLE 正等待配网结果。

| 标记 | 含义 |
| --- | --- |
| `WIFI_BOOT path=NEEDS_CONFIG` | 启动时走需要配网的分支 |
| `WIFI_BOOT path=SAVED_WIFI` | 启动时走保存 WiFi 的自动连接分支 |
| `CFG_START` / `BLE_INIT` | 开始配网 / 初始化配网 BLE |
| `BLE_CONNECTED` / `BLE_CONNECTION_EVENT` | BLE 连接完成或连接事件 |
| `CFG_INFO_ACCEPTED` / `APP_CFG_INFO` | 接收配网信息并上报 / 应用接收该事件 |
| `CFG_WIFI_CONNECT source=BLE` | 使用 BLE 提供的信息请求连接 WiFi |
| `WIFI_ASSOCIATED` | 已关联路由器，仍需等待 IP |
| `WIFI_IP_READY` | 网络栈报告获取 IP 成功，附 WiFi IPv4 地址 |
| `WIFI_POST_CONNECTED` | 低内存配置直接上报联网事件 |
| `NET_CHECK_POST_CONNECTED` | 非低内存配置完成服务器连通性检查后上报联网事件 |
| `APP_CONNECTED` | 应用收到联网事件 |
| `BLE_CFG_RESULT` | BLE 配网结果处理入口；`complete=0` 时原逻辑直接返回 |
| `BLE_NOTIFY_SUCCESS_REQUEST` / `BLE_NOTIFY_SUCCESS_SUBMIT` | 请求通知手机 / 提交发送，`ret=0` 表示发送接口返回成功 |
| `CFG_STOP_SCHEDULE` / `BLE_EXIT_SCHEDULE` | 原有的 500ms / 1000ms 关闭定时器安排位置 |
| `CFG_STOP_REQUEST` / `CFG_STOP_RETURN` | 请求结束配网 / 结束函数返回；不代表 BLE 异步断开已完成 |
| `BLE_EXIT_REQUEST` / `BLE_MODULE_REQUEST en=0` | 请求退出 BLE / 关闭模块入口 |
| `BLE_EXIT_DEFER` | `complete=1` 导致本次退出延后 |
| `BLE_DISCONNECT_REQUEST` / `BLE_DISCONNECTED` | 请求断开连接 / 收到断开完成事件 |
| `BLE_ADV_REQUEST` / `BLE_ADV_CMD` | 请求启停配网广播 / 向协议栈提交命令及返回值 |
| `BLE_ADV_REJECT` / `BLE_ADV_UNCHANGED` | 广播请求被拒绝 / 状态相同而未重复提交命令 |
| `WIFI_DISCONNECTED` / `APP_DISCONNECTED` | WiFi 底层断开 / 应用接收断网事件 |
| `WIFI_CONNECT_FAILED` / `WIFI_DHCP_TIMEOUT` | WiFi 关联失败或超时 / 获取 IP 超时 |

`BLE_ADV_CMD en=0 ret=0` 仅表示关闭广播命令提交成功，不是控制器停止广播的完成事件。
手机收到配网成功通知需要在手机侧确认，发送接口返回成功不代表手机已收到。
存在 BLE 连接时退出走异步断开路径，不一定出现 `BLE_ADV_CMD en=0`，应核对
`BLE_DISCONNECTED`、`adv_ctrl=0`，以及之后是否又提交 `BLE_ADV_CMD en=1`。

## 原始硬件验证顺序（历史参考）

1. 使用当前 CodeBlocks 工程编译、烧录，串口从上电开始保存完整日志。记录所用固件版本。
2. **首次配网**：通过设备现有操作进入配网，用原手机配网功能发送 WiFi 信息。
   核对 `CFG_START → BLE_INIT → CFG_INFO_ACCEPTED → WIFI_ASSOCIATED → WIFI_IP_READY → APP_CONNECTED`。
   检查 `BLE_NOTIFY_SUCCESS_SUBMIT` 返回值及手机实际提示；随后核对配网结束、BLE 退出、断开完成。
   `CFG_INFO_ACCEPTED`、应用事件处理和 `BLE_CFG_PENDING` 位于不同任务，允许相邻日志交错。
3. **保存 WiFi 后重启**：保持路由器可用，重启设备。
   预期走 `WIFI_BOOT path=SAVED_WIFI`，出现 `WIFI_IP_READY` 与 `APP_CONNECTED net_ch=0`。
   此时通常无需 `BLE_INIT` 或手机配网成功通知，未创建的 BLE 连接也不会出现断开完成日志。
4. **断网恢复**：关闭路由器，记录断网或连接超时日志，再开启路由器。
   核对 WiFi 恢复获取 IP，以及应用是否重新收到联网事件；若自动进入配网，记录完整分支。
5. **再次配网**：联网后通过现有操作重新进入配网，确认手机仍能发现设备、发送配置、收到结果。
   再次确认退出后是否发生意外的配网广播重启。

获取 IP 的事件也可能对应 SDK 的缓存 IP 路径，不能仅凭该日志认定进行了完整 DHCP 交互。
非低内存配置的 `APP_CONNECTED` 还要等待服务器连通性检查，可能晚于 `WIFI_IP_READY`。
500ms 与 1000ms 的关闭路径保持原样，本步重点确认通知与退出的实际顺序。

## 原始验收说明（历史参考）

保存上述四组场景的完整日志，核对获取 IP、应用联网、配网通知、BLE 退出的时间间隔。
发回日志时先删除已有日志中的 WiFi 密码和账户信息；本次新增日志不输出这些字段。

完成硬件核对后，再决定以获取 IP 事件还是应用联网事件作为联网门槛，并结合配网退出完成状态
进入第二步扫描实验。不能仅根据 `WIFI_ASSOCIATED` 或 `config_network_stop()` 返回就认为可以切换。
