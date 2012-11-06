#include <stdlib.h>
#include "8e.h"

/**
 * Retorna uma string contendo o mnemônico de um opcode.
 */
const char *get_opcode_mnemonic(int opcode)
{
#define OPM(m) \
    case m:\
        return #m ;
    switch (OPCODE(opcode)) {
        OPM(HLT)
        OPM(NOP)
        OPM(NOT)
        OPM(RET)
        OPM(JMP)
        OPM(JEQ)
        OPM(JGT)
        OPM(JGE)
        OPM(JCY)
        OPM(CAL)
        OPM(SHL)
        OPM(SHR)
        OPM(SRA)
        OPM(ROL)
        OPM(ROR)
        OPM(STO)
        OPM(LOD)
        OPM(CMP)
        OPM(ADD)
        OPM(SUB)
        OPM(AND)
        OPM(XOR)
        OPM(ORL)
        default:
            return "\0";
    }
}

/**
 * Retorna algum valor diferente de 0 se o valor passado puder ser identificado
 * como um opcode válido, ou 0 caso contrário.
 */
int is_opcode(int opcode)
{
    return *get_opcode_mnemonic(opcode) != '\0';
}



/**********************
 * O código da CPU
 ************************************/

cpu8e *cpu8e_new()
{
    cpu8e *self = (cpu8e *) malloc(sizeof(cpu8e));
    self->memory = NULL;
    self->memory_size = 0;
    return self;
}

void cpu8e_init(cpu8e *self)
{
    self->memory_size = 256;
    self->memory = malloc(self->memory_size);
}

void cpu8e_destroy(cpu8e *self)
{
    free(self->memory);
    free(self);
}

cpu8e *cpu8e_new_with_init()
{
    cpu8e *self = cpu8e_new();
    cpu8e_init(self);
    return self;
}

int *cpu8e_continue(cpu8e *self)
{
    return 0;
}
