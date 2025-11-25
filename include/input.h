// include/input.h
#ifndef INPUT_H
#define INPUT_H

#include <stddef.h>

// 입력 결과 코드
#define INPUT_OK       0
#define INPUT_INVALID  1
#define INPUT_TIMEOUT  2

// 유저 모드에서 사용할 기본 경고 / 타임아웃 시간(초)
#define INPUT_WARN_SEC     10
#define INPUT_TIMEOUT_SEC  13

// 정수 입력 (warn_sec/timeout_sec 둘 다 0이면 타임아웃 없이 블로킹 입력)
int timed_read_int(const char *prompt, int *out,
                   int warn_sec, int timeout_sec);

// 한 줄 문자열 입력 (warn_sec/timeout_sec 둘 다 0이면 타임아웃 없이 블로킹 입력)
int timed_read_line(const char *prompt, char *buf, size_t size,
                    int warn_sec, int timeout_sec);

#endif
