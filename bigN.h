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

static inline void addBigN(struct BigN *output, struct BigN x, struct BigN y)
{
    output->upper = x.upper + y.upper;
    if (y.lower > ~x.lower)
        output->upper++;
    output->lower = x.lower + y.lower;
}
