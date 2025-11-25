// src/admin_sales_payment.c
#include <stdio.h>
#include <string.h>
#include "price.h"

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

static int parse_method(const char *line, char *out, size_t out_size) {
    char *p_method = strstr(line, "method=");
    if (!p_method) return 0;
    if (sscanf(p_method, "method=%15[^,]", out) != 1) return 0;
    out[out_size - 1] = '\0';
    return 1;
}

// -------- 2. Sales by payment method --------
void show_sales_by_payment(void) {
    FILE *fp = open_orders_log();
    if (!fp) return;

    char line[512];
    char method[16];
    int price;

    long long sum_cash = 0, sum_card = 0, sum_kakao = 0;
    int cnt_cash = 0, cnt_card = 0, cnt_kakao = 0;
    long long total_sum = 0;
    int total_cnt = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (!parse_total(line, &price)) continue;
        if (!parse_method(line, method, sizeof(method))) continue;

        total_sum += price;
        total_cnt++;

        if (strcmp(method, "CASH") == 0) {
            sum_cash += price;
            cnt_cash++;
        } else if (strcmp(method, "CARD") == 0) {
            sum_card += price;
            cnt_card++;
        } else if (strcmp(method, "KAKAOPAY") == 0 || strcmp(method, "KAKAO") == 0) {
            sum_kakao += price;
            cnt_kakao++;
        }
    }
    fclose(fp);

    char total_buf[32], cash_buf[32], card_buf[32], kakao_buf[32];
    format_price_with_comma((int)total_sum, total_buf, sizeof(total_buf));
    format_price_with_comma((int)sum_cash,  cash_buf,  sizeof(cash_buf));
    format_price_with_comma((int)sum_card,  card_buf,  sizeof(card_buf));
    format_price_with_comma((int)sum_kakao, kakao_buf, sizeof(kakao_buf));

    printf("\n=== Sales by Payment Method ===\n");
    printf("Total orders : %d, Total sales : %s won\n\n", total_cnt, total_buf);
    printf("  [Cash]     %4d orders, %10s won\n", cnt_cash,  cash_buf);
    printf("  [Card]     %4d orders, %10s won\n", cnt_card,  card_buf);
    printf("  [KakaoPay] %4d orders, %10s won\n\n", cnt_kakao, kakao_buf);
}
