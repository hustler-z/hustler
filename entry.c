#include <stdio.h>

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

unsigned int udiv(int a, int b)
{
    unsigned int ret;
    __asm ("udiv %[res], %[div], %[fac]"
            : [res] "=r"  (ret)
            : [div] "r" (a), [fac] "r" (b)
            : );
    return ret;
}

int main(void)
{
    int a = 3, b = 13, x = 3;
    info();

    printf("sum = %d\n", func(a, b));
    printf("PSTATE: 0x%016x\n", pstate());
    printf("factorial of %d: %d\n", x, factorial(x));
    printf("%d / %d = %d\n", b, a, udiv(b, a));
  
    return 0;
}

