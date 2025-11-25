ifeq ($(OS),Windows_NT)
    EXE_EXT = .exe
    RM = del /Q
else
    EXE_EXT =
    RM = rm -f
endif

CC     = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude

SRCS = \
    src/main.c \
    src/user.c \
    src/menu.c \
    src/cart.c \
    src/payment.c \
    src/price.c \
    src/order_log.c \
    src/input.c \
    src/admin_login.c \
    src/admin_main.c \
    src/admin_sales.c \
    src/admin_sales_total.c \
    src/admin_sales_payment.c \
    src/admin_sales_menu.c \
    src/admin_sales_place.c \
    src/admin_sales_log.c \
    src/admin_menu_edit.c \
    src/admin_stock.c \
    src/admin_refund.c

OBJS   = $(SRCS:.c=.o)

TARGET = cafe_kiosk$(EXE_EXT)

# 기본 타깃: 빌드
all: $(TARGET)

# 실행 파일 만들기
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

# 각 .c → .o 컴파일 규칙
src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# 빌드 + 실행
run: all
	./$(TARGET)

# 생성 파일 지우기
clean:
	del /Q src\*.o 2>nul
	del /Q cafe_kiosk.exe 2>nul
