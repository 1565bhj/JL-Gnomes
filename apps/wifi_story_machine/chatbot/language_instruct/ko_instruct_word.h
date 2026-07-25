#ifndef __KO_INSTRUCT_WORD_H__
#define __KO_INSTRUCT_WORD_H__

#ifdef CONFIG_INSTRUCTION_KO
//=================한국어 명령어=====================//

//=================시스템 명령=====================//
#define SYS_CLOSE_KO \
    "꺼라", "닫아", "정지해", "물러가라", "대화 끊어", "대화 종료", "대화 닫아", "대화 정지", \
    "대화 끝내", "더 이상 말하기 싫어"

#define SYS_STR_POWER_OFF_KO \
    "전원 끄기", "기기 꺼라", "종료해"

#define SYS_30MIN_KO \
    "한 시간 반 후", "한 시간 반 지나고", "한 시간 반 뒤", \
    "삼십 분 후", "삼십 분 지나고", "삼십 분 뒤", \
    "반 시간 후", "반 시간 지나고", "반 시간 뒤"

#define SYS_90MIN_KO \
    "아흔 분 후", "아흔 분 지나고", "아흔 분 뒤"

#define SYS_HOUR_TIMEOUT_KO \
    "시간 후", "시간 지나고", "시간 뒤"

#define SYS_MIN_TIMEOUT_KO \
    "분 후", "분 지나고", "분 뒤"

#define SYS_COUNTOWN_TIMEOUT_KO \
    "카운트다운 시작", "타이머 설정", "시간 재기"

#define SYS_COUNTOWN_TIMEOUT_CLOSE_KO \
    "타이머 취소", "타이머 삭제", "카운트다운 취소", "카운트다운 종료"

//=================음악 명령=====================//
#define SYS_MUSIC_PLAY_KO \
    "음악 재생", "노래 틀어", "음악 틀어"

#define SYS_MUSIC_CONTINUE_KO \
    "계속 재생", "노래 계속 틀어", "음악 이어서 재생"

#define SYS_MUSIC_STOP_KO \
    "일시 정지", "음악 일시정지", "노래 잠시 멈춰"

#define SYS_MUSIC_CLOSE_KO \
    "노래 멈춰", "재생 중지", "음악 끄기", "음악 닫기", "노래 정지", "음악 중단"

#define SYS_MUSIC_NEXT_KO \
    "다음 곡", "다음 노래 틀어", "곡 바꿔", "노래 교체", "트랙 넘겨"

#define SYS_MUSIC_LAST_KO \
    "이전 곡", "이전 노래 틀어", "앞 곡 재생"

#define SYS_MUSIC_LOOP_KO \
    "한 곡 반복", "단곡 반복 재생"

#define SYS_MUSIC_LOOP_CLOSE_KO \
    "반복 끄기", "반복 종료", "반복 취소", \
    "단곡 반복 취소", "단곡 반복 종료", "반복 기능 해제"

//=================기기 모드 제어 명령=====================//
#define SYS_MUSIC_APP_AI_KO \
    "AI 모드", "인공지능 모드", "스마트 에이전트 모드"

#define SYS_MUSIC_APP_UDISK_KO \
    "USB 재생", "USB 모드", "USB 음악 모드"

#define SYS_MUSIC_APP_SD_KO \
    "SD카드 재생", "TF카드 재생", "SD카드 모드", "TF카드 모드"

#define SYS_MUSIC_APP_BT_KO \
    "블루투스 재생", "블루투스 모드"

#define SYS_MUSIC_APP_AUX_KO \
    "라인 입력 모드", "보조 입력 모드"

//=================알람 명령=====================//
#define SYS_ALARM_DEL_KO \
    "알람 삭제", "알람 취소", "알림 취소", "알람 끄기"

#define SYS_ALARM_DEL_ALL_KO \
    "모든 알람 삭제", "전체 알람 취소", "모든 알림 취소", "전체 알람 종료"

#define SYS_ALARM_LOOK_KO \
    "알람 확인", "설정된 알람 보여줘", "알람 페이지 열어", "알람 목록 확인"

//=================음량 조절 명령=====================//
#define SYS_VOLUME_SET_KO  "음량 "

#define SYS_MUSIC_VOLUME_KO \
    "소리 작게", "음악 소리 작게", "음량 낮춰", "볼륨 줄여", \
    "소리 크게", "음악 소리 크게", "음량 높여", "볼륨 키워"

#define SYS_MUSIC_MAX_VOLUME_KO \
    "최대 음량", "가장 큰 소리", "음량 최대로", "볼륨 최고"

#define SYS_MUSIC_MIN_VOLUME_KO \
    "최소 음량", "가장 작은 소리", "음량 최저로", "볼륨 최소"

#endif
#endif // __KO_INSTRUCT_WORD_H__