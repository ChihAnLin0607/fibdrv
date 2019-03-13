#ifdef DEV_FIBONACCI_NAME
#include <linux/kernel.h>
#else
#include <stdio.h>
#endif

#define MAX_LENGTH 186

struct BigN {
    unsigned long long lower;
    unsigned long long upper;
};

static inline void printBigN(struct BigN n)
{
#ifdef DEV_FIBONACCI_NAME
    printk(KERN_INFO "0x%016llX %016llX", n.upper, n.lower);
#else
    printf("0x%016llX %016llX", n.upper, n.lower);
#endif
}

static inline void shift_l_BigN(struct BigN *output, struct BigN x)
{
    output->upper = (x.upper << 4) + (x.lower >> 60);
    output->lower = x.lower << 4;
}

static inline void addBigN(struct BigN *output, struct BigN x, struct BigN y)
{
    output->upper = x.upper + y.upper;
    if (y.lower > ~x.lower)
        output->upper++;
    output->lower = x.lower + y.lower;
}

static inline short getDigit(struct BigN x, short i)
{
    if (i < 16)
        return (x.lower >> i * 4) & 0xf;
    else
        return (x.upper >> (i - 16) * 4) & 0xf;
}

static inline void minusBigN(struct BigN *output, struct BigN x, struct BigN y)
{
    if (x.lower < y.lower) {
        output->lower = (-1 - y.lower) + 1 + x.lower;
        x.upper--;
    } else
        output->lower = x.lower - y.lower;
    output->upper = x.upper - y.upper;
}

static inline void multiBigN(struct BigN *output, struct BigN x, struct BigN y)
{

    int i = 0, j = 0;
    short digit;
    output->lower = 0;
    output->upper = 0;

    for (i = 0; i < 32; i++) {
        digit = getDigit(y, i);
        if (digit) {
            j = 0;
            while (j++ < digit)
                addBigN(output, *output, x);
        }
        shift_l_BigN(&x, x);
    }
}
