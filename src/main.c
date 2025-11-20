#include <stdio.h>

void run_user_mode(void);
void run_admin_mode(void);

int main(void) {
    int choice;

    while (1) {
        printf("=================================\n");
        printf("          Cafe Kiosk\n");
        printf("=================================\n");
        printf("1. Order (User)\n");
        printf("2. Admin mode\n");
        printf("0. Exit\n");
        printf("---------------------------------\n");
        printf("Select: ");

        if (scanf("%d", &choice) != 1) {
            // flush invalid input
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF)
                ;
            printf("Invalid input. Please enter a number.\n\n");
            continue;
        }

        printf("\n");

        switch (choice) {
        case 1:
            run_user_mode();
            break;
        case 2:
            run_admin_mode();
            break;
        case 0:
            printf("Bye.\n");
            return 0;
        default:
            printf("No such menu. Try again.\n\n");
            break;
        }
    }

    return 0;
}

void run_user_mode(void) {
    printf("[User mode] (not implemented yet)\n\n");
}

void run_admin_mode(void) {
    printf("[Admin mode] (not implemented yet)\n\n");
}
