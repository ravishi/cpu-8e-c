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
    return (cpu8e *) malloc(sizeof(cpu8e));
}

void cpu8e_init(cpu8e *self)
{
    self->memory_size = 256;
    self->memory = malloc(self->memory_size);
    self->mar = 0;
    self->mdr = 0;
    self->pc = 0;
    self->sp = 0;
    self->acc = 0;
    self->ra = 0;
    self->rb = 0;
    self->ri = 0;
    self->state = 0;
    self->tracer.trace = NULL;
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

#define CPU8E_ST_00     0x0
#define CPU8E_ST_01     0x1
#define CPU8E_ST_10     0x2
#define CPU8E_ST_HLT    0x4
#define CPU8E_ST_INVALID 0x5

#define CPU8E_ULA_ADD   1
#define CPU8E_ULA_SUB   2
#define CPU8E_ULA_AND   3
#define CPU8E_ULA_XOR   4
#define CPU8E_ULA_ORL   5
#define CPU8E_ULA_NOT   6
#define CPU8E_ULA_SHL   7
#define CPU8E_ULA_SHR   8
#define CPU8E_ULA_SRA   9
#define CPU8E_ULA_ROL   10
#define CPU8E_ULA_ROR   11
#define CPU8E_ULA_LOD   12

c8word cpu8e_memory_get(cpu8e *self, c8addr address)
{
    return ((c8word *) self->memory)[address];
}
void cpu8e_memory_set(cpu8e *self, c8addr address, c8word value)
{
    c8word *memory = (c8word *) self->memory;
    memory[address] = value;
}

void cpu8e_substep_00(cpu8e *self)
{
    // primeira fase: obtenção do opcode
    self->mar = self->pc;
    self->mdr = cpu8e_memory_get(self, self->mar);
    self->pc += 1;
    self->ri = self->mdr;

    // o próximo estado dependerá de se o opcode possui operando ou não.
    self->state = IS_MULTIWORD(self->ri) ? CPU8E_ST_01 : CPU8E_ST_10;
}

void cpu8e_substep_01(cpu8e *self)
{
    self->mar = self->pc;
    self->pc += 1;
    // se o endereçamento for direto, carrega o endereço do operando
    if (ADDRESSING_MODE(self->ri) == ADDRESSING_DIRECT) {
        self->mdr = cpu8e_memory_get(self, self->mar);
        self->mar = self->mdr;
    }
    self->state = CPU8E_ST_10;
}

c8word cpu8e_ula(cpu8e *self, int op)
{
    c8word result;
    switch (op)
    {
        case CPU8E_ULA_NOT:
            result = !self->ra;
            break;
        case CPU8E_ULA_SHL:
            result = self->ra << self->rb;
            break;
        case CPU8E_ULA_SHR:
            result = self->ra >> self->rb;
            break;
        case CPU8E_ULA_SRA:
            // TODO implementar essa operação. não lembro o que é isso.
            result = 0;
            break;
        case CPU8E_ULA_ROL:
            // TODO implementar essa operação. não lembro o que é isso.
            result = 0;
            break;
        case CPU8E_ULA_ROR:
            // TODO implementar essa operação. não lembro o que é isso.
            result = 0;
            break;
        case CPU8E_ULA_LOD:
            result = self->ra;
            break;
        case CPU8E_ULA_SUB:
            result = self->ra - self->rb;
            break;
        case CPU8E_ULA_ADD:
            result = self->ra + self->rb;
            break;
        case CPU8E_ULA_AND:
            result = self->ra && self->rb;
            break;
        case CPU8E_ULA_XOR:
            // TODO não lembro como implementar isso.
            result = 0;
            break;
        case CPU8E_ULA_ORL:
            result = self->ra || self->rb;
            break;
        default:
            // TODO arrumar um jeito de notificar operações inválidas
            // realizadas através da ULA?
            result = 0;
            break;
    }

    // atualizar os bits de status da ULA.
    // Z indica se a última operação resultou em um valor nulo (zero)
    self->ula_state_z = result == 0;

    // N indica se a última operação resultou em um valor negativo
    self->ula_state_n = result < 0;

    // C indica se ocorreu um *carry* do bit mais significativo.
    // TODO implementar o C
    self->ula_state_c = 0;

    return result;
}

void cpu8e_substep_10(cpu8e *self)
{
    // execução do opcode. aqui é que a mágina acontece :-)
    
    switch (OPCODE(self->ri))
    {
        case HLT:
            self->state = CPU8E_ST_HLT;
            break;
        case NOP:
            break;
        case NOT:
            self->ra = self->acc;
            self->acc = cpu8e_ula(self, CPU8E_ULA_NOT);
            break;
        case RET:
            self->sp += 1; // incrementa o stackpointer
            self->mar = self->sp; // endereça o último elemento da pilha
            self->mdr = cpu8e_memory_get(self, self->mar);
            self->pc = self->mdr;
            break;
        case JMP:
            self->pc = self->mar; // jump incondicional
            break;
        case JEQ:
            if (self->ula_state_z) {
                self->pc = self->mar;
            }
            break;
        case JGT:
            if (!self->ula_state_n && !self->ula_state_z) {
                self->pc = self->mar;
            }
            break;
        case JGE:
            if (!self->ula_state_n) {
                self->pc = self->mar;
            }
            break;
        case JCY:
            if (self->ula_state_c) {
                self->pc = self->mar;
            }
            break;
        case CAL:
            // end. de retorno
            self->mdr = self->pc;
            // end. da rotina chamada
            self->pc = self->mar;
            // endereça o topo da pilha
            self->mar = self->sp;
            // salva o endereço de retorno na pilha
            cpu8e_memory_set(self, self->mar, self->mdr);
            // wtf? "Nono" topo da pilha? lol.
            self->sp -= 1;
            // recupera o endereço de retorno (jump)
            self->pc = self->mdr;
            break;
        case SHL:
            // lê contagem para o shift;
            self->mdr = cpu8e_memory_get(self, self->mar);

            self->ra = self->acc;
            self->rb = self->mdr;
            self->acc = cpu8e_ula(self, CPU8E_ULA_SHL);
            break;
        case SHR:
            // lê a contagem para o shift;
            self->mdr = cpu8e_memory_get(self, self->mar);

            self->ra = self->acc;
            self->rb = self->mdr;
            self->acc = cpu8e_ula(self, CPU8E_ULA_SHR);
            break;
        case SRA:
            // lê a contagem para o shift;
            self->mdr = cpu8e_memory_get(self, self->mar);

            self->ra = self->acc;
            self->rb = self->mdr;
            self->acc = cpu8e_ula(self, CPU8E_ULA_SRA);
            break;
        case ROL:
            // lê a contagem para o rotate;
            self->mdr = cpu8e_memory_get(self, self->mar);

            self->ra = self->acc;
            self->rb = self->mdr;
            self->acc = cpu8e_ula(self, CPU8E_ULA_ROL);
            break;
        case ROR:
            // lê a contagem para o rotate;
            self->mdr = cpu8e_memory_get(self, self->mar);

            self->ra = self->acc;
            self->rb = self->mdr;
            self->acc = cpu8e_ula(self, CPU8E_ULA_ROR);
            break;
        case STO:
            // o operando é o acc
            self->mdr = self->acc;
            cpu8e_memory_set(self, self->mar, self->mdr);
            break;
        case LOD:
            // lê o operando
            self->mdr = self->mar;
            self->ra = self->mdr;
            // carrega A em ACC via ULA
            self->acc = cpu8e_ula(self, CPU8E_ULA_LOD);
            break;
        case CMP:
            // lê o segundo operando
            self->mdr = cpu8e_memory_get(self, self->mar);
            self->ra = self->acc;
            self->rb = self->mdr;
            // subtrai os operandos, sem alterar o ACC. essa operação altera um
            // daqueles bits da ula (que agora eu n lembro qual é), que será
            // utilizado depois pelo CMP. genial!
            cpu8e_ula(self, CPU8E_ULA_SUB);
            break;
        case ADD:
            // lê o segundo operando
            self->mdr = cpu8e_memory_get(self, self->mar);
            self->ra = self->acc;
            self->rb = self->mdr;
            // soma os operandos e guarda o resultado em ACC.
            self->acc = cpu8e_ula(self, CPU8E_ULA_ADD);
            break;
        case SUB:
            // lê o segundo operando
            self->mdr = cpu8e_memory_get(self, self->mar);
            self->ra = self->acc;
            self->rb = self->mdr;
            // subtrai os operandos e guarda o resultado em ACC.
            self->acc = cpu8e_ula(self, CPU8E_ULA_SUB);
            break;
        case AND:
            // lê o segundo operando
            self->mdr = cpu8e_memory_get(self, self->mar);
            self->ra = self->acc;
            self->rb = self->mdr;
            self->acc = cpu8e_ula(self, CPU8E_ULA_AND);
            break;
        case XOR:
            // lê o segundo operando
            self->mdr = cpu8e_memory_get(self, self->mar);
            self->ra = self->acc;
            self->rb = self->mdr;
            self->acc = cpu8e_ula(self, CPU8E_ULA_XOR);
            break;
        case ORL:
            // lê o segundo operando
            self->mdr = cpu8e_memory_get(self, self->mar);
            self->ra = self->acc;
            self->rb = self->mdr;
            self->acc = cpu8e_ula(self, CPU8E_ULA_ORL);
            break;
        default:
            // deixa a cpu num estado inválido, para que o programa
            // simplesmente dê pau
            self->state = CPU8E_ST_INVALID;
            break;
    }

    // ao final da execução de qualquer opcode, o próximo estado da CPU será o
    // 00. exceto se a instrução for HLT.
    if (self->state != CPU8E_ST_HLT && self->state != CPU8E_ST_INVALID) {
        self->state = CPU8E_ST_00;
    }
}

void cpu8e_tracer_set(cpu8e *self, void (*trace)(const cpu8e *, void *), void *data)
{
    self->tracer.trace = trace;
    self->tracer.data = data;
}

void cpu8e_tracer_unset(cpu8e *self)
{
    self->tracer.trace = NULL;
}

void cpu8e_trace(cpu8e *self)
{
    if (self->tracer.trace != NULL) {
        self->tracer.trace(self, self->tracer.data);
    }
}

/**
 * Continua a execução da parada. Retorna 0 caso a execução seja concluída com
 * sucesso. Retorna algo diferente de zero caso dê pau. 1 significa que a CPU
 * alcançou um estado inesperado. Outros códigos de erro serão definidos
 * posteriormente.
 */
int cpu8e_continue(cpu8e *self)
{
    // a cpu continua em execução enquanto o estado não for HLT
    while (self->state != CPU8E_ST_HLT)
    {
        switch (self->state) {
            case CPU8E_ST_00:
                cpu8e_trace(self);
                cpu8e_substep_00(self);
                break;
            case CPU8E_ST_01:
                cpu8e_substep_01(self);
                break;
            case CPU8E_ST_10:
                cpu8e_substep_10(self);
                break;
            default:
                return 1;
        }
    }

    return 0;
}
