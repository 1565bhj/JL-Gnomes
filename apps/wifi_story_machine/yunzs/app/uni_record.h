#ifndef __UNI_RECORD_H__
#define __UNI_RECORD_H__

int uni_record_init(void);

int uni_record_suspend(void);

int uni_record_resume(void);

// 离线算法处理后数据回调
typedef void (*user_kws_data_cb)(char* data, int len);
// 咪头数据回调
void user_set_mic_data_cb(user_kws_data_cb ucb);
// 算法处理后数据回调
void user_set_kws_data_cb(user_kws_data_cb ucb);
// 录音控制
int user_kws_request(int req_type, void *arg);

#endif