### 一、工程配置
1. 板型配置
文件：board_7916A_develop_cfg.h  
配置选择4G板  
#define SXY_QYAI_4G_DEMO_BOARD		    1   //QYAI-4G-demo板  

2. 应用层  
产品名称后缀以4G结尾，文件apps\wifi_story_machine\app_main.c修改如下：
```c
    //用户产品名称
    char *app_user_product_name(void)
    {
        //return "TestBox"; //英文名称
        return "TestBox4G";
    }
```

### 二、应用层示例
参考这两个文件：  
\apps\wifi_story_machine\chatbot\audio\ai_speaker.c  
\apps\wifi_story_machine\chatbot\lte\usb_net_at_cmd.c  
