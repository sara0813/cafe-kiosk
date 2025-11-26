// src/admin_refund.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "input.h"

// 로그 파일 경로
#define ORDERS_LOG_PATH      "data/logs/orders.log"
#define ORDERS_LOG_TMP_PATH  "data/logs/orders.tmp"
#define REFUNDS_LOG_PATH     "data/logs/refunds.log"

typedef struct {
    int  order_no;
    char time_str[32];
    char menu[64];
    int  qty;
    int  total;
    char method[16];
    char place[16];
} OrderLogEntry;

// --------- 공통 파싱 헬퍼 ---------

// 한 줄의 주문 로그를 OrderLogEntry로 파싱
// 예: time=2025-11-21 13:40:01, order=3, menu=아메리카노, qty=2, total=6000, method=CARD, place=HERE
// "REFUND time=..." 형태도 지원 (strstr로 찾기 때문)
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

// 현재 시각을 "YYYY-MM-DD HH:MM:SS" 문자열로
static void get_now_string(char *buf, size_t sz)
{
    time_t t = time(NULL);
    struct tm *tm_now = localtime(&t);
    if (!tm_now) {
        snprintf(buf, sz, "unknown");
        return;
    }
    strftime(buf, sz, "%Y-%m-%d %H:%M:%S", tm_now);
}

// --------- 주문 / 환불 표시 함수들 ---------

// 특정 주문번호(order_no)의 모든 로그를 화면에 출력 (orders.log 기준)
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

    printf("=== Order #%d (from orders.log) ===\n", order_no);

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

// 전체 주문 로그 간단 출력 (orders.log)
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

// 환불 로그 전체 보기 (refunds.log)
static void show_refund_logs(void)
{
    FILE *fp;
    char line[512];

    fp = fopen(REFUNDS_LOG_PATH, "r");
    if (!fp) {
        printf("No refund logs (%s not found).\n\n", REFUNDS_LOG_PATH);
        return;
    }

    printf("=== Refund Logs (from %s) ===\n", REFUNDS_LOG_PATH);

    while (fgets(line, sizeof(line), fp)) {
        OrderLogEntry e;
        if (!parse_order_log_line(line, &e))
            continue;

        printf("REFUND #%d | %s | %s x%d | total %d | %s / %s\n",
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

// --------- 환불 처리 관련 ---------

// refunds.log 에 order_no에 해당하는 줄들을 복사(append) + REFUND 태그, refund_time 포함
static int write_refund_log_for_order(int order_no)
{
    FILE *fp_in, *fp_out;
    char line[512];
    int found = 0;
    char refund_time[32];

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

    get_now_string(refund_time, sizeof(refund_time));

    while (fgets(line, sizeof(line), fp_in)) {
        OrderLogEntry e;
        if (!parse_order_log_line(line, &e))
            continue;
        if (e.order_no != order_no)
            continue;

        found = 1;
        // 형식: REFUND time=..., order=..., menu=..., ..., refund_time=...
        // 원래 라인(line) 안에 time=, order= 등 들어있으니, 앞에 REFUND 붙이고
        // 끝에 refund_time 필드 추가
        // line은 개행 포함되어 있으니 잘라서 씀
        char *p_nl = strchr(line, '\n');
        if (p_nl) *p_nl = '\0';

        fprintf(fp_out, "REFUND %s, refund_time=%s\n", line, refund_time);
    }

    fclose(fp_in);
    fclose(fp_out);

    if (!found) {
        printf("No order lines to refund for order #%d.\n\n", order_no);
        return 0;
    }

    return 1;
}

// orders.log 에서 해당 order_no 줄들을 제거 (임시 파일을 통해 재작성)
static int remove_order_from_orders_log(int order_no)
{
    FILE *fp_in, *fp_out;
    char line[512];
    int removed_any = 0;

    fp_in = fopen(ORDERS_LOG_PATH, "r");
    if (!fp_in) {
        printf("Cannot open %s\n\n", ORDERS_LOG_PATH);
        return 0;
    }

    fp_out = fopen(ORDERS_LOG_TMP_PATH, "w");
    if (!fp_out) {
        printf("Cannot open temp file %s\n\n", ORDERS_LOG_TMP_PATH);
        fclose(fp_in);
        return 0;
    }

    while (fgets(line, sizeof(line), fp_in)) {
        OrderLogEntry e;
        if (parse_order_log_line(line, &e) && e.order_no == order_no) {
            // 이 주문은 제거
            removed_any = 1;
            continue;
        }
        fputs(line, fp_out);
    }

    fclose(fp_in);
    fclose(fp_out);

    if (!removed_any) {
        // 삭제된 것이 없으면 temp 파일은 지우고 원본은 그대로 둠
        remove(ORDERS_LOG_TMP_PATH);
        printf("No matching order found to remove for #%d.\n\n", order_no);
        return 0;
    }

    // 원본 삭제 후 temp -> 원본 이름 변경
    if (remove(ORDERS_LOG_PATH) != 0) {
        printf("Failed to remove original log %s\n\n", ORDERS_LOG_PATH);
        // temp는 남겨 두더라도 상관 없음
        return 0;
    }
    if (rename(ORDERS_LOG_TMP_PATH, ORDERS_LOG_PATH) != 0) {
        printf("Failed to rename temp log to %s\n\n", ORDERS_LOG_PATH);
        return 0;
    }

    return 1;
}

// 환불 영수증 출력 (refunds.log 기준)
static void print_refund_receipt(int order_no)
{
    FILE *fp;
    char line[512];
    int found = 0;
    int total_sum = 0;

    fp = fopen(REFUNDS_LOG_PATH, "r");
    if (!fp) {
        printf("No refund logs to print receipt.\n\n");
        return;
    }

    printf("\n===== REFUND RECEIPT =====\n");
    printf("Order #%d\n", order_no);

    while (fgets(line, sizeof(line), fp)) {
        OrderLogEntry e;
        if (!parse_order_log_line(line, &e))
            continue;
        if (e.order_no != order_no)
            continue;

        found = 1;
        printf(" %s x%d  -%d\n",
               e.menu,
               e.qty,
               e.total);
        total_sum += e.total;
    }

    if (!found) {
        printf("No refund data for this order.\n");
    } else {
        printf("--------------------------\n");
        printf("Total refunded: -%d\n", total_sum);
    }
    printf("==========================\n\n");

    fclose(fp);
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

    // 주문 내역이 있는지 먼저 보여줌 (orders.log)
    if (!show_order_by_number(order_no)) {
        // 없는 주문번호
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

    // 1) refunds.log 에 환불 기록 남기기
    if (!write_refund_log_for_order(order_no)) {
        printf("Failed to write refund log.\n\n");
        return;
    }

    // 2) orders.log 에서 해당 주문 삭제
    if (!remove_order_from_orders_log(order_no)) {
        printf("Warning: refund log written, but failed to remove from orders.log.\n\n");
        return;
    }

    printf("Order #%d refunded and removed from order log.\n", order_no);

    // 3) 환불 영수증 출력 여부
    char buf2[16];
    printf("Print refund receipt? (y/n): ");
    if (!fgets(buf2, sizeof(buf2), stdin)) {
        printf("Input error. No receipt printed.\n\n");
        return;
    }

    if (buf2[0] == 'y' || buf2[0] == 'Y') {
        print_refund_receipt(order_no);
    } else {
        printf("No receipt printed.\n\n");
    }
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
        printf("4. Show refund logs\n");
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
        case 4:
            show_refund_logs();
            break;
        default:
            printf("Please select 0~4.\n\n");
            break;
        }
    }
}
