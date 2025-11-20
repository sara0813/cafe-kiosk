// src/input.c
#include <stdio.h>
#include <stdlib.h>

#include "input.h"

#ifdef _WIN32
// ---------- Windows (MinGW, MSVC)용 ----------
#include <conio.h>
#include <windows.h>

// 키보드 입력이 준비될 때까지 대기
static int wait_for_input(int warn_sec, int timeout_sec) {
    int elapsed = 0;
    int warned = 0;

    while (elapsed < timeout_sec) {
        if (_kbhit()) {   // 키가 눌렸으면 입력 준비됨
            return 1;
        }

        Sleep(1000);      // 1초 대기
        elapsed++;

        if (!warned && elapsed >= warn_sec) {
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

static int wait_for_input(int warn_sec, int timeout_sec) {
    int elapsed = 0;
    int warned = 0;

    while (elapsed < timeout_sec) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(0, &rfds);   // stdin (fd 0)

        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int ret = select(1, &rfds, NULL, NULL, &tv);
        if (ret > 0) {
            return 1;       // 입력 준비됨
        } else if (ret < 0) {
            return 0;       // 에러도 그냥 timeout 취급
        }

        elapsed++;

        if (!warned && elapsed >= warn_sec) {
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

// ---------- 공통 부분 ----------

int timed_read_int(const char *prompt, int *out,
                   int warn_sec, int timeout_sec) {
    if (prompt) {
        printf("%s", prompt);
        fflush(stdout);
    }

    if (!wait_for_input(warn_sec, timeout_sec)) {
        return INPUT_TIMEOUT;
    }

    char buf[64];
    if (!fgets(buf, sizeof(buf), stdin)) {
        return INPUT_TIMEOUT;
    }

    char *endptr;
    long val = strtol(buf, &endptr, 10);

    if (endptr == buf) {
        return INPUT_INVALID;   // 숫자 하나도 못 읽음
    }

    while (*endptr == ' ' || *endptr == '\t' ||
           *endptr == '\r' || *endptr == '\n') {
        endptr++;
    }
    if (*endptr != '\0') {
        return INPUT_INVALID;   // 뒤에 이상한 문자 더 있음
    }

    *out = (int)val;
    return INPUT_OK;
}
