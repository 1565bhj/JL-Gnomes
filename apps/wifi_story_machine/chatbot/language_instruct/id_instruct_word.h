#ifndef __ID_INSTRUCT_WORD_H__
#define __ID_INSTRUCT_WORD_H__

#ifdef CONFIG_INSTRUCTION_ID
//=================Perintah Instruksi Bahasa Indonesia=====================//

//=================Perintah Sistem=====================//
#define SYS_CLOSE_ID \
    "matikan", "tutup", "berhenti", "minggir", "akhiri percakapan", "matikan obrolan", "tutup obrolan", "hentikan obrolan", \
    "berhenti bicara", "tidak mau bicara lagi"

#define SYS_STR_POWER_OFF_ID \
    "matikan perangkat", "matikan daya", "shutdown"

#define SYS_30MIN_ID \
    "satu setengah jam kemudian", "setelah satu setengah jam", "lewat satu setengah jam", \
    "tiga puluh menit kemudian", "setelah tiga puluh menit", "lewat tiga puluh menit", \
    "setengah jam kemudian", "setelah setengah jam", "lewat setengah jam"

#define SYS_90MIN_ID \
    "sembilan puluh menit kemudian", "setelah sembilan puluh menit", "lewat sembilan puluh menit"

#define SYS_HOUR_TIMEOUT_ID \
    "jam kemudian", "setelah satu jam", "lewat jam"

#define SYS_MIN_TIMEOUT_ID \
    "menit kemudian", "setelah beberapa menit", "lewat beberapa menit"

#define SYS_COUNTOWN_TIMEOUT_ID \
    "mulai hitung mundur", "atur timer", "mulai waktu hitung"

#define SYS_COUNTOWN_TIMEOUT_CLOSE_ID \
    "batalkan timer", "hapus timer", "batalkan hitung mundur", "akhiri hitung mundur"

//=================Perintah Musik=====================//
#define SYS_MUSIC_PLAY_ID \
    "putar musik", "mainkan lagu", "putar lagu"

#define SYS_MUSIC_CONTINUE_ID \
    "lanjutkan putar", "lanjutkan lagu", "putar musik lagi"

#define SYS_MUSIC_STOP_ID \
    "jeda sejenak", "jeda musik", "henti lagu sementara"

#define SYS_MUSIC_CLOSE_ID \
    "henti lagu", "berhenti putar", "matikan musik", "tutup musik", "akhiri lagu", "berhenti main musik"

#define SYS_MUSIC_NEXT_ID \
    "lagu selanjutnya", "putar lagu berikutnya", "ganti lagu", "pindah lagu", "ganti trek"

#define SYS_MUSIC_LAST_ID \
    "lagu sebelumnya", "putar lagu lama", "kembali ke lagu sebelumnya"

#define SYS_MUSIC_LOOP_ID \
    "ulangi satu lagu", "putar ulang satu trek"

#define SYS_MUSIC_LOOP_CLOSE_ID \
    "matikan mode ulang", "akhiri ulang", "batalkan mode ulang", \
    "matikan ulang satu lagu", "akhiri putar ulang satu trek", "nonaktifkan fungsi ulang"

//=================Perintah Kontrol Mode Perangkat=====================//
#define SYS_MUSIC_APP_AI_ID \
    "mode AI", "mode kecerdasan buatan", "mode asisten pintar"

#define SYS_MUSIC_APP_UDISK_ID \
    "putar USB", "mode USB", "mode putar musik USB"

#define SYS_MUSIC_APP_SD_ID \
    "putar kartu SD", "putar kartu TF", "mode kartu SD", "mode kartu TF"

#define SYS_MUSIC_APP_BT_ID \
    "putar bluetooth", "mode bluetooth"

#define SYS_MUSIC_APP_AUX_ID \
    "mode input jalur", "mode input tambahan"

//=================Perintah Alarm=====================//
#define SYS_ALARM_DEL_ID \
    "hapus alarm", "batalkan alarm", "batalkan pengingat", "matikan alarm"

#define SYS_ALARM_DEL_ALL_ID \
    "hapus semua alarm", "batalkan seluruh alarm", "batalkan semua pengingat", "matikan semua alarm"

#define SYS_ALARM_LOOK_ID \
    "lihat alarm", "tampilkan daftar alarm", "buka halaman alarm", "cek daftar alarm"

//=================Perintah Atur Volume=====================//
#define SYS_VOLUME_SET_ID  "atur volume"

#define SYS_MUSIC_VOLUME_ID \
    "suara lebih kecil", "musik lebih pelan", "turunkan volume", "kecilkan suara", \
    "suara lebih keras", "musik lebih nyaring", "naikkan volume", "besarkan suara"

#define SYS_MUSIC_MAX_VOLUME_ID \
    "volume maksimal", "suara paling keras", "atur volume maksimum", "suara tertinggi"

#define SYS_MUSIC_MIN_VOLUME_ID \
    "volume minimal", "suara paling pelan", "atur volume minimum", "suara terendah"

#endif
#endif // __ID_INSTRUCT_WORD_H__