#ifndef __UNI_SSP_H__
#define __UNI_SSP_H__

int uni_ssp_init(void);
int uni_ssp_process(int16_t* mic_in, int16_t** out, int32_t* out_size);
void uni_ssp_release(void);

#endif