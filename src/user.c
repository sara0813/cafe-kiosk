// src/user.c
#include <stdio.h>
#include "menu.h"

void run_user_mode(void) {
    int cat;
    const char *filepath = NULL;

    printf("=== Select Category ===\n");
    printf("1. Coffee\n");
    printf("2. Non-coffee\n");
    printf("3. Tea\n");
    printf("4. Dutch\n");
    printf("5. Signature\n");
    printf("6. Smoothie / Juice\n");
    printf("7. Dessert\n");
    printf("0. Back\n");
    printf("------------------------\n");
    printf("Select: ");

    if (scanf("%d", &cat) != 1) {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF)
            ;
        printf("Invalid input.\n\n");
        return;
    }

    switch (cat) {
    case 1: filepath = "data/menus/coffee.txt"; break;
    case 2: filepath = "data/menus/noncoffee.txt"; break;
    case 3: filepath = "data/menus/tea.txt"; break;
    case 4: filepath = "data/menus/dutch.txt"; break;
    case 5: filepath = "data/menus/signature.txt"; break;
    case 6: filepath = "data/menus/smoothie_juice.txt"; break;
    case 7: filepath = "data/menus/dessert.txt"; break;
    case 0:
        printf("\n");
        return;
    default:
        printf("No such category.\n\n");
        return;
    }

    MenuItem items[MAX_MENU_ITEMS];
    int count = load_menu(filepath, items, MAX_MENU_ITEMS);
    if (count <= 0) {
        printf("Failed to load menu from %s\n\n", filepath);
        return;
    }

    print_menu(items, count);

    // TODO: next step
    // - select menu
    // - add to cart
    // - go to payment
}
