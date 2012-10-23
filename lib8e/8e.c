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
