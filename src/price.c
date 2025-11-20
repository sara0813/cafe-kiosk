// src/price.c
#include <stdio.h>
#include "price.h"

// 1234567 -> "1,234,567"
void format_price_with_comma(int price, char *out, size_t out_size) {
    char tmp[32];
    int len = snprintf(tmp, sizeof(tmp), "%d", price);

    if (out_size == 0) return;

    if (len <= 0) {
        out[0] = '\0';
        return;
    }

    int comma_count = (len - 1) / 3;
    int total_len = len + comma_count;

    if (total_len + 1 > (int)out_size) {
        total_len = (int)out_size - 1;
    }

    out[total_len] = '\0';

    int src = len - 1;
    int dst = total_len - 1;
    int group = 0;

    while (src >= 0 && dst >= 0) {
        out[dst--] = tmp[src--];
        group++;

        if (group == 3 && src >= 0 && dst >= 0) {
            out[dst--] = ',';
            group = 0;
        }
    }
}
