// src/admin_sales_total.c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "price.h"

#define ORDERS_LOG_PATH "data/logs/orders.log"
#define MAX_DATE_STATS  365   // 최대 날짜 종류 수 (1년치 정도)

// 날짜별 통계 구조체
typedef struct {
    char date[11];     // "YYYY-MM-DD" + '\0'
    long long sales;   // 해당 날짜 총 매출
    int orders;        // 해당 날짜 주문 수
} DateStat;

static FILE *open_orders_log(void) {
    FILE *fp = fopen(ORDERS_LOG_PATH, "r");
    if (!fp) {
        printf("\n[Error] Cannot open orders log file: %s\n\n", ORDERS_LOG_PATH);
        return NULL;
    }
    return fp;
}

// line 에서 total= 뒤의 금액 읽기
static int parse_total(const char *line, int *out_total) {
    char *p_total = strstr(line, "total=");
    if (!p_total) return 0;

    int t;
    if (sscanf(p_total, "total=%d", &t) != 1) return 0;
    *out_total = t;
    return 1;
}

// line 에서 time=YYYY-MM-DD ... 중 YYYY-MM-DD 부분만 추출
static int parse_date(const char *line, char *out_date, size_t date_size) {
    char *p_time = strstr(line, "time=");
    if (!p_time) return 0;

    // 기대 형식: time=2025-11-26 13:40:01
    //             ^^^^^^^^^^ 여기 10글자만 복사
    p_time += 5; // "time=" 건너뛰기

    if (strlen(p_time) < 10 || date_size < 11) {
        return 0;
    }

    strncpy(out_date, p_time, 10);
    out_date[10] = '\0';
    return 1;
}

// 날짜 문자열 오름차순 정렬용
static int cmp_date_stat(const void *a, const void *b) {
    const DateStat *da = (const DateStat *)a;
    const DateStat *db = (const DateStat *)b;
    return strcmp(da->date, db->date);
}

// -------- 1. View total sales (by date) --------
void show_total_sales(void) {
    FILE *fp = open_orders_log();
    if (!fp) return;

    char line[512];
    DateStat stats[MAX_DATE_STATS];
    int date_count = 0;

    long long grand_total = 0;
    int grand_orders = 0;

    while (fgets(line, sizeof(line), fp)) {
        int price;
        char date[11];

        if (!parse_total(line, &price)) continue;
        if (!parse_date(line, date, sizeof(date))) continue;

        grand_total  += price;
        grand_orders += 1;

        // 이 날짜가 이미 있는지 확인
        int idx = -1;
        for (int i = 0; i < date_count; i++) {
            if (strcmp(stats[i].date, date) == 0) {
                idx = i;
                break;
            }
        }

        // 새 날짜면 추가
        if (idx == -1) {
            if (date_count >= MAX_DATE_STATS) {
                // 너무 많으면 더 이상 추가 안 함
                continue;
            }
            idx = date_count++;
            strncpy(stats[idx].date, date, sizeof(stats[idx].date));
            stats[idx].date[sizeof(stats[idx].date) - 1] = '\0';
            stats[idx].sales  = 0;
            stats[idx].orders = 0;
        }

        stats[idx].sales  += price;
        stats[idx].orders += 1;
    }
    fclose(fp);

    printf("\n=== Total Sales by Date ===\n");
    if (grand_orders == 0) {
        printf("No order logs.\n\n");
        return;
    }

    // 날짜 기준 오름차순 정렬
    qsort(stats, date_count, sizeof(DateStat), cmp_date_stat);

    printf("%-12s %10s %15s\n", "Date", "Orders", "Sales");
    printf("-------------------------------------------------\n");
    for (int i = 0; i < date_count; i++) {
        char buf[32];
        format_price_with_comma((int)stats[i].sales, buf, sizeof(buf));
        printf("%-12s %10d %15s won\n",
               stats[i].date, stats[i].orders, buf);
    }

    // 전체 합계
    char total_buf[32];
    format_price_with_comma((int)grand_total, total_buf, sizeof(total_buf));

    printf("-------------------------------------------------\n");
    printf("Total           %10d %15s won\n\n",
           grand_orders, total_buf);
}
