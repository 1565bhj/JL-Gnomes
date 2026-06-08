#include "app_config.h"
#include "system/includes.h"
#include "device/device.h"
#include "fs/fs.h"
#include "device/gpio.h"
#include "os/os_api.h"
#include "system/app_core.h"
#include "event/key_event.h"
#include "event/device_event.h"
#include "json_c/json.h"
#include "json_c/json_tokener.h"

#if TCFG_ADKEY_ENABLE
extern int http_ai_intell_set(char *str, void (*set_callbak)(char *buf, int role_idx));
extern int http_ai_intell_set_id(int id_index, void (*set_callbak)(char *buf, int role_idx));

extern char *user_sys_get_lte_pai_code(void);
extern void user_sys_clear_lte_pai_code(void);
extern int http_wifi_device_bind_app(char *phone);
extern int http_lte_device_bind_app(char *code, char *phone, char *imei);
extern int http_device_unbind_app(void);

//角色列表示例，用户根据实际情况自定义角色列表
static const char *ai_intelligent_str[] = {
    "豆包", "宠物行为指导师", "智谱AI", "赛罗奥特曼", "迪迦奥特曼", "天才科学家", "智启宝贝（早教版）", "儿童陪伴故事-小鸭子", "奶龙",
    NULL,
};

static int idx_cnt = 0;
static char *role_select = NULL;
int on_off = 0;

//同步网络传输过程
#define SYNC_TIME_OUT (300)
static OS_SEM web_sync_sem;

//=====================设备鉴权===============================
//wifi设备鉴权(绑定),注册传入手机号或邮箱
int http_ai_wifi_dev_bind_app(void)
{
    int ret = http_wifi_device_bind_app("13392982656");
    return ret;
}

//4G设备鉴权(绑定)，传入参数：配对码+手机号或邮箱+4G模组的IMEI码
int http_ai_4g_dev_bind_app(void)
{
    int ret;
    //char *code=user_sys_get_lte_pai_code();
    char *code = "810270";
    char *phone = "13392982656";
    char *imei = " ";
    if (code[0] != '\0') {
        //绑定
        ret = http_lte_device_bind_app(code, phone, imei);
        //ret = http_wifi_device_bind_app("13392985656");
    } else {
        printf("incorrect 4G pairing code:%s", code);
    }
    return ret;
}

//解除设备绑定
int http_ai_device_unbind_app(void)
{
    int ret;
    //user_sys_clear_lte_pai_code();
    ret = http_device_unbind_app();
    return ret;
}
//============================================================


//=====================智能体切换=============================
/* 通过name设置，成功则buf为角色名称
 * 通过id设置，成功则buf返回的是NULL
 */
static void ai_dev_intell_set_cb(char *buf, int role_idx)
{
    printf("\r\n----------------智能体设置-----------\r\n");
    if (role_idx > 0) {
        printf("-> 智能体设置成功：id = %d, 名称：%s\n", role_idx, buf);
    } else if (role_idx < 0) {
        printf("-> 设置智能体失败: %s\n", buf);
    }
    printf("----------------------------------------\r\n");
    os_sem_post(&web_sync_sem);
}

//调用示例：http_ai_intell_init("奶龙");
static int http_ai_intell_set_by_name(char *str)
{
    os_sem_set(&web_sync_sem, 0);
    int ret = http_ai_intell_set_str(str, ai_dev_intell_set_cb);
    if (OS_TIMEOUT == os_sem_pend(&web_sync_sem, SYNC_TIME_OUT)) {
        printf("wrror: sem waitTime OUT: %d !!!", __LINE__);
    }
    return ret;
}

//调用示例：http_ai_intell_set_by_id(256);
//先使用http_ai_intell_set_by_name("role")获取整个列表，找出对应的index后再使用该API设置（速度会加快2秒）
static int http_ai_intell_set_by_id(int id)
{
    os_sem_set(&web_sync_sem, 0);
    int ret = http_ai_intell_set_id(id, ai_dev_intell_set_cb);
    if (OS_TIMEOUT == os_sem_pend(&web_sync_sem, SYNC_TIME_OUT)) {
        printf("wrror: sem waitTime OUT: %d !!!", __LINE__);
    }
    return ret;
}
//============================================================


//=====================图片下载===============================

//返回图片URL列表
static int ai_dev_get_img_url_list_cb(char *buf, int len, int payloadlen, char fin)
{
    json_object *new_obj = NULL;
    json_object *data_array = NULL;
    json_object *item_obj = NULL;
    json_object *parm = NULL;
    int i, array_len;

    new_obj = json_tokener_parse(buf);
    if (!new_obj) {
        return -1;
    }
    printf("-----------------------------------------");
    //获取"message"字段
    parm = json_object_object_get(new_obj, "message");
    if (parm) {
        char *str_message = json_object_get_string(parm);
        printf("message: %s\n", str_message);
    }

    //获取"code"字段
    parm = json_object_object_get(new_obj, "code");
    if (parm) {
        int code = json_object_get_int(parm);
        printf("code: %d\n", code);
    }

    //获取"total"字段
    parm = json_object_object_get(new_obj, "total");
    if (parm) {
        int total = json_object_get_int(parm);
        printf("total: %d\n", total);
    }
    printf("-----------------------------------------");

    //处理data数组
    data_array = json_object_object_get(new_obj, "data");
    if (data_array && json_object_is_type(data_array, json_type_array)) {
        array_len = json_object_array_length(data_array);
        printf("Found %d data items:\n", array_len);

        for (i = 0; i < array_len; i++) {
            item_obj = json_object_array_get_idx(data_array, i);
            if (item_obj) {
                json_object *id_obj = json_object_object_get(item_obj, "id");
                json_object *title_obj = json_object_object_get(item_obj, "title");
                json_object *image1_obj = json_object_object_get(item_obj, "image1");
                json_object *upload_time_obj = json_object_object_get(item_obj, "upload_time");
                int id = id_obj ? json_object_get_int(id_obj) : 0;
                const char *title = title_obj ? json_object_get_string(title_obj) : "NULL";
                const char *image1 = image1_obj ? json_object_get_string(image1_obj) : "NULL";
                const char *upload_time = upload_time_obj ? json_object_get_string(upload_time_obj) : "NULL";
                printf("-----------------------------------------");
                printf("Item %d: id=%d, title=%s, image1=%s, upload_time=%s\n", i + 1, id, title, image1, upload_time);
            }
        }
    }
    printf("-----------------------------------------");

    json_object_put(new_obj);
    os_sem_post(&web_sync_sem);
    return 0;
}

//图片数据回调
static int ai_dev_download_url_cb(char *buf, int len)
{
    char filename[256];
    printf("len:%d");
    //put_buf(buf, len);
    if (len != 0) {
        snprintf(filename, sizeof(filename), "%simg/download_img_%d.jpg", CONFIG_ROOT_PATH, len);
        //服务器返回的图片数据写到SD卡
        FILE *write_fd = fopen(filename, "wb");
        if (write_fd) {
            fwrite(buf, 1, len, write_fd);
            fclose(write_fd);
            printf("保存文件: %s (大小: %d 字节)\n", filename, len);
        } else {
            printf("错误: 无法写入文件\r\n");
        }
    }

    return 0;
}

//获取图片URL列表
int http_ai_get_img_url_app(void)
{
    int page = 1;
    int Pagesize = 10;
    int ret = http_ai_get_img_url(page, Pagesize, ai_dev_get_img_url_list_cb);
    if (OS_TIMEOUT == os_sem_pend(&web_sync_sem, SYNC_TIME_OUT)) {
        printf("wrror: sem waitTime OUT: %d !!!", __LINE__);
    }
}

//下载图片
int http_ai_download_img_app(void)
{
    char *select_img = "https://qingyuai.oss-cn-shenzhen.aliyuncs.com/temp-musicians/images/storage/4b1fc958-0db8-4bca-a92d-62e8fd16edc9.jpg";
    int ret = http_ai_download_img(select_img, ai_dev_download_url_cb);
    if (OS_TIMEOUT == os_sem_pend(&web_sync_sem, SYNC_TIME_OUT)) {
        printf("wrror: sem waitTime OUT: %d !!!", __LINE__);
    }
    return ret;
}
//============================================================

//=====================设备喝水量上报=========================
static void ai_dev_water_dat_report_cb(int err_code, char *msg)
{
    printf("---------------喝水量上报---------------\r\n");
    printf("err_code:%d, message:%s", err_code, msg);
    printf("----------------------------------------\r\n");
    os_sem_post(&web_sync_sem);
}

static void ai_dev_water_dat_get_cb(int err_code, int *msg)
{
    printf("---------------喝水量查询---------------\r\n");
    printf("err_code:%d, water_intake:%d", err_code, *msg);
    printf("----------------------------------------\r\n");
    os_sem_post(&web_sync_sem);
}

//对应产品名称是WATERCUP, 而且要有对应客户UUID
//设备端喝水量上报
int http_ai_report_dev_water_dat_app(void)
{
    int water_intake = 80;
    int year = 2026;
    int month = 01;
    int day = 29;
    char hour = 16;
    char min = 12;
    char sec = 01;
	int water_tmp=80;
    //年月日一定要对上，查询的是当天的喝水量，否则返回的查询数据是0
    int ret = http_ai_dev_water_dat_post(water_intake, year, month, day, hour, min, sec, water_tmp, ai_dev_water_dat_report_cb);
    if (OS_TIMEOUT == os_sem_pend(&web_sync_sem, SYNC_TIME_OUT)) {
        printf("wrror: sem waitTime OUT: %d !!!", __LINE__);
    }
    return ret;
}

//设备端喝水量获取
int http_ai_get_dev_water_dat_app(void)
{
    int ret = http_ai_dev_water_dat_get(ai_dev_water_dat_get_cb);
    if (OS_TIMEOUT == os_sem_pend(&web_sync_sem, SYNC_TIME_OUT)) {
        printf("wrror: sem waitTime OUT: %d !!!", __LINE__);
    }
    return ret;
}
//============================================================

static int app_key_state_machine(struct application *app, enum app_state state, struct intent *it)
{
    return 0;
}


//获取内容列表
extern void http_ai_get_content_list_app(void);
//获取内容数据
extern void http_ai_get_content_data_app(void);

//获取播客列表
extern void http_ai_get_podcast_list_app(void);
//获取播客内容数据
extern void http_ai_get_podcast_data_app(void);

//闹钟上报
void http_ai_dev_alarm_report_app(void);

//设备异常信息上报
void http_ai_dev_err_info_report_app(void);

static int app_key_event_handler(struct application *app, struct sys_event *sys_event)
{
    switch (sys_event->type) {
    case SYS_KEY_EVENT:
        struct key_event* key = (struct key_event *)sys_event->payload;
        printf("key->action=%d; key->value=%d\n", key->action, key->value);
        //开发板左键，设备绑定或解绑
        if (key->action == 4) { //双击，设备绑定(鉴权)
            if (key->value == 6) {
                /* 设备鉴权(4G设备绑定) */
                //http_ai_4g_dev_bind_app();
				http_ai_wifi_dev_bind_app();
            }
        } else if (key->action == 5) { //三击，解除设备绑定(鉴权)
            if (key->value == 6) {
                /* 解除设备绑定 */
                http_ai_device_unbind_app();
            }

        } else if (key->action == 0) { //按键单击事件
            if (key->value == 17) { //左键单击，开始测试多模态
                on_off = (on_off) ? 0 : 1;
            } else if (key->value == 12) { //中键单击
#if 0 //智能体切换
                role_select = ai_intelligent_str[idx_cnt++];
                http_ai_intell_set_by_name(role_select);
                if (idx_cnt == sizeof(ai_intelligent_str) / sizeof(ai_intelligent_str[0]) - 1) {
                    idx_cnt = 0;
                }

                http_ai_intell_set_by_id(2254);
#endif

#if 0 //图片上传下载
                /* 获取图片URL地址列表 */
                http_ai_get_img_url_app();
                /* 下载图片 */
                http_ai_download_img_app();
#endif

#if 1 //喝水量上报与获取(对应的要是水杯设备)
                /* 设备端喝水量上报 */
                http_ai_report_dev_water_dat_app();
                /* 设备端喝水量获取 */
                http_ai_get_dev_water_dat_app();

#endif

#if 0 //内容列表获取和播报
                //http_ai_get_content_list_app();
                http_ai_get_content_data_app();
#endif

#if 0 //获取播客列表和内容
                //http_ai_get_podcast_list_app(void);
                http_ai_get_podcast_data_app();
#endif

#if 0  //闹钟数据上报
                http_ai_dev_alarm_report_app();
#endif

#if 0  //设备端异常信息上报
                http_ai_dev_err_info_report_app();
#endif
            } else if (key->value == 6) { //会话打断按键(右键)
                qyai_mutilmodal_int_transfer();
            }
        }
        break;

    case SYS_NET_EVENT:
        break;
    default:
        return false;
    }

    return 0;
}

static const struct application_operation app_demo_ops = {
    .state_machine  = app_key_state_machine,
    .event_handler  = app_key_event_handler,
};

REGISTER_APPLICATION(adc_key_app) = {
    .name   = "app_key_demo",
    .ops    = &app_demo_ops,
    .state  = APP_STA_DESTROY,
};

static void app_key_task(void *p)
{
    while (1) {
        os_time_dly(1000);
        printf("app_key_task\r\n");
    }
}

void app_key_main()
{
#ifdef CONFIG_CXX_SUPPORT
    void cpp_run_init(void);
    cpp_run_init();
#endif

    os_sem_create(&web_sync_sem, 0);
    printf("\r\n\r\n\r\n\r\n\r\n ------------app_key_main %s-------------\r\n\r\n\r\n\r\n\r\n", __TIME__);
    os_task_create(app_key_task, NULL, 10, 1000, 0, "app_key_task");
}

late_initcall(app_key_main);
#endif
