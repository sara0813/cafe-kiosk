// src/admin_sales.c
#include <stdio.h>
#include "input.h"

void show_total_sales(void);
void show_sales_by_payment(void);
void show_sales_by_menu(void);
void show_top5_menus(void);
void show_place_stats(void);
void show_order_log(void);

void run_sales_menu(void) {
    int choice;
    int result;

    while (1) {
        printf("=== Sales / Statistics ===\n");
        printf("1. View total sales\n");
        printf("2. Sales by payment method\n");
        printf("3. Sales by menu item\n");
        printf("4. Top 5 best-selling menus\n");
        printf("5. Dine-in vs Takeout statistics\n");
        printf("6. View order log\n");
        printf("0. Back\n");

        result = timed_read_int("Select: ", &choice, 0, 0);

        if (result == INPUT_INVALID) {
            printf("Invalid input.\n\n");
            continue;
        }

        printf("\n");

        switch (choice) {
        case 1: show_total_sales();      break;
        case 2: show_sales_by_payment(); break;
        case 3: show_sales_by_menu();    break;
        case 4: show_top5_menus();       break;
        case 5: show_place_stats();      break;
        case 6: show_order_log();        break;
        case 0:
            printf("Returning to admin main menu.\n\n");
            return;
        default:
            printf("Please select a number between 0 and 6.\n\n");
            break;
        }
    }
}
