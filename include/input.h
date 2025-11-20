// include/input.h
#ifndef INPUT_H
#define INPUT_H

#define INPUT_OK       1   // normal input
#define INPUT_TIMEOUT  0   // timeout
#define INPUT_INVALID -1   // not a valid integer

// default seconds (for all screens)
#define INPUT_WARN_SEC    5   // show warning after 5s
#define INPUT_TIMEOUT_SEC 8   // auto cancel after 8s

// warn_sec 초 동안 입력 없으면 경고,
// timeout_sec 초까지 입력 없으면 TIMEOUT 리턴
int timed_read_int(const char *prompt, int *out,
                   int warn_sec, int timeout_sec);

#endif
