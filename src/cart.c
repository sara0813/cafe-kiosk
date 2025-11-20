// src/cart.c
#include <stdio.h>
#include "cart.h"

static CartItem cart[MAX_CART_ITEMS];
static int cart_count = 0;

void cart_init(void) {
    cart_count = 0;
}

void cart_add(const MenuItem *item, int qty) {
    if (!item || qty <= 0) return;

    // 이미 있으면 수량만 증가
    for (int i = 0; i < cart_count; i++) {
        if (cart[i].item.id == item->id) {
            cart[i].quantity += qty;
            return;
        }
    }

    if (cart_count >= MAX_CART_ITEMS) {
        printf("Cart is full.\n\n");
        return;
    }

    cart[cart_count].item = *item;
    cart[cart_count].quantity = qty;
    cart_count++;
}

int cart_is_empty(void) {
    return cart_count == 0;
}

int cart_total_price(void) {
    int total = 0;
    for (int i = 0; i < cart_count; i++) {
        total += cart[i].item.price * cart[i].quantity;
    }
    return total;
}

void cart_print(void) {
    if (cart_is_empty()) {
        printf("Cart is empty.\n\n");
        return;
    }

    printf("==== Cart ====\n");
    for (int i = 0; i < cart_count; i++) {
        int line = cart[i].item.price * cart[i].quantity;
        printf("%2d. %s x%d  (%d won)\n",
               i + 1, cart[i].item.name, cart[i].quantity, line);
    }
    printf("Total: %d won\n", cart_total_price());
    printf("==============\n\n");
}
