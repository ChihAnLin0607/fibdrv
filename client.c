#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include "bigN.h"
#include "common.h"

#define FIB_DEV "/dev/fibonacci"

int main()
{
    int fd;
    long kernel_time;

    struct BigN buf;
    char write_buf[] = "testing writing";
    int i = 0;

    fd = open(FIB_DEV, O_RDWR);

    if (fd < 0) {
        perror("Failed to open character device");
        exit(1);
    }

    struct timespec start, end;
    long kernel_time_all = 0;
    printf(
        "|n|\t|user-space 時間差|\t|kernel "
        "計算所花時間|\t|kernel傳遞到userspace時間開銷|\n");
    for (i = 0; i <= MAX_LENGTH; i += 5) {
        lseek(fd, i, SEEK_SET);
        buf.lower = 0;
        buf.upper = 0;

        clock_gettime(CLOCK_MONOTONIC, &start);
        kernel_time = read(fd, &buf, sizeof(struct BigN));
        clock_gettime(CLOCK_MONOTONIC, &end);

        printf("%3d\t\t%ld\t\t\t%ld\t\t\t\t%ld ", i,
               end.tv_nsec - start.tv_nsec, kernel_time,
               end.tv_nsec - start.tv_nsec - kernel_time);
        //    printBigN(buf);
        printf("\n");
        kernel_time_all += kernel_time;
    }
#ifdef PERFORMACE_TRACE
    lseek(fd, MAX_LENGTH + 1, SEEK_SET);
    long multi_time = read(fd, NULL, 0);
    printf("multi_time = %ld\n", multi_time);
    printf("kernel_all_time = %ld\n", kernel_time_all);
    printf("multi_time / kernel_all_time = %f\n",
           (double) multi_time / kernel_time_all);
#endif
    /*
    struct BigN x = {{0xFFFFFF, 0x0, 0x0, 0x0, 0x0, 0x0}};
    struct BigN y = {{0x1, 0, 0, 0}};
    struct BigN ret;
    multiBigN(&ret, x, x);
    printBigN(ret);
    printf("\n");*/
    close(fd);
    return 0;
}
