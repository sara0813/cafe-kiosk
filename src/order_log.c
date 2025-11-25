// src/order_log.c

#include <stdio.h>
#include <time.h>
#include "order_log.h"

#define ORDERS_LOG_PATH "data/logs/orders.log"

// 결제 수단 번호 -> 로그용 문자열
static const char *get_method_name(int method) {
    switch (method) {
    case 1: return "CASH";
    case 2: return "CARD";
    case 3: return "KAKAOPAY";
    default: return "UNKNOWN";
    }
}

// 주문 장소 번호 -> 로그용 문자열
static const char *get_place_name(int order_place) {
    switch (order_place) {
    case 1: return "HERE";      // 매장
    case 2: return "TOGO";      // 포장
    default: return "UNKNOWN";
    }
}


int write_order_log(int order_no,
                    const char *menu_name,
                    int qty,
                    int line_total,
                    int method,
                    int order_place)
{
    FILE *fp = fopen(ORDERS_LOG_PATH, "a");
    if (!fp) {
        // 파일 열기 실패
        return 0;
    }

    const char *method_str = get_method_name(method);
    const char *place_str  = get_place_name(order_place);

    // 현재 시간 문자열 만들기
    time_t now = time(NULL);
    struct tm *tmv = localtime(&now);

    char tbuf[32];
    if (tmv) {
        strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M:%S", tmv);
    } else {
        // localtime 실패 시 대비
        snprintf(tbuf, sizeof(tbuf), "unknown-time");
    }

    fprintf(fp,
        "time=%s, order=%d, menu=%s, qty=%d, total=%d, method=%s, place=%s\n",
        tbuf, order_no, menu_name, qty, line_total, method_str, place_str);

    fclose(fp);
    return 1;
}
