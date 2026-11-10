#include "asm/pwm.h"
#include "asm/gpio.h"
#include "device/device.h"
#include "system/includes.h"
#include "app_config.h"
#include "board_config.h"

#ifdef TCFG_SERVO_ENABLE

#define SERVO_PULSE_MIN_US  500
#define SERVO_PULSE_MAX_US  2500
#define SERVO_CENTER_US     1500
#define SERVO_ANGLE_MIN     0
#define SERVO_ANGLE_MAX     180
#define SERVO_MOVE_TICK     1       // OS_TICKS_PER_SEC = 100, so this is 10 ms

static void *servo_dev[SERVO_COUNT] = {NULL};
static struct pwm_platform_data servo_pwm[SERVO_COUNT] = {0};
static unsigned char servo_current_angle[SERVO_COUNT] = {90, 90, 90};
static const char *const servo_dev_name[SERVO_COUNT] = {
    "pwm1",
    "pwm2",
    "pwm3",
};

static void servo_set_enable(int enable)
{
    gpio_direction_output(TCFG_SERVOEN_PORT, enable ? 1 : 0);
}

static int servo_index_get(uint8_t motor)
{
    if (motor < 1 || motor > SERVO_COUNT) {
        return -1;
    }

    return motor - 1;
}

int servo_set_pulse_us(uint8_t motor, unsigned short pulse_us)
{
    struct pwm_platform_data *pwm;
    int servo_index;
    int ret;

    servo_index = servo_index_get(motor);
    if (servo_index < 0 || !servo_dev[servo_index]) {
        return -1;
    }

    if (pulse_us < SERVO_PULSE_MIN_US) {
        pulse_us = SERVO_PULSE_MIN_US;
    } else if (pulse_us > SERVO_PULSE_MAX_US) {
        pulse_us = SERVO_PULSE_MAX_US;
    }

    // 20ms 周期下：
    // duty = pulse_us / 20000 * 100 * 10^2
    // 所以 500us->250, 1500us->750, 2500us->1250
    pwm = &servo_pwm[servo_index];
    pwm->duty = pulse_us / 200.0f;

    ret = dev_ioctl(servo_dev[servo_index], PWM_SET_DUTY, (u32)pwm);
    if (ret) {
        printf("[pwm] servo %u set duty fail: %d\n", motor, ret);
        return ret;
    }

    // printf("[pwm] servo %u pulse=%uus duty=%.2f%%\n", motor, pulse_us, pwm->duty);

    /* Keep the software position synchronized for later speed-controlled moves. */
    servo_current_angle[servo_index] =
        ((pulse_us - SERVO_PULSE_MIN_US) * SERVO_ANGLE_MAX +
         (SERVO_PULSE_MAX_US - SERVO_PULSE_MIN_US) / 2) /
        (SERVO_PULSE_MAX_US - SERVO_PULSE_MIN_US);

    return 0;
}

int servo_set_angle(uint8_t motor, unsigned char angle)
{
    unsigned short pulse_us;
    int servo_index;
    int ret;

    if (angle > SERVO_ANGLE_MAX) {
        angle = SERVO_ANGLE_MAX;
    }

    pulse_us = 500 + angle * 2000 / 180;
    // printf("[pwm] servo_set_angle");
    ret = servo_set_pulse_us(motor, pulse_us);
    if (ret) {
        return ret;
    }

    servo_index = servo_index_get(motor);
    if (servo_index >= 0) {
        servo_current_angle[servo_index] = angle;
    }

    return 0;
}

/*
 * Move a servo to target_angle at approximately speed_dps degrees per second.
 * This function blocks the calling task until the target angle is reached.
 */
int servo_move_to_angle(uint8_t motor, unsigned short speed_dps,
                        unsigned char target_angle)
{
    int servo_index;
    int current_angle;
    int target;
    int direction;
    int step;
    unsigned int speed_accumulator = 0;
    int ret;

    if (speed_dps == 0) {
        return -2;
    }

    servo_index = servo_index_get(motor);
    if (servo_index < 0 || !servo_dev[servo_index]) {
        return -1;
    }

    if (target_angle > SERVO_ANGLE_MAX) {
        target_angle = SERVO_ANGLE_MAX;
    }

    current_angle = servo_current_angle[servo_index];
    target = target_angle;
    if (current_angle == target) {
        return 0;
    }

    direction = target > current_angle ? 1 : -1;
    servo_set_enable(1);

    while (current_angle != target) {
        /*
         * One tick is 10 ms, so speed_dps / 100 is the angle increment
         * for one tick. The accumulator preserves the fractional part.
         */
        speed_accumulator += speed_dps;
        step = speed_accumulator / 100;
        speed_accumulator %= 100;

        if (step > 0) {
            if (step > (direction > 0 ? target - current_angle
                                      : current_angle - target)) {
                step = direction > 0 ? target - current_angle
                                     : current_angle - target;
            }

            current_angle += direction * step;
            ret = servo_set_angle(motor, current_angle);
            if (ret) {
                servo_set_enable(0);
                return ret;
            }
        }

        if (current_angle != target) {
            os_time_dly(SERVO_MOVE_TICK);
        }
    }

    servo_set_enable(0);
    return 0;
}

/*
 * Move two servos at the same speed. Both servos are updated in the same
 * scheduler tick, so their movement starts together.
 */
int servo_move_two_to_angle(uint8_t motor1, unsigned char target_angle1,
                            uint8_t motor2, unsigned char target_angle2,
                            unsigned short speed_dps)
{
    int servo_index1;
    int servo_index2;
    int current_angle1;
    int current_angle2;
    int target1;
    int target2;
    int direction1;
    int direction2;
    int step1;
    int step2;
    unsigned int speed_accumulator1 = 0;
    unsigned int speed_accumulator2 = 0;
    int ret;

    if (speed_dps == 0) {
        return -2;
    }

    servo_index1 = servo_index_get(motor1);
    servo_index2 = servo_index_get(motor2);
    if (servo_index1 < 0 || servo_index2 < 0 ||
        !servo_dev[servo_index1] || !servo_dev[servo_index2] ||
        servo_index1 == servo_index2) {
        return -1;
    }

    if (target_angle1 > SERVO_ANGLE_MAX) {
        target_angle1 = SERVO_ANGLE_MAX;
    }
    if (target_angle2 > SERVO_ANGLE_MAX) {
        target_angle2 = SERVO_ANGLE_MAX;
    }

    current_angle1 = servo_current_angle[servo_index1];
    current_angle2 = servo_current_angle[servo_index2];
    target1 = target_angle1;
    target2 = target_angle2;

    direction1 = target1 > current_angle1 ? 1 : -1;
    direction2 = target2 > current_angle2 ? 1 : -1;

    if (current_angle1 == target1 && current_angle2 == target2) {
        return 0;
    }

    servo_set_enable(1);

    while (current_angle1 != target1 || current_angle2 != target2) {
        /* One OS tick is 10 ms, therefore the rate is speed_dps / 100. */
        speed_accumulator1 += speed_dps;
        speed_accumulator2 += speed_dps;
        step1 = speed_accumulator1 / 100;
        step2 = speed_accumulator2 / 100;
        speed_accumulator1 %= 100;
        speed_accumulator2 %= 100;

        if (current_angle1 != target1 && step1 > 0) {
            if (step1 > (direction1 > 0 ? target1 - current_angle1
                                        : current_angle1 - target1)) {
                step1 = direction1 > 0 ? target1 - current_angle1
                                       : current_angle1 - target1;
            }
            current_angle1 += direction1 * step1;
            ret = servo_set_angle(motor1, current_angle1);
            if (ret) {
                servo_set_enable(0);
                return ret;
            }
        }

        if (current_angle2 != target2 && step2 > 0) {
            if (step2 > (direction2 > 0 ? target2 - current_angle2
                                        : current_angle2 - target2)) {
                step2 = direction2 > 0 ? target2 - current_angle2
                                       : current_angle2 - target2;
            }
            current_angle2 += direction2 * step2;
            ret = servo_set_angle(motor2, current_angle2);
            if (ret) {
                servo_set_enable(0);
                return ret;
            }
        }

        if (current_angle1 != target1 || current_angle2 != target2) {
            os_time_dly(SERVO_MOVE_TICK);
        }
    }

    servo_set_enable(0);
    return 0;
}

int servo_init(void)
{
    int servo_index;
    int ret;

    servo_set_enable(1);

    if (servo_dev[0] && servo_dev[1] && servo_dev[2]) {
        return 0;
    }

    for (servo_index = 0; servo_index < SERVO_COUNT; servo_index++) {
        servo_dev[servo_index] = dev_open(servo_dev_name[servo_index],
                                          &servo_pwm[servo_index]);
        if (!servo_dev[servo_index]) {
            printf("[pwm] open %s fail\n", servo_dev_name[servo_index]);
            goto fail;
        }
    }

    ret = servo_set_pulse_us(1, SERVO_CENTER_US);
    ret |= servo_set_pulse_us(2, SERVO_CENTER_US);
    ret |= servo_set_pulse_us(3, SERVO_CENTER_US);
    if (ret) {
        printf("[pwm] set motor center position fail\n");
        goto fail;
    }

    for (servo_index = 0; servo_index < SERVO_COUNT; servo_index++) {
        ret = dev_ioctl(servo_dev[servo_index], PWM_RUN,
                        (u32)&servo_pwm[servo_index]);
        if (ret) {
            printf("[pwm] motor %u run fail: %d\n", servo_index + 1, ret);
            goto fail;
        }
    }
    os_time_dly(SERVO_MOVE_TICK);
    servo_set_enable(0);
    return 0;

fail:
    for (servo_index = 0; servo_index < SERVO_COUNT; servo_index++) {
        if (servo_dev[servo_index]) {
            dev_close(servo_dev[servo_index]);
            servo_dev[servo_index] = NULL;
        }
    }
    return -1;
}

static int servo_pwm_init_call(void)
{
    if (servo_init()) {
        printf("[pwm_test] motor init fail\n");
        return -1;
    }
    return 0;
}
late_initcall(servo_pwm_init_call);

#ifdef SERVO_PWM_TEST
static void pwm_test(void)
{
    servo_move_to_angle(1, 120, 90);
    while (1) {
        servo_move_to_angle(2, 120, 25);
        servo_move_to_angle(3, 120, 110);
        os_time_dly(100);
        servo_move_to_angle(2, 120, 155);
        servo_move_to_angle(3, 120, 70);
        os_time_dly(100);
        servo_move_to_angle(2, 120, 90);
        servo_move_to_angle(3, 120, 90);
    }
}
static int c_main(void)
{
    os_task_create(pwm_test, NULL, 12, 1000, 0, "pwm_test");
    return 0;
}
late_initcall(c_main);
#endif
#endif
