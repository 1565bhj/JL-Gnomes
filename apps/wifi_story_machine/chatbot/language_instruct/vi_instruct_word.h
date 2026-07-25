#ifndef __VI_INSTRUCT_WORD_H__
#define __VI_INSTRUCT_WORD_H__

#ifdef CONFIG_INSTRUCTION_VI
//=================越南语指令词=====================//

//=================系统指令=====================//
#define SYS_CLOSE_VI \
    "tắt","đóng","dừng","thoát","kết thúc cuộc hội thoại","dừng trò chuyện", \
    "kết thúc đối thoại","tôi không muốn nói chuyện với bạn nữa"

#define SYS_STR_POWER_OFF_VI \
    "tắt thiết bị"

#define SYS_30MIN_VI \
    "một tiếng rưỡi sau","sau một tiếng rưỡi", \
    "ba mươi phút sau","sau ba mươi phút", \
    "nửa tiếng sau","sau nửa tiếng"

#define SYS_90MIN_VI \
    "một tiếng rưỡi sau","sau một tiếng rưỡi"

#define SYS_HOUR_TIMEOUT_VI \
    "một tiếng sau","sau một tiếng"

#define SYS_MIN_TIMEOUT_VI \
    "một phút sau","sau vài phút"

#define SYS_COUNTOWN_TIMEOUT_VI \
    "đếm ngược","đặt bộ hẹn giờ"

#define SYS_COUNTOWN_TIMEOUT_CLOSE_VI \
    "hủy đếm ngược","xóa bộ hẹn giờ","tắt bộ hẹn giờ"

//=================音乐指令=====================//
#define SYS_MUSIC_PLAY_VI \
    "phát nhạc","mở bài hát","chơi nhạc"

#define SYS_MUSIC_CONTINUE_VI \
    "tiếp tục phát","tiếp tục bài hát","chạy nhạc tiếp"

#define SYS_MUSIC_STOP_VI \
    "tạm dừng","tạm dừng nhạc","tạm dừng bài hát"

#define SYS_MUSIC_CLOSE_VI \
    "dừng nhạc","tắt nhạc","kết thúc bài hát"

#define SYS_MUSIC_NEXT_VI \
    "bài hát tiếp theo","bài kế tiếp","đổi bài hát","bỏ qua bài hát"

#define SYS_MUSIC_LAST_VI \
    "bài hát trước đó","bài cũ hơn"

#define SYS_MUSIC_LOOP_VI \
    "lặp lại một bài hát","chế độ lặp đơn bài"

#define SYS_MUSIC_LOOP_CLOSE_VI \
    "tắt chế độ lặp","thoát chế độ lặp","hủy lặp lại", \
    "tắt lặp một bài hát"

//=================设备控制指令=====================//
#define SYS_MUSIC_APP_AI_VI \
    "chế độ AI","chế độ thông minh AI","chế độ trợ lý thông minh"

#define SYS_MUSIC_APP_UDISK_VI \
    "phát USB","chế độ USB"

#define SYS_MUSIC_APP_SD_VI \
    "phát thẻ SD","phát thẻ TF","chế độ SD","chế độ TF"

#define SYS_MUSIC_APP_BT_VI \
    "chế độ Bluetooth","phát qua Bluetooth"

#define SYS_MUSIC_APP_AUX_VI \
    "chế độ đầu vào đường line","chế độ AUX"

//=================闹钟指令=====================//
#define SYS_ALARM_DEL_VI \
    "xóa báo thức","hủy báo thức","tắt báo thức","xóa lời nhắc"

#define SYS_ALARM_DEL_ALL_VI \
    "xóa tất cả báo thức","hủy toàn bộ báo thức","tắt tất cả lời nhắc"

#define SYS_ALARM_LOOK_VI \
    "xem danh sách báo thức","các báo thức đã cài đặt","mở trang báo thức"

//=================音量调节指令=====================//
#define SYS_VOLUME_SET_VI  "đặt âm lượng thành"

#define SYS_MUSIC_VOLUME_VI \
    "giảm âm lượng","giảm tiếng nhạc", \
    "tăng âm lượng","tăng tiếng nhạc"

#define SYS_MUSIC_MAX_VOLUME_VI \
    "âm lượng cực đại","tiếng to nhất có thể"

#define SYS_MUSIC_MIN_VOLUME_VI \
    "âm lượng cực tiểu","tiếng nhỏ nhất có thể"

#endif
#endif // __VI_INSTRUCT_WORD_H__
