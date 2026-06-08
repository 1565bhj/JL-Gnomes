#include "system/includes.h"
#include "device/ioctl_cmds.h"
#include "asm/system_reset_reason.h"

#if 0

extern int uni_local_asr_init(void);
void uni_main(void)
{
    int ret;
    uni_local_asr_init();
    uni_record_init();
    while (1) {
        os_time_dly(100);
    }
}
#endif
