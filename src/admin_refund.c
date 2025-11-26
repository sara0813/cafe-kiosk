// src/admin_refund.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "input.h"

// 로그 파일 경로 (admin_sales.c, order_log.c 에서 쓰던 것과 동일)
#define ORDERS_LOG_PATH   "data/logs/orders.log"
#define REFUNDS_LOG_PATH  "data/logs/refunds.log"

typedef struct {
    int  order_no;
    char time_str[32];
    char menu[64];
    int  qty;
    int  total;
    char method[16];
    char place[16];
} OrderLogEntry;

// --------- internal helpers ---------

// 한 줄의 주문 로그를 OrderLogEntry로 파싱
static int parse_order_log_line(const char *line, OrderLogEntry *out)
{
    char *p_time, *p_order, *p_menu, *p_qty, *p_total, *p_method, *p_place;

    if (!line || !out) return 0;

    memset(out, 0, sizeof(*out));

    p_time   = strstr(line, "time=");
    p_order  = strstr(line, "order=");
    p_menu   = strstr(line, "menu=");
    p_qty    = strstr(line, "qty=");
    p_total  = strstr(line, "total=");
    p_method = strstr(line, "method=");
    p_place  = strstr(line, "place=");

    if (!p_order || !p_menu || !p_qty || !p_total)
        return 0;

    if (p_time) {
        p_time += 5; // skip "time="
        const char *comma = strchr(p_time, ',');
        int len = comma ? (int)(comma - p_time) : (int)strlen(p_time);
        if (len > (int)sizeof(out->time_str) - 1)
            len = (int)sizeof(out->time_str) - 1;
        strncpy(out->time_str, p_time, len);
        out->time_str[len] = '\0';
    } else {
        out->time_str[0] = '\0';
    }

    if (sscanf(p_order, "order=%d", &out->order_no) != 1)
        return 0;
    if (sscanf(p_menu,  "menu=%63[^,]", out->menu) != 1)
        return 0;
    if (sscanf(p_qty,   "qty=%d", &out->qty) != 1)
        return 0;
    if (sscanf(p_total, "total=%d", &out->total) != 1)
        return 0;

    if (p_method) {
        sscanf(p_method, "method=%15[^,]", out->method);
    } else {
        out->method[0] = '\0';
    }

    if (p_place) {
        sscanf(p_place, "place=%15[^,\n]", out->place);
    } else {
        out->place[0] = '\0';
    }

    return 1;
}

// refunds.log 에서 이미 환불된 주문인지 체크
static int is_already_refunded(int order_no)
{
    FILE *fp;
    char line[512];

    fp = fopen(REFUNDS_LOG_PATH, "r");
    if (!fp) {
        // 파일이 없으면 아직 환불 내역이 없는 것
        return 0;
    }

    while (fgets(line, sizeof(line), fp)) {
        OrderLogEntry e;
        if (!parse_order_log_line(line, &e))
            continue;
        if (e.order_no == order_no) {
            fclose(fp);
            return 1;
        }
    }

    fclose(fp);
    return 0;
}

// 특정 주문번호(order_no)의 모든 로그를 화면에 출력
static int show_order_by_number(int order_no)
{
    FILE *fp;
    char line[512];
    int found = 0;

    fp = fopen(ORDERS_LOG_PATH, "r");
    if (!fp) {
        printf("Cannot open %s\n\n", ORDERS_LOG_PATH);
        return 0;
    }

    printf("=== Order #%d ===\n", order_no);

    while (fgets(line, sizeof(line), fp)) {
        OrderLogEntry e;
        if (!parse_order_log_line(line, &e))
            continue;
        if (e.order_no != order_no)
            continue;

        found = 1;
        printf(" [%s] %s x%d  | line total: %d  | %s / %s\n",
               e.time_str[0] ? e.time_str : "no time",
               e.menu,
               e.qty,
               e.total,
               e.method[0] ? e.method : "-",
               e.place[0] ? e.place : "-");
    }

    if (!found) {
        printf("No logs found for order #%d.\n", order_no);
    }

    printf("\n");
    fclose(fp);
    return found;
}

// 전체 주문 로그 간단 출력
static void show_all_orders(void)
{
    FILE *fp;
    char line[512];

    fp = fopen(ORDERS_LOG_PATH, "r");
    if (!fp) {
        printf("Cannot open %s\n\n", ORDERS_LOG_PATH);
        return;
    }

    printf("=== All Orders (from %s) ===\n", ORDERS_LOG_PATH);

    while (fgets(line, sizeof(line), fp)) {
        OrderLogEntry e;
        if (!parse_order_log_line(line, &e))
            continue;

        printf("#%d | %s | %s x%d | total %d | %s / %s\n",
               e.order_no,
               e.time_str[0] ? e.time_str : "no time",
               e.menu,
               e.qty,
               e.total,
               e.method[0] ? e.method : "-",
               e.place[0] ? e.place : "-");
    }

    printf("\n");
    fclose(fp);
}

// 환불 로그에 order_no에 해당하는 줄들을 그대로 복사(append)
static int write_refund_log_for_order(int order_no)
{
    FILE *fp_in, *fp_out;
    char line[512];
    int found = 0;

    fp_in = fopen(ORDERS_LOG_PATH, "r");
    if (!fp_in) {
        printf("Cannot open %s\n\n", ORDERS_LOG_PATH);
        return 0;
    }

    fp_out = fopen(REFUNDS_LOG_PATH, "a");
    if (!fp_out) {
        printf("Cannot open %s for append.\n\n", REFUNDS_LOG_PATH);
        fclose(fp_in);
        return 0;
    }

    while (fgets(line, sizeof(line), fp_in)) {
        OrderLogEntry e;
        if (!parse_order_log_line(line, &e))
            continue;
        if (e.order_no != order_no)
            continue;

        found = 1;

        // 간단하게 맨 앞에 "REFUND " 라는 태그를 붙여서 기록
        // 예) "REFUND time=..., order=3, ..."
        fprintf(fp_out, "REFUND %s", line);
    }

    fclose(fp_in);
    fclose(fp_out);

    if (!found) {
        printf("No order lines to refund for order #%d.\n\n", order_no);
        return 0;
    }

    return 1;
}

// 실제 환불 처리 플로우
static void do_refund_flow(void)
{
    int order_no;
    int result;

    printf("\n--- Refund Order ---\n");

    // 관리자 모드: 타임아웃 없이 입력 대기
    result = timed_read_int("Enter order number (0 = cancel): ",
                            &order_no,
                            0,  // no warn
                            0); // no timeout

    if (result == INPUT_INVALID) {
        printf("Invalid input.\n\n");
        return;
    }
    if (order_no == 0) {
        printf("Cancel refund.\n\n");
        return;
    }

    // 주문 내역이 있는지 먼저 보여줌
    if (!show_order_by_number(order_no)) {
        // 없는 주문번호
        return;
    }

    // 이미 환불된 주문인지 확인
    if (is_already_refunded(order_no)) {
        printf("Order #%d has already been refunded.\n\n", order_no);
        return;
    }

    // 정말 환불할지 물어보기
    char buf[16];
    printf("Do you really want to refund this order? (y/n): ");
    if (!fgets(buf, sizeof(buf), stdin)) {
        printf("Input error. Cancel refund.\n\n");
        return;
    }

    if (buf[0] != 'y' && buf[0] != 'Y') {
        printf("Refund cancelled.\n\n");
        return;
    }

    if (!write_refund_log_for_order(order_no)) {
        printf("Failed to write refund log.\n\n");
        return;
    }

    printf("Order #%d refunded. (recorded in %s)\n\n",
           order_no, REFUNDS_LOG_PATH);
}

// --------- public entry point ---------

void run_refund_menu(void)
{
    int choice;
    int result;

    while (1) {
        printf("=== Order / Refund Management ===\n");
        printf("1. Show all order logs\n");
        printf("2. Search order by order number\n");
        printf("3. Refund an order\n");
        printf("0. Back\n");
        printf("-------------------------------\n");

        // 관리자 모드: 타임아웃 없이 입력 대기
        result = timed_read_int("Select: ",
                                &choice,
                                0,   // no warn
                                0);  // no timeout

        if (result == INPUT_INVALID) {
            printf("Invalid input.\n\n");
            continue;
        }

        switch (choice) {
        case 0:
            printf("\nBack to admin main menu.\n\n");
            return;
        case 1:
            show_all_orders();
            break;
        case 2: {
            int order_no;
            int r = timed_read_int("Enter order number: ",
                                   &order_no,
                                   0,   // no warn
                                   0);  // no timeout
            if (r == INPUT_INVALID) {
                printf("Invalid input.\n\n");
                break;
            }
            show_order_by_number(order_no);
            break;
        }
        case 3:
            do_refund_flow();
            break;
        default:
            printf("Please select 0~3.\n\n");
            break;
        }
    }
}
