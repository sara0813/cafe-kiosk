// src/admin_sales.c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "input.h"

#define ORDERS_LOG_PATH "data/logs/orders.log"
#define MAX_MENU_STATS  256

// 주문 로그 한 줄의 형식 예시:
// time=2025-11-21 13:40:01, order=3, menu=아메리카노, qty=2, total=6000, method=CARD, place=HERE

typedef struct {
    char name[64];
    int  qty;           // 총 판매 수량
    long long sales;    // 총 매출액
} MenuStat;

// ---------- 공통 파싱 헬퍼 ----------

// line 에서 "total=" 뒤의 정수 읽기 (못 읽으면 0 리턴)
static int parse_total(const char *line, int *out_total) {
    char *p_total = strstr(line, "total=");
    if (!p_total) return 0;
    int t;
    if (sscanf(p_total, "total=%d", &t) != 1) return 0;
    *out_total = t;
    return 1;
}

// line 에서 "method=" 뒤의 문자열 읽기 (예: CASH/CARD/KAKAOPAY)
static int parse_method(const char *line, char *buf, size_t size) {
    char *p_method = strstr(line, "method=");
    if (!p_method) return 0;
    if (sscanf(p_method, "method=%15[^, \n]", buf) != 1) return 0;
    buf[size - 1] = '\0';
    return 1;
}

// line 에서 "place=" 뒤의 문자열 읽기 (HERE/TOGO)
static int parse_place(const char *line, char *buf, size_t size) {
    char *p_place = strstr(line, "place=");
    if (!p_place) return 0;
    if (sscanf(p_place, "place=%15[^, \n]", buf) != 1) return 0;
    buf[size - 1] = '\0';
    return 1;
}

// line 에서 "menu=", "qty=" 파싱
static int parse_menu_qty(const char *line, char *menu, size_t menu_size,
                          int *out_qty)
{
    char *p_menu = strstr(line, "menu=");
    char *p_qty  = strstr(line, "qty=");
    if (!p_menu || !p_qty) return 0;

    if (sscanf(p_menu, "menu=%63[^,]", menu) != 1) return 0;
    menu[menu_size - 1] = '\0';

    int q;
    if (sscanf(p_qty, "qty=%d", &q) != 1) return 0;
    *out_qty = q;
    return 1;
}

// ---------- 메뉴별 통계 빌드 ----------

static int find_or_add_menu(MenuStat stats[], int *p_count,
                            const char *name)
{
    int count = *p_count;

    for (int i = 0; i < count; i++) {
        if (strcmp(stats[i].name, name) == 0) {
            return i;
        }
    }
    if (count >= MAX_MENU_STATS) {
        return -1;  // 더 못 넣음
    }

    strncpy(stats[count].name, name, sizeof(stats[count].name) - 1);
    stats[count].name[sizeof(stats[count].name) - 1] = '\0';
    stats[count].qty   = 0;
    stats[count].sales = 0;
    *p_count = count + 1;
    return count;
}

// orders.log 를 읽어서 메뉴별 통계를 채운다.
// (menu_stats 배열과 menu_count 세팅)
static void build_menu_stats(MenuStat menu_stats[], int *p_count) {
    *p_count = 0;

    FILE *fp = fopen(ORDERS_LOG_PATH, "r");
    if (!fp) {
        printf("Cannot open %s\n\n", ORDERS_LOG_PATH);
        return;
    }

    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        char menu[64];
        int  qty;
        int  total;

        if (!parse_menu_qty(line, menu, sizeof(menu), &qty))
            continue;
        if (!parse_total(line, &total))
            continue;

        int idx = find_or_add_menu(menu_stats, p_count, menu);
        if (idx < 0) continue;

        menu_stats[idx].qty   += qty;
        menu_stats[idx].sales += total;
    }

    fclose(fp);
}

// ---------- 1. 총 매출액 보기 ----------

static void show_total_sales(void) {
    FILE *fp = fopen(ORDERS_LOG_PATH, "r");
    if (!fp) {
        printf("Cannot open %s\n\n", ORDERS_LOG_PATH);
        return;
    }

    long long total_sum = 0;
    int       total_cnt = 0;

    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        int t;
        if (!parse_total(line, &t))
            continue;
        total_sum += t;
        total_cnt++;
    }

    fclose(fp);

    printf("\n=== Total Sales ===\n");
    printf("Total orders : %d\n", total_cnt);
    printf("Total amount : %lld won\n", total_sum);
    printf("===================\n\n");
}

// ---------- 2. 결제수단별 매출 ----------

static void show_sales_by_payment(void) {
    FILE *fp = fopen(ORDERS_LOG_PATH, "r");
    if (!fp) {
        printf("Cannot open %s\n\n", ORDERS_LOG_PATH);
        return;
    }

    long long sum_cash = 0, sum_card = 0, sum_kakao = 0;
    int       cnt_cash = 0, cnt_card = 0, cnt_kakao = 0;

    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        char method[16];
        int  t;

        if (!parse_method(line, method, sizeof(method)))
            continue;
        if (!parse_total(line, &t))
            continue;

        if (strcmp(method, "CASH") == 0) {
            sum_cash += t;
            cnt_cash++;
        } else if (strcmp(method, "CARD") == 0) {
            sum_card += t;
            cnt_card++;
        } else if (strcmp(method, "KAKAOPAY") == 0) {
            sum_kakao += t;
            cnt_kakao++;
        }
    }

    fclose(fp);

    printf("\n=== Sales by Payment Method ===\n");
    printf("CASH    : %4d orders, %10lld won\n", cnt_cash,  sum_cash);
    printf("CARD    : %4d orders, %10lld won\n", cnt_card,  sum_card);
    printf("KAKAOPAY: %4d orders, %10lld won\n", cnt_kakao, sum_kakao);
    printf("================================\n\n");
}

// ---------- 3. 메뉴별 판매량 ----------

static void show_sales_by_menu(void) {
    MenuStat stats[MAX_MENU_STATS];
    int count = 0;

    build_menu_stats(stats, &count);

    if (count == 0) {
        printf("\nNo menu sales data.\n\n");
        return;
    }

    printf("\n=== Sales by Menu ===\n");
    for (int i = 0; i < count; i++) {
        printf("%-20s : %4d cups, %8lld won\n",
               stats[i].name, stats[i].qty, stats[i].sales);
    }
    printf("======================\n\n");
}

// ---------- 4. TOP 5 잘 팔린 메뉴 ----------

static int compare_menu_stat_desc(const void *a, const void *b) {
    const MenuStat *ma = (const MenuStat *)a;
    const MenuStat *mb = (const MenuStat *)b;

    if (ma->qty < mb->qty) return 1;
    if (ma->qty > mb->qty) return -1;
    return 0;
}

static void show_top5_menu(void) {
    MenuStat stats[MAX_MENU_STATS];
    int count = 0;

    build_menu_stats(stats, &count);

    if (count == 0) {
        printf("\nNo menu sales data.\n\n");
        return;
    }

    // 많이 팔린 순으로 정렬
    qsort(stats, count, sizeof(MenuStat), compare_menu_stat_desc);

    int n = (count < 5) ? count : 5;

    printf("\n=== TOP %d Best-selling Menus ===\n", n);
    for (int i = 0; i < n; i++) {
        printf("%d) %-20s : %4d cups, %8lld won\n",
               i + 1, stats[i].name, stats[i].qty, stats[i].sales);
    }
    printf("=================================\n\n");
}

// ---------- 5. 매장 vs 포장 매출 통계 ----------

static void show_sales_by_place(void) {
    FILE *fp = fopen(ORDERS_LOG_PATH, "r");
    if (!fp) {
        printf("Cannot open %s\n\n", ORDERS_LOG_PATH);
        return;
    }

    long long sum_here = 0, sum_togo = 0;
    int       cnt_here = 0, cnt_togo = 0;
    char line[512];

    while (fgets(line, sizeof(line), fp)) {
        char place[16];
        int  t;

        if (!parse_place(line, place, sizeof(place)))
            continue;
        if (!parse_total(line, &t))
            continue;

        if (strcmp(place, "HERE") == 0) {
            sum_here += t;
            cnt_here++;
        } else if (strcmp(place, "TOGO") == 0) {
            sum_togo += t;
            cnt_togo++;
        }
    }

    fclose(fp);

    printf("\n=== Sales by Place (Here vs To-go) ===\n");
    printf("Here (매장) : %4d orders, %10lld won\n", cnt_here, sum_here);
    printf("To-go(포장): %4d orders, %10lld won\n", cnt_togo, sum_togo);
    printf("======================================\n\n");
}

// ---------- 6. 주문 로그 원본 보기 ----------

static void show_order_log_raw(void) {
    FILE *fp = fopen(ORDERS_LOG_PATH, "r");
    if (!fp) {
        printf("Cannot open %s\n\n", ORDERS_LOG_PATH);
        return;
    }

    printf("\n===== Order Log (raw) =====\n");

    char line[512];
    int line_no = 0;
    while (fgets(line, sizeof(line), fp)) {
        line_no++;
        printf("%3d) %s", line_no, line);
        if (line[strlen(line) - 1] != '\n')
            printf("\n");
    }
    if (line_no == 0) {
        printf("(No orders yet)\n");
    }

    printf("===========================\n\n");
    fclose(fp);
}

// ---------- 외부에서 부르는 진입점 ----------

void run_sales_menu(void) {
    int choice, result;

    while (1) {
        printf("=== Sales / Statistics ===\n");
        printf("1. 총 매출액 보기\n");
        printf("2. 결제수단별 매출\n");
        printf("3. 메뉴별 판매량\n");
        printf("4. TOP 5 잘 팔린 메뉴\n");
        printf("5. 매장 vs 포장 매출 통계\n");
        printf("6. 주문 로그 보기\n");
        printf("0. 뒤로\n");

        result = timed_read_int("Select: ", &choice,
                                INPUT_WARN_SEC, INPUT_TIMEOUT_SEC);

        if (result == INPUT_TIMEOUT) {
            printf("\nTimeout. Back to admin menu.\n\n");
            return;
        }
        if (result == INPUT_INVALID) {
            printf("Invalid input.\n\n");
            continue;
        }

        switch (choice) {
        case 0:
            printf("\nBack to admin menu.\n\n");
            return;
        case 1:
            show_total_sales();
            break;
        case 2:
            show_sales_by_payment();
            break;
        case 3:
            show_sales_by_menu();
            break;
        case 4:
            show_top5_menu();
            break;
        case 5:
            show_sales_by_place();
            break;
        case 6:
            show_order_log_raw();
            break;
        default:
            printf("No such menu.\n\n");
            break;
        }
    }
}
