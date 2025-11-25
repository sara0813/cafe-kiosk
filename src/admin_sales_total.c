// src/admin_sales_total.c
#include <stdio.h>
#include <string.h>

#define ORDERS_LOG_PATH "data/logs/orders.log"

// 이 파일 안에서만 쓰는 공통 함수들
static FILE *open_orders_log(void) {
    FILE *fp = fopen(ORDERS_LOG_PATH, "r");
    if (!fp) {
        printf("\n[Error] Cannot open orders log file: %s\n\n", ORDERS_LOG_PATH);
        return NULL;
    }
    return fp;
}

// line 에서 "total=" 뒤의 정수 읽기
static int parse_total(const char *line, int *out_total) {
    char *p_total = strstr(line, "total=");
    if (!p_total) return 0;

    int t;
    if (sscanf(p_total, "total=%d", &t) != 1) return 0;
    *out_total = t;
    return 1;
}

// -------- 1. View total sales --------
void show_total_sales(void) {
    FILE *fp = open_orders_log();
    if (!fp) return;

    char line[512];
    long long total_sales = 0;
    int order_count = 0;
    int price;

    while (fgets(line, sizeof(line), fp)) {
        if (parse_total(line, &price)) {
            total_sales += price;
            order_count++;
        }
    }
    fclose(fp);

    printf("\n=== Total Sales ===\n");
    printf("Total number of orders : %d\n", order_count);
    printf("Total sales amount     : %lld won\n\n", total_sales);
}
