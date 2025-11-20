// include/order_log.h
#ifndef ORDER_LOG_H
#define ORDER_LOG_H

// payment_method : 1=Cash, 2=Card, 3=KakaoPay
void write_order_log(int order_no, int total, int payment_method);

#endif
