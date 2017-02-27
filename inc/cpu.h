#ifndef MOXIE_CPU_H
#define MOXIE_CPU_H

#include <stdint.h>

#define MOXIE_REGS 16 ///< Number of registers
#define MOXIE_SPEC_REGS 256 ///< Number of special registers

enum moxie_reg {
    REG_FP = 0, // Frame Pointer
    REG_SP, // Stack Pointer

    // General Purpose:
    REG_R0,
    REG_R1,
    REG_R2,
    REG_R3,
    REG_R4,
    REG_R5,
    REG_R6,
    REG_R7,
    REG_R8,
    REG_R9,
    REG_R10,
    REG_R11,
    REG_R12,
    REG_R13,

};

enum moxie_spec_reg {
    SR_STATUS = 0,
    SR_EXCEP_PTR, // Not used in this case
    SR_EXCEP_VAL,
    SR_SWI_N, // Syscall number
    SR_SUPER_STACK, // Not used in this case
    SR_EXCEP_RET_ADDR, // Not used in this case
    SR_RESERVED_0,
    SR_RESERVED_1,
    SR_RESERVED_2,
    SR_DEV_TREE, // Not used in this case

	SR_SYSCALL_START = 0x80 // Special registers 0x80 and up act as syscall arguments
};

enum moxie_exceptions {
	EXCEP_NONE = 0, // Everything is fine
	EXCEP_IRQ  = 1, // IRQ, not used here
	EXCEP_DIV0 = 2, // Divide by zero
	EXCEP_ILL  = 3, // Illegal instruction
	EXCEP_SWI  = 4 // Really just syscalls
};

enum cmp_flags
{
    CMP_EQ = 0x01,
    CMP_LT = 0x02,
    CMP_GT = 0x04,
    CMP_LTU = 0x08,
    CMP_GTU = 0x10,

    CMP_LE = CMP_LT | CMP_EQ,
    CMP_LEU = CMP_LTU | CMP_EQ,
    CMP_GE = CMP_GT | CMP_EQ,
    CMP_GEU = CMP_GTU | CMP_EQ,
};

#define GET_BYTE(R, A) (*(uint8_t  *)(R + A))

#define GET_WORD(R, A) ((uint16_t)GET_BYTE(R, A) << 8 | (uint16_t)GET_BYTE(R, A + 1))
#define GET_LONG(R, A) ((uint32_t)GET_WORD(R, A) << 16 | (uint32_t)GET_WORD(R, A + 2))



#define GET_SWORD(R, A) (int16_t)GET_WORD(R, A)

#define SET_BYTE(R, A, V) *(uint8_t  *)(R + A) = (V)

#define SET_WORD(R, A, V) do { SET_BYTE((R), (A), (uint8_t)((V) >> 8)); SET_BYTE((R), A + 1, (uint8_t)(V)); } while(0)
#define SET_LONG(R, A, V) do { SET_WORD((R), (A), (uint16_t)((V) >> 16)); SET_WORD((R), A + 2, (uint16_t)(V)); } while(0)

//#define CPU_PUSH(C, V) do { C->regs[REG_SP] -= 4; SET_LONG(C->ram, C->regs[REG_SP], V); } while(0)
//#define CPU_POP(C, V) do { V = GET_LONG(C->ram, C->regs[REG_SP]); C->regs[REG_SP] += 4; } while(0)


typedef struct moxie_cpu {
    int32_t regs[MOXIE_REGS]; ///< Registers
    int32_t spec_regs[MOXIE_SPEC_REGS]; ///< Special registers

    uint32_t pc; ///< Program counter

    void *ram; ///< Memory

    int cmp_res; ///< Result of CMP instruction
} moxie_cpu_t;

void moxie_cpu_init(moxie_cpu_t *cpu, int mem);

void moxie_cpu_step(moxie_cpu_t *cpu);

static inline void CPU_PUSH(moxie_cpu_t *cpu, uint32_t value) {
    cpu->regs[REG_SP] -= 4;
	//printf(" {PUSH %08X %08X}", cpu->regs[REG_SP], value);
    SET_LONG(cpu->ram, cpu->regs[REG_SP], value);
}

static inline uint32_t CPU_POP(moxie_cpu_t *cpu) {
    uint32_t ret = GET_LONG(cpu->ram, cpu->regs[REG_SP]);
	//printf(" {POP %08X %08X}", cpu->regs[REG_SP], ret);
    cpu->regs[REG_SP] += 4;
    return ret;
}

static inline void show_stack(moxie_cpu_t *cpu) {
    printf(" {STK: ");
    for (int i = 0; i < 4; i++)
    {
        printf("%08X ", GET_LONG(cpu->ram, cpu->regs[REG_SP] - (i * 4)));
    }
    printf("}");
}

#endif
