#include "system/includes.h"
#include "server/audio_server.h"
#include "server/server_core.h"
#include "app_config.h"
#include "qyai_config.h"
#include "cJSON.h"
#include "syscfg/syscfg_id.h"

typedef struct qy_cfg {
    unsigned char config_init;
    unsigned char reserve;//预留
    unsigned char product_batch;             // 11
    unsigned char product_num;               // 20
    unsigned char keywords_index;            // 0
    unsigned char network_config_index;      // 0
    unsigned char chat_addr_set;             // 0
    unsigned char chat_addr_version;         // 0
    unsigned char keywords_list_index;       // 2
    unsigned char keywords_list_index_set;   // 设置的ndex
    unsigned char keywords_list_index_num;   // kws数量

    unsigned char product_name[32];          // "产品名称"
    unsigned char product_uuid[32];          // "客户UUID"
    unsigned char mlp[32];                   // "xf_mlp.bin"
    unsigned char keywords[32];              // "xf_keywords.bin"
    unsigned char chat_audio_enc_format[32]; // "pcm"
    unsigned char chat_audio_dec_format[32]; // "mp3"
    unsigned char mic_audio_enc_format[32];  // "pcm"
    unsigned char chat_language[32];         // "zh"
    unsigned char keywords_text[32];         // "小飞小飞"
    unsigned char note[160];                 // "NULL"
} QYAI_CFG;
static QYAI_CFG qyai_cfg;

int user_app_keywords_index_config(char opt, unsigned char index);
static void *parse_string_field(cJSON *root, const char *field_name, unsigned char *dest, size_t dest_size)
{
    cJSON *item = cJSON_GetObjectItem(root, field_name);
    if (item && cJSON_IsString(item) && item->valuestring[0] != '\0') {
        if (dest) {
            strncpy((char*)dest, item->valuestring, dest_size - 1);
            dest[dest_size - 1] = '\0';
        }
        return item;
    }
    return NULL;
}

static int parse_int_field(cJSON *root, const char *field_name, unsigned char *dest)
{
    cJSON *item = cJSON_GetObjectItem(root, field_name);
    if (item && cJSON_IsNumber(item)) {
        if (dest) {
            *dest = (unsigned char)item->valueint;
        }
        return item;
    }
    return NULL;
}

static void qyai_config_parse(cJSON *root, void *priv, void *(keywords_callback)(void *priv, int kws_used_index, int kws_list_index, char *kws_text))
{
    parse_string_field(root, "product_name", qyai_cfg.product_name, sizeof(qyai_cfg.product_name));
    parse_string_field(root, "product_uuid", qyai_cfg.product_uuid, sizeof(qyai_cfg.product_uuid));
    parse_string_field(root, "chat_audio_enc_format", qyai_cfg.chat_audio_enc_format, sizeof(qyai_cfg.chat_audio_enc_format));
    parse_string_field(root, "chat_audio_dec_format", qyai_cfg.chat_audio_dec_format, sizeof(qyai_cfg.chat_audio_dec_format));
    parse_string_field(root, "mic_audio_enc_format", qyai_cfg.mic_audio_enc_format, sizeof(qyai_cfg.mic_audio_enc_format));
    parse_string_field(root, "chat_language", qyai_cfg.chat_language, sizeof(qyai_cfg.chat_language));
    parse_string_field(root, "note", qyai_cfg.note, sizeof(qyai_cfg.note));

    parse_int_field(root, "product_batch", &qyai_cfg.product_batch);
    parse_int_field(root, "product_num", &qyai_cfg.product_num);
    parse_int_field(root, "keywords_index", &qyai_cfg.keywords_index);
    parse_int_field(root, "network_config_index", &qyai_cfg.network_config_index);
    parse_int_field(root, "chat_addr_set", &qyai_cfg.chat_addr_set);
    parse_int_field(root, "chat_addr_version", &qyai_cfg.chat_addr_version);
    parse_int_field(root, "keywords_list_index", &qyai_cfg.keywords_list_index);

    /* 读取 keywords_list 数组，根据 keywords_list_index 选取对应项 */
    cJSON *list = cJSON_GetObjectItem(root, "keywords_list");
    if (list && cJSON_IsArray(list)) {
        cJSON *item = NULL;
        int list_size = cJSON_GetArraySize(list);
        int idx = qyai_cfg.keywords_list_index_set > 0 ? qyai_cfg.keywords_list_index_set : qyai_cfg.keywords_list_index;
        qyai_cfg.keywords_list_index_set = idx;
        qyai_cfg.keywords_list_index_num = list_size;
        printf("list_size = %dm, set idx : %d\n", list_size, idx);
        if (keywords_callback) {
            for (int i = 0; i < list_size; i++) {
                item = cJSON_GetArrayItem(list, i);
                if (item && cJSON_IsObject(item)) {
                    cJSON *item_keywords_text = parse_string_field(item, "keywords_text", NULL, 0);
                    cJSON *item_mlp = parse_string_field(item, "mlp", NULL, 0);
                    int keywords_index = parse_int_field(item, "keywords_index", NULL);
                    if (item_keywords_text && item_mlp) {
                        keywords_callback(priv, idx, i, item_keywords_text->valuestring);
                    }
                }
            }
        } else {
            if (idx >= 0 && idx < list_size) {
                item = cJSON_GetArrayItem(list, idx);
                if (item && cJSON_IsObject(item)) {
                    parse_string_field(item, "keywords", qyai_cfg.keywords, sizeof(qyai_cfg.keywords));
                    parse_string_field(item, "mlp", qyai_cfg.mlp, sizeof(qyai_cfg.mlp));
                    parse_string_field(item, "keywords_text", qyai_cfg.keywords_text, sizeof(qyai_cfg.keywords_text));
                    parse_int_field(item, "keywords_index", &qyai_cfg.keywords_index);
                    parse_int_field(item, "network_config_index", &qyai_cfg.network_config_index);
                    printf("ifly set index = %d, %s, %s, %s\n", idx, qyai_cfg.keywords_text, qyai_cfg.keywords, qyai_cfg.mlp);
                }
            }
        }
    }
}

int qyai_config_kws_get_index(void)
{
    qyai_config_res_init(0, NULL, NULL);
    return qyai_cfg.keywords_list_index_set;
}

void *qyai_config_res_get(int index)
{
    qyai_config_res_init(0, NULL, NULL);
    if (!qyai_cfg.config_init) {
        return NULL;
    }
    switch (index) {
    case CFG_IDX_PRODUCT_BATCH:
        if (qyai_cfg.product_batch != 0) {
            return &qyai_cfg.product_batch;
        }
        break;
    case CFG_IDX_PRODUCT_NUM:
        if (qyai_cfg.product_num != 0) {
            return &qyai_cfg.product_num;
        }
        break;
    case CFG_IDX_KEYWORDS_INDEX:
        return &qyai_cfg.keywords_index;
    case CFG_IDX_KEYWORDS_TEXT:
        if (qyai_cfg.keywords_text[0] != '\0') {
            return qyai_cfg.keywords_text;
        }
        break;
    case CFG_IDX_KEYWORDS_SET:
        return &qyai_cfg.keywords_list_index_set;
    case CFG_IDX_KEYWORDS_NUM:
        return &qyai_cfg.keywords_list_index_num;
    case CFG_IDX_NETWORK_CONFIG_INDEX:
    case CFG_IDX_CHAT_ADDR_SET:
        if (qyai_cfg.chat_addr_set != 0) {
            return &qyai_cfg.chat_addr_set;
        }
        break;
    case CFG_IDX_CHAT_ADDR_VERSION:
        return &qyai_cfg.chat_addr_version;
    case CFG_IDX_PRODUCT_NAME:
        if (qyai_cfg.product_name[0] != '\0') {
            return qyai_cfg.product_name;
        }
        break;
    case CFG_IDX_PRODUCT_UUID:
        if (qyai_cfg.product_uuid[0] != '\0') {
            return qyai_cfg.product_uuid;
        }
        break;
    case CFG_IDX_MLP:
        if (qyai_cfg.mlp[0] != '\0') {
            return qyai_cfg.mlp;
        }
        break;
    case CFG_IDX_KEYWORDS:
        if (qyai_cfg.keywords[0] != '\0') {
            return qyai_cfg.keywords;
        }
        break;
    case CFG_IDX_CHAT_AUDIO_ENC_FORMAT:
        if (qyai_cfg.chat_audio_enc_format[0] != '\0') {
            return qyai_cfg.chat_audio_enc_format;
        }
        break;
    case CFG_IDX_CHAT_AUDIO_DEC_FORMAT:
        if (qyai_cfg.chat_audio_dec_format[0] != '\0') {
            return qyai_cfg.chat_audio_dec_format;
        }
        break;
    case CFG_IDX_MIC_AUDIO_ENC_FORMAT:
        if (qyai_cfg.mic_audio_enc_format[0] != '\0') {
            return qyai_cfg.mic_audio_enc_format;
        }
        break;
    case CFG_IDX_CHAT_LANGUAGE:
        if (qyai_cfg.chat_language[0] != '\0') {
            return qyai_cfg.chat_language;
        }
        break;
    default:
        return NULL;
    }
    return NULL;
}

int qyai_config_res_init(char reinit, void *priv, void *(keywords_callback)(void *priv, int kws_used_index, int kws_list_index, char *kws_text))
{
    if (qyai_cfg.config_init && !reinit) {
        return 0;
    }

    FILE* fp = fopen(CONFIG_QYAI_RES_CFG_NAME, "rb");
    if (fp) {
        qyai_cfg.config_init = false;
        int len = flen(fp);
        char *buf = 0;
        if (len > 0) {
            buf = malloc(len + 32);
        }
        if (buf) {
            memset(buf, 0, len + 32);
            fread(buf, 1, len, fp);
            //printf("QYAI_RES_CFG : %s\n", buf);
            cJSON *root = cJSON_Parse(buf);
            if (root != NULL) {
                if (!qyai_cfg.config_init) {
                    qyai_cfg.keywords_list_index_set = user_app_keywords_index_config(0, 0);
                }
                qyai_config_parse(root, priv, keywords_callback);
                cJSON_Delete(root);
                qyai_cfg.config_init = true;
            } else {
                qyai_cfg.config_init = false;
                printf("root err\n");
            }
            printf("QYAI_RES_CFG OK\n");
            free(buf);
        } else {
            qyai_cfg.config_init = false;
        }
        fclose(fp);
    } else {
        printf("QYAI_RES_CFG open err : %s\n", CONFIG_QYAI_RES_CFG_NAME);
    }
    return 0;
}

