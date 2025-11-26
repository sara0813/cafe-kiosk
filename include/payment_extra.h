// include/payment_extra.h
#ifndef PAYMENT_EXTRA_H
#define PAYMENT_EXTRA_H

int ask_point(void);

void save_point(void);

void ask_receipt_and_print(int order_no);

void log_order_items(int order_no, int method, int order_place);

#endif
