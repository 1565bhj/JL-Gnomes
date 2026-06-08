#ifndef __AIUI_MESSAGE_PARSE_H__
#define __AIUI_MESSAGE_PARSE_H__

int aiui_message_parse(const char *msg, const int len, bool *vad);
void aiui_set_filter();
#endif // __AIUI_MESSAGE_PARSE_H__
