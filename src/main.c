#include "cart.h"

#include <stdio.h>

int main(int argc, char *argv[]) {
    if (cart_run() == 0) return -1;
    printf("exiting");
    return 0;
}
