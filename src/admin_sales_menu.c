// src/admin_sales_menu.c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "price.h"   // ★ 가격에 콤마 찍기

#define ORDERS_LOG_PATH "data/logs/orders.log"
#define MAX_MENU_STATS  256

typedef struct {
    char name[64];
    int  qty;           // 총 판매 수량
    long long sales;    // 총 매출액
} MenuStat;

static FILE *open_orders_log(void) {
    FILE *fp = fopen(ORDERS_LOG_PATH, "r");
    if (!fp) {
        printf("\n[Error] Cannot open orders log file: %s\n\n", ORDERS_LOG_PATH);
        return NULL;
    }
    return fp;
}

// line 에서 menu, qty, total 파싱
static int parse_menu_stat_line(const char *line,
                                char *out_name, size_t name_size,
                                int *out_qty, int *out_total)
{
    char *p_menu  = strstr(line, "menu=");
    char *p_qty   = strstr(line, "qty=");
    char *p_total = strstr(line, "total=");
    if (!p_menu || !p_qty || !p_total) return 0;

    if (sscanf(p_menu,  "menu=%63[^,]", out_name) != 1) return 0;
    if (sscanf(p_qty,   "qty=%d",        out_qty) != 1) return 0;

    int total;
    if (sscanf(p_total, "total=%d", &total)       != 1) return 0;

    *out_total = total;
    out_name[name_size - 1] = '\0';
    return 1;
}

// 로그 전체를 읽어서 메뉴별 통계 수집
static int collect_menu_stats(MenuStat *arr, int *out_count) {
    FILE *fp = open_orders_log();
    if (!fp) return 0;

    char line[512];
    char name[64];
    int qty, total;
    int count = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (!parse_menu_stat_line(line, name, sizeof(name), &qty, &total))
            continue;

        int idx = -1;
        for (int i = 0; i < count; i++) {
            if (strcmp(arr[i].name, name) == 0) {
                idx = i;
                break;
            }
        }

        if (idx == -1) {
            if (count >= MAX_MENU_STATS) {
                // 너무 많으면 더 이상 추가하지 않음
                continue;
            }
            idx = count++;
            strncpy(arr[idx].name, name, sizeof(arr[idx].name));
            arr[idx].name[sizeof(arr[idx].name) - 1] = '\0';
            arr[idx].qty = 0;
            arr[idx].sales = 0;
        }

        arr[idx].qty   += qty;
        arr[idx].sales += total;
    }

    fclose(fp);
    *out_count = count;
    return 1;
}

// -------- 3. Sales by menu item --------
void show_sales_by_menu(void) {
    MenuStat stats[MAX_MENU_STATS];
    int count = 0;

    if (!collect_menu_stats(stats, &count)) {
        return;
    }

    printf("\n=== Sales by Menu Item ===\n");
    if (count == 0) {
        printf("No order logs.\n\n");
        return;
    }

    printf("%-30s %8s %15s\n", "Menu name", "Quantity", "Sales");
    printf("--------------------------------------------------------------\n");
    for (int i = 0; i < count; i++) {
        char buf[32];
        // long long → int 캐스팅 (가격 범위는 int 안이라고 가정)
        format_price_with_comma((int)stats[i].sales, buf, sizeof(buf));

        printf("%-30s %8d %15s won\n",
               stats[i].name, stats[i].qty, buf);
    }
    printf("\n");
}

// 정렬 함수: 수량 내림차순, 같으면 매출 내림차순
static int cmp_menu_qty_desc(const void *a, const void *b) {
    const MenuStat *ma = (const MenuStat *)a;
    const MenuStat *mb = (const MenuStat *)b;

    if (mb->qty != ma->qty)
        return (mb->qty - ma->qty);

    if (mb->sales > ma->sales) return 1;
    if (mb->sales < ma->sales) return -1;
    return 0;
}

// -------- 4. Top 5 best-selling menus --------
void show_top5_menus(void) {
    MenuStat stats[MAX_MENU_STATS];
    int count = 0;

    if (!collect_menu_stats(stats, &count)) {
        return;
    }

    printf("\n=== Top 5 Best-Selling Menus (by quantity) ===\n");
    if (count == 0) {
        printf("No order logs.\n\n");
        return;
    }

    qsort(stats, count, sizeof(MenuStat), cmp_menu_qty_desc);

    int top = (count < 5) ? count : 5;
    printf("%-4s %-30s %8s %15s\n", "Rank", "Menu name", "Quantity", "Sales");
    printf("--------------------------------------------------------------\n");
    for (int i = 0; i < top; i++) {
        char buf[32];
        format_price_with_comma((int)stats[i].sales, buf, sizeof(buf));

        printf("%2d   %-30s %8d %15s won\n",
               i + 1, stats[i].name, stats[i].qty, buf);
    }
    printf("\n");
}
