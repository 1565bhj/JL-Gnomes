1、小飞小飞
xy_keywords.bin、xf_mlp.bin
"keywords_index": 0,
"network_config_index": 8,

2、小语小语
xy_keywords.bin、xy_mlp.bin
"keywords_index": 1,
"network_config_index": 2,

3、哈喽悠悠
en_keywords.bin、en_mlp.bin
"keywords_index": 0,
"network_config_index": 1,

4、小搭小搭
xd_keywords.bin、xd_mlp.bin
"keywords_index": 0,
"network_config_index": 1,

5、小晴小晴
xq_keywords.bin、xq_mlp.bin
"keywords_index": 0,
"network_config_index": 1,

keywords_list_index: n 默认使用的唤醒词，值是列表keywords_list的数组序号，从0开始，值是对应数组序号里的唤醒词
chat_addr_set: 0正式地址，1测试地址
chat_addr_version：0国内、1海外版本
chat_language: "zh"中文, "en"英文

如果需要加多个唤醒词，则从ifly_kws_res_all复制到ifly_res，然后运行cfg_create.bat重新生成qyai.cfg，再修改qyai.cfg
