#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#define FIB_DEV "/dev/fibonacci"

int main()
{
    int fd;
    long long sz;

    unsigned int kernel_time;
    char write_buf[] = "testing writing";
    int offset = 92;  // TODO: test something bigger than the limit
    int i = 0;

    fd = open(FIB_DEV, O_RDWR);

    if (fd < 0) {
        perror("Failed to open character device");
        exit(1);
    }


    struct timespec start, end;
    printf(
        "|n|\t|user-space 時間差|\t|kernel "
        "計算所花時間|\t|kernel傳遞到userspace時間開銷|\n");
    for (i = 0; i <= offset; i++) {
        lseek(fd, i, SEEK_SET);

        clock_gettime(CLOCK_MONOTONIC, &start);
        sz = read(fd, &kernel_time, sizeof(unsigned int));
        clock_gettime(CLOCK_MONOTONIC, &end);
        printf("%3d\t\t%ld\t\t\t%u\t\t\t\t%ld ", i, end.tv_nsec - start.tv_nsec,
               kernel_time, end.tv_nsec - start.tv_nsec - kernel_time);
        printf("%lld", sz);
        printf("\n");
    }

    close(fd);
    return 0;
}
