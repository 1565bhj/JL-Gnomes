一、角色切换API接口及示例代码    
1. ** API说明**  
```c
/*
 * char *str: 角色名称(字符串)，不同用户分区有不同的角色
 */
int http_ai_intell_init(char *str);
```
---  

```c
2. 示例：按键切换智能体

extern int http_ai_intell_set(char *str, void (*set_callbak)(char *buf, int len));

//回调智能体是否设置成功
static void ai_intell_set_callbak(char *buf, int len)
{
	//buf非空则为智能体名称，则len代表对应智能体id(id > 0)
    if(buf)
    {
        printf("-> 智能体id = %d, 名称：%s\n", len, buf);
    } else {
        if(len == 0) //设置智能体失败
        {
            printf("-> 设置智能体失败\n");
        }
        else if(len == -1)  //网络异常或未绑定设备
        {
            printf("-> 网络异常或未绑定设备\n");
        }
    }
}

//比如：http_ai_intell_init("奶龙");
static int http_ai_intell_init(char *str)
{
    return http_ai_intell_set(str, ai_intell_set_callbak);
}

//根据服务器返回的信息，创建该表格。
static const char *ai_intelligent_str[] = {
    "豆包","宠物行为指导师","智谱AI","赛罗奥特曼","迪迦奥特曼","天才科学家","智启宝贝（早教版）","儿童陪伴故事-小鸭子","奶龙",
    NULL,
};

//按键触发：切换智能体、中断传输
static int app_key_event_handler(struct application *app, struct sys_event *sys_event)
{
    switch (sys_event->type)
    {
	    case SYS_KEY_EVENT:
	        struct key_event* key = (struct key_event *)sys_event->payload;
	        printf("key=%d  value=%d \n",key->action, key->value);

			//按键单击事件
			if(key->action==0){
				if(key->value==13) { //开始测试按键
					on_off=(on_off)?0:1;
				} else if(key->value==12) { //角色切换按键(角色切换，重点关注这里的代码逻辑)
				/////////////////////////////调用角色切换API/////////////////////////////////
					role_select=ai_intelligent_str[idx_cnt++];
					http_ai_intell_init(role_select); /* 智能体角色切换API，参数：角色名称 */
					if(idx_cnt==9)
						idx_cnt=0;
				//////////////////////////////////////////////////////////////////////////

					//TODO: TTS提示音合成
					
				} else if(key->value==6) { //打断按键
					//调用多模态打断接口
					qyai_mutilmodal_int_transfer();
				}
			}

	    break;

	    case SYS_NET_EVENT:
	        break;
	    default:
	        return false;
    }
}
```
---