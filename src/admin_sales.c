// src/admin_sales.c
#include <stdio.h>
#include <string.h>

#define ORDERS_LOG_PATH "data/logs/orders.log"

static void flush_input(void) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
        ;
}

static void show_sales_by_payment(void) {
    FILE *fp = fopen(ORDERS_LOG_PATH, "r");
    if (!fp) {
        printf("Cannot open %s\n\n", ORDERS_LOG_PATH);
        return;
    }

    long long total_sum = 0;
    int total_cnt = 0;

    long long sum_cash = 0, sum_card = 0, sum_kakao = 0;
    int cnt_cash = 0, cnt_card = 0, cnt_kakao = 0;

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        char method[16];
        int price = 0;

        char *p_method = strstr(line, "method=");
        char *p_total  = strstr(line, "total=");
        if (!p_method || !p_total)
            continue;

        if (sscanf(p_method, "method=%15[^,]", method) != 1)
            continue;
        if (sscanf(p_total, "total=%d", &price) != 1)
            continue;

        total_sum += price;
        total_cnt++;

        if (strcmp(method, "Cash") == 0) {
            sum_cash += price;
            cnt_cash++;
        } else if (strcmp(method, "Card") == 0) {
            sum_card += price;
            cnt_card++;
        } else if (strcmp(method, "KakaoPay") == 0) {
            sum_kakao += price;
            cnt_kakao++;
        }
    }

    fclose(fp);

    printf("\n=== Sales by payment method ===\n");
    printf("Total orders : %d\n", total_cnt);
    printf("Total sales  : %lld won\n\n", total_sum);

    printf("Cash    : %d orders, %lld won\n",  cnt_cash,  sum_cash);
    printf("Card    : %d orders, %lld won\n",  cnt_card,  sum_card);
    printf("KakaoPay: %d orders, %lld won\n",  cnt_kakao, sum_kakao);
    printf("================================\n\n");
}

static void show_sales_by_menu(void) {
    printf("\n[Sales by menu] is not implemented yet.\n");
    printf("You can extend orders.log to include menu info later.\n\n");
}

static void show_order_logs(void) {
    FILE *fp = fopen(ORDERS_LOG_PATH, "r");
    if (!fp) {
        printf("Cannot open %s\n\n", ORDERS_LOG_PATH);
        return;
    }

    char line[256];
    printf("\n=== Order logs ===\n");
    while (fgets(line, sizeof(line), fp)) {
        fputs(line, stdout);
    }
    printf("===================\n\n");

    fclose(fp);
}

// Called from admin_main.c
void run_sales_menu(void) {
    int choice;

    while (1) {
        printf("=== Sales management ===\n");
        printf("1. Sales by payment method\n");
        printf("2. Sales by menu (TODO)\n");
        printf("3. View raw order logs\n");
        printf("0. Back\n");
        printf("Select: ");

        if (scanf("%d", &choice) != 1) {
            flush_input();
            printf("Invalid input.\n\n");
            continue;
        }

        switch (choice) {
        case 0:
            printf("\n");
            return;
        case 1:
            show_sales_by_payment();
            break;
        case 2:
            show_sales_by_menu();
            break;
        case 3:
            show_order_logs();
            break;
        default:
            printf("No such menu.\n\n");
            break;
        }
    }
}
