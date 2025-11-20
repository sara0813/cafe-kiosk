// src/menu.c
#include <stdio.h>
#include <string.h>
#include "menu.h"

int load_menu(const char *filepath, MenuItem *items, int max_count) {
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        perror("fopen");
        return -1;
    }

    char line[256];
    int count = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (count >= max_count) {
            break;
        }

        // remove newline
        line[strcspn(line, "\r\n")] = '\0';

        MenuItem item;
        // format: id,name,price
        if (sscanf(line, "%d,%63[^,],%d",
                   &item.id, item.name, &item.price) == 3) {
            items[count++] = item;
        }
        // malformed line → ignore
    }

    fclose(fp);
    return count;
}

void print_menu(const MenuItem *items, int count) {
    printf("==== Menu ====\n");
    for (int i = 0; i < count; i++) {
        printf("%2d. %s  (%d won)\n",
               items[i].id, items[i].name, items[i].price);
    }
    printf("==============\n\n");
}
