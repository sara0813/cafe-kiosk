// include/menu.h
#ifndef MENU_H
#define MENU_H

#define MAX_NAME_LEN 64
#define MAX_MENU_ITEMS 100

typedef struct {
    int id;        
    char category[16];
    char name[50];
    int price;
} MenuItem;


int load_menu(const char *filepath, MenuItem *items, int max_count);
void print_menu(const MenuItem *items, int count);

#endif
