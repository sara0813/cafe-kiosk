// src/payment_extra.c
#include <stdio.h>
#include "cart.h"
#include "order_log.h"
#include "input.h"
#include "payment_extra.h"

// return: 1 = save, 0 = not save
int ask_point(void) {
    int choice;

    while (1) {
        printf("\nDo you want to save points?\n");
        printf("1. Yes\n");
        printf("2. No\n");

        int result = timed_read_int("Select: ", &choice,
                                    INPUT_WARN_SEC, INPUT_TIMEOUT_SEC);

        if (result == INPUT_TIMEOUT) {
            // 포인트만 스킵하고 주문/결제 흐름은 계속 진행
            printf("\nTimeout. Skip point saving.\n\n");
            return 0;  // 그냥 적립 안 하고 진행
        }
        if (result == INPUT_INVALID) {
            printf("Invalid input.\n\n");
            continue;
        }

        if (choice == 1 || choice == 2)
            break;

        printf("Please select 1 or 2.\n\n");
    }

    return (choice == 1);
}

// 전화번호 입력 (숫자만 11자리, 타임아웃 포함)
void save_point(void) {
    char phone[32];

    while (1) {
        int result = timed_read_line(
            "Enter your phone number (11 digits): ",
            phone, sizeof(phone),
            INPUT_WARN_SEC, INPUT_TIMEOUT_SEC
        );

        if (result == INPUT_TIMEOUT) {
            printf("\nTimeout. Skip point saving.\n\n");
            return;   // 포인트 적립 자체를 건너뜀
        }

        // timed_read_line은 지금 INPUT_OK 또는 TIMEOUT만 쓰지만,
        // 혹시 나중을 위해 다른 값이 오면 다시 입력 받도록 처리
        if (result != INPUT_OK) {
            printf("Invalid input.\n\n");
            continue;
        }

        int len = 0;
        int ok = 1;
        for (const char *p = phone; *p; ++p) {
            if (*p < '0' || *p > '9') { // 숫자가 아니면 실패
                ok = 0;
                break;
            }
            len++;
        }

        if (!ok || len != 11) {
            printf("Phone number must be 11 digits of numbers only.\n\n");
            continue;   // 다시 입력 받기
        }

        // 여기까지 왔으면 유효한 번호
        break;
    }

    printf("Points saved for %s.\n\n", phone);
}

// 영수증 출력 + 대기번호 표시
void ask_receipt_and_print(int order_no) {
    int choice;

    while (1) {
        printf("\nDo you want a receipt?\n");
        printf("1. Yes\n");
        printf("2. No\n");

        int result = timed_read_int("Select: ", &choice,
                                    INPUT_WARN_SEC, INPUT_TIMEOUT_SEC);

        if (result == INPUT_TIMEOUT) {
            // 영수증만 스킵, 주문은 그대로 완료
            printf("\nTimeout. No receipt.\n");
            choice = 2;   // 영수증 없이 진행
            break;
        }
        if (result == INPUT_INVALID) {
            printf("Invalid input.\n");
            continue;
        }

        if (choice == 1 || choice == 2)
            break;

        printf("Please select 1 or 2.\n");
    }

    if (choice == 1) {
        printf("\n===== Receipt =====\n");
        cart_print();
        printf("Waiting number: %d\n", order_no);
        printf("===================\n");
    } else {
        printf("\nWaiting number: %d\n", order_no);
    }
}

// 장바구니 내용을 주문 로그에 기록
void log_order_items(int order_no, int method, int order_place) {
    int count = cart_get_count();

    for (int i = 0; i < count; i++) {
        const CartItem *ci = cart_get_item(i);
        if (!ci) continue;

        int line_total = ci->item.price * ci->quantity;

        // 로그 한 줄:
        // time=..., order=3, menu=Americano, qty=2, total=6600, method=CARD, place=HERE
        write_order_log(order_no,
                        ci->item.name,
                        ci->quantity,
                        line_total,
                        method,
                        order_place);
    }
}
