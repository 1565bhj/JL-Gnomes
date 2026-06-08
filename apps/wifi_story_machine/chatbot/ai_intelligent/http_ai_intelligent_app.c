#include "system/includes.h"
#include "app_config.h"
#include "update/update.h"
#include "fs/fs.h"
#include "update/update_loader_download.h"
#include "update/net_update.h"
#include "json_c/json.h"
#include "json_c/json_tokener.h"
#include "ai_uart_ctrol.h"

#ifdef CONFIG_NET_ENABLE
//使用智能体名称[字符串的形式]设定
extern int http_ai_intell_set(char *str, void (*set_callbak)(char *buf, int len));
//使用id设置智能体（先使用http_ai_intell_init找出对应的index才能使用该API设置，速度会加快2秒）
extern int http_ai_intell_set_id(int id_index, void (*set_callbak)(char *buf, int len));
//检查当前设置智能体状态（0 正在设置， 1设置成功， -1设置失败, -2未设置）
extern int http_ai_intell_set_success_status(void);

//static const char *ai_intelligent_str[] = {
//    "豆包大模型","奶龙","国学","口才","奥特曼","英语口语","辅导老师","地理老师","暴躁老王",
//    "迪迦奥特曼","智启宝贝（早教版）","育儿专家",
//    NULL,
//};

//同步网络传输过程
static OS_SEM web_sync_sem;
#define SYNC_TIME_OUT (500)

/* 通过name设置，成功则buf为角色名称
 * 通过id设置，成功则buf返回的是NULL
 */
static void app_dev_intell_set_cb(char *buf, int role_idx)
{
    if (role_idx == 0) {//智能体设置成功
        os_sem_post(&web_sync_sem);
        printf("-> 智能体设置成功：%s\n", buf ? buf : "(未知)");
    } else if (role_idx > 0) {//遍历智能体列表
        printf("-> 遍历智能体：id = %d, 名称：%s\n", role_idx, buf);
    } else if (role_idx < 0) {//智能体设置失败，未绑定账号或者网络异常
        printf("-> 设置智能体失败\n");
    }
#ifdef AT_UART_CMD_ENABLE
    if (buf) {
        char sbuf[160] = {0};
        sprintf(sbuf, "%s, %d \r\n", buf, role_idx);
        at_uart_cmd_send(AI_UART_CMD_AI_INTELL_AGENT_INFO, sbuf);
    }
#endif
}

//调用示例：http_ai_intell_init("奶龙");
int http_ai_intell_set_by_name(char *str)
{
    os_sem_set(&web_sync_sem, 0);
    int ret = http_ai_intell_set_str(str, app_dev_intell_set_cb);
    if (OS_TIMEOUT == os_sem_pend(&web_sync_sem, SYNC_TIME_OUT)) {
        printf("wrror: sem waitTime OUT: %d !!!", __LINE__);
        return -1;
    }
    return ret;
}

//调用示例：http_ai_intell_set_by_id(256);
//先使用http_ai_intell_set_by_name("role")获取整个列表，找出对应的index后再使用该API设置（速度会加快2秒）
int http_ai_intell_set_by_id(int id)
{
    os_sem_set(&web_sync_sem, 0);
    int ret = http_ai_intell_set_id(id, app_dev_intell_set_cb);
    if (OS_TIMEOUT == os_sem_pend(&web_sync_sem, SYNC_TIME_OUT)) {
        printf("wrror: sem waitTime OUT: %d !!!", __LINE__);
        return -1;
    }
    return ret;
}

void app_http_ai_intelligent_main(void)
{
#ifdef CONFIG_CXX_SUPPORT
    void cpp_run_init(void);
    cpp_run_init(); //使用c++时，必须先调用该接口进行初始化
#endif

    os_sem_create(&web_sync_sem, 0);
}

late_initcall(app_http_ai_intelligent_main);
#endif

