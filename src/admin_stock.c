// src/admin_stock.c
#include <stdio.h>
#include <string.h>
#include "input.h"
#include "menu.h"

#define STOCK_FILE_COFFEE        "data/stocks/coffee_stock.txt"
#define STOCK_FILE_NONCOFFEE     "data/stocks/noncoffee_stock.txt"
#define STOCK_FILE_TEA           "data/stocks/tea_stock.txt"
#define STOCK_FILE_DUTCH         "data/stocks/dutch_stock.txt"
#define STOCK_FILE_SIGNATURE     "data/stocks/signature_stock.txt"
#define STOCK_FILE_SMOOTHIE      "data/stocks/smoothie_juice_stock.txt"
#define STOCK_FILE_DESSERT       "data/stocks/dessert_stock.txt"

// --- 카테고리 공통 헬퍼 (admin_menu_edit.c 랑 맞춰서 사용) ---

static const char *get_category_name(int cat) {
    switch (cat) {
    case 1: return "Coffee";
    case 2: return "Non-coffee";
    case 3: return "Tea";
    case 4: return "Dutch";
    case 5: return "Signature";
    case 6: return "Smoothie / Juice";
    case 7: return "Dessert";
    default: return "Unknown";
    }
}

// 메뉴 파일 경로 (menu.c 에서 쓰는 것과 동일하게 맞춰야 함)
static const char *get_menu_filepath(int cat) {
    switch (cat) {
    case 1: return "data/menus/coffee.txt";
    case 2: return "data/menus/noncoffee.txt";
    case 3: return "data/menus/tea.txt";
    case 4: return "data/menus/dutch.txt";
    case 5: return "data/menus/signature.txt";
    case 6: return "data/menus/smoothie_juice.txt";
    case 7: return "data/menus/dessert.txt";
    default: return NULL;
    }
}

// 재고 파일 경로
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

// --- 재고 파일 로드 & 저장 ---

// stock 배열에 [0..count-1] 까지 채움. 없는 부분은 0.
static void load_stock(const char *filepath, int *stock, int count) {
    // 기본값 0으로 초기화
    for (int i = 0; i < count; i++) {
        stock[i] = 0;
    }

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        // 파일이 없어도 에러는 아님 (처음 실행 등)
        return;
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
}

// 메뉴 id와 stock을 같이 파일에 저장
static int save_stock(const char *filepath, MenuItem *items, int *stock, int count) {
    FILE *fp = fopen(filepath, "w");
    if (!fp) {
        printf("[Error] Cannot open %s for writing.\n\n", filepath);
        return 0;
    }

    for (int i = 0; i < count; i++) {
        fprintf(fp, "%d,%d\n", items[i].id, stock[i]);
    }

    fclose(fp);
    return 1;
}

// --- 카테고리별 재고 화면 ---

static void show_stock_list(const char *menu_path,
                            const char *stock_path,
                            const char *cat_name)
{
    MenuItem items[MAX_MENU_ITEMS];
    int count = load_menu(menu_path, items, MAX_MENU_ITEMS);

    if (count <= 0) {
        printf("\nNo menu items in %s.\n\n", cat_name);
        return;
    }

    int stock[MAX_MENU_ITEMS];
    load_stock(stock_path, stock, count);

    printf("\n=== Stock List (%s) ===\n", cat_name);
    printf("%-4s %-4s %-30s %10s\n", "No.", "ID", "Menu name", "Stock");
    printf("-----------------------------------------------------------\n");

    for (int i = 0; i < count; i++) {
        printf("%3d  %3d  %-30s %10d\n",
               i + 1, items[i].id, items[i].name, stock[i]);
    }
    printf("\n");
}

static void edit_stock_item(const char *menu_path,
                            const char *stock_path,
                            const char *cat_name)
{
    MenuItem items[MAX_MENU_ITEMS];
    int count = load_menu(menu_path, items, MAX_MENU_ITEMS);

    if (count <= 0) {
        printf("\nNo menu items in %s.\n\n", cat_name);
        return;
    }

    int stock[MAX_MENU_ITEMS];
    load_stock(stock_path, stock, count);

    // 현재 재고 먼저 보여주기
    show_stock_list(menu_path, stock_path, cat_name);

    int index;
    int result;
    while (1) {
        result = timed_read_int(
            "Select menu number to edit stock (0 = cancel): ",
            &index, 0, 0);  // no timeout

        if (result == INPUT_INVALID) {
            printf("Invalid input.\n\n");
            continue;
        }
        if (index == 0) {
            printf("Edit cancelled.\n\n");
            return;
        }
        if (index < 1 || index > count) {
            printf("Please select between 1 and %d.\n\n", count);
            continue;
        }
        break;
    }

    int idx = index - 1;
    printf("Selected: %s (ID=%d), current stock = %d\n",
           items[idx].name, items[idx].id, stock[idx]);

    int new_stock;
    while (1) {
        result = timed_read_int("Enter new stock (>= 0): ", &new_stock, 0, 0);
        if (result == INPUT_INVALID) {
            printf("Invalid input.\n\n");
            continue;
        }
        if (new_stock < 0) {
            printf("Stock cannot be negative.\n\n");
            continue;
        }
        break;
    }

    stock[idx] = new_stock;

    if (!save_stock(stock_path, items, stock, count)) {
        printf("Failed to save stock file.\n\n");
        return;
    }

    printf("Stock for \"%s\" updated to %d.\n\n",
           items[idx].name, new_stock);
}

// 카테고리 하나에 대한 재고 관리 메뉴
static void run_stock_menu_for_category(int cat) {
    const char *menu_path  = get_menu_filepath(cat);
    const char *stock_path = get_stock_filepath(cat);
    const char *cat_name   = get_category_name(cat);

    if (!menu_path || !stock_path) {
        printf("Invalid category.\n\n");
        return;
    }

    int choice;
    int result;

    while (1) {
        printf("=== Stock Management (%s) ===\n", cat_name);
        printf("1. Show stock list\n");
        printf("2. Edit stock of a menu item\n");
        printf("0. Back to category selection\n");

        result = timed_read_int("Select: ", &choice, 0, 0); // no timeout here

        if (result == INPUT_INVALID) {
            printf("Invalid input.\n\n");
            continue;
        }

        printf("\n");

        switch (choice) {
        case 1:
            show_stock_list(menu_path, stock_path, cat_name);
            break;
        case 2:
            edit_stock_item(menu_path, stock_path, cat_name);
            break;
        case 0:
            printf("Back to category selection.\n\n");
            return;
        default:
            printf("No such menu.\n\n");
            break;
        }
    }
}

// --- 외부에서 호출하는 엔트리포인트 ---
// admin_main.c 에서 이 함수를 호출함

void run_stock_menu(void) {
    int cat;
    int result;

    while (1) {
        printf("=== Stock Management ===\n");
        printf("1. Coffee\n");
        printf("2. Non-coffee\n");
        printf("3. Tea\n");
        printf("4. Dutch\n");
        printf("5. Signature\n");
        printf("6. Smoothie / Juice\n");
        printf("7. Dessert\n");
        printf("0. Back to admin main menu\n");

        result = timed_read_int("Select category: ",
                                &cat, INPUT_WARN_SEC, INPUT_TIMEOUT_SEC);

        if (result == INPUT_TIMEOUT) {
            printf("\nTimeout. Back to admin main menu.\n\n");
            return;
        }
        if (result == INPUT_INVALID) {
            printf("Invalid input.\n\n");
            continue;
        }

        if (cat == 0) {
            printf("Back to admin main menu.\n\n");
            return;
        }
        if (cat < 0 || cat > 7) {
            printf("Please select 0~7.\n\n");
            continue;
        }

        run_stock_menu_for_category(cat);
    }
}
