#include "system/includes.h"
#include "server/audio_server.h"
#include "server/server_core.h"
#include "app_config.h"
#include "syscfg/syscfg_id.h"
#include "volume.h"

#define SET_VOLUME(x) ((x) * HW_VOLUME_VALUE / 100)
#define GET_VOLUME(r) ((r) * 100 / HW_VOLUME_VALUE)

#ifndef PRODUCTION_TEST_VOLUME
#define PRODUCTION_TEST_VOLUME (MAX_VOLUME_VALUE - 20)
#endif

static u8 sys_volume;
static u8 maxmin_volume;
int GET_SET_VOLUME(int volume)
{
    return SET_VOLUME(volume);
}

int sys_volume_read(u8 *volume)
{
#if (defined PRODUCTION_ALL_TEST_ENABLE || defined PRODUCTION_TEST_ENABLE)
    if (is_production_test_enter(0)) { //厂测模式音量固定位初始音量
        if (volume) {
            *volume = PRODUCTION_TEST_VOLUME;
        }
        return PRODUCTION_TEST_VOLUME;
    }
#endif
    u8 read_volume = 0;
#ifdef CONFIG_STORE_VOLUME
    if (syscfg_read(CFG_MUSIC_VOL, &read_volume, sizeof(read_volume)) < 0 ||
        read_volume < MIN_VOLUME_VALUE || read_volume > MAX_VOLUME_VALUE) {
        read_volume = INIT_VOLUME_VALUE;
        sys_volume = INIT_VOLUME_VALUE;
        if (volume) {
            *volume = read_volume;
        }
        return INIT_VOLUME_VALUE;
    }
#endif // CONFIG_STORE_VOLUME
    if (volume) {
        *volume = read_volume;
    }
    sys_volume = read_volume;
    return read_volume;
}
u8 sys_volume_chack(int volume)
{
    u8 check = 0;
    volume = volume > MAX_VOLUME_VALUE ? MAX_VOLUME_VALUE : volume;
    volume = volume < MIN_VOLUME_VALUE ? MIN_VOLUME_VALUE : volume;
    check = (u8)volume;
    return check;
}
int sys_volume_write_note(u8 *volume, char note)
{
#ifdef PRODUCTION_ALL_TEST_ENABLE
    if (is_production_test_enter(0)) { //厂测模式音量固定位初始音量
        return 0;
    }
#endif
    u8 check_volume = 0;
    sys_volume_read(&check_volume);
    if (check_volume == *volume) {
        if (note) {
            if (check_volume == MAX_VOLUME_VALUE && maxmin_volume != MAX_VOLUME_VALUE) { //最大音量最小音量提示
                music_play_res_file("MaxVolume.mp3");
                maxmin_volume = MAX_VOLUME_VALUE;
            } else if (check_volume == MIN_VOLUME_VALUE && maxmin_volume != MIN_VOLUME_VALUE) {
                music_play_res_file("MinVolume.mp3");
                maxmin_volume = MIN_VOLUME_VALUE;
            } else {
                maxmin_volume = check_volume;
            }
        }
        return 0;
    }
    sys_volume = *volume;
    sys_volume = sys_volume > MAX_VOLUME_VALUE ? MAX_VOLUME_VALUE : sys_volume;
    sys_volume = sys_volume < MIN_VOLUME_VALUE ? MIN_VOLUME_VALUE : sys_volume;
#ifdef CONFIG_STORE_VOLUME
    syscfg_write(CFG_MUSIC_VOL, &sys_volume, sizeof(sys_volume));
#endif // CONFIG_STORE_VOLUME
    if (note) {
        if (sys_volume == MAX_VOLUME_VALUE && maxmin_volume != MAX_VOLUME_VALUE) { //最大音量最小音量提示
            music_play_res_file("MaxVolume.mp3");
            maxmin_volume = MAX_VOLUME_VALUE;
        } else if (sys_volume == MIN_VOLUME_VALUE && maxmin_volume != MIN_VOLUME_VALUE) {
            music_play_res_file("MinVolume.mp3");
            maxmin_volume = MIN_VOLUME_VALUE;
        } else {
            maxmin_volume = sys_volume;
        }
    }
    return 0;
}
int sys_volume_write(u8 *volume)
{
    return sys_volume_write_note(volume, 1);
}
int sys_volume_write_step(int step)
{
#ifdef PRODUCTION_ALL_TEST_ENABLE
    if (is_production_test_enter(0)) { //厂测模式音量固定位初始音量
        return 0;
    }
#endif
    sys_volume_read(NULL);
    int volume = sys_volume;
    volume += step;
    volume = volume > MAX_VOLUME_VALUE ? MAX_VOLUME_VALUE : volume;
    volume = volume < MIN_VOLUME_VALUE ? MIN_VOLUME_VALUE : volume;
    sys_volume = (u8)volume;
#ifdef CONFIG_STORE_VOLUME
    syscfg_write(CFG_MUSIC_VOL, &sys_volume, sizeof(sys_volume));
#endif // CONFIG_STORE_VOLUME
    if (sys_volume == MAX_VOLUME_VALUE && maxmin_volume != MAX_VOLUME_VALUE) { //最大音量最小音量提示
        music_play_res_file("MaxVolume.mp3");
        maxmin_volume = MAX_VOLUME_VALUE;
    } else if (sys_volume == MIN_VOLUME_VALUE && maxmin_volume != MIN_VOLUME_VALUE) {
        music_play_res_file("MinVolume.mp3");
        maxmin_volume = MIN_VOLUME_VALUE;
    } else {
        maxmin_volume = sys_volume;
    }
    printf("->sys_volume = %d \n", sys_volume);
    return 0;
}

int sys_volume_clear(void)
{
#ifdef CONFIG_STORE_VOLUME
    u8 read_volume = 0;
    if (syscfg_read(CFG_MUSIC_VOL, &read_volume, sizeof(read_volume)) < 0) {
        return 0;
    }
#endif // CONFIG_STORE_VOLUME
    u8 volume = INIT_VOLUME_VALUE;
    sys_volume_write(&volume);
    return 0;
}
