### 一、移植说明
以移植到AC791N_SDK_V1.2.0和AC792N_SDK_V2.2.2为例

## 关于lwip协议栈  
主要是移植应用层，lwip协议栈直接使用公版自带的库lte_module.a，公版SDK已经有指定这个链接库。  
项目->构建选项->连接器设置->其他连接器选项：
../../../../cpu/wl83/liba/lte_module.a
---

### 二、应用层移植
1. 工程项目新增文件  
  (1) lte_wifi_config.c  
  (2) multiple_ethernetif.c  
  (3) app_main.c与4G-LTE相关的初始化代码
```c
void app_main()
{
    ................................................................
    int net_ch = 0;
#if (defined CONFIG_WIFI_ENABLE && defined CONFIG_LTE_PHY_ENABLE)
    net_ch = sys_net_channel_read();
    if(net_ch < 0 || (net_ch != 0 && net_ch != 1)){
        sys_net_channel_write(0);
        net_ch = sys_net_channel_read();
    }
#elif (defined CONFIG_LTE_PHY_ENABLE)
    net_ch = 1;
#endif

#ifdef CONFIG_LTE_PHY_ENABLE
    if(net_ch == 1){
#if (defined CONFIG_WIFI_ENABLE && defined CONFIG_LTE_PHY_ENABLE)
		//4G关闭静态ip冲突检测
        IPV4_ADDR_CONFLICT_DETECT = 0;
#endif
        lte_net_init();
    }else{
        lte_power_cntrol(0);
    }
#endif
    ................................................................
}
```
2. 宏定义  
```c
  (1) 工程配置中定义这个宏：CONFIG_SXY_QYAI_ENABLE  
项目->构建选项->#define宏定义

  (2) 依赖这两个线程：tcpip_thread和lte_rx_task，线程的创建信息在app_main中有定义：  
  #ifdef CONFIG_NET_ENABLE  
    { "tcpip_thread",        16,     800,      0 },  
  #endif  
  
  #if TCFG_LTE_PHY_ENABLE  
    { "lte_rx_task",          7,     256,      0 },  
  #endif  
  
  所以需要定义这两个宏：CONFIG_NET_ENABLE、TCFG_LTE_PHY_ENABLE，
  比如在文件apps\wifi_story_machine\board\wl83\board_develop_AC7925A.h中定义  
  #define TCFG_LTE_PHY_ENABLE  1  //开启4G-LTE  
  #define CONFIG_NET_ENABLE
```

3. 修改产品名称后缀     
```c
 //产品名称以4G为后缀
char *app_user_product_name(void)
{
   //return "TestBox";
   return "TestBox4G";//产品名称要以4G为后缀
}
```
---

### 三、添加链接库
1. 更新并添加这个库即可  
 sxy_qyai.a
---




### 四、4G模块测试说明
AC792N只有USB2.0的High-Speed硬件口，固件下载后，接入High-Speed硬件口，观察日志，成功联网，会有打印：  
```c
lnet connected checkok  
JNET EUENT CONNECTED  
Slkill for:net check succ  
```

板子联网后，就可以调用AI对话或者多模态接口进行应用程序的开发。  












