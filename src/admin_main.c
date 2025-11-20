// src/admin_main.c
#include <stdio.h>
#include "admin.h"

// functions implemented in other admin_*.c files
void run_sales_menu(void);
void run_menu_edit_menu(void);
void run_stock_menu(void);
void run_refund_menu(void);

static void flush_input(void) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
        ;
}

void run_admin_mode(void) {
    int choice;

    while (1) {
        printf("=== Admin Mode ===\n");
        printf("1. Sales management (by payment/menu/log)\n");
        printf("2. Menu / price edit\n");
        printf("3. Stock management\n");
        printf("4. Refund\n");
        printf("0. Back to main menu\n");
        printf("Select: ");

        if (scanf("%d", &choice) != 1) {
            flush_input();
            printf("Invalid input.\n\n");
            continue;
        }

        switch (choice) {
        case 0:
            printf("\nBack to main menu.\n\n");
            return;
        case 1:
            run_sales_menu();
            break;
        case 2:
            run_menu_edit_menu();
            break;
        case 3:
            run_stock_menu();
            break;
        case 4:
            run_refund_menu();
            break;
        default:
            printf("No such menu.\n\n");
            break;
        }
    }
}
