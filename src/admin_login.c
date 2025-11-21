// src/admin_login.c
#include <stdio.h>
#include <string.h>
#include "admin.h"

#define ADMIN_ID "team2"
#define ADMIN_PW "1234"

// Login: return 1 on success, 0 on failure
int admin_login(void) {
    char id[32];
    char pw[32];
    int attempts = 0;

    while (attempts < 3) {
        printf("\n=== Admin Login ===\n");
        printf("ID: ");
        fflush(stdout);

        int ch;
        while (1) {
            ch = getchar();
            if (ch == EOF) {
                break;
            }
            if (ch == '\n') {
                // 이전 입력에서 남아있는 개행은 그냥 버림
                continue;
            }
            // 첫 번째 실제 문자면 다시 넣어두고 fgets가 읽게 함
            ungetc(ch, stdin);
            break;
        }

        if (!fgets(id, sizeof(id), stdin)) {
            printf("Invalid input.\n\n");
            continue;
        }
        char *nl = strchr(id, '\n');
        if (nl) *nl = '\0';

        printf("Password: ");
        fflush(stdout);
        if (!fgets(pw, sizeof(pw), stdin)) {
            printf("Invalid input.\n\n");
            continue;
        }
        nl = strchr(pw, '\n');
        if (nl) *nl = '\0';

        if (strcmp(id, ADMIN_ID) == 0 && strcmp(pw, ADMIN_PW) == 0) {
            printf("Login success.\n\n");
            return 1;
        } else {
            attempts++;
            int remain = 3 - attempts;
            printf("ID or password is incorrect.\n");
            if (remain > 0) {
                printf("Remaining attempts: %d\n\n", remain);
            }
        }
    }

    printf("Too many failed attempts. Back to main menu.\n\n");
    return 0;
}
