#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>

#include "vm.h"
#include "helpers.h"
#include "instruction.h"


ui32 registers[REGISTER_COUNT];
unsigned char memspace[MEMSIZE];
ui32 program_halted = 0;

void dump_registers(void) {
    char *str, *imm_str;
    for (ui32 i = 0; i < REGISTER_COUNT; ++i) {
        if (i >= F0 && i <= F3) {
            printf("%s: %f\n", reg_to_str(i), *((float*)registers + i));
        }
        else {
            imm_str = imm_to_str(registers[i], "%d");
            str = str_cat(3, reg_to_str(i), ": ", imm_str);
            puts(str);
            free(str);
            free(imm_str);
        }
    }
}

void dump_text_section(void) {
    ui32 col = 0;
    ui32 *start, *end;
    start = (ui32*)(memspace+TEXT_SECTION_START);
    end = (ui32*)(memspace+TEXT_SECTION_START + TEXT_SECTION_SIZE);
    for (col = 0; start != end; ++start, ++col) {
        if (col == 7) {
            printf("\n");
            col = 0;
        }
        printf(" %08x ", *start);
    }
    printf("\n");
}

void dump_data_section(void) {
    ui32 col = 0;
    unsigned char *tmp = &memspace[DATA_SECTION_START];
    unsigned char *end = &memspace[DATA_SECTION_START + DATA_SECTION_SIZE];
    unsigned char data, *start = tmp;
    for (; start != end; ++start, ++col) {
        data = *start;
        if (col == 19) {
            printf("\n");
            col = 0;
        }
        if (is_printable(data)) {
            printf("%2c ", data);
        }
        else {
            printf("%02x ", data);
        }
    }
    printf("\n");
}

void disassemble_program(unsigned char *program, size_t size) {
    for (size_t i = 0; i < size / sizeof (ui32); ++i) {
        print_dissassembly(((ui32*)program)[i]);
    }
}

void load_program(unsigned char *program, size_t size) {
    if (size > TEXT_SECTION_SIZE) {
        fputs("Program size too big.\n", stderr);
        exit(1);
    }
    memcpy(memspace+TEXT_SECTION_START, program, size);
}

void load_data(unsigned char *data, size_t size) {
    if (size > DATA_SECTION_SIZE) {
        fputs("Data size too big.\n", stderr);
        exit(1);
    }
    memcpy(memspace+DATA_SECTION_START, data, size);
}

void pprint_stack_helper(ui32 addr, ui32 offs, ui32 value) {
    printf(" - <stack@0x%x:%+d=0x%x>", addr, offs, value);
    if (offs == 0) {
        printf(" <~~ $sp");
    }
    printf("\n");
}
void pprint_stack(ui32 num) {
    ui32 *stack_addr;
    ui32 offs;
    stack_addr = ((ui32*)(memspace + num));
    printf("\n");
    for (int i = 5; i > 0; --i) {
        offs = i * sizeof(ui32);
        if (num+offs < MEMSIZE) {
            pprint_stack_helper(num+offs, offs, *(stack_addr+i));
        }
    }
    offs = 0 * sizeof(ui32);
    pprint_stack_helper(num+offs, offs, *(stack_addr+0));
    for (int i = -1; i >= -5; --i) {
        offs = i * sizeof(ui32);
        pprint_stack_helper(num+offs, offs, *(stack_addr+i));
    }
}

void pprint(char *oper, ui32 num) {
    printf("%s=0x%x", oper, num);
    if (50000 < num && num < MEMSIZE) {
        pprint_stack(num);
    }
    printf("\n");
}
void debug_disasm(ui32 ins) {
    instruction_t *instr = disassemble_instruction(ins);
    puts("===========================");
    puts(instr->disassembled_str);
    switch (instr->type) {
        default:
        case NO_OPERANDS:
            break;
        case REGISTER_REGISTER:
            pprint("oper1", registers[instr->operand1.reg]);
            pprint("oper2", registers[instr->operand2.reg]);
            break;
        case REGISTER_IMMEDIATE:
            pprint("oper1", registers[instr->operand1.reg]);
            pprint("oper2", instr->operand2.imm);
            break;
        case REGISTER_NO_IMMEDIATE:
            pprint("oper1", registers[instr->operand1.imm]);
            break;
        case IMMEDIATE_NO_REGISTER:
            pprint("oper1", instr->operand1.imm);
            break;
    }
    free_instruction(&instr);
}

void run(void) {
    size_t pc; 
    ui32 ins;
    while (program_halted == 0) {
        pc = registers[PC];
        ins = *((ui32*)(memspace+pc));
#ifdef DEBUG_DISASSEM
        debug_disasm(ins);
#endif
        execute(ins);
    }
}

void init(void) {
    memset(memspace, 0, MEMSIZE);
    memset(registers, 0, REGISTER_COUNT * sizeof (register_t));
    registers[PC] = TEXT_SECTION_START;
    registers[SP] = STACK_START;
    registers[FP] = STACK_START;
}

ui32 syscall(ui32 func);
void execute(ui32 ins) {
    opcode_t op;
    register_t r1, r2;
    ui32 imm16, imm21;

    op = get_opcode(ins);
    r1 = get_r1(ins);
    r2 = get_r2(ins);
    imm21 = get_imm21(ins);
    imm16 = get_imm16(ins);


    switch (op) {
        case OPCODE_COUNT:
        case NOT_AN_OPCODE:
            fputs("'NOT AN OPCODE'", stderr);
            abort();
          
        /* NO_OPERANDS */
        /* oooooo 00000000000000000000000000 */
        /*    6              26              */
        case HALT:
            program_halted = 1;
            break;
        case NOP:
            registers[PC] += 4;
            break;
        case RET:
            registers[SP] += 4;
            registers[PC] = *((ui32*)(memspace + registers[SP]));
            break;
        case SYSCALL:
            registers[R1] = syscall(registers[SYS]);
            registers[PC] += 4;
            break;

        /* REGISTER_REGISTER */
        /* oooooo rrrrr rrrrr 0000000000000000 */
        /*    6     5     5         16         */
        case ADD:
            registers[r1] += registers[r2];
            registers[PC] += 4;
            break;
        case MUL:
            registers[r1] *= registers[r2];
            registers[PC] += 4;
            break;
        case DIV:
            registers[r1] /= registers[r2];
            registers[PC] += 4;
            break;
        case EQ:
            registers[Z] = registers[r1] == registers[r2];
            registers[PC] += 4;
            break;
        case NE:
            registers[Z] = registers[r1] != registers[r2];
            registers[PC] += 4;
            break;
        case LT:
            registers[Z] = registers[r1] < registers[r2];
            registers[PC] += 4;
            break;
        case LE:
            registers[Z] = registers[r1] <= registers[r2];
            registers[PC] += 4;
            break;
        case GT:
            registers[Z] = registers[r1] > registers[r2];
            registers[PC] += 4;
            break;
        case GE:
            registers[Z] = registers[r1] >= registers[r2];
            registers[PC] += 4;
            break;
        case AND:
            registers[r1] &= registers[r2];
            registers[PC] += 4;
            break;
        case OR:
            registers[r1] |= registers[r2];
            registers[PC] += 4;
            break;
        case XOR:
            registers[r1] ^= registers[r2];
            registers[PC] += 4;
            break;
        case SLL:
            registers[r1] <<= registers[r2];
            registers[PC] += 4;
            break;
        case SRL:
            registers[r1] >>= registers[r2];
            registers[PC] += 4;
            break;
        case MOV:
            registers[r1] = registers[r2];
            registers[PC] += 4;
            break;


        /* REGISTER_REGISTER_OFFSET */
        /* oooooo rrrrr rrrrr mmmmmmmmmmmmmmmm */
        /*    6     5     5         16         */
        case LW:
            registers[r1] = *((ui32*)(imm16 + memspace + registers[r2]));
            registers[PC] += 4;
            break;
        case SW:
            *((ui32*)(imm16 + memspace + registers[r2])) = registers[r1];
            registers[PC] += 4;
            break;
        case LB:
            registers[r1] = *((ui8*)(imm16 + memspace + registers[r2]));
            registers[PC] += 4;
            break;
        case SB:
            *((ui8*)(imm16 + memspace + registers[r2])) = registers[r1];
            registers[PC] += 4;
            break;


        /* REGISTER_IMMEDIATE */
        /* oooooo rrrrr mmmmmmmmmmmmmmmmmmmmm */
        /*    6     5           21            */
        case ADDI:
            registers[r1] += imm21;
            registers[PC] += 4;
            break;
        case MULI:
            registers[r1] *= imm21;
            registers[PC] += 4;
            break;
        case DIVI:
            registers[r1] /= imm21;
            registers[PC] += 4;
            break;
        case LI:
            registers[r1] = imm21;
            registers[PC] += 4;
            break;

        /* REGISTER_NO_IMMEDIATE */
        /* oooooo rrrrr 000000000000000000000 */
        /*    6     5            21           */
        case JR:
            registers[PC] = registers[r1];
            break;
        case PUSH:
            *((ui32*)(memspace + registers[SP])) = registers[r1];
            registers[SP] -= 4;
            registers[PC] += 4;
            break;
        case POP:
            registers[SP] += 4;
            registers[r1] = *((ui32*)(memspace + registers[SP]));
            registers[PC] += 4;
            break;
        case PRINTS:
            printf("%s", ((char*)(memspace + registers[r1])));
            registers[PC] += 4;
            break;
        case PRINTI:
            printf("%d", registers[r1]);
            registers[PC] += 4;
            break;

        /* IMMEDIATE_NO_REGISTER */
        /* oooooo 00000 mmmmmmmmmmmmmmmmmmmmm  */
        /*    6                  21            */
        case J:
            registers[PC] = imm21;
            break;
        case JS:
            registers[PC] += imm21;
            break;
        case JZ:
            if (registers[Z]) {
                registers[PC] = imm21;
                registers[Z] = 0;
            }
            else {
                registers[PC] += 4;
            }
            break;
        case JZS:
            if (registers[Z]) {
                registers[PC] += imm21;
                registers[Z] = 0;
            }
            else {
                registers[PC] += 4;
            }
            break;
        case CALL:
            /* Push return address to stack */
            *((ui32*)(memspace + registers[SP])) = registers[PC] + 4;
            registers[SP] -= 4;
            registers[PC] = imm21;
            break;
        case PUSHI:
            *((ui32*)(memspace + registers[SP])) = imm21;
            registers[SP] -= 4;
            registers[PC] += 4;
            break;
        case PRINTC:
            printf("%c", imm21);
            registers[PC] += 4;
            break;
    }
}

typedef ui32(*func)(void);

func sys_funcs[] = {
    NULL,
    (func)clock,
};

ui32 syscall(ui32 func) {
    ui32 result = sys_funcs[func]();
    return result;
}
