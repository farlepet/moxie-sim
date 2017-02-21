#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

#include <cpu.h>

#define RAM_SIZE    0x10000 // 32K
#define ENTRY_POINT 0x1000

int main(int argc, char **argv) {
    printf("moxie_sim\n");

    moxie_cpu_t *mcpu = (moxie_cpu_t *)malloc(sizeof(moxie_cpu_t));
    moxie_cpu_init(mcpu, RAM_SIZE);

    if(argc > 1) {
        char *filename = argv[1];
        int fd = open(filename, O_RDONLY);
        read(fd, mcpu->ram + ENTRY_POINT, RAM_SIZE - ENTRY_POINT);
        close(fd);
    }

    mcpu->pc = ENTRY_POINT;

    int i = 0;
    for (; i < 32; i++) {
        moxie_cpu_step(mcpu);
    }

    printf("-- Simulation complete --\n");

    for (i = 0; i < 16; i++)
    {
        if(i % 4 == 0) printf("0x%04X: ", i * 4);
        printf("0x%08X ", GET_LONG(mcpu->ram, i * 4));
        if(i % 4 == 3) printf("\n");
    }

    return 0;
}