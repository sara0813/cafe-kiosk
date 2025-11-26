// src/user_order.c
#include <stdio.h>
#include "menu.h"
#include "cart.h"
#include "price.h"
#include "input.h"
#include "stock.h"
#include "user_order.h"

// 카테고리 번호 -> 메뉴 파일 경로 (이 파일 안에서만 사용)
static const char *get_menu_filepath(int cat) {
    switch (cat) {
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

/*
 * 한 카테고리 안에서 메뉴를 선택하고 장바구니에 담기
 *   return  0 : 정상 종료(메뉴를 담았거나, 0을 눌러 빠져나옴)
 *   return -1 : 타임아웃 → 메인으로
 */
int order_in_category(int cat) {
    const char *menu_path = get_menu_filepath(cat);
    if (!menu_path) {
        printf("Invalid category.\n\n");
        return 0;
    }

    MenuItem items[MAX_MENU_ITEMS];
    int count = load_menu(menu_path, items, MAX_MENU_ITEMS);
    if (count <= 0) {
        printf("No menu items in this category.\n\n");
        return 0;
    }

    int stock[MAX_MENU_ITEMS];
    stock_load_category(cat, stock, count);  // 재고 읽기

    while (1) {
        // --- 메뉴 + 재고 출력 ---
        printf("\n=== Menu (Category %d) ===\n", cat);
        printf("%-4s %-30s %10s %10s\n", "No.", "Menu name", "Price", "Stock");
        printf("----------------------------------------------------------\n");

        for (int i = 0; i < count; i++) {
            char price_buf[32];
            format_price_with_comma(items[i].price, price_buf, sizeof(price_buf));

            if (stock[i] <= 0) {
                // 재고 0이면 SOLD OUT 표시
                printf("%3d  %-30s %10s  %10s\n",
                       i + 1, items[i].name, price_buf, "SOLD OUT");
            } else {
                printf("%3d  %-30s %10s  %10d\n",
                       i + 1, items[i].name, price_buf, stock[i]);
            }
        }
        printf("0. Back\n\n");

        // --- 메뉴 번호 선택 ---
        int sel;
        int result = timed_read_int("Select menu (0 = back): ",
                                    &sel, INPUT_WARN_SEC, INPUT_TIMEOUT_SEC);

        if (result == INPUT_TIMEOUT) {
            printf("\nTimeout. Back to main menu.\n\n");
            return -1;    // 타임아웃
        }
        if (result == INPUT_INVALID) {
            printf("Invalid input.\n\n");
            continue;
        }

        if (sel == 0) {
            printf("Back to category selection.\n\n");
            return 0;
        }
        if (sel < 1 || sel > count) {
            printf("Please select 1~%d.\n\n", count);
            continue;
        }

        int idx = sel - 1;

        // SOLD OUT 체크
        if (stock[idx] <= 0) {
            printf("Sorry, \"%s\" is SOLD OUT.\n\n", items[idx].name);
            continue;
        }

        // --- 수량 입력 ---
        int qty;
        while (1) {
            result = timed_read_int("Quantity: ", &qty,
                                    INPUT_WARN_SEC, INPUT_TIMEOUT_SEC);
            if (result == INPUT_TIMEOUT) {
                printf("\nTimeout. Back to main menu.\n\n");
                return -1;   // 타임아웃
            }
            if (result == INPUT_INVALID) {
                printf("Invalid input.\n\n");
                continue;
            }
            if (qty <= 0) {
                printf("Quantity must be >= 1.\n\n");
                continue;
            }
            if (qty > stock[idx]) {
                printf("Only %d left in stock.\n\n", stock[idx]);
                continue;
            }
            break;
        }

        // 재고 차감 + 파일 저장 + 장바구니 추가
        stock[idx] -= qty;

        if (!stock_save_category(cat, items, stock, count)) {
            printf("Failed to update stock file.\n\n");
            // 저장 실패해도 일단 장바구니는 유지
        }

        cart_add(&items[idx], qty);
        printf("Added %d x \"%s\" to cart.\n\n", qty, items[idx].name);

        // ✅ 여기서 바로 상위(run_user_mode)로 돌아가서
        //    장바구니 / 결제 STEP으로 넘어가도록 한다.
        return 0;
    }
}
