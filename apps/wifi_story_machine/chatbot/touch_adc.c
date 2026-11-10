#include "app_config.h"
#include "system/includes.h"
#include "asm/adc_api.h"
#include "asm/gpio.h"
#include "device/gpio.h"
#include "ai_uart_ctrol.h"

#ifdef CONFIG_TOUCH_ADC_ENABLE

#define TOUCH_ADC_SCAN_TICKS        20
#define TOUCH_ADC_TRIGGER_MS        200
#define TOUCH_ADC_TRIGGER_TICKS     ((TOUCH_ADC_TRIGGER_MS * OS_TICKS_PER_SEC) / 1000)
#define TOUCH_ADC_THREAD_PRIO       20

#define TOUCH_ADC1_MIN_MV     1300
#define TOUCH_ADC1_MAX_MV     1700

#define TOUCH_ADC3_MIN_MV     0
#define TOUCH_ADC3_MAX_MV     200

#define TOUCH_ADC4_MIN_MV     800
#define TOUCH_ADC4_MAX_MV     1200

#define TOUCH_ADC2_MIN_MV     1800
#define TOUCH_ADC2_MAX_MV     2200

extern int music_play_res_file(const char *name);
extern int play_face_emoji(int avi_index);
extern int system_keyword_start(void);
extern int music_buf_play_stop_status(void);
extern int music_play_stop_status(void);
extern int net_music_play_stop_status(void);

enum touch_adc_state {
    TOUCH_ADC_STATE_IDLE,
    TOUCH_ADC_STATE_ZMOTOU,
    TOUCH_ADC_STATE_ZMOBEI,
};

static int touch_adc_is_pressed(u16 voltage_mv)
{
    return (voltage_mv >= TOUCH_ADC1_MIN_MV && voltage_mv < TOUCH_ADC1_MAX_MV) ||
           (voltage_mv >= TOUCH_ADC2_MIN_MV && voltage_mv < TOUCH_ADC2_MAX_MV) ||
           (voltage_mv >= TOUCH_ADC3_MIN_MV && voltage_mv < TOUCH_ADC3_MAX_MV) ||
           (voltage_mv >= TOUCH_ADC4_MIN_MV && voltage_mv < TOUCH_ADC4_MAX_MV);
}

static int touch_adc_is_blocked(void)
{
    return system_keyword_start() ||
           !music_buf_play_stop_status() ||
           !music_play_stop_status() ||
           !net_music_play_stop_status();
}

void servo_touch_happy_action()
{
    printf("[pwm] servo_touch_action");
    extern int servo_move_two_to_angle(uint8_t motor1, unsigned char target_angle1,
                                       uint8_t motor2, unsigned char target_angle2,
                                       unsigned short speed_dps);
    u8 wag_count;

    for (wag_count = 0; wag_count < 4; wag_count++) {
        servo_move_two_to_angle(1, 65, 3, 115, 200);
        servo_move_two_to_angle(1, 115, 3, 65, 200);
    }

    servo_move_two_to_angle(1, 90, 3, 90, 180);
}

void servo_touch_sad_action()
{
    extern int servo_move_two_to_angle(uint8_t motor1, unsigned char target_angle1,
                                       uint8_t motor2, unsigned char target_angle2,
                                       unsigned short speed_dps);

    servo_move_two_to_angle(1, 110, 3, 110, 45);
    os_time_dly(80);
    servo_move_two_to_angle(1, 90, 3, 90, 45);
}

void servo_touch_angry_action()
{
    extern int servo_move_two_to_angle(uint8_t motor1, unsigned char target_angle1,
                                       uint8_t motor2, unsigned char target_angle2,
                                       unsigned short speed_dps);
    u8 shake_count;

    for (shake_count = 0; shake_count < 2; shake_count++) {
        servo_move_two_to_angle(1, 55, 3, 125, 360);
        servo_move_two_to_angle(1, 125, 3, 55, 360);
    }

    servo_move_two_to_angle(1, 90, 3, 90, 240);
}

static void touch_adc_task(void *priv)
{
    enum touch_adc_state state = TOUCH_ADC_STATE_IDLE;
    // u8 wait_touch_release = 0;
    u16 touch_press_ticks = 0;

    os_time_dly(200);
    servo_set_angle(1, 90);
    while (1) {
        u16 raw_value = adc_get_value(TOUCH_ADC_CHANNEL);
        u16 voltage_mv = adc_get_voltage(TOUCH_ADC_CHANNEL);

        // printf("[touch_adc] touch adc: raw=%u, voltage=%u mV\n", raw_value, voltage_mv);

        if (touch_adc_is_blocked()) {
            state = TOUCH_ADC_STATE_IDLE;
            touch_press_ticks = 0;
            // wait_touch_release = 1;
            os_time_dly(TOUCH_ADC_SCAN_TICKS);
            continue;
        }

        // if (wait_touch_release) {
        //     if (!touch_adc_is_pressed(voltage_mv)) {
        //         wait_touch_release = 0;
        //         touch_press_ticks = 0;
        //     }
        //     os_time_dly(TOUCH_ADC_SCAN_TICKS);
        //     continue;
        // }

        if (!touch_adc_is_pressed(voltage_mv)) {
            state = TOUCH_ADC_STATE_IDLE;
            touch_press_ticks = 0;
            os_time_dly(TOUCH_ADC_SCAN_TICKS);
            continue;
        }

        // 触摸时长到达TOUCH_ADC_TRIGGER_TICKS才会触发
        if (touch_press_ticks < TOUCH_ADC_TRIGGER_TICKS) {
            touch_press_ticks += TOUCH_ADC_SCAN_TICKS;
            os_time_dly(TOUCH_ADC_SCAN_TICKS);
            continue;
        }

        if (voltage_mv >= TOUCH_ADC1_MIN_MV && voltage_mv < TOUCH_ADC1_MAX_MV) {
            if (state != TOUCH_ADC_STATE_ZMOTOU) {
                music_play_res_file("head.mp3");
                play_face_emoji(AI_UART_CMD_EMOJI_GIEVAN);
                servo_touch_sad_action();
                state = TOUCH_ADC_STATE_ZMOTOU;
            }
        } else if (voltage_mv >= TOUCH_ADC2_MIN_MV && voltage_mv < TOUCH_ADC2_MAX_MV) {
            if (state != TOUCH_ADC_STATE_ZMOBEI) {
                // music_play_res_file("laugh2.mp3");
                // play_face_emoji(100);
                // servo_touch_happy_action();
                state = TOUCH_ADC_STATE_ZMOBEI;
            }
        } else if (voltage_mv >= TOUCH_ADC3_MIN_MV && voltage_mv < TOUCH_ADC3_MAX_MV) {
            if (state != TOUCH_ADC_STATE_ZMOBEI) {
                music_play_res_file("lface.mp3");
                play_face_emoji(AI_UART_CMD_EMOJI_ANGRY);
                servo_touch_angry_action();
                state = TOUCH_ADC_STATE_ZMOBEI;
            }
        } else if (voltage_mv >= TOUCH_ADC4_MIN_MV && voltage_mv < TOUCH_ADC4_MAX_MV) {
            if (state != TOUCH_ADC_STATE_ZMOBEI) {
                music_play_res_file("rface.mp3");
                play_face_emoji(AI_UART_CMD_EMOJI_REVEL);
                servo_touch_happy_action();
                state = TOUCH_ADC_STATE_ZMOBEI;
            }
        } 
        os_time_dly(TOUCH_ADC_SCAN_TICKS);
    }
}

int touch_adc_init(void)
{   
    printf("[touch_adc] adc_port init！");
    gpio_direction_input(TOUCH_ADC_PORT);
    gpio_set_pull_up(TOUCH_ADC_PORT, 0);
    gpio_set_pull_down(TOUCH_ADC_PORT, 0);
    gpio_set_die(TOUCH_ADC_PORT, 0);
    gpio_set_dieh(TOUCH_ADC_PORT, 0);
    adc_add_sample_ch(TOUCH_ADC_CHANNEL);

    return thread_fork("touch_adc", TOUCH_ADC_THREAD_PRIO, 768, 0, NULL, touch_adc_task, NULL);
}
#endif
