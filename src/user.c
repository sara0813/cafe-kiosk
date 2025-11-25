// src/user.c
#include <stdio.h>
#include "menu.h"
#include "cart.h"
#include "user.h"
#include "payment.h"
#include "input.h"

// ---------- internal helpers ----------

// 카테고리 선택 (0~7)
//  <0 : 타임아웃 → 메인으로
//   0 : 메인으로 (Back)
//  1~7 : 정상 카테고리
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
            return -1;  // ★ 타임아웃 → 메인으로
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

// 메뉴 id 선택 (0: 카테고리로, <0: 타임아웃→메인)
static int select_menu_id(void) {
    int id;
    int result;

    while (1) {
        result = timed_read_int(
            "Enter menu id to order (0: back to category): ",
            &id, INPUT_WARN_SEC, INPUT_TIMEOUT_SEC);

        if (result == INPUT_TIMEOUT) {
            printf("\nTimeout. Back to main menu.\n\n");
            return -1;   // ★ 타임아웃
        }
        if (result == INPUT_INVALID) {
            printf("Invalid input. Please enter number.\n\n");
            continue;
        }

        if (id < 0) {
            printf("Invalid id.\n\n");
            continue;
        }

        return id;  // 0 or positive
    }
}

static int select_quantity(void) {
    int qty;
    int result;

    while (1) {
        result = timed_read_int("Quantity: ", &qty,
                                INPUT_WARN_SEC, INPUT_TIMEOUT_SEC);

        if (result == INPUT_TIMEOUT) {
            printf("\nTimeout. Back to main menu.\n\n");
            return -1;   // ★ 타임아웃
        }
        if (result == INPUT_INVALID) {
            printf("Invalid input. Please enter number.\n\n");
            continue;
        }
        if (qty <= 0) {
            printf("Invalid quantity. Must be >= 1.\n\n");
            continue;
        }
        return qty;
    }
}

// 메뉴 담은 뒤 "메뉴 추가 / 결제" 선택
//  0 : 전체 주문 취소하고 메인으로
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
            return 0;          // ★ 메인으로
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
            return -1;   // ★ 타임아웃
        }
        if (result == INPUT_INVALID) {
            printf("Invalid input. Please enter number.\n\n");
            continue;
        }

        if (choice < 0 || choice > 2) {
            printf("Please select 0~2.\n\n");
            continue;
        }

        return choice; // -1, 0, 1, 2
    }
}

// ---------- public ----------

void run_user_mode(void) {
    cart_init();  // 유저 모드 들어올 때마다 장바구니 초기화

    while (1) {
        // 1) 카테고리 선택
        int category = select_category();

        if (category < 0) {
            // 타임아웃
            printf("\n");
            return;
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

        // 2) 메뉴 로드 + 출력
        MenuItem items[MAX_MENU_ITEMS];
        int count = load_menu(filepath, items, MAX_MENU_ITEMS);
        if (count <= 0) {
            printf("Failed to load menu from %s\n\n", filepath);
            continue;
        }

        print_menu(items, count);

        // 3) 메뉴 id 선택
        int id = select_menu_id();
        if (id < 0) {
            // 타임아웃 -> 메인
            printf("\n");
            return;
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

        // 4) 수량 선택
        int qty = select_quantity();
        if (qty < 0) {
            // 타임아웃 -> 메인
            printf("\n");
            return;
        }

        // 5) 장바구니 담기
        cart_add(selected, qty);
        printf("\nAdded to cart: %s x%d\n\n", selected->name, qty);

        // 6) 장바구니 / 결제 루프
        while (1) {
            cart_print();

            int next = show_after_add_menu();
            if (next == 0) {
                // timeout 또는 전체 취소 -> 메인으로
                return;
            } else if (next == 1) {
                // add more -> 카테고리 선택부터 다시
                break;
            } else if (next == 2) {
                // === 여기서 먼저 '먹고가기 / 포장' 선택 ===
                int place = select_order_place();
                if (place < 0) {
                    // 타임아웃 -> 메인
                    return;
                }
                if (place == 0) {
                    // 주문 방식 선택만 취소 -> 장바구니 화면으로 돌아가기
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
