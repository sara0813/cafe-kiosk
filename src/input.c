// src/input.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "input.h"

#ifdef _WIN32
// ---------- Windows (MinGW, MSVC)용 ----------
#include <conio.h>
#include <windows.h>

// 타임아웃이 있는 경우에만 사용하는, 입력 대기 함수
static int wait_for_input_with_timeout(int warn_sec, int timeout_sec) {
    fflush(stdout);

    int elapsed = 0;
    int warned = 0;

    while (elapsed < timeout_sec) {
        if (_kbhit()) {
            return 1;   // 입력 준비됨
        }

        Sleep(1000);
        elapsed++;

        if (!warned && warn_sec > 0 && elapsed >= warn_sec) {
            int remaining = timeout_sec - elapsed;
            if (remaining < 0) remaining = 0;

            printf("\n[Warning] No input for %d seconds.\n", elapsed);
            if (remaining > 0) {
                printf("If you do nothing, this screen will be cancelled in %d seconds.\n",
                       remaining);
            }
            fflush(stdout);
            warned = 1;
        }
    }
    return 0;   // timeout
}

#else
// ---------- Linux (PuTTY 서버)용 ----------
#include <sys/select.h>
#include <unistd.h>

static int wait_for_input_with_timeout(int warn_sec, int timeout_sec) {
    int elapsed = 0;
    int warned  = 0;

    while (elapsed < timeout_sec) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);   // stdin (fd 0)

        struct timeval tv;
        tv.tv_sec  = 1;
        tv.tv_usec = 0;

        int ret = select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv);
        if (ret > 0) {
            return 1;       // 입력 준비됨
        } else if (ret < 0) {
            return 0;       // 에러도 그냥 timeout 취급
        }

        elapsed++;

        if (!warned && warn_sec > 0 && elapsed >= warn_sec) {
            int remaining = timeout_sec - elapsed;
            if (remaining < 0) remaining = 0;

            printf("\n[Warning] No input for %d seconds.\n", elapsed);
            if (remaining > 0) {
                printf("If you do nothing, this screen will be cancelled in %d seconds.\n",
                       remaining);
            }
            fflush(stdout);
            warned = 1;
        }
    }
    return 0;   // timeout
}
#endif

// ---------- 공통: 정수 입력 ----------

int timed_read_int(const char *prompt, int *out,
                   int warn_sec, int timeout_sec)
{
    char buf[64];

    // 1) 프롬프트 출력
    if (prompt) {
        printf("%s", prompt);
        fflush(stdout);
    }

    // 2) 타임아웃이 없는 경우: 그냥 블로킹 fgets 사용
    if (timeout_sec <= 0) {
        if (!fgets(buf, sizeof(buf), stdin)) {
            return INPUT_TIMEOUT;
        }
    } else {
        // 3) 타임아웃이 있는 경우: 먼저 입력 준비를 기다림
        if (!wait_for_input_with_timeout(warn_sec, timeout_sec)) {
            // 시간 내에 아무 입력도 안 옴
            return INPUT_TIMEOUT;
        }
        if (!fgets(buf, sizeof(buf), stdin)) {
            return INPUT_TIMEOUT;
        }
    }

    // 4) 공백/개행만 들어왔는지 체크
    char *p = buf;
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') {
        p++;
    }
    if (*p == '\0') {
        return INPUT_INVALID;
    }

    // 5) 정수 파싱
    char *endptr;
    long val = strtol(buf, &endptr, 10);

    if (endptr == buf) {
        return INPUT_INVALID;   // 숫자 하나도 못 읽음
    }

    // 6) 뒤에 공백/개행 외의 문자가 있으면 invalid
    while (*endptr == ' ' || *endptr == '\t' ||
           *endptr == '\r' || *endptr == '\n') {
        endptr++;
    }
    if (*endptr != '\0') {
        return INPUT_INVALID;
    }

    *out = (int)val;
    return INPUT_OK;
}

// ---------- 공통: 문자열 입력 ----------

int timed_read_line(const char *prompt, char *buf, size_t size,
                    int warn_sec, int timeout_sec)
{
    if (prompt) {
        printf("%s", prompt);
        fflush(stdout);
    }

    if (timeout_sec <= 0) {
        if (!fgets(buf, size, stdin)) {
            return INPUT_TIMEOUT;
        }
    } else {
        if (!wait_for_input_with_timeout(warn_sec, timeout_sec)) {
            return INPUT_TIMEOUT;
        }
        if (!fgets(buf, size, stdin)) {
            return INPUT_TIMEOUT;
        }
    }

    char *nl = strchr(buf, '\n');
    if (nl) *nl = '\0';

    return INPUT_OK;
}
