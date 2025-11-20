// src/order_log.c
#include <stdio.h>
#include <time.h>
#include "order_log.h"

static const char *get_method_name(int method) {
    switch (method) {
    case 1: return "Cash";
    case 2: return "Card";
    case 3: return "KakaoPay";
    default: return "Unknown";
    }
}

void write_order_log(int order_no, int total, int payment_method) {
    FILE *fp = fopen("data/logs/orders.log", "a");
    if (!fp) {
        perror("orders.log open failed");
        return;
    }

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (!t) {
        fclose(fp);
        return;
    }

    // 날짜/시간 + 주문 정보 한 줄로 기록
    fprintf(fp, "%04d-%02d-%02d %02d:%02d:%02d, ",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);

    fprintf(fp, "order=%d, method=%s, total=%d\n",
            order_no, get_method_name(payment_method), total);

    fclose(fp);
}
