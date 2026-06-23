
# music.c 与 screen_music.c 重复，同时编译会导致 music_info / 重进页状态各两份，歌词被误清为「加载中」
GEN_CSRCS += $(filter-out music.c,$(notdir $(wildcard $(PRJ_DIR)/custom/*.c)))

DEPPATH += --dep-path $(PRJ_DIR)/custom
VPATH += :$(PRJ_DIR)/custom

CFLAGS += "-I$(PRJ_DIR)/custom"
