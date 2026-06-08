#ifndef _SC7A20H_H
#define _SC7A20H_H

#define WRITE_COMMAND_FOR_SC7A20H       0x32  // I2C写入命令  0x32
#define READ_COMMAND_FOR_SC7A20H        0x33  // I2C读取命令

// 低通滤波频率定义
#define LPF_CUTOFF_800HZ  0x00  // CTRL_REG2: 00xx xxxx
#define LPF_CUTOFF_200HZ  0x40  // CTRL_REG2: 01xx xxxx
#define LPF_CUTOFF_100HZ  0x80  // CTRL_REG2: 10xx xxxx
#define LPF_CUTOFF_50HZ   0xC0  // CTRL_REG2: 11xx xxxx

// SC7A20H寄存器地址定义
#define SC7A20H_WHO_AM_I         0x0F
#define SC7A20H_VERSION          0x70

// 控制寄存器
#define SC7A20H_CTRL_REG1        0x20
#define SC7A20H_CTRL_REG2        0x21
#define SC7A20H_CTRL_REG3        0x22
#define SC7A20H_CTRL_REG4        0x23
#define SC7A20H_CTRL_REG5        0x24

// 状态寄存器
#define SC7A20H_STATUS_REG       0x27

// 数据寄存器
#define SC7A20H_OUT_X_L          0x28
#define SC7A20H_OUT_X_H          0x29
#define SC7A20H_OUT_Y_L          0x2A
#define SC7A20H_OUT_Y_H          0x2B
#define SC7A20H_OUT_Z_L          0x2C
#define SC7A20H_OUT_Z_H          0x2D

// FIFO寄存器
#define SC7A20H_FIFO_CTRL_REG    0x2E
#define SC7A20H_FIFO_SRC_REG     0x2F

// 中断寄存器
#define SC7A20H_INT1_CFG         0x30
#define SC7A20H_INT1_SRC         0x31
#define SC7A20H_INT1_THS         0x32
#define SC7A20H_INT1_DURATION    0x33

// 函数声明
extern unsigned char sc7a20h_sensor_command(unsigned char register_address, unsigned char function_command);
extern unsigned char sc7a20h_sensor_get_data(unsigned char register_address);

extern void sc7a20h_resolution_range(unsigned char range);
extern void sc7a20h_work_mode(unsigned char work_mode);
extern void sc7a20h_set_lpf_cutoff(unsigned char cutoff_freq);

extern char sc7a20h_check(void);
extern void sc7a20h_init(void);
extern int sc7a20h_gravity_sensity(unsigned char gsid);
extern void sc7a20h_get_acceleration(unsigned int cnt);
extern int sc7a20h_prepare_power_off(void);
extern unsigned short get_total_water_intake_ml(void);
extern void reset_total_water_intake_ml(void);

#endif // _SC7A20H_H
