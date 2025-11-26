// src/user.c
#include <stdio.h>
#include "menu.h"
#include "cart.h"
#include "user.h"
#include "payment.h"
#include "price.h"
#include "input.h"
#include "stock.h"

// ---------- internal helpers ----------

// 카테고리 선택 (0~7)
//  -1 : 타임아웃 -> 메인으로
static int select_category(void) {
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

// 카테고리 번호 -> 메뉴 파일 경로
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

// 메뉴 담은 뒤 "메뉴 추가 / 결제" 선택
//  0 : (타임아웃 등) 전체 주문 취소하고 메인으로
//  1 : 메뉴 더 담기
//  2 : 결제 진행
static int show_after_add_menu(void) {
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
static int select_order_place(void) {
    int choice;
    int result;

    while (1) {
        printf("=== Order type ===\n");
        printf("1. For here\n");
        printf("2. To go\n");
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

/*
 * 한 카테고리 안에서 메뉴를 선택하고 장바구니에 담는 루프
 *   return 0  : 정상 종료(사용자가 0을 눌러 카테고리 화면 빠져나옴)
 *   return -1 : 타임아웃 → 메인으로
 */
static int order_in_category(int cat) {
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
    }
}

// ---------- public ----------

void run_user_mode(void) {
    cart_init();  // 유저 모드 들어올 때마다 장바구니 초기화

    while (1) {
        // 1) 카테고리 선택
        int category = select_category();

        if (category < 0) {
            // 타임아웃 -> 메인으로
            printf("\n");
            return;
        }
        if (category == 0) {
            // 메인 화면으로
            printf("\n");
            return;
        }

        // 2) 해당 카테고리에서 메뉴 선택 + 재고 차감 + 장바구니 담기
        int r = order_in_category(category);
        if (r < 0) {
            // 카테고리 안에서 타임아웃 발생 -> 메인으로
            return;
        }

        // 카테고리에서 그냥 뒤로 나온 경우:
        if (cart_is_empty()) {
            // 장바구니가 비어있으면 그냥 다시 카테고리 선택으로
            printf("Cart is empty. Back to category selection.\n\n");
            continue;
        }

        // 3) 장바구니 / 결제 루프
        while (1) {
            cart_print();

            int next = show_after_add_menu();
            if (next == 0) {
                // 타임아웃 또는 전체 취소 -> 메인으로
                return;
            } else if (next == 1) {
                // Add more menu -> 다시 카테고리 선택으로 나감
                break;
            } else if (next == 2) {
                // === 주문 방식 선택 ===
                int place = select_order_place();
                if (place < 0) {
                    // 타임아웃 -> 메인
                    return;
                }
                if (place == 0) {
                    // 주문 방식 선택만 취소 -> 장바구니 화면으로
                    printf("\nOrder type selection cancelled. Back to previous screen.\n\n");
                    continue;
                }

                if (place == 1) {
                    printf("\nOrder type: For here\n");
                } else if (place == 2) {
                    printf("\nOrder type: To go\n");
                }

                int paid = run_payment_flow(place);
                if (paid == 1) {
                    // 정상 결제 완료 -> 메인 메뉴
                    return;
                } else if (paid == 2) {
                    // 결제 도중 타임아웃 -> 메인 메뉴
                    return;
                } else {
                    printf("\nPayment cancelled. Back to previous screen.\n\n");
                }
            }
        } 
    }
}
