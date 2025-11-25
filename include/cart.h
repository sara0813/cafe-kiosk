// include/cart.h
#ifndef CART_H
#define CART_H

#include "menu.h"

typedef struct {
    MenuItem item;
    int quantity;
} CartItem;

#define MAX_CART_ITEMS 100

void cart_init(void);
void cart_add(const MenuItem *item, int qty);
int  cart_is_empty(void);
int  cart_total_price(void);
void cart_print(void);

int  cart_get_count(void);
const CartItem *cart_get_item(int index);

#endif
