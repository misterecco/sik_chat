#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("Ending with CR only\r");
    printf("Ending with LF only\n");
    printf("Ending with CRLF\r\n");
    exit(EXIT_SUCCESS);
}