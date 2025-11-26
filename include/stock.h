// include/stock.h
#ifndef STOCK_H
#define STOCK_H

#include "menu.h"

int stock_load_category(int cat, int *stock, int count);

int stock_save_category(int cat, const MenuItem *items, const int *stock, int count);

#endif
