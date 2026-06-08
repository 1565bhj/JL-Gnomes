#ifndef _SYS_VOLUME_H_
#define _SYS_VOLUME_H_

int sys_volume_read(u8 *volume);
int sys_volume_write(u8 *volume);
u8 sys_volume_chack(int volume);
int sys_volume_write_step(int step);

#endif
