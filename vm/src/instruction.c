#include <stdlib.h> /* NULL, malloc, free */
#include <stdio.h> /* printf */
#include "instruction.h"
#include "helpers.h"

void do_strings(instruction_t *ins) {
    ins->opcode_str = op_to_str(ins->opcode);
    switch(ins->type) {
        case INSTRUCTION_TYPE_COUNT:
        case INVALID_INSTRUCTION_TYPE:
            return;
        case NO_OPERANDS:
            ins->disassembled_str = str_cat(1, ins->opcode_str);
            return;
        case REGISTER_REGISTER:
            ins->operand1_str = reg_to_str(ins->operand1.reg);
            ins->operand2_str = reg_to_str(ins->operand2.reg);
            ins->disassembled_str =
                str_cat(5, ins->opcode_str, " ", ins->operand1_str, ", ", ins->operand2_str);
            return;
        case REGISTER_IMMEDIATE:
            ins->operand1_str = reg_to_str(ins->operand1.reg);
            ins->operand2_str = imm_to_str(ins->operand2.imm, "%d");
            ins->disassembled_str =
                str_cat(5, ins->opcode_str, " ", ins->operand1_str, ", ", ins->operand2_str);
            return;
        case REGISTER_NO_IMMEDIATE:
            ins->operand1_str = reg_to_str(ins->operand1.reg);
            ins->disassembled_str =
                str_cat(3, ins->opcode_str, " ", ins->operand1_str);
            return;
        case IMMEDIATE_NO_REGISTER:
            ins->operand1_str = imm_to_str(ins->operand1.imm, "%d");
            ins->disassembled_str =
                str_cat(3, ins->opcode_str, " ", ins->operand1_str);
            return;
    }
}

void do_operands(instruction_t *ins) {
    switch(ins->type) {
        case INSTRUCTION_TYPE_COUNT:
        case INVALID_INSTRUCTION_TYPE:
            return;
        case NO_OPERANDS:
            ins->operand1.imm = 0;
            ins->operand2.imm = 0;
            return;
        case REGISTER_REGISTER:
            ins->operand1.reg = get_r1(ins->assembled_value);
            ins->operand2.reg = get_r2(ins->assembled_value);
            return;
        case REGISTER_IMMEDIATE:
            ins->operand1.reg = get_r1(ins->assembled_value);
            ins->operand2.imm = get_imm(ins->assembled_value);
            return;
        case REGISTER_NO_IMMEDIATE:
            ins->operand1.reg = get_r1(ins->assembled_value);
            ins->operand2.imm = 0;
            return;
        case IMMEDIATE_NO_REGISTER:
            ins->operand1.imm = get_imm(ins->assembled_value);
            ins->operand2.imm = 0;
            return;
    }
}

instruction_t *make_no_operand_instruction(opcode_t opcode) {
    return make_instruction(opcode, 0, 0);
}
instruction_t *make_one_operand_instruction(opcode_t opcode, int operand1) {
    return make_instruction(opcode, operand1, 0);
}
instruction_t *make_two_operand_instruction(opcode_t opcode, int operand1, int operand2) {
    return make_instruction(opcode, operand1, operand2);
}

/* Makes a new instruction with all of the components to print,
 * and the assembled value of the instruction. */
instruction_t *make_instruction(opcode_t opcode, int operand1, int operand2) {
    instruction_t *ins = malloc(sizeof *ins);
    ins->type = get_type(opcode);
    ins->opcode = opcode;
    ins->assembled_value = assemble_instruction(opcode, operand1, operand2);
    do_operands(ins);
    do_strings(ins);
    return ins;
}

/* Frees the instruction and sets it equal to null. */
void free_instruction(instruction_t **instruction) {
    instruction_t *ins;
    if (instruction && *instruction) {
        ins = *instruction;
        if (ins->type == IMMEDIATE_NO_REGISTER) {
            free(ins->operand1_str);
        }
        if (ins->type == REGISTER_IMMEDIATE) {
            free(ins->operand2_str);
        }
        free(ins->disassembled_str);
        free(ins);
        *instruction = NULL;
    }
}

unsigned int assemble_no_operands(opcode_t opcode) {
    return opcode << OPCODE_SHIFT;
}
unsigned int assemble_register_register(opcode_t op, register_t r1, register_t r2 ) {
    unsigned int ins = 0;
    ins |= (op << OPCODE_SHIFT);
    ins |= (r1 << R1_SHIFT);
    ins |= (r2 << R2_SHIFT);
    return ins;
}
unsigned int assemble_register_immediate(opcode_t op, register_t r, int imm) {
    unsigned int ins = 0;
    int sign_bit = (imm & SIGN_BIT_MASK_32BIT) >> SIGN_BIT_SHIFT_21BIT;
    imm &= IMMEDIATE_MASK;
    imm |= sign_bit;

    ins |= (op << OPCODE_SHIFT);
    ins |= (r << R1_SHIFT);
    ins |= imm;
    return ins;
}
unsigned int assemble_register_no_immediate(opcode_t op, register_t r) {
    unsigned int ins = 0;
    ins |= (op << OPCODE_SHIFT);
    ins |= (r << R1_SHIFT);
    return ins;
}
unsigned int assemble_immediate_no_register(opcode_t op, int imm) {
    unsigned int ins = 0;
    int sign_bit = (imm & SIGN_BIT_MASK_32BIT) >> SIGN_BIT_SHIFT_21BIT;
    imm &= IMMEDIATE_MASK;
    imm |= sign_bit;


    ins |= (op << OPCODE_SHIFT);
    ins |= imm;
    return ins;
}

/* Compiles an instruction to it's binary representation. */
unsigned int assemble_instruction(opcode_t opcode, int operand1, int operand2) {
    instruction_type_t ty = get_type(opcode);

    switch(ty) {
        case INSTRUCTION_TYPE_COUNT:
        case INVALID_INSTRUCTION_TYPE:
            return -1;
        case NO_OPERANDS:
            return assemble_no_operands(opcode);
        case REGISTER_REGISTER:
            return assemble_register_register(opcode, operand1, operand2);
        case REGISTER_IMMEDIATE:
            return assemble_register_immediate(opcode, operand1, operand2);
        case REGISTER_NO_IMMEDIATE:
            return assemble_register_no_immediate(opcode, operand1);
        case IMMEDIATE_NO_REGISTER:
            return assemble_immediate_no_register(opcode, operand1);
    }
}


instruction_t *disassemble_instruction(unsigned int instruction) {
    opcode_t op = get_opcode(instruction);
    register_t r1 = get_r1(instruction);
    register_t r2 = get_r2(instruction);
    int imm = get_imm(instruction);
    instruction_type_t ty = get_type(op);
    switch(ty) {
        case INSTRUCTION_TYPE_COUNT:
        case INVALID_INSTRUCTION_TYPE:
            return NULL;
        case NO_OPERANDS:
            return make_no_operand_instruction(op);
        case REGISTER_REGISTER:
            return make_two_operand_instruction(op, r1, r2);
        case REGISTER_IMMEDIATE:
            return make_two_operand_instruction(op, r1, imm);
        case REGISTER_NO_IMMEDIATE:
            return make_one_operand_instruction(op, r1);
        case IMMEDIATE_NO_REGISTER:
            return make_one_operand_instruction(op, imm);
    }
}


instruction_type_t get_type(opcode_t opcode) {
    switch(opcode) {
        case OPCODE_COUNT:
        case NOT_AN_OPCODE:
            return INVALID_INSTRUCTION_TYPE;

        case HALT:
        case NOP:
            return NO_OPERANDS;

        case ADD:
        case MUL:
        case DIV:
        case EQ:
        case NE:
        case LT:
        case LE:
        case GT:
        case GE:
        case AND:
        case OR:
        case XOR:
        case SLL:
        case SRL:
        case MOV:
        case LW:
        case SW:
            return REGISTER_REGISTER;

        case ADDI:
        case MULI:
        case DIVI:
        case LI:
            return REGISTER_IMMEDIATE;

        case JR:
        case PUSH:
        case POP:
            return REGISTER_NO_IMMEDIATE;

        case J:
        case JS:
        case JZ:
        case JZS:
        case CALL:
        case PUSHI:
            return IMMEDIATE_NO_REGISTER;
    }

    return INVALID_INSTRUCTION_TYPE;
}


