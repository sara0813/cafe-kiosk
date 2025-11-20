// src/payment.c
#include <stdio.h>
#include "cart.h"
#include "payment.h"
#include "order_log.h" 
#include "input.h"

// 결제 수단 선택
// return: 1=Cash, 2=Card, 3=KakaoPay, 0=Cancel
static int select_payment_method(void) {
    int method;

    while (1) {
        printf("Select payment method:\n");
        printf("1. Cash\n");
        printf("2. Card\n");
        printf("3. KakaoPay\n");
        printf("0. Cancel\n");

        int result = timed_read_int("Select: ", &method,
                                    INPUT_WARN_SEC, INPUT_TIMEOUT_SEC);

        if (result == INPUT_TIMEOUT) {
            printf("\nTimeout. Payment cancelled.\n\n");
            return 0;  // 0 = cancel
        }
        if (result == INPUT_INVALID) {
            printf("Invalid input.\n\n");
            continue;
        }

        if (method >= 0 && method <= 3)
            return method;

        printf("Please select 0~3.\n\n");
    }
}

// return: 1 = save, 0 = not save
static int ask_point(void) {
    int choice;

    while (1) {
        printf("\nDo you want to save points?\n");
        printf("1. Yes\n");
        printf("2. No\n");

        int result = timed_read_int("Select: ", &choice,
                                    INPUT_WARN_SEC, INPUT_TIMEOUT_SEC);

        if (result == INPUT_TIMEOUT) {
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


// 전화번호 입력 (숫자만 11자리)
static void save_point(void) {
    char phone[32];

    while (1) {
        printf("Enter your phone number (11 digits): ");

        int len = 0;
        int ok = 1;
        for (const char *p = phone; *p; ++p) {
            if (*p < '0' || *p > '9') { // 숫자만
                ok = 0;
                break;
            }
            len++;
        }

        if (!ok || len != 11) {
            printf("Phone number must be 11 digits of numbers only.\n\n");
            continue;
        }

        break; // 유효한 번호
    }

    printf("Points saved for %s.\n\n", phone);
}

// 영수증 / 대기번호
static int next_order_no = 1;

static void ask_receipt_and_print(int order_no) {
    int choice;

    while (1) {
        printf("\nDo you want a receipt?\n");
        printf("1. Yes\n");
        printf("2. No\n");

        int result = timed_read_int("Select: ", &choice,
                                    INPUT_WARN_SEC, INPUT_TIMEOUT_SEC);

        if (result == INPUT_TIMEOUT) {
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


// 결제 전체 플로우
// 결제 완료: 1, 취소: 0
int run_payment_flow(void) {
    if (cart_is_empty()) {
        printf("Cart is empty. Nothing to pay.\n\n");
        return 0;
    }

    int total = cart_total_price();

    printf("==== Payment ====\n");
    printf("Total amount: %d won\n\n", total);

    // 1) 결제 수단 선택
    int method = select_payment_method();
    if (method == 0) {
        printf("Payment cancelled.\n\n");
        return 0;   // 이전 화면으로
    }

    // 2) 실제 결제 처리
    switch (method) {
    case 1:
        printf("\n[Cash]\n");
        printf("Please pay %d won in cash.\n", total);
        printf("Cash payment complete.\n");
        break;
    case 2:
        printf("\n[Card]\n");
        printf("Please tap or insert your card (%d won).\n", total);
        printf("Card payment approved.\n");
        break;
    case 3:
        printf("\n[KakaoPay]\n");
        printf("Please complete KakaoPay on your phone (%d won).\n", total);
        printf("KakaoPay payment complete.\n");
        break;
    default:
        printf("Invalid payment method.\n\n");
        return 0;
    }

    int order_no = next_order_no++;

    write_order_log(order_no, total, method);

    // 3) 포인트 적립 (결제 후)
    if (ask_point()) {
        save_point();
    }

    // 4) 영수증 + 대기번호
    ask_receipt_and_print(order_no);

    // 5) 결제 완료 후 장바구니 비우기
    cart_init();

    printf("\nYour order has been completed.\n\n");
    return 1;
}
