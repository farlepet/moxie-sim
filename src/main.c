#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

#include <cpu.h>

#define RAM_SIZE    0x10000 // 32K
#define ENTRY_POINT 0x1000

/*
 * Arguments:
 *   moxie_sim [binary1] [addr1] <binary2> <loc2> ...
       binary: Binary file to load
       loc:    Address at which to load the file
 */

int main(int argc, char **argv) {
    printf("moxie_sim\n");

    moxie_cpu_t *mcpu = (moxie_cpu_t *)malloc(sizeof(moxie_cpu_t));
    moxie_cpu_init(mcpu, RAM_SIZE);

    if(argc < 3) {
        fprintf(stderr, "You must specify at least one binary file and address!\n");
        return 1;
    }

    int i = 1;

    while(i < argc - 1) {
        char *filename = argv[i++];
        int fd = open(filename, O_RDONLY);

        if(fd < 0) {
            fprintf(stderr, "Could not open file %s!\n", filename);
            return 0;
        }

        uint32_t addr = (uint32_t)strtoul(argv[i++], NULL, 0);

        read(fd, mcpu->ram + addr, RAM_SIZE - addr);
        close(fd);
    }

    /*if(argc > 1) {
        char *filename = argv[1];
        int fd = open(filename, O_RDONLY);
        read(fd, mcpu->ram + ENTRY_POINT, RAM_SIZE - ENTRY_POINT);
        close(fd);
    }*/

    mcpu->pc = ENTRY_POINT;
    mcpu->regs[REG_SP] = 0x100; // Dummy stack pointer for testing
    mcpu->regs[REG_FP] = 0x100;

    for (i = 0; i < 512; i++) {
        moxie_cpu_step(mcpu);
    }

    printf("-- Simulation complete --\n");

    SET_LONG(mcpu->ram, 0x10, 0xFEDBCA98);

    for (i = 0; i < 16; i++)
    {
        if(i % 4 == 0) printf("0x%04X: ", i * 4);
        printf("0x%08X ", GET_LONG(mcpu->ram, i * 4));
        if(i % 4 == 3) printf("\n");
    }

    return 0;
}
