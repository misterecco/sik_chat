#include <stdio.h>
#include <stdlib.h>

int main() {
//    printf("Ending with CR only\r");
//    printf("Wrong message with zero\0 at the ens\n");
    printf("Ending with LF only\n");
//    printf("Ending with CRLF\r\n");
    for (int i = 0; i < 150; i++) {
        printf("abcdefghij");
    }
    printf("\n");
    for (int i = 0; i < 100; i++) {
        printf("hahahahaha");
    }
    printf("\n");
    for (int i = 0; i < 1001; i++) {
        printf("a");
    }
    printf("\n");
    exit(EXIT_SUCCESS);
}