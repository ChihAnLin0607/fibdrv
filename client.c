#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include "bigN.h"

#define FIB_DEV "/dev/fibonacci"

int main()
{
    int fd;
    long long sz;

    struct BigN buf;
    char write_buf[] = "testing writing";
    int offset = 1000;  // TODO: test something bigger than the limit
    int i = 0;

    fd = open(FIB_DEV, O_RDWR);

    if (fd < 0) {
        perror("Failed to open character device");
        exit(1);
    }
    /*
        for (i = 0; i <= offset; i++) {
            sz = write(fd, write_buf, strlen(write_buf));
            printf("Writing to " FIB_DEV ", returned the sequence %lld\n", sz);
        }
    */

    struct timespec start, end;
    for (i = 0; i <= MAX_LENGTH; i++) {
        lseek(fd, i, SEEK_SET);
        buf.lower = 0;
        buf.upper = 0;
        clock_gettime(CLOCK_MONOTONIC, &start);
        sz = read(fd, &buf, sizeof(struct BigN));
        clock_gettime(CLOCK_MONOTONIC, &end);
        /*        printf("Reading from " FIB_DEV
                       " at offset %d, returned the sequence "
                       "%lld.\n",
                       i, sz);
        */
        printf("i = %3d,\t %ld\tns, f[%d] =\t", i, end.tv_nsec - start.tv_nsec,
               i);
        printBigN(buf);
    }
    /*
        for (i = offset; i >= 0; i--) {
            lseek(fd, i, SEEK_SET);
            sz = read(fd, buf, 1);
            printf("Reading from " FIB_DEV
                   " at offset %d, returned the sequence "
                   "%lld.\n",
                   i, sz);
        }
    */
    close(fd);
    return 0;
}
