#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <instructions.h>
#include <cpu.h>

static char *reg_names[MOXIE_REGS] __attribute__((unused)) = {
	"FP", "SP",
	"R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7", "R8", "R9", "R10", "R11", "R12", "R13"
};

void moxie_cpu_init(moxie_cpu_t *cpu, int mem) {
    memset(cpu->regs, 0, sizeof(cpu->regs));
    memset(cpu->spec_regs, 0, sizeof(cpu->spec_regs));
    cpu->ram = malloc(mem);
}

void moxie_cpu_step(moxie_cpu_t *cpu) {
    uint16_t op = GET_WORD(cpu->ram, cpu->pc);  //(uint16_t)GET_BYTE(cpu->ram, cpu->pc) << 8 | (uint16_t)GET_BYTE(cpu->ram, cpu->pc + 1);

    int inc_pc = 2; // Number of bytes to increment PC by

    if(op & 0x8000) {
        if(op & 0x4000) { // 6-bit branch instruction identifier
            int branch = 0; // Whether or not to branch
            int add = (op & 0x3FF) << 1; // Value to add to PC
            if(op & 0x200) { add = (add & 0x1FF) - (add & 0x200); }


            switch (op >> 10) {
                case 0b110000: // BEQ
                    branch = (cpu->cmp_res & CMP_EQ);
                    break;

                case 0b110110: // BGE
                    branch = (cpu->cmp_res & CMP_GE);
                    break;

                case 0b111000: // BGEU
                    branch = (cpu->cmp_res & CMP_GEU);
                    break;

                case 0b110011: // BGT
                    branch = (cpu->cmp_res & CMP_GT);
                    break;

                case 0b110101: // BGTU
                    branch = (cpu->cmp_res & CMP_GTU);
                    break;

                case 0b110111: // BLE
                    branch = (cpu->cmp_res & CMP_LE);
                    break;

                case 0b111001: // BLEU
                    branch = (cpu->cmp_res & CMP_LEU);
                    break;

                case 0b110010: // BLT
                    branch = (cpu->cmp_res & CMP_LT);
                    break;

                case 0b110100: // BLTU
                    branch = (cpu->cmp_res & CMP_LTU);
                    break;

                case 0b110001: // BNE
                    branch = !((cpu->cmp_res & CMP_EQ) != 0);
                    break;

				default:
					cpu->spec_regs[SR_EXCEP_VAL] = EXCEP_ILL;
					break;
            }

            if(branch != 0) {
                inc_pc = add;
            }
        }
        else
        { // 4-bit instruction identifier
            uint8_t reg = (op >> 8) & 0xF;
            uint8_t val = op & 0xFF;

            switch (op >> 12)
            {
                case 0b1001: // DEC
                    cpu->regs[reg] -= val;
                    break;

                case 0b1010: // GSR
                    cpu->regs[reg] = cpu->spec_regs[val];
                    break;

                case 0b1000: // INC
                    cpu->regs[reg] += val;
                    break;

                case 0b1011: // SSR
                    cpu->spec_regs[val] = cpu->regs[reg];
                    break;

				default:
					cpu->spec_regs[SR_EXCEP_VAL] = EXCEP_ILL;
					break;
            }
        }
    } else { // 8-bit instruction identifier
        uint8_t reg_a = (op >> 4) & 0xF;
        uint8_t reg_b = op & 0xF;

        int32_t *A = &cpu->regs[reg_a];
        int32_t *B = &cpu->regs[reg_b];

        int32_t a = *A;
        int32_t b = *B;

        switch (op >> 8)
        {
            case 0b00100110: // AND
                *A = a & b;
                break;

            case 0b00000101: // ADD
                *A = a + b;
                break;

            case 0b00101000: // ASHL
                *A = a << b;
                break;

            case 0b00110101: // BRK - Breakpoint
                // TODO: Implement!
                break;

            case 0b00101101: // ASHR
                *A = a >> b;
                break;

            case 0b00001110: // CMP
                moxie_cmp_instr(cpu, op);
                break;

            case 0b00110001: // DIV
                if(b == 0) { cpu->spec_regs[SR_EXCEP_VAL] = EXCEP_DIV0; break; }
                if(b == -1 && a == INT32_MIN)
                    *A = INT32_MIN; // Special case
                else
                    *A = a / b;
                break;

            case 0b00100101: // JMP
                cpu->pc = (uint32_t)a;
                inc_pc = 0;
                break;

            case 0b00011010: // JMPA
                cpu->pc = GET_LONG(cpu->ram, cpu->pc + 2);
                inc_pc = 0;
                break;

            case 0b00011001: // JSR
				cpu->regs[REG_SP] -= 4; // Static chain
                CPU_PUSH(cpu, cpu->pc + 2);
                CPU_PUSH(cpu, cpu->regs[REG_FP]);

                cpu->regs[REG_FP] = cpu->regs[REG_SP];
                cpu->pc = a;
                inc_pc = 0;
                break;

            case 0b00000011: // JSRA
				cpu->regs[REG_SP] -= 4; // Static chain
                CPU_PUSH(cpu, cpu->pc + 6);
                CPU_PUSH(cpu, cpu->regs[REG_FP]);

                cpu->regs[REG_FP] = cpu->regs[REG_SP];
                cpu->pc = GET_LONG(cpu->ram, cpu->pc + 2);
                inc_pc = 0;
                break;

            case 0b00011100: // LD.b
                *A = GET_BYTE(cpu->ram, b);
                break;

            case 0b00001010: // LD.l
                *A = GET_LONG(cpu->ram, b);
                break;

            case 0b00100001: // LD.s
                *A = GET_WORD(cpu->ram, b);
                break;

            case 0b00011101: // LDA.b
                *A = GET_BYTE(cpu->ram, GET_LONG(cpu->ram, cpu->pc + 2));
                inc_pc = 6;
                break;

            case 0b00001000: // LDA.l
                *A = GET_LONG(cpu->ram, GET_LONG(cpu->ram, cpu->pc + 2));
                inc_pc = 6;
                break;

            case 0b00100010: // LDA.s
                *A = GET_WORD(cpu->ram, GET_LONG(cpu->ram, cpu->pc + 2));
                inc_pc = 6;
                break;

            case 0b00000001: // LDI.l
                *A = GET_LONG(cpu->ram, cpu->pc + 2);
                inc_pc = 6;
                break;

            case 0b00011011: // LDI.b
                *A = GET_BYTE(cpu->ram, cpu->pc + 2);
                inc_pc = 6;
                break;

            case 0b00100000: // LDI.s
                *A = GET_WORD(cpu->ram, cpu->pc + 2);
                inc_pc = 6;
                break;

            case 0b00110110: // LDO.b
                *A = GET_BYTE(cpu->ram, b + GET_SWORD(cpu->ram, cpu->pc + 2));
                inc_pc = 4;
                break;

            case 0b00001100: // LDO.l
                *A = GET_LONG(cpu->ram, b + GET_SWORD(cpu->ram, cpu->pc + 2));
                inc_pc = 4;
                break;

            case 0b00111000: // LDO.s
                *A = GET_WORD(cpu->ram, b + GET_SWORD(cpu->ram, cpu->pc + 2));
                inc_pc = 4;
                break;

            case 0b00100111: // LSHR
                *A = ((uint32_t)a) << b;
                break;

            case 0b00110011: // MOD
                *A = a % b;
                break;

            case 0b00000010: // MOV
                *A = b;
                break;

            case 0b00101111: // MUL
                *A = (uint32_t)a * (uint32_t)b;
                break;

            case 0b00010101: // MUL.x
                *A = (int32_t)(((int64_t)a * (int64_t)b) >> 32);
                break;

            case 0b00001111: // NOP
                break;

            case 0b00101100: // NOT
                *A = ~b;
                break;

            case 0b00101011: // OR
                *A = a | b;
                break;

            case 0b00000111: // POP
                *A = GET_LONG(cpu->ram, b);
                *B = b + 4;
                break;

            case 0b00000110: // PUSH
                *A = a - 4;
                SET_LONG(cpu->ram, a, b);
                break;

            case 0b00000100: // RET
				cpu->regs[REG_SP] = cpu->regs[REG_FP];
                cpu->regs[REG_FP] = CPU_POP(cpu); // Pop frame pointer
                cpu->pc = CPU_POP(cpu);
				cpu->regs[REG_SP] += 4; // Static chain
                inc_pc = 0;
                break;

            case 0b00010000: // SEX.b
                *A = (int32_t)(int8_t)(uint8_t)b;
                break;

            case 0b00010001: // SEX.s
                *A = (int32_t)(int16_t)(uint16_t)b;
                break;

            case 0b00011110: // ST.b
                SET_BYTE(cpu->ram, a, b);
                break;

            case 0b00001011: // ST.l
                SET_LONG(cpu->ram, a, b);
                break;

            case 0b00100011: // ST.s
                SET_WORD(cpu->ram, a, b);
                break;

            case 0b00011111: // STA.b
                SET_BYTE(cpu->ram, GET_LONG(cpu->ram, cpu->pc + 2), a);
                inc_pc = 6;
                break;

            case 0b00001001: // STA.l
                SET_LONG(cpu->ram, GET_LONG(cpu->ram, cpu->pc + 2), a);
                inc_pc = 6;
                break;

            case 0b00100100: // STA.s
                SET_WORD(cpu->ram, GET_LONG(cpu->ram, cpu->pc + 2), a);
                inc_pc = 6;
                break;

            case 0b00110111: // STO.b
                SET_BYTE(cpu->ram, a + GET_SWORD(cpu->ram, cpu->pc + 2), a);
                inc_pc = 4;
                break;

            case 0b00001101: // STO.l
                SET_LONG(cpu->ram, a + GET_SWORD(cpu->ram, cpu->pc + 2), b);
                inc_pc = 4;
                break;

            case 0b00111001: // STO.s
                SET_WORD(cpu->ram, a + GET_SWORD(cpu->ram, cpu->pc + 2), b);
                inc_pc = 4;
                break;

            case 0b00101001: // SUB
                *A = a - b;
                break;

            case 0b00110000: // SWI
				cpu->spec_regs[SR_SWI_N] = GET_LONG(cpu->ram, cpu->pc + 2);
				inc_pc = 6;
                // TODO: Software interrupt
                break;

            case 0b00110010: // UDIV
                *A = (uint32_t)a / (uint32_t)b;
                break;

            case 0b00110100: // UMOD
                *A = (uint32_t)a % (uint32_t)b;
                break;

            case 0b00010100: // UMUL.x
                *A = (int32_t)(((uint64_t)a * (uint64_t)b) >> 32);
                break;

            case 0b00101110: // XOR
                *A = a ^ b;
                break;

            case 0b00010010: // ZEX.b
                *A = b & 0xFF;
                break;

            case 0b00010011: // ZEX.s
                *A = b & 0xFFFF;
                break;

			default:
				cpu->spec_regs[SR_EXCEP_VAL] = EXCEP_ILL;
				break;
        }
    }

    if(cpu->spec_regs[SR_EXCEP_VAL] != EXCEP_NONE) {
		// TODO: Call lambda-os functions based on value.
	}

    cpu->pc += inc_pc;
}
