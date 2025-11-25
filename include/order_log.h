// include/order_log.h
#ifndef ORDER_LOG_H
#define ORDER_LOG_H


int write_order_log(int order_no,
                    const char *menu_name,
                    int qty,
                    int line_total,
                    int method,
                    int order_place);
                    
#endif
