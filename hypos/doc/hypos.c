#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef union test_blk {
    uint32_t head_tail;
    struct {
        uint16_t head;
        uint16_t tail;
    };
} test_u;

/* gcc doc/hypos.c -o hypos
 */
void main()
{
    printf("              ___  __  ____ \n");
    printf("    /\\_/\\/\\/\\/ _ \\/  \\/ __/ \n");
    printf("    \\  _ \\  / ___/ / /\\__ \\ \n");
    printf("     \\/ \\/_/\\/   \\__/ /___/ PIECE OF SHiT\n");


    printf("              _________    ____ ___          \n");
    printf(" /\\__/\\/\\  /\\/ __/_  _/\\  / __// _ \\         \n");
    printf(" \\  __ \\ \\_\\ \\__ \\/ // /_/ __\\/ _  /         \n");
    printf("  \\/  \\/\\____/___/\\/ \\___\\___\\\\/ \\/  EDU 2024\n");

    test_u test;
    test_u test_0;
    test.head_tail = 0x10000;
    printf("\n");
    printf("TEST   head = %04x, tail = %04x\n", test.head, test.tail);
    test.head_tail = __sync_fetch_and_add(&test_0.head_tail, test.head_tail);
    printf("TEST 0 head = %04x, tail = %04x\n", test_0.head, test_0.tail);
    printf("TEST   head = %04x, tail = %04x\n", test.head, test.tail);
}
