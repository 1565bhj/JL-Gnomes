#ifndef __JA_INSTRUCT_WORD_H__
#define __JA_INSTRUCT_WORD_H__

#ifdef CONFIG_INSTRUCTION_JA
//=================日本語 コマンドワード=====================//

//=================システム系コマンド=====================//
#define SYS_CLOSE_JA \
    "閉じる", "止める", "切る", "下がれ", "会話終了", "会話切る", "会話閉じる", "会話止める", \
    "話すのやめる", "もう話したくない"

#define SYS_STR_POWER_OFF_JA \
    "電源オフ", "シャットダウン", "電源切る"

#define SYS_30MIN_JA \
    "一時間半後", "一時間半経ったら", "一時間半過ぎたら", \
    "三十分後", "三十分経ったら", "三十分過ぎたら", \
    "半時間後", "半時間経ったら", "半時間過ぎたら"

#define SYS_90MIN_JA \
    "九十分後", "九十分経ったら", "九十分過ぎたら"

#define SYS_HOUR_TIMEOUT_JA \
    "時間後", "時間経ったら", "時間過ぎたら"

#define SYS_MIN_TIMEOUT_JA \
    "分後", "分経ったら", "分過ぎたら"

#define SYS_COUNTOWN_TIMEOUT_JA \
    "カウントダウン", "タイマーセット", "時間計測"

#define SYS_COUNTOWN_TIMEOUT_CLOSE_JA \
    "タイマーキャンセル", "タイマー削除", "カウントダウン取消", "カウントダウン終了"

//=================音楽系コマンド=====================//
#define SYS_MUSIC_PLAY_JA \
    "再生", "曲をかける", "音楽再生"

#define SYS_MUSIC_CONTINUE_JA \
    "再生再開", "曲続けてかける", "音楽続き再生"

#define SYS_MUSIC_STOP_JA \
    "一時停止", "音楽一時停止", "曲一旦止める"

#define SYS_MUSIC_CLOSE_JA \
    "曲止める", "再生停止", "音楽オフ", "音楽閉じる", "曲停止", "音楽中断"

#define SYS_MUSIC_NEXT_JA \
    "次の曲", "次の曲かけて", "曲送り", "曲切り替え", "トラック送る"

#define SYS_MUSIC_LAST_JA \
    "前の曲", "前の曲再生", "一曲戻る"

#define SYS_MUSIC_LOOP_JA \
    "一曲リピート", "単曲ループ再生"

#define SYS_MUSIC_LOOP_CLOSE_JA \
    "リピート解除", "ループ終了", "リピート取消", \
    "一曲リピート解除", "単曲ループ終了", "リピート機能オフ"

//=================機器モード制御コマンド=====================//
#define SYS_MUSIC_APP_AI_JA \
    "AIモード", "人工知能モード", "スマートエージェントモード"

#define SYS_MUSIC_APP_UDISK_JA \
    "USB再生", "USBモード", "USB音楽モード"

#define SYS_MUSIC_APP_SD_JA \
    "SDカード再生", "TFカード再生", "SDカードモード", "TFカードモード"

#define SYS_MUSIC_APP_BT_JA \
    "Bluetooth再生", "Bluetoothモード"

#define SYS_MUSIC_APP_AUX_JA \
    "ライン入力モード", "補助入力モード"

//=================アラーム系コマンド=====================//
#define SYS_ALARM_DEL_JA \
    "アラーム削除", "アラーム取消", "通知キャンセル", "アラームオフ"

#define SYS_ALARM_DEL_ALL_JA \
    "すべてのアラーム削除", "全アラーム取消", "全通知キャンセル", "全アラーム終了"

#define SYS_ALARM_LOOK_JA \
    "アラーム確認", "設定アラーム表示", "アラーム画面開く", "アラーム一覧確認"

//=================音量調整コマンド=====================//
#define SYS_VOLUME_SET_JA  "ボリューム"

#define SYS_MUSIC_VOLUME_JA \
    "音を小さく", "音楽の音を小さく", "ボリューム下げる", "音量低く", \
    "音を大きく", "音楽の音を大きく", "ボリューム上げる", "音量高く"

#define SYS_MUSIC_MAX_VOLUME_JA \
    "最大ボリューム", "一番大きい音", "ボリューム最大", "音量最高"

#define SYS_MUSIC_MIN_VOLUME_JA \
    "最小ボリューム", "一番小さい音", "ボリューム最小", "音量最低"

#endif
#endif // __JA_INSTRUCT_WORD_H__