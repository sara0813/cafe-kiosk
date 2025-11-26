// src/payment.c
#include <stdio.h>
#include "cart.h"
#include "payment.h"
#include "payment_extra.h"
#include "input.h"

#define ORDER_NO_PATH "data/logs/order_no.txt"

// 주문번호 관리용
static int next_order_no = 0;      // 0이면 아직 로드 안 한 상태
static int order_no_loaded = 0;    // 한번만 읽게 플래그

static void load_order_no(void) {
    if (order_no_loaded) return;   // 이미 로드했으면 패스

    FILE *fp = fopen(ORDER_NO_PATH, "r");
    if (fp) {
        int n = 0;
        if (fscanf(fp, "%d", &n) == 1 && n > 0) {
            next_order_no = n;
        }
        fclose(fp);
    }

    if (next_order_no <= 0) {
        // 파일이 없거나 이상한 값이면 1부터 시작
        next_order_no = 1;
    }

    order_no_loaded = 1;
}

static void save_order_no(void) {
    FILE *fp = fopen(ORDER_NO_PATH, "w");
    if (!fp) {
        // 저장 실패해도 프로그램 흐름은 계속 진행
        return;
    }
    fprintf(fp, "%d\n", next_order_no);
    fclose(fp);
}

// 결제 수단 선택
// return: 1=Cash, 2=Card, 3=KakaoPay, 0=Cancel, -1=Timeout
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
            // ★ 유저 모드 전체를 메인으로 돌려보낼 타임아웃
            printf("\nTimeout. Back to main menu.\n\n");
            return -1;  // timeout
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

// 결제 전체 플로우
// order_place: 1 = 매장, 2 = 포장
// return: 1 = 결제 완료(또는 타임아웃으로 메인으로 복귀), 0 = 결제 취소(이전 화면으로)
int run_payment_flow(int order_place) {
    if (cart_is_empty()) {
        printf("Cart is empty. Nothing to pay.\n\n");
        return 0;
    }

    int total = cart_total_price();

    printf("==== Payment ====\n");
    printf("Total amount: %d won\n\n", total);

    // 1) 결제 수단 선택
    int method = select_payment_method();
    if (method < 0) {
        // ★ 타임아웃: 주문 전체 취소 후 메인으로 나갈 것
        printf("Payment timeout. Order cancelled and back to main menu.\n\n");
        cart_init();   // 주문 비우기
        return 1;      // run_user_mode에서 if (paid) return; → 메인으로
    }
    if (method == 0) {
        printf("Payment cancelled.\n\n");
        return 0;   // 이전 화면(장바구니)으로
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

    // 주문번호 로드 (프로그램 첫 결제 시 한 번만)
    load_order_no();

    int order_no = next_order_no++;

    // 다음에 쓸 번호를 파일에 저장
    save_order_no();

    // 2-1) 주문 로그 기록 (장바구니 항목별 한 줄씩)
    log_order_items(order_no, method, order_place);

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
