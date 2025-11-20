// src/cart.c
#include <stdio.h>
#include "cart.h"
#include "price.h"


static CartItem cart[MAX_CART_ITEMS];
static int cart_count = 0;

#include <stdio.h>

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

        char price_str[32];
        format_price_with_comma(line, price_str, sizeof(price_str));

        printf("%2d. %s x%d  (%s won)\n",
               i + 1, cart[i].item.name, cart[i].quantity, price_str);
    }

    int total = cart_total_price();
    char total_str[32];
    format_price_with_comma(total, total_str, sizeof(total_str));

    printf("Total: %s won\n", total_str);
    printf("==============\n\n");
}
