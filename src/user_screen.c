// src/user_screen.c
#include <stdio.h>
#include "input.h"
#include "cart.h"
#include "user_screen.h"

// 카테고리 선택 (0~7)
//  -1 : 타임아웃 -> 메인으로
int select_category(void) {
    int cat;
    int result;

    while (1) {
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

        result = timed_read_int("Select: ", &cat,
                                INPUT_WARN_SEC, INPUT_TIMEOUT_SEC);

        if (result == INPUT_TIMEOUT) {
            printf("\nTimeout. Back to main menu.\n\n");
            return -1;  // 타임아웃 → 메인으로
        }
        if (result == INPUT_INVALID) {
            printf("Invalid input. Please enter number.\n\n");
            continue;
        }

        if (cat < 0 || cat > 7) {
            printf("Please select 0~7.\n\n");
            continue;
        }

        return cat;     // 0~7 중 하나
    }
}

// 메뉴 담은 뒤 "메뉴 추가 / 결제" 선택
//  0 : (타임아웃 등) 전체 주문 취소하고 메인으로
//  1 : 메뉴 더 담기
//  2 : 결제 진행
int show_after_add_menu(void) {
    int choice;
    int result;

    while (1) {
        printf("1. Add more menu\n");
        printf("2. Payment\n");

        result = timed_read_int("Select: ", &choice,
                                INPUT_WARN_SEC, INPUT_TIMEOUT_SEC);

        if (result == INPUT_TIMEOUT) {
            printf("\nTimeout. Order cancelled and back to main menu.\n\n");
            cart_init();       // 장바구니 비우기
            return 0;          // 메인으로
        }
        if (result == INPUT_INVALID) {
            printf("Invalid input. Please enter number.\n\n");
            continue;
        }

        if (choice == 1 || choice == 2)
            return choice;

        printf("Please select 1 or 2.\n\n");
    }
}

// 주문 장소 선택
//  <0 : 타임아웃→메인
//   0 : 뒤로 (장바구니 화면으로)
//  1,2 : 정상
int select_order_place(void) {
    int choice;
    int result;

    while (1) {
        printf("=== Order type ===\n");
        printf("1. Dine-in\n");
        printf("2. Take-out\n");
        printf("0. Back\n");
        printf("------------------\n");

        result = timed_read_int("Select: ", &choice,
                                INPUT_WARN_SEC, INPUT_TIMEOUT_SEC);

        if (result == INPUT_TIMEOUT) {
            printf("\nTimeout. Back to main menu.\n\n");
            return -1;   // 타임아웃
        }
        if (result == INPUT_INVALID) {
            printf("Invalid input. Please enter number.\n\n");
            continue;
        }

        if (choice < 0 || choice > 2) {
            printf("Please select 0~2.\n\n");
            continue;
        }

        return choice; // 0,1,2
    }
}
