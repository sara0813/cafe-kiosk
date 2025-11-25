// src/main.c
#include <stdio.h>
#include "user.h"
#include "admin.h"
#include "input.h"

int main(void) {
    int choice;

    while (1) {
        printf("=================================\n");
        printf("          Cafe Kiosk\n");
        printf("=================================\n");
        printf("1. User mode (Order)\n");
        printf("2. Admin mode\n");
        printf("0. Exit\n");
        printf("---------------------------------\n");

        int result = timed_read_int("Select: ", &choice,
                                    0, 0);   // ★ 타임아웃 없이 블로킹

        if (result == INPUT_INVALID) {
            printf("Invalid input. Please enter a number.\n\n");
            continue;
        }
        if (result == INPUT_TIMEOUT) {
            // theoretically 거의 안 나옴 (EOF 등)
            printf("Input error.\n\n");
            continue;
        }

        printf("\n");

        switch (choice) {
        case 1:
            run_user_mode();
            break;
        case 2:
            if (admin_login()) {   // 로그인 성공 시에만
                run_admin_mode();
            }
            break;
        case 0:
            printf("Bye.\n");
            return 0;
        default:
            printf("No such menu. Try again.\n\n");
            break;
        }
    }

    return 0;
}
