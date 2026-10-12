#ifndef __QYAI_CONFIG_H__
#define __QYAI_CONFIG_H__

//服务器地址选择
typedef enum {
    SADDR_CHAT = 0,  //正式地址
    SADDR_TEST = 1, //测试地址
    SADDR_PREVIEW = 2  //灰度地址
};

// QYAI 定义配置项索引枚举
typedef enum {
    CFG_IDX_PRODUCT_BATCH = 0,
    CFG_IDX_PRODUCT_NUM,
    CFG_IDX_KEYWORDS_INDEX,
    CFG_IDX_NETWORK_CONFIG_INDEX,
    CFG_IDX_CHAT_ADDR_SET,
    CFG_IDX_CHAT_ADDR_VERSION,
    CFG_IDX_PRODUCT_NAME,
    CFG_IDX_PRODUCT_UUID,
    CFG_IDX_MLP,
    CFG_IDX_KEYWORDS,
    CFG_IDX_KEYWORDS_TEXT,
    CFG_IDX_KEYWORDS_SET,
    CFG_IDX_KEYWORDS_NUM,
    CFG_IDX_CHAT_AUDIO_ENC_FORMAT,
    CFG_IDX_CHAT_AUDIO_DEC_FORMAT,
    CFG_IDX_MIC_AUDIO_ENC_FORMAT,
    CFG_IDX_CHAT_LANGUAGE,
    CFG_IDX_NOTE,
    CFG_IDX_MAX
};

extern void *qyai_config_res_get(int index);
extern int qyai_config_res_init(char reinit, void *priv, void *(keywords_callback)(void *priv, int kws_used_index, int kws_list_index, char *kws_text));

#define QYAI_CFG_RETURN_STR(idx) \
        unsigned char *str_param = qyai_config_res_get(idx);\
        if (str_param) {\
            return str_param;\
        }

#define QYAI_CFG_RETURN_INT(idx) \
        unsigned char *int_param = qyai_config_res_get(idx);\
        if (int_param) { \
            return (int)*int_param;\
        }


#define QYAI_CFG_GET(idx) qyai_config_res_get(idx)


#endif
