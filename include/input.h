// include/input.h
#ifndef INPUT_H
#define INPUT_H

#include <stddef.h>

// 입력 결과 코드
#define INPUT_OK       0
#define INPUT_INVALID  1
#define INPUT_TIMEOUT  2

// 경고 / 타임아웃 기본 시간(초)
#define INPUT_WARN_SEC     5   // 5초 동안 아무 입력 없으면 경고
#define INPUT_TIMEOUT_SEC  8   // 총 8초 지나면 화면 취소

// 정수 입력
int timed_read_int(const char *prompt, int *out,
                   int warn_sec, int timeout_sec);

// 한 줄 문자열 입력
int timed_read_line(const char *prompt, char *buf, size_t size,
                    int warn_sec, int timeout_sec);

#endif
