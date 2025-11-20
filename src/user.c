// src/user.c
#include <stdio.h>
#include "menu.h"
#include "cart.h"
#include "user.h"
#include "payment.h"
#include "input.h"


// ---------- internal helpers ----------


static int select_category(void) {
    int cat;

    printf("=== Select Category ===\n");
    printf("1. Coffee\n");
    printf("2. Non-coffee\n");
    printf("3. Tea\n");
    printf("4. Dutch\n");
    printf("5. Signature\n");
    printf("6. Smoothie / Juice\n");
    printf("7. Dessert\n");
    printf("0. Back (to main)\n");
    printf("------------------------\n");

    int result = timed_read_int("Select: ", &cat,
                                INPUT_WARN_SEC, INPUT_TIMEOUT_SEC);

    if (result == INPUT_TIMEOUT) {
        printf("\nTimeout. Back to main menu.\n\n");
        // 0을 리턴하면 run_user_mode에서 메인으로 나감
        return 0;
    }
    if (result == INPUT_INVALID) {
        printf("Invalid input.\n\n");
        return -1;
    }

    return cat;
}



static const char *get_menu_filepath(int category) {
    switch (category) {
    case 1: return "data/menus/coffee.txt";
    case 2: return "data/menus/noncoffee.txt";
    case 3: return "data/menus/tea.txt";
    case 4: return "data/menus/dutch.txt";
    case 5: return "data/menus/signature.txt";
    case 6: return "data/menus/smoothie_juice.txt";
    case 7: return "data/menus/dessert.txt";
    default: return NULL;
    }
}

static int select_menu_id(void) {
    int id;
    int result = timed_read_int(
        "Enter menu id to order (0: back to category): ",
        &id, INPUT_WARN_SEC, INPUT_TIMEOUT_SEC);

    if (result == INPUT_TIMEOUT) {
        printf("\nTimeout. Back to category.\n\n");
        // 0은 "back to category" 의미
        return 0;
    }
    if (result == INPUT_INVALID) {
        printf("Invalid input.\n\n");
        return -1;
    }
    return id;
}


static int select_quantity(void) {
    int qty;
    int result = timed_read_int("Quantity: ", &qty,
                                INPUT_WARN_SEC, INPUT_TIMEOUT_SEC);

    if (result == INPUT_TIMEOUT) {
        printf("\nTimeout. Cancel this item.\n\n");
        // 음수나 0이면 run_user_mode에서 그냥 다시 루프
        return -1;
    }
    if (result == INPUT_INVALID || qty <= 0) {
        printf("Invalid quantity.\n\n");
        return -1;
    }
    return qty;
}


// 메뉴 담은 뒤 "메뉴 추가 / 결제" 선택
static int show_after_add_menu(void) {
    int choice;

    while (1) {
        printf("1. Add more menu\n");
        printf("2. Payment\n");

        int result = timed_read_int("Select: ", &choice,
                                    INPUT_WARN_SEC, INPUT_TIMEOUT_SEC);

        if (result == INPUT_TIMEOUT) {
            printf("\nTimeout. Order cancelled and back to main menu.\n\n");
            cart_init();       // 장바구니 비우기
            return 0;          // 0 = 전체 취소
        }
        if (result == INPUT_INVALID) {
            printf("Invalid input.\n\n");
            continue;
        }

        if (choice == 1 || choice == 2)
            return choice;

        printf("Please select 1 or 2.\n\n");
    }
}



// ---------- public ----------

void run_user_mode(void) {
    cart_init();  // 유저 모드 들어올 때마다 장바구니 초기화

    while (1) {
        int category = select_category();
        if (category == -1) { // 잘못된 입력 → 다시 카테고리 선택
            continue;
        }
        if (category == 0) {
            // 메인 화면으로
            printf("\n");
            return;
        }

        const char *filepath = get_menu_filepath(category);
        if (!filepath) {
            printf("No such category.\n\n");
            continue;
        }

        MenuItem items[MAX_MENU_ITEMS];
        int count = load_menu(filepath, items, MAX_MENU_ITEMS);
        if (count <= 0) {
            printf("Failed to load menu from %s\n\n", filepath);
            continue;
        }

        print_menu(items, count);

        int id = select_menu_id();
        if (id == -1) {
            continue; // invalid
        }
        if (id == 0) {
            // 카테고리로 돌아가기
            printf("\n");
            continue;
        }

        const MenuItem *selected = NULL;
        for (int i = 0; i < count; i++) {
            if (items[i].id == id) {
                selected = &items[i];
                break;
            }
        }

        if (!selected) {
            printf("No such menu id.\n\n");
            continue;
        }

        int qty = select_quantity();
        if (qty <= 0) {
            continue;
        }

                cart_add(selected, qty);
        printf("\nAdded to cart: %s x%d\n\n", selected->name, qty);

        // cart / payment loop
        while (1) {
            cart_print();

            int next = show_after_add_menu();
            if (next == 0) {
                // timeout or user cancelled -> 메인으로
                return;
            } else if (next == 1) {
                // add more
                break;
            } else if (next == 2) {
                int paid = run_payment_flow();
                if (paid) {
                    return;   // payment success -> 메인
                } else {
                    printf("\nPayment cancelled. Back to previous screen.\n\n");
                    // 다시 cart_print -> show_after_add_menu
                }
            }
        }
    }
}
