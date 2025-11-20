// src/main.c
#include <stdio.h>
#include "user.h"
#include "admin.h"

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
        printf("Select: ");

        if (scanf("%d", &choice) != 1) {
            // flush invalid input
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF)
                ;
            printf("Invalid input. Please enter a number.\n\n");
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
