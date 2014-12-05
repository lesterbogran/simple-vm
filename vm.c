#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "irr.h"
#include "irm.h"
#include "helpers.h"
#include "smart-compile.h"

#define MEMSIZE (64000)
#define TEXT_SECTION_START (200)
#define TEXT_SECTION_SIZE (2500)
#define DATA_SECTION_START (TEXT_SECTION_START + TEXT_SECTION_SIZE + 1)
#define DATA_SECTION_SIZE (8000)
#define HEAP_STACK_SECTION_START (DATA_SECTION_START + DATA_SECTION_SIZE + 1)
#define HEAP_STACK_SECTION_STOP (MEMSIZE - 1)

register_t registers[REGISTER_COUNT];
unsigned int memspace[MEMSIZE];
int program_halted = 0;

void execute(unsigned int ins);

void dump_registers(void) {
    char *str, *imm_str;
    for (int i = 0; i < REGISTER_COUNT; ++i) {
        imm_str = imm_to_str(registers[i], "%d");
        str = str_cat(3, reg_to_str(i), ": ", imm_str);
        puts(str);
        free(str);
        free(imm_str);
    }
}

void dump_text_section(void) {
    int col = 0;
    for (size_t i = 0; i < TEXT_SECTION_SIZE; ++i, ++col) {
        if (col == 11) {
            printf("\n");
            col = 0;
        }
        printf(" 0x%08x ", memspace[TEXT_SECTION_START + i]);
    }
    printf("\n");
}

void dump_data_section(void) {
    int col = 0;
    unsigned int *tmp = &memspace[DATA_SECTION_START];
    unsigned int *end = &memspace[DATA_SECTION_START + DATA_SECTION_SIZE];
    char data, *start = (char*)tmp;
    for (; (unsigned int*)start != end; ++start, ++col) {
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

void disassemble_program(unsigned int *program, size_t program_size) {
    for (size_t i = 0; i < program_size; ++i) {
        print_dissassembly(program[i]);
    }
}

void load_program(unsigned int *program, size_t program_size) {
    if (program_size > TEXT_SECTION_SIZE) {
        fputs("Program size too big.\n", stderr);
        exit(1);
    }
    for (size_t i = 0; i < program_size; ++i) {
        memspace[TEXT_SECTION_START + i] = program[i];
    }
}

void load_data(char *data, size_t data_size) {
    if (data_size > DATA_SECTION_SIZE) {
        fputs("Data size too big.\n", stderr);
        exit(1);
    }
    unsigned int *tmp = &memspace[DATA_SECTION_START];
    char *start = (char*)tmp;
    char *end = (char*)tmp + data_size;
    for (; start != end; ++start, ++data) {
        *start = *data;
    }
}

void run(void) {
    size_t pc;
    while (program_halted == 0) {
        pc = registers[PC];
        execute(memspace[pc]);
    }
}

void init(void) {
    for (size_t i = 0; i < MEMSIZE; ++i) {
        memspace[i] = 0;
    }
    registers[PC] = TEXT_SECTION_START;
}


int main(void) {
    unsigned int *program = calloc(50, sizeof *program);
    int i = 0;
    /* Simple factorial(12) (Largest signed 32 bit) program. */
    program[i++] = smart_compile(LI, G0, 1);   /* Start */
    program[i++] = smart_compile(LI, G1, 5);   /* End */
    program[i++] = smart_compile(LI, G3, 1);   /* Product */
    program[i++] = smart_compile(GT, G0, G1); /* Check if G0 > G1. */
    program[i++] = smart_compile(JZS, NUL, 4); /* If compare succeeds, exit loop. */
    program[i++] = smart_compile(MUL, G3, G0); /* Mul the product by counter. */
    program[i++] = smart_compile(ADDI, G0, 1); /* Increment counter. */
    program[i++] = smart_compile(JS, NUL, -4); /* Jump to compare */
    program[i++] = HALT;
    init();
    load_program(program, i);

    char *test_data = "Hello world!!";
    load_data(test_data, strlen(test_data)+1);

    //dump_text_section();
    //dump_data_section();
    disassemble_program(program, i);
    printf("\n");

    free(program);
    run();
    dump_registers();
    return 0;
}

void execute(unsigned int ins) {
    opcode_t op;
    register_t r1, r2;
    int imm;

    op = get_opcode(ins);
    r1 = get_r1(ins);
    r2 = get_r2(ins);
    imm = get_imm(ins);


    switch (op) {
        case OPCODE_COUNT:
        default:
            fputs("'NOT AN OPCODE'", stderr);
            abort();
        case HALT:
            program_halted = 1;
            break;
        case NOP:
            registers[PC]++;
            break;

        case ADD:
            registers[r1] += registers[r2];
            registers[PC]++;
            break;
        case MUL:
            registers[r1] *= registers[r2];
            registers[PC]++;
            break;
        case DIV:
            registers[r1] /= registers[r2];
            registers[PC]++;
            break;
        case EQ:
            registers[Z] = registers[r1] == registers[r2];
            registers[PC]++;
            break;
        case NE:
            registers[Z] = registers[r1] != registers[r2];
            registers[PC]++;
            break;
        case LT:
            registers[Z] = registers[r1] < registers[r2];
            registers[PC]++;
            break;
        case LE:
            registers[Z] = registers[r1] <= registers[r2];
            registers[PC]++;
            break;
        case GT:
            registers[Z] = registers[r1] > registers[r2];
            registers[PC]++;
            break;
        case GE:
            registers[Z] = registers[r1] >= registers[r2];
            registers[PC]++;
            break;
        case AND:
            registers[r1] &= registers[r2];
            registers[PC]++;
            break;
        case OR:
            registers[r1] |= registers[r2];
            registers[PC]++;
            break;
        case XOR:
            registers[r1] ^= registers[r2];
            registers[PC]++;
            break;
        case SLL:
            registers[r1] <<= registers[r2];
            registers[PC]++;
            break;
        case SRL:
            registers[r1] >>= registers[r2];
            registers[PC]++;
            break;
        case MOV:
            registers[r1] = registers[r2];
            registers[PC]++;
            break;
        case LW:
            registers[r1] = memspace[registers[r2]];
            registers[PC]++;
            break;
        case SW:
            memspace[registers[r2]] = registers[r1];
            registers[PC]++;
            break;

        case ADDI:
            registers[r1] += imm;
            registers[PC]++;
            break;
        case MULI:
            registers[r1] *= imm;
            registers[PC]++;
            break;
        case DIVI:
            registers[r1] /= imm;
            registers[PC]++;
            break;
        case LI:
            registers[r1] = imm;
            registers[PC]++;
            break;

        case JR:
            registers[PC] = registers[r1];
            break;
        case PUSH:
            memspace[registers[SP]] = registers[r1];
            registers[SP]--;
            registers[PC]++;
            break;
        case POP:
            registers[r1] = memspace[registers[SP]];
            registers[SP]++;
            registers[PC]++;
            break;

        case J:
            registers[PC] = imm;
            break;
        case JS:
            registers[PC] += imm;
            break;
        case JZ:
            if (registers[Z]) {
                registers[PC] = imm;
                registers[Z] = 0;
            }
            else {
                registers[PC]++;
            }
            break;
        case JZS:
            if (registers[Z]) {
                registers[PC] += imm;
                registers[Z] = 0;
            }
            else {
                registers[PC]++;
            }
            break;
        case CALL:
            /* Push return address to stack */
            memspace[registers[SP]] = registers[PC] + 1;
            /* Save stack pointer to return address in frame pointer. */
            registers[FP] = registers[SP];
            registers[SP]--;
            registers[PC] = imm;
            break;
        case PUSHI:
            memspace[registers[SP]] = imm;
            registers[SP]--;
            registers[PC]++;
            break;

    }
}


