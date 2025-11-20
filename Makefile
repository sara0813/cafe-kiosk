# Makefile (Windows + Linux 공용)

# OS에 따라 실행 파일 이름 / 삭제 명령 다르게 설정
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
    src/admin.c \
    src/menu.c \
    src/cart.c \
    src/payment.c \
    src/price.c

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
	$(RM) $(OBJS) $(TARGET)
