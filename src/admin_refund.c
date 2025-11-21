// src/admin_refund.c
#include <stdio.h>
#include <string.h>
#include "input.h"

#define ORDERS_LOG_PATH  "data/logs/orders.log"
#define REFUND_LOG_PATH  "data/logs/refunds.log"

// -------------------- 헬퍼: 주문 로그 전체 보기 --------------------
static void show_order_log(void) {
    FILE *fp = fopen(ORDERS_LOG_PATH, "r");
    if (!fp) {
        printf("Cannot open %s\n\n", ORDERS_LOG_PATH);
        return;
    }

    printf("\n===== Order Log =====\n");

    char line[256];
    int line_no = 0;
    while (fgets(line, sizeof(line), fp)) {
        line_no++;
        printf("%3d) %s", line_no, line);   // 줄 번호 붙여서 보여줌
        if (line[strlen(line) - 1] != '\n')
            printf("\n");
    }

    if (line_no == 0) {
        printf("(No orders yet)\n");
    }

    printf("=====================\n\n");
    fclose(fp);
}

// -------------------- 헬퍼: 특정 주문 찾기 --------------------
// 찾으면 1 리턴, 못 찾으면 0 리턴
static int find_order(int target_no,
                      int *out_total,
                      char *out_method, size_t method_size)
{
    FILE *fp = fopen(ORDERS_LOG_PATH, "r");
    if (!fp) {
        printf("Cannot open %s\n\n", ORDERS_LOG_PATH);
        return 0;
    }

    char line[256];
    int found = 0;

    while (fgets(line, sizeof(line), fp)) {
        int order_no = 0;
        int total = 0;
        char method[32] = {0};

        char *p_order  = strstr(line, "order=");
        char *p_total  = strstr(line, "total=");
        char *p_method = strstr(line, "method=");

        if (!p_order || !p_total || !p_method)
            continue;

        if (sscanf(p_order,  "order=%d", &order_no) != 1)
            continue;
        if (sscanf(p_total,  "total=%d", &total) != 1)
            continue;
        if (sscanf(p_method, "method=%31[^,\n]", method) != 1)
            continue;

        if (order_no == target_no) {
            if (out_total) *out_total = total;
            if (out_method && method_size > 0)
                strncpy(out_method, method, method_size - 1);
            found = 1;
            break;
        }
    }

    fclose(fp);
    return found;
}

// -------------------- 헬퍼: 환불 로그에 기록 --------------------
static void write_refund_log(int order_no, int total, const char *method) {
    FILE *fp = fopen(REFUND_LOG_PATH, "a");
    if (!fp) {
        printf("Cannot open %s for write.\n\n", REFUND_LOG_PATH);
        return;
    }

    fprintf(fp, "order=%d, total=%d, method=%s, status=REFUND\n",
            order_no, total, method ? method : "UNKNOWN");
    fclose(fp);
}

// -------------------- 환불 처리 --------------------
static void do_refund(void) {
    int order_no;
    int result;

    while (1) {
        result = timed_read_int(
            "\nEnter order number to refund (0: cancel): ",
            &order_no, INPUT_WARN_SEC, INPUT_TIMEOUT_SEC);

        if (result == INPUT_TIMEOUT) {
            printf("\nTimeout. Cancel refund.\n\n");
            return;
        }
        if (result == INPUT_INVALID) {
            printf("Invalid input. Please enter number.\n");
            continue;
        }
        if (order_no == 0) {
            printf("Cancel refund.\n\n");
            return;
        }
        if (order_no < 0) {
            printf("Order number must be positive.\n");
            continue;
        }

        break;
    }

    int total = 0;
    char method[32] = {0};
    if (!find_order(order_no, &total, method, sizeof(method))) {
        printf("Order #%d not found in log.\n\n", order_no);
        return;
    }

    printf("\n--- Order Info ---\n");
    printf("Order no : %d\n", order_no);
    printf("Total    : %d\n", total);
    printf("Method   : %s\n", method);
    printf("-------------------\n");

    int confirm;
    result = timed_read_int(
        "Refund this order? (1: Yes, 0: No): ",
        &confirm, INPUT_WARN_SEC, INPUT_TIMEOUT_SEC);

    if (result == INPUT_TIMEOUT) {
        printf("\nTimeout. Cancel refund.\n\n");
        return;
    }
    if (result == INPUT_INVALID || (confirm != 0 && confirm != 1)) {
        printf("Invalid input. Refund cancelled.\n\n");
        return;
    }
    if (confirm == 0) {
        printf("Refund cancelled.\n\n");
        return;
    }

    // 실제 환불 처리 로직은 여기서:
    //  - 나중에 매출 합계에서 빼줄 때 REFUND 로그를 참고
    write_refund_log(order_no, total, method);

    printf("\nRefund completed for order #%d.\n\n", order_no);
}

// -------------------- 외부에서 부르는 함수 --------------------
void run_refund_menu(void) {
    int choice;
    int result;

    while (1) {
        printf("=== Order Management ===\n");
        printf("1. View order log\n");
        printf("2. Refund order\n");
        printf("0. Back to admin menu\n");
        printf("------------------------\n");

        result = timed_read_int("Select: ", &choice,
                                INPUT_WARN_SEC, INPUT_TIMEOUT_SEC);

        if (result == INPUT_TIMEOUT) {
            printf("\nTimeout. Back to admin menu.\n\n");
            return;
        }
        if (result == INPUT_INVALID) {
            printf("Invalid input. Please enter number.\n\n");
            continue;
        }

        switch (choice) {
        case 0:
            printf("\nBack to admin menu.\n\n");
            return;
        case 1:
            show_order_log();
            break;
        case 2:
            do_refund();
            break;
        default:
            printf("No such menu.\n\n");
            break;
        }
    }
}
