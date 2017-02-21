#ifndef MOXIE_INSTRUCTIONS_H
#define MOXIE_INSTRUCTIONS_H

#include <cpu.h>

/**
 * CMP instruction
 */
static inline void moxie_cmp_instr(moxie_cpu_t *cpu, uint16_t op) {
    int reg_a = (op >> 4) & 0x0F;
    int reg_b = op & 0x0F;

    // Get register values
    int a = cpu->regs[reg_a];
    int b = cpu->regs[reg_b];

    if(a == b) { // Equal
        cpu->cmp_res = CMP_EQ;
    } else {
        // </> and unsigned </>
        cpu->cmp_res = ((a > b) ? CMP_GT : CMP_LT) | (((uint32_t)a > (uint32_t)b) ? CMP_GTU : CMP_LTU);
    }
}

#endif