// src/admin_menu_edit.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "input.h"
#include "menu.h"
#include "price.h"

// 실제 메뉴 파일 경로들
#define MENU_FILE_COFFEE        "data/menus/coffee.txt"
#define MENU_FILE_NONCOFFEE     "data/menus/noncoffee.txt"
#define MENU_FILE_TEA           "data/menus/tea.txt"
#define MENU_FILE_DUTCH         "data/menus/dutch.txt"
#define MENU_FILE_SIGNATURE     "data/menus/signature.txt"
#define MENU_FILE_SMOOTHIE      "data/menus/smoothie_juice.txt"
#define MENU_FILE_DESSERT       "data/menus/dessert.txt"

// -------- category helpers --------

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

static const char *get_category_filepath(int cat) {
    switch (cat) {
    case 1: return MENU_FILE_COFFEE;
    case 2: return MENU_FILE_NONCOFFEE;
    case 3: return MENU_FILE_TEA;
    case 4: return MENU_FILE_DUTCH;
    case 5: return MENU_FILE_SIGNATURE;
    case 6: return MENU_FILE_SMOOTHIE;
    case 7: return MENU_FILE_DESSERT;
    default: return NULL;
    }
}

// 카테고리 선택 (0 = back)
static int select_category(void) {
    int cat;
    int result;

    while (1) {
        printf("=== Select Category ===\n");
        printf("1. Coffee\n");
        printf("2. Non-coffee\n");
        printf("3. Tea\n");
        printf("4. Dutch\n");
        printf("5. Signature\n");
        printf("6. Smoothie / Juice\n");
        printf("7. Dessert\n");
        printf("0. Back to admin menu\n");

        result = timed_read_int("Select: ", &cat, 0, 0);  // no timeout

        if (result == INPUT_INVALID) {
            printf("Invalid input.\n\n");
            continue;
        }

        if (cat < 0 || cat > 7) {
            printf("Please select 0~7.\n\n");
            continue;
        }
        return cat;
    }
}

// -------- file helpers --------

// menu.c 에서 제공하는: int load_menu(const char *, MenuItem *, int);

// 메뉴를 파일에 저장 (id 는 1부터 다시 재번호 부여)
static int save_menu_items(const char *filepath, MenuItem *items, int count) {
    FILE *fp = fopen(filepath, "w");
    if (!fp) {
        printf("[Error] Cannot open %s for writing.\n\n", filepath);
        return 0;
    }

    for (int i = 0; i < count; i++) {
        int id = i + 1;
        // 포맷: id, name,price  (공백은 조금 달라도 파싱에는 문제 없음)
        fprintf(fp, "%d, %s,%d\n", id, items[i].name, items[i].price);
    }

    fclose(fp);
    return 1;
}

// 카테고리 메뉴 전체 출력
static void show_category_menu(const char *filepath, const char *cat_name) {
    MenuItem items[MAX_MENU_ITEMS];
    int count = load_menu(filepath, items, MAX_MENU_ITEMS);

    printf("\n=== Menu / Price List (%s) ===\n", cat_name);
    if (count <= 0) {
        printf("No menu items.\n\n");
        return;
    }

    printf("%-4s %-4s %-30s %15s\n", "No.", "ID", "Menu name", "Price");
    printf("---------------------------------------------------------------\n");

    for (int i = 0; i < count; i++) {
        char price_str[32];
        format_price_with_comma(items[i].price, price_str, sizeof(price_str));
        printf("%3d  %3d  %-30s %15s won\n",
               i + 1, items[i].id, items[i].name, price_str);
    }
    printf("\n");
}

// 입력 버퍼에 남아 있을 수 있는 개행 제거용
static void clear_stdin_line(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        // discard
    }
}

// -------- edit operations for one category --------

// 가격 수정
static void edit_price_in_category(const char *filepath, const char *cat_name) {
    MenuItem items[MAX_MENU_ITEMS];
    int count = load_menu(filepath, items, MAX_MENU_ITEMS);

    if (count <= 0) {
        printf("\nNo menu items to edit in %s.\n\n", cat_name);
        return;
    }

    show_category_menu(filepath, cat_name);

    int index;
    int result;

    while (1) {
        result = timed_read_int(
            "Select menu number to edit (0 = cancel): ",
            &index, 0, 0);

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

    int new_price;
    while (1) {
        result = timed_read_int("Enter new price: ", &new_price, 0, 0);
        if (result == INPUT_INVALID) {
            printf("Invalid input.\n\n");
            continue;
        }
        if (new_price < 0) {
            printf("Price cannot be negative.\n\n");
            continue;
        }
        break;
    }

    int idx = index - 1;
    char old_buf[32], new_buf[32];
    format_price_with_comma(items[idx].price, old_buf, sizeof(old_buf));
    format_price_with_comma(new_price,       new_buf, sizeof(new_buf));

    printf("Changing price of \"%s\" from %s won to %s won.\n",
           items[idx].name, old_buf, new_buf);
    items[idx].price = new_price;

    if (!save_menu_items(filepath, items, count)) {
        printf("Failed to save menu file.\n\n");
        return;
    }

    printf("Price updated successfully.\n\n");
}

// 메뉴 추가
static void add_item_to_category(const char *filepath, const char *cat_name) {
    MenuItem items[MAX_MENU_ITEMS];
    int count = load_menu(filepath, items, MAX_MENU_ITEMS);

    if (count < 0) {
        count = 0;  // 파일이 없거나 에러여도 새로 작성 가능
    }
    if (count >= MAX_MENU_ITEMS) {
        printf("\nCannot add more menu items (max = %d).\n\n", MAX_MENU_ITEMS);
        return;
    }

    printf("\n=== Add New Menu Item (%s) ===\n", cat_name);

    // 숫자 입력 후 남은 '\n' 제거
    clear_stdin_line();

    char name[64];
    printf("Enter menu name: ");
    if (!fgets(name, sizeof(name), stdin)) {
        printf("Input error.\n\n");
        return;
    }
    name[strcspn(name, "\r\n")] = '\0';

    if (name[0] == '\0') {
        printf("Menu name cannot be empty.\n\n");
        return;
    }

    int price;
    int result;
    while (1) {
        result = timed_read_int("Enter price: ", &price, 0, 0);
        if (result == INPUT_INVALID) {
            printf("Invalid input.\n\n");
            continue;
        }
        if (price < 0) {
            printf("Price cannot be negative.\n\n");
            continue;
        }
        break;
    }

    // 새 항목 추가 (id 는 저장할 때 다시 번호 매김)
    strncpy(items[count].name, name, sizeof(items[count].name));
    items[count].name[sizeof(items[count].name) - 1] = '\0';
    items[count].price = price;
    count++;

    if (!save_menu_items(filepath, items, count)) {
        printf("Failed to save menu file.\n\n");
        return;
    }

    char buf[32];
    format_price_with_comma(price, buf, sizeof(buf));
    printf("Menu item \"%s\" (%s won) added successfully.\n\n", name, buf);
}

// 메뉴 삭제
static void delete_item_from_category(const char *filepath, const char *cat_name) {
    MenuItem items[MAX_MENU_ITEMS];
    int count = load_menu(filepath, items, MAX_MENU_ITEMS);

    if (count <= 0) {
        printf("\nNo menu items to delete in %s.\n\n", cat_name);
        return;
    }

    show_category_menu(filepath, cat_name);

    int index;
    int result;

    while (1) {
        result = timed_read_int(
            "Select menu number to delete (0 = cancel): ",
            &index, 0, 0);

        if (result == INPUT_INVALID) {
            printf("Invalid input.\n\n");
            continue;
        }
        if (index == 0) {
            printf("Delete cancelled.\n\n");
            return;
        }
        if (index < 1 || index > count) {
            printf("Please select between 1 and %d.\n\n", count);
            continue;
        }
        break;
    }

    int idx = index - 1;
    char price_buf[32];
    format_price_with_comma(items[idx].price, price_buf, sizeof(price_buf));

    printf("Deleting \"%s\" (%s won).\n",
           items[idx].name, price_buf);

    // 뒤의 것들을 앞으로 한 칸씩 당기기
    for (int i = idx; i < count - 1; i++) {
        items[i] = items[i + 1];
    }
    count--;

    if (!save_menu_items(filepath, items, count)) {
        printf("Failed to save menu file.\n\n");
        return;
    }

    printf("Menu item deleted successfully.\n\n");
}

// -------- category-level menu --------

static void run_category_edit_menu(int cat) {
    const char *filepath = get_category_filepath(cat);
    const char *cat_name = get_category_name(cat);

    if (!filepath) {
        printf("Invalid category.\n\n");
        return;
    }

    int choice;
    int result;

    while (1) {
        printf("=== Menu / Price Edit (%s) ===\n", cat_name);
        printf("1. Show menu / price list\n");
        printf("2. Edit price of a menu item\n");
        printf("3. Add new menu item\n");
        printf("4. Delete menu item\n");
        printf("0. Back to category selection\n");

        result = timed_read_int("Select: ", &choice, 0, 0); // no timeout

        if (result == INPUT_INVALID) {
            printf("Invalid input.\n\n");
            continue;
        }

        printf("\n");

        switch (choice) {
        case 1:
            show_category_menu(filepath, cat_name);
            break;
        case 2:
            edit_price_in_category(filepath, cat_name);
            break;
        case 3:
            add_item_to_category(filepath, cat_name);
            break;
        case 4:
            delete_item_from_category(filepath, cat_name);
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

// -------- external entry point --------

void run_menu_edit_menu(void) {
    while (1) {
        int cat = select_category();
        if (cat == 0) {
            printf("Back to admin main menu.\n\n");
            return;
        }
        run_category_edit_menu(cat);
    }
}
