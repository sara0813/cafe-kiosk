// src/payment.c
#include <stdio.h>
#include "cart.h"
#include "payment.h"

static void flush_input(void) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
        ;
}

void run_payment_flow(void) {
    if (cart_is_empty()) {
        printf("Cart is empty. Nothing to pay.\n\n");
        return;
    }

    int total = cart_total_price();

    printf("==== Payment ====\n");
    printf("Total amount: %d won\n", total);
    printf("Select payment method:\n");
    printf("1. Cash\n");
    printf("2. Card\n");
    printf("3. KakaoPay\n");
    printf("0. Cancel\n");
    printf("-----------------\n");
    printf("Select: ");

    int method;
    if (scanf("%d", &method) != 1) {
        flush_input();
        printf("Invalid input.\n\n");
        return;
    }

    if (method == 0) {
        printf("Payment cancelled.\n\n");
        return;
    }

    const char *method_name = NULL;
    switch (method) {
    case 1: method_name = "Cash"; break;
    case 2: method_name = "Card"; break;
    case 3: method_name = "KakaoPay"; break;
    default:
        printf("No such payment method.\n\n");
        return;
    }

    printf("\nPaid %d won with %s.\n", total, method_name);
    printf("Order complete! Thank you.\n\n");

    cart_init();  // 결제 끝났으니 장바구니 비우기
}
