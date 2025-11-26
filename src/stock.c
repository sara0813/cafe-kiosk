// src/stock.c
#include <stdio.h>
#include "stock.h"

// 메뉴, 재고 파일 경로는 admin_menu_edit에서 쓴 것과 똑같이 맞춰야 함
#define STOCK_FILE_COFFEE        "data/stocks/coffee_stock.txt"
#define STOCK_FILE_NONCOFFEE     "data/stocks/noncoffee_stock.txt"
#define STOCK_FILE_TEA           "data/stocks/tea_stock.txt"
#define STOCK_FILE_DUTCH         "data/stocks/dutch_stock.txt"
#define STOCK_FILE_SIGNATURE     "data/stocks/signature_stock.txt"
#define STOCK_FILE_SMOOTHIE      "data/stocks/smoothie_juice_stock.txt"
#define STOCK_FILE_DESSERT       "data/stocks/dessert_stock.txt"

static const char *get_stock_filepath(int cat) {
    switch (cat) {
    case 1: return STOCK_FILE_COFFEE;
    case 2: return STOCK_FILE_NONCOFFEE;
    case 3: return STOCK_FILE_TEA;
    case 4: return STOCK_FILE_DUTCH;
    case 5: return STOCK_FILE_SIGNATURE;
    case 6: return STOCK_FILE_SMOOTHIE;
    case 7: return STOCK_FILE_DESSERT;
    default: return NULL;
    }
}

// cat 카테고리의 재고를 읽어서 stock[] 에 채움.
// 파일이 없거나 더 짧으면 남은 부분은 0.
int stock_load_category(int cat, int *stock, int count) {
    const char *path = get_stock_filepath(cat);
    if (!path) return 0;

    // 기본값 0
    for (int i = 0; i < count; i++)
        stock[i] = 0;

    FILE *fp = fopen(path, "r");
    if (!fp) {
        // 파일이 없어도 에러는 아님
        return 1;
    }

    char line[256];
    int idx = 0;
    while (idx < count && fgets(line, sizeof(line), fp)) {
        int id, s;
        if (sscanf(line, "%d,%d", &id, &s) == 2) {
            stock[idx] = s;
            idx++;
        }
    }

    fclose(fp);
    return 1;
}

// cat 카테고리의 메뉴/재고를 파일로 저장
int stock_save_category(int cat, const MenuItem *items, const int *stock, int count) {
    const char *path = get_stock_filepath(cat);
    if (!path) return 0;

    FILE *fp = fopen(path, "w");
    if (!fp) {
        printf("[Error] Cannot open %s for writing.\n\n", path);
        return 0;
    }

    for (int i = 0; i < count; i++) {
        fprintf(fp, "%d,%d\n", items[i].id, stock[i]);
    }

    fclose(fp);
    return 1;
}
