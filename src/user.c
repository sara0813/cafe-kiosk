// src/user.c
#include <stdio.h>
#include "cart.h"
#include "user.h"
#include "user_screen.h"
#include "user_order.h"
#include "payment.h"

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
                    printf("\nOrder type: Dine-in\n");
                } else if (place == 2) {
                    printf("\nOrder type: Take-out\n");
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
