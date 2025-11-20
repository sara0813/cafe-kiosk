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
        if (scanf("%31s", id) != 1) {
            printf("Invalid input.\n\n");
            continue;
        }

        printf("Password: ");
        if (scanf("%31s", pw) != 1) {
            printf("Invalid input.\n\n");
            continue;
        }

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
