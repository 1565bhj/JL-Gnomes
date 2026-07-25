#ifndef __MS_INSTRUCT_WORD_H__
#define __MS_INSTRUCT_WORD_H__

#ifdef CONFIG_INSTRUCTION_MS
//=================马来语指令词=====================//

//=================系统指令=====================//
#define SYS_CLOSE_MS \
    "matikan","tutup ia","tutup","hentikan","hentikan","penutupan","hentikan ia","hentikan","selamat tinggal","selamat tinggal", \
    "Matikan","Tutup ia","Tutup","Hentikan","Hentikan","Penutupan","Hentikan ia","Hentikan","Selamat tinggal","Selamat tinggal", \
    "keluar_sistem","keluar sistem", \
    "Keluar_sistem","Keluar sistem", \
    "system_exit","system exit", \
    "System_exit","System exit"

#define SYS_STR_POWER_OFF_MS \
    "matikan kuasa","Matikan kuasa", \
    "tutup kuasa","Tutup kuasa"

#define SYS_30MIN_MS \
    "selepas setengah jam", \
    "Selepas setengah jam"

#define SYS_90MIN_MS \
    "Selepas satu setengah jam", \
    "selepas satu setengah jam"

#define SYS_HOUR_TIMEOUT_MS \
    "jam selepas"

#define SYS_MIN_TIMEOUT_MS \
    "beberapa minit kemudian"

#define SYS_COUNTOWN_TIMEOUT_MS \
    "Kiraan detik","kirakan detik"

#define SYS_COUNTOWN_TIMEOUT_CLOSE_MS \
    "Batal kiraan detik","Padam kiraan detik", \
    "batal kiraan detik","padam kiraan detik", \
    "Batal pemasa","Padam pemasa","Batal kira detik","Padam kira detik", \
    "batal pemasa","padam pemasa","batal kira detik","padam kira detik"

//=================音乐指令=====================//
#define SYS_MUSIC_PLAY_MS \
    "Main","Main lagu","Main muzik", \
    "main","main lagu","main muzik"

#define SYS_MUSIC_CONTINUE_MS \
    "Teruskan bermain","Teruskan memainkan lagu","Teruskan memainkan muzik", \
    "teruskan bermain","teruskan memainkan lagu","teruskan memainkan muzik", \
    "sambung_muzik","sambung muzik", \
    "Sambung_muzik","Sambung muzik", \
    "Teruskan main","Teruskan main lagu","Teruskan main muzik", \
    "teruskan main","teruskan main lagu","teruskan main muzik"

#define SYS_MUSIC_STOP_MS \
    "Jeda","Jeda main balik","Jeda muzik","Jeda main", \
    "jeda","jeda main balik","jeda muzik","jeda main", \
    "muzik_jeda", \
    "Muzik_jeda"

#define SYS_MUSIC_CLOSE_MS \
    "Berhenti lagu","Berhenti main","Berhenti muzik","Matikan muzik","Muzik tutup","Matikan lagu","Lagu tutup","Muzik berhenti","Berhenti muzik", \
    "berhenti lagu","berhenti main","berhenti muzik","matikan muzik","muzik tutup","matikan lagu","lagu tutup","muzik berhenti","berhenti muzik", \
    "muzik_berhenti","muzik berhenti", \
    "Muzik_berhenti","Muzik berhenti", \
    "Henti lagu","Henti main","Henti muzik","Muzik dimatikan","Matikan lagu","Lagu dimatikan", \
    "henti lagu","henti main","henti muzik","muzik dimatikan","matikan lagu","lagu dimatikan"

#define SYS_MUSIC_NEXT_MS \
    "Lagu seterusnya","Main seterusnya","Tukar satu","Tukar lagu","Tukar muzik","Langkau lagu","Tukar trek", \
    "lagu seterusnya","main seterusnya","tukar satu","tukar lagu","tukar muzik","langkau lagu","tukar trek", \
    "muzik_trek_seterusnya","muzik seterusnya", \
    "Muzik_trek_seterusnya","Muzik seterusnya", \
    "Main lagu seterusnya","Langkah lagu", \
    "main lagu seterusnya","langkah lagu"

#define SYS_MUSIC_LAST_MS \
    "Lagu sebelumnya","Main lagu sebelumnya","Main lagu sebelumnya","Main muzik sebelumnya", \
    "Lagu sebelumnya","Main sebelumnya","Main lagu sebelumnya","Main muzik sebelumnya", \
    "muzik_trek_sebelumnya","muzik trek sebelumnya", \
    "Muzik_trek_sebelumnya","Muzik trek sebelumnya"

#define SYS_MUSIC_LOOP_MS \
    "Gelung trek tunggal","Gelung lagu tunggal","Kitar lagu tunggal", \
    "gelung trek tunggal","gelung lagu tunggal","kitar lagu tunggal", \
    "musikan_gelung_tunggal","gelung muzik tunggal", \
    "Musik_gelung_tunggal","Gelung muzik tunggal", \
    "Kitar satu lagu", \
    "kitar satu lagu"

#define SYS_MUSIC_LOOP_CLOSE_MS \
    "Matikan gelung","Keluar gelung","Batal gelung", \
    "Matikan gelung trek tunggal","Keluar gelung trek tunggal","Batal gelung trek tunggal", \
    "Matikan gelung lagu tunggal","Keluar gelung lagu tunggal","Batal gelung lagu tunggal", \
    "matikan gelung","keluar gelung","batal gelung", \
    "matikan gelung trek tunggal","keluar gelung trek tunggal","batal gelung trek tunggal", \
    "matikan gelung lagu tunggal","keluar gelung lagu tunggal","batal gelung lagu tunggal", \
    "Matikan kitar","Keluar kitar","Batal kitar","Matikan kitar satu lagu","Keluar kitar satu lagu","Batal kitar satu lagu", \
    "matikan kitar","keluar kitar","batal kitar","matikan kitar satu lagu","keluar kitar satu lagu","batal kitar satu lagu"

//=================设备控制指令=====================//
#define SYS_MUSIC_APP_AI_MS \
    "Mod AI","Mod Ejen AI","Mod Ejen", \
    "mod ai","mod ejen ai","mod ejen", \
    "mod AI","mod rjen AI","mod Ejen", \
    "Mod AI","Mod AI","Mod Ejen Pintar AI","Mod Ejen Pintar", \
    "mod AI","mod AI","mod Ejen Pintar AI","mod Ejen Pintar"

#define SYS_MUSIC_APP_UDISK_MS \
    "Main cakera USB","mod main cakera USB","mod cakera USB", \
    "main cakera USB","mod main cakera usb","mod cakera usb", \
    "Main USB","Mod main USB","Mod USB", \
    "main USB","mod main USB","mod USB", \
    "sumber_usb","sumber usb", \
    "Sumber_usb","Sumber usb"

#define SYS_MUSIC_APP_SD_MS \
    "Main kad SD","Main kad TF","Mod main kad SD","Mod main kad TF","Mod kad SD","Mod kad TF", \
    "Mod main kad SD","Mod main kad TF","Mod kad SD","Mod kad TF","Mod SD","Mod TF", \
    "main kad SD","main kad TF","mod main kad SD","mod main kad TF","mod kad SD","mod kad TF", \
    "mod main kad SD","mod main kad TF","mod kad SD","mod kad TF","mod SD","mod TF", \
    "sumber_kadSD","sumber kadSD", \
    "Sumber_kadSD","Sumber kadSD"

#define SYS_MUSIC_APP_BT_MS \
    "Main Bluetooth","Mod Bluetooth","Mod main Bluetooth", \
    "main Bluetooth","mod Bluetooth","mod main Bluetooth"

#define SYS_MUSIC_APP_AUX_MS \
    "Mod Garis","Mod input garisan", \
    "mod garis","mod input garisan", \
    "Mod Input","Mod Input Line", \
    "mod input","mod input line"

//=================闹钟指令=====================//
#define SYS_ALARM_DEL_MS \
    "Padam amaran","Batal amaran","Batal peringatan","Matikan amaran", \
    "padam amaran","batal amaran","batal peringatan","matikan amaran", \
    "Padam penggera","Batal penggera","Matikan penggera", \
    "padam penggera","batal penggera","matikan penggera"

#define SYS_ALARM_DEL_ALL_MS \
    "Padam semua amaran","Batalkan semua amaran","Batalkan semua peringatan","Matikan semua amaran", \
    "padam semua amaran","batalkan semua amaran","batalkan semua peringatan","matikan semua amaran", \
    "Padam semua penggera","Batal semua penggera","Batal semua peringatan","Matikan semua penggera", \
    "padam semua penggera","batal semua penggera","batal semua peringatan","matikan semua penggera"

#define SYS_ALARM_LOOK_MS \
    "Lihat amaran","Apakah amaran yang ada","Halaman amaran","Masuk halaman amaran","Lihat halaman amaran", \
    "lihat amaran","apakah amaran yang ada","halaman amaran","masuk halaman amaran","lihat halaman amaran"

//=================音量调节指令=====================//
#define SYS_VOLUME_SET_MS  "volume_set_"

#define SYS_MUSIC_VOLUME_MS \
    "Kurangkan bunyi","Bunyinya lebih senyap","Turunkan volum","Kecilkan volum", \
    "Tambah bunyi","Bunyinya lebih kuat","Naikkan volum","Besarkan volum"

#define SYS_MUSIC_MAX_VOLUME_MS \
    "Volum maksimum","Volum tertinggi","Volum paling besar","Volum tertinggi", \
    "volum maksimum","volum tertinggi","volum paling besar","volum tertinggi", \
    "Bunyi maksimum","Bunyi tertinggi","Bunyi paling kuat","Bunyi tertinggi", \
    "bunyi maksimum","bunyi tertinggi","bunyi paling kuat","bunyi tertinggi"

#define SYS_MUSIC_MIN_VOLUME_MS \
    "Volum minimum","Volum terendah","Volum paling kecil","Volum terendah", \
    "volum minimum","volum terendah","volum paling kecil","volum terendah", \
    "Bunyi minimum","Bunyi terendah","Bunyi paling lembut","Bunyi terendah", \
    "bunyi minimum","bunyi terendah","bunyi paling lembut","bunyi terendah"

#endif // CONFIG_INSTRUCTION_MS

#endif // __MS_INSTRUCT_WORD_H__
