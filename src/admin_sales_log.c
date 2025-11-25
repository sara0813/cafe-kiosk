// src/admin_sales_log.c
#include <stdio.h>

#define ORDERS_LOG_PATH "data/logs/orders.log"

static FILE *open_orders_log(void) {
    FILE *fp = fopen(ORDERS_LOG_PATH, "r");
    if (!fp) {
        printf("\n[Error] Cannot open orders log file: %s\n\n", ORDERS_LOG_PATH);
        return NULL;
    }
    return fp;
}

// -------- 6. View order log --------
void show_order_log(void) {
    FILE *fp = open_orders_log();
    if (!fp) return;

    printf("\n=== Order Log ===\n");

    char line[512];
    int line_no = 1;
    while (fgets(line, sizeof(line), fp)) {
        printf("%4d: %s", line_no++, line);
    }
    fclose(fp);

    if (line_no == 1) {
        printf("No orders recorded in the log.\n");
    }
    printf("\n");
}
