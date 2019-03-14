#ifdef DEV_FIBONACCI_NAME
#include <linux/kernel.h>
#else
#include <stdio.h>
#endif

#define MAX_LENGTH 186
#define BIGN_PART_COUNT 6
#define BIGN_BIT_EACH_PART 24

struct BigN {
    unsigned long long num_part[BIGN_PART_COUNT];
    unsigned long long lower;
    unsigned long long upper;
};

static inline void printBigN(struct BigN n)
{
    int i = 0;
#ifdef DEV_FIBONACCI_NAME
    printk(KERN_INFO "0x");
    while (i < BIGN_PART_COUNT)
        printk(KERN_INFO "%06llX ", n.num_part[BIGN_PART_COUNT - 1 - i++]);
    printk(KERN_INFO "\n");
#else
    printf("0x");
    while (i < BIGN_PART_COUNT)
        printf("%06llX ", n.num_part[BIGN_PART_COUNT - 1 - i++]);
    printf("\n");
#endif
}

static inline void shift_l_BigN(struct BigN *output, struct BigN x)
{
    output->upper = (x.upper << 4) + (x.lower >> 60);
    output->lower = x.lower << 4;
}

static inline void addBigN(struct BigN *output, struct BigN x, struct BigN y)
{
    int i = 0;
    while (i < BIGN_PART_COUNT)
        output->num_part[i++] = 0;
    i = 0;
    while (i < BIGN_PART_COUNT) {
        output->num_part[i] =
            output->num_part[i] + x.num_part[i] + y.num_part[i];
        if (i < BIGN_BIT_EACH_PART &&
            output->num_part[i] >
                ((unsigned long long) ~0 >> (64 - BIGN_BIT_EACH_PART))) {
            output->num_part[i] -=
                ((unsigned long long) 1 << BIGN_BIT_EACH_PART);
            output->num_part[i + 1]++;
        }
        i++;
    }
    /*
        output->upper = x.upper + y.upper;
        if (y.lower > ~x.lower)
            output->upper++;
        output->lower = x.lower + y.lower;
    */
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
    int i = 0, j;
    while (i < BIGN_PART_COUNT) {
        output->num_part[i] = x.num_part[i] - y.num_part[i];
        if (x.num_part[i] < y.num_part[i]) {
            output->num_part[i] &= 0xFFFFFF;
            j = i + 1;
            while (j < BIGN_PART_COUNT) {
                x.num_part[j]--;
                if (x.num_part[j] + 1)
                    break;
                x.num_part[j] &= 0xFFFFFF;
                j++;
            }
        }

        i++;
    }
    /*    if (x.lower < y.lower) {
            output->lower = (-1 - y.lower) + 1 + x.lower;
            x.upper--;
        } else
            output->lower = x.lower - y.lower;
        output->upper = x.upper - y.upper;
    */
}

static inline void multiBigN(struct BigN *output, struct BigN x, struct BigN y)
{
    int xi, yi;
    for (xi = 0; xi < BIGN_PART_COUNT; xi++)
        output->num_part[xi] = 0;

    for (yi = 0; yi < BIGN_PART_COUNT; yi++) {
        if (!y.num_part[yi])
            continue;
        for (xi = 0; xi < BIGN_PART_COUNT - yi; xi++) {
            if (!x.num_part[xi])
                continue;
            output->num_part[xi + yi] += x.num_part[xi] * y.num_part[yi];
#ifndef DEV_FIBONACCI_NAME
            printf("xi = %d, yi = %d, output->num_part[%d] = 0x%llX\n", xi, yi,
                   xi + yi, output->num_part[xi + yi]);
#endif
        }
    }

    for (xi = 0; xi < BIGN_PART_COUNT - 1; xi++) {
        if (output->num_part[xi] & 0xFFFFFFFFFF000000) {
            output->num_part[xi + 1] +=
                (output->num_part[xi] & 0xFFFFFFFFFF000000) >>
                BIGN_BIT_EACH_PART;
            output->num_part[xi] &= 0xFFFFFF;
        }
    }

    /*
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
    */
}
