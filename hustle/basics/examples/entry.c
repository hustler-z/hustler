#include <stdio.h>
#include <stdlib.h>

/*
 * ----------------------------------------------------
 * The __asm keyword can incorporate inline GCC syntax
 * assembly code into a function.
 *
 * ----------------------------------------------------
 */

#define barrier() __asm__ __volatile__("": : :"memory")

extern int func(int a, int b);
extern int info(void);
extern int factorial(int x); 
extern int pstate(void);
extern int debug(int x);

unsigned int udiv(int a, int b)
{
    unsigned int ret;
    __asm ("udiv %[res], %[div], %[fac]"
            : [res] "=r"  (ret)
            : [div] "r" (a), [fac] "r" (b)
            : );
    return ret;
}

void banner(void)
{

printf(
    "\n"
    "                  _____ _____       _____ ____  \n"
    " /A__/A  /A  /A  / ___//_  _//A    / ___// __ A \n"
    " V  __ A V A_V A V___A  / / / /_  / ___A A __ / \n"
    "  V_A V_A V____//____/  V/  V___A V____A V   V  HYPERM\n"
    "\n");

}

int main(void)
{
    int a = 3, b = 13, x = 3;
    banner();
    info();
    printf("sum = %d\n", func(a, b));
    printf("PSTATE: 0x%016x\n", pstate());
    printf("factorial of %d: %d\n", x, factorial(x));
    printf("%d / %d = %d\n", b, a, udiv(b, a));
    printf("x=%d, y=%d\n", 0, debug(0));
    printf("x=%d, y=%d\n", 2, debug(2));
    return 0;
}
