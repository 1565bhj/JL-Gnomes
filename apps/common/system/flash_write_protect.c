#include "system/includes.h"
#include "device/ioctl_cmds.h"
#include "asm/sfc_norflash_api.h"
#include "app_config.h"

struct flash_protect {
    u32 id;//id
    u32 cmd;//默认写保护参数
};

//TODO: 要关闭，否则开机VM碎片整理出错
//#define FLASH_WRITE_PROTECT_EN    1 //1:开启flash写保护参数

//flash写保护参数配置:https://doc.zh-jieli.com/AC79/zh-cn/master/module_example/peripherals/flash_protect.html
static const struct flash_protect flash_protect_index[] = {
    //id      //写全保护参数， 看S1 S2寄存器
    // CMP bit为1则BIT(16) = 1
    //BP4 BP3 为BIT(9) BIT(0), BP2 BP1 BP0为BIT(2) BIT(1) BIT(0)

    /******华邦W25QXX*********/
    0xEF4014, 0x34,//W25Q80
    0xEF4015, 0x38,//W25Q16
    0xEF4016, 0x3C,//W25Q32
    0xEF4017, 0x3C,//W25Q64
    0xEF4018, 0x1003C,//W25Q128
    0xEF4019, 0x1003C,//W25Q256

    /******普雅25Q64-全保护*********/
    0x852017, 0x1C /* 0x14000 */,//PY25Q64

//    /******普雅25Q64-前6M写保护*********/
//    0x852017, 0x14014,//PY25Q64

    /******复旦微25Q64*********/
    0xA14017, 0x1C /* 0x14000 */,//FM25Q64

    /******博雅25Q64*********/
    0x684017, 0x1C /* 0x14000 */,//BY25Q64

};

static void flash_test(void)
{
    u32 cap_size = 0;
    u32 flash_addr = 0;
    u8 data[32];

    //全写保护打开后，测试：打开写保护为0(全部解除写保护)和不打开的区别
    /*norflash_ioctl(NULL, IOCTL_SET_WRITE_PROTECT, 0);*/
    norflash_ioctl(NULL, IOCTL_GET_CAPACITY, (u32)&cap_size);
    for (int j = 0; j < sizeof(data); j++) {
        data[j] = j;
    }
    //测试地址：一半的容量
    flash_addr = cap_size / 2;
    printf("flash test addr = 0x%x \n", flash_addr);
    norflash_ioctl(NULL, IOCTL_ERASE_SECTOR, flash_addr);
    put_buf(data, sizeof(data));
    norflash_write(NULL, data, sizeof(data), flash_addr);
    memset(data, 0, sizeof(data));
    norflash_origin_read(data, flash_addr, sizeof(data));
    put_buf(data, sizeof(data));
}

int flash_write_protect(void)
{
#ifdef FLASH_WRITE_PROTECT_EN
    u32 i;
    u32 cmd = 0;
    u32 id = 0;
    struct flash_protect *index = (struct flash_protect *)&flash_protect_index;

    //1.打开flash设备
    norflash_open(NULL, NULL, NULL);

    //2.获取ID
    norflash_ioctl(NULL, IOCTL_GET_ID, (u32)&id);
    printf("flash id = 0x%x\n", id);

    //3.查询预留的ID和使用对应的参数
    for (i = 0; i < sizeof(flash_protect_index) / sizeof(flash_protect_index[0]); i++, index++) {
        if (index->id == id) {
            norflash_write_protect(index->cmd);
            //norflash_ioctl(NULL, IOCTL_SET_WRITE_PROTECT, index->cmd);
            printf("flash write_protect = 0x%x\n", index->cmd);
            goto exit;
        }
    }
    printf("error in flash id=0x%x , no write protect cmd!!!\n\n", id);

exit:
    //4.测试修改写保护参数结果
    //flash_test();
#endif
    return 0;
}

/*
#ifdef FLASH_WRITE_PROTECT_EN
__initcall(flash_write_protect);
#endif
*/

