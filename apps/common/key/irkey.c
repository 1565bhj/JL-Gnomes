#include "key/key_driver.h"
#include "key/irkey.h"
#include "device/gpio.h"
#include "asm/irflt.h"
#include "event/key_event.h"
#include "app_config.h"

#if TCFG_IRKEY_ENABLE

//按键驱动扫描参数列表
struct key_driver_para irkey_scan_para = {
    .scan_time        = 10,             //按键扫描频率, 单位: ms
    .last_key         = NO_KEY,         //上一次get_value按键值, 初始化为NO_KEY;
    .filter_time      = 2,              //按键消抖延时;
    .long_time        = 75,             //按键判定长按数量
    .hold_time        = (75 + 15),      //按键判定HOLD数量
    .click_delay_time = 20,             //按键被抬起后等待连击延时数量
    .key_type         = KEY_DRIVER_TYPE_IR,
    .get_value        = ir_get_key_value,
};

static const u8 IRTabFF00[] = {
    NKEY_00, NKEY_01, NKEY_02, NKEY_03, NKEY_04, NKEY_05, NKEY_06, IR_06, IR_15, IR_08, NKEY_0A, NKEY_0B, IR_12, IR_11, NKEY_0E, NKEY_0F,
    NKEY_10, NKEY_11, NKEY_12, NKEY_13, NKEY_14, IR_07, IR_09, NKEY_17, IR_13, IR_10, NKEY_1A, NKEY_1B, IR_16, NKEY_1D, NKEY_1E, NKEY_1F,
    NKEY_20, NKEY_21, NKEY_22, NKEY_23, NKEY_24, NKEY_25, NKEY_26, NKEY_27, NKEY_28, NKEY_29, NKEY_2A, NKEY_2B, NKEY_2C, NKEY_2D, NKEY_2E, NKEY_2F,
    NKEY_30, NKEY_31, NKEY_32, NKEY_33, NKEY_34, NKEY_35, NKEY_36, NKEY_37, NKEY_38, NKEY_39, NKEY_3A, NKEY_3B, NKEY_3C, NKEY_3D, NKEY_3E, NKEY_3F,
    IR_04, NKEY_41, IR_18, IR_05, IR_03, IR_00, IR_01, IR_02, NKEY_48, NKEY_49, IR_20, NKEY_4B, NKEY_4C, NKEY_4D, NKEY_4E, NKEY_4F,
    NKEY_50, NKEY_51, IR_19, NKEY_53, NKEY_54, NKEY_55, NKEY_56, NKEY_57, NKEY_58, NKEY_59, IR_17, NKEY_5B, NKEY_5C, NKEY_5D, IR_14, NKEY_5F,
};

/*----------------------------------------------------------------------------*/
/**@brief   获取ir按键值
   @param   void
   @param   void
   @return  void
   @note    void get_irkey_value(void)
*/
/*----------------------------------------------------------------------------*/
u8 ir_get_key_value(void)
{
    u8 tkey = 0xff;
    tkey = get_irflt_value();
    if (tkey == 0xff) {
        return tkey;
    }
//    printf("-> tkey = %d \n",tkey);
//    tkey = IRTabFF00[tkey];
    /*
    index =
    69：开关
    70：mode
    7：eq
    71:静音
    9：vol+音量加
    21：vol-音量减
    67：下一首
    64：上一首
    68：暂停
    13：U/SD U盘和SD卡播放模式
    25：RPT
    */
    switch (tkey) {
    case 9:
        tkey = KEY_VOLUME_INC;
        break;//音量+
    case 21:
        tkey = KEY_VOLUME_DEC;
        break;//音量-
    case 67:
        tkey = KEY_DOWN;
        break;//下一首
    case 64:
        tkey = KEY_UP;
        break;//上一首
    case 69:
        tkey = KEY_POWER;
        break;//开/关机
    case 70:
        tkey = KEY_MODE;
        break;//模式
    case 71:
        tkey = KEY_MUTE;
        break;//静音
    case 68:
        tkey = KEY_OK;
        break;//暂停/播放
    default:
        tkey = NO_KEY;
        break;
    }
    return tkey;
}

/*----------------------------------------------------------------------------*/
/**@brief   ir按键初始化
   @param   void
   @param   void
   @return  void
   @note    void ir_key_init(void)
*/
/*----------------------------------------------------------------------------*/
int irkey_init(const struct irkey_platform_data *irkey_data)
{
    ir_input_io_sel(irkey_data->port);

    ir_output_timer_sel();

    irflt_config();

    ir_timeout_set();

    return 0;
}

#endif

