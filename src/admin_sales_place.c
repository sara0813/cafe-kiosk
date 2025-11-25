// src/admin_sales_place.c
#include <stdio.h>
#include <string.h>
#include "price.h"   // ★ 가격 포맷용

#define ORDERS_LOG_PATH "data/logs/orders.log"

static FILE *open_orders_log(void) {
    FILE *fp = fopen(ORDERS_LOG_PATH, "r");
    if (!fp) {
        printf("\n[Error] Cannot open orders log file: %s\n\n", ORDERS_LOG_PATH);
        return NULL;
    }
    return fp;
}

static int parse_total(const char *line, int *out_total) {
    char *p_total = strstr(line, "total=");
    if (!p_total) return 0;

    int t;
    if (sscanf(p_total, "total=%d", &t) != 1) return 0;
    *out_total = t;
    return 1;
}

static int parse_place(const char *line, char *out, size_t out_size) {
    char *p_place = strstr(line, "place=");
    if (!p_place) return 0;
    if (sscanf(p_place, "place=%15[^,\n]", out) != 1) return 0;
    out[out_size - 1] = '\0';
    return 1;
}

// -------- 5. Dine-in vs Takeout statistics --------
void show_place_stats(void) {
    FILE *fp = open_orders_log();
    if (!fp) return;

    char line[512];
    char place[16];
    int price;

    long long sum_here = 0, sum_takeout = 0;
    int cnt_here = 0, cnt_takeout = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (!parse_total(line, &price)) continue;
        if (!parse_place(line, place, sizeof(place))) continue;

        if (strcmp(place, "HERE") == 0) {
            sum_here += price;
            cnt_here++;
        } else if (strcmp(place, "TAKEOUT") == 0 ||
                   strcmp(place, "TOGO") == 0 ||
                   strcmp(place, "PACK") == 0) {
            sum_takeout += price;
            cnt_takeout++;
        }
    }
    fclose(fp);

    printf("\n=== Dine-in vs Takeout Sales ===\n");
    if (cnt_here == 0 && cnt_takeout == 0) {
        printf("No order logs.\n\n");
        return;
    }

    char here_buf[32], takeout_buf[32];
    // long long → int 캐스팅 (금액이 int 범위 안이라고 가정)
    format_price_with_comma((int)sum_here,    here_buf,    sizeof(here_buf));
    format_price_with_comma((int)sum_takeout, takeout_buf, sizeof(takeout_buf));

    printf("  [Dine-in]  %4d orders, %10s won\n", cnt_here,    here_buf);
    printf("  [Takeout]  %4d orders, %10s won\n\n", cnt_takeout, takeout_buf);
}
