#ifndef SIMPLE_VM_VM_INC_INSTRUCTION_H
#define SIMPLE_VM_VM_INC_INSTRUCTION_H

#include <stdint.h>

#include "opdefs.h"
#include "typedefs.h"

#define INSTRUCTION_SIZE (sizeof(i32))

typedef enum {
    INVALID_INSTRUCTION_TYPE = 0,
    /* oooooo 00000000000000000000000000 */
    /*    6              26              */
    NO_OPERANDS,

    /* oooooo rrrrr rrrrr 0000000000000000 */
    /*    6     5     5         16         */
    REGISTER_REGISTER,

    /* oooooo rrrrr rrrrr mmmmmmmmmmmmmmmm */
    /*    6     5     5         16         */
    REGISTER_REGISTER_OFFSET,

    /* oooooo rrrrr mmmmmmmmmmmmmmmmmmmmm */
    /*    6     5           21            */
    REGISTER_IMMEDIATE,

    /* oooooo rrrrr 000000000000000000000 */
    /*    6     5            21           */
    REGISTER_NO_IMMEDIATE,

    /* oooooo 00000 mmmmmmmmmmmmmmmmmmmmm  */
    /*    6                  21            */
    IMMEDIATE_NO_REGISTER,

    INSTRUCTION_TYPE_COUNT
} instruction_type_t;

typedef struct instruction {
    instruction_type_t type;

    opcode_t opcode;
    union {
        register_t reg;
        i32 imm;
    } operand1;

    union {
        register_t reg;
        i32 imm;
    } operand2;

    union {
        register_t reg;
        i32 imm;
    } operand3;

    i32 assembled_value;

    char *opcode_str, *operand1_str, *operand2_str, *operand3_str;
    char *disassembled_str;
} instruction_t;

/* Makes a new instruction with all of the components to print,
 * and the assembled value of the instruction. */
instruction_t *make_instruction(opcode_t opcode, i32 operand1, i32 operand2, i32 operand3);
instruction_t *make_no_operand_instruction(opcode_t opcode);
instruction_t *make_one_operand_instruction(opcode_t opcode, i32 operand1);
instruction_t *make_two_operand_instruction(opcode_t opcode, i32 operand1, i32 operand2);
instruction_t *make_three_operand_instruction(opcode_t opcode, i32 operand1, i32 operand2, i32 operand3);

/* Disassembles an assembled instruction */
instruction_t *disassemble_instruction(i32 instruction);

/* Frees the instruction and sets it to null. */
void free_instruction(instruction_t **instruction);

/* Compiles an instruction to it's binary representation. */
i32 assemble_instruction(opcode_t opcode, i32 operand1, i32 operand2, i32 operand3);


/* Returns the bit type for the given opcode. */
instruction_type_t get_type(opcode_t opcode);

#endif

