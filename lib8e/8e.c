#include <stdlib.h>
#include "8e.h"

#define CPU8E_ST_00         0x0
#define CPU8E_ST_01         0x1
#define CPU8E_ST_10         0x2
#define CPU8E_ST_HLT        0x4
#define CPU8E_ST_INVALID    0x5

typedef enum
{
    CPU8E_ULA_ADD,
    CPU8E_ULA_SUB,
    CPU8E_ULA_AND,
    CPU8E_ULA_XOR,
    CPU8E_ULA_ORL,
    CPU8E_ULA_NOT,
    CPU8E_ULA_SHL,
    CPU8E_ULA_SHR,
    CPU8E_ULA_SRA,
    CPU8E_ULA_ROL,
    CPU8E_ULA_ROR,
    CPU8E_ULA_LOD
} cpu8e_ula_operation;

int cpu8e_is_opcode(c8word byte)
{
    return *cpu8e_get_opcode_mnemonic(byte) != '\0';
}

const char *cpu8e_get_opcode_mnemonic(c8word byte)
{
#define OPM(m) \
    case CPU8E_##m:\
        return #m ;
    switch (CPU8E_OPCODE(byte)) {
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

cpu8e *cpu8e_new_with_init()
{
    cpu8e *self = cpu8e_new();
    cpu8e_init(self);
    return self;
}

void cpu8e_destroy(cpu8e *self)
{
    free(self->memory);
    free(self);
}

c8word cpu8e_memory_get(cpu8e *self, c8addr address)
{
    return ((c8word *) self->memory)[address];
}

void cpu8e_memory_set(cpu8e *self, c8addr address, c8word value)
{
    c8word *memory = (c8word *) self->memory;
    memory[address] = value;
}

c8word cpu8e_get_register(const cpu8e *self, cpu8e_register reg)
{
    switch (reg) {
        case CPU8E_ACC:
            return self->acc;
        case CPU8E_MAR:
            return self->mar;
        case CPU8E_MDR:
            return self->mdr;
        case CPU8E_PC:
            return self->pc;
        case CPU8E_SP:
            return self->sp;
        case CPU8E_RA:
            return self->ra;
        case CPU8E_RB:
            return self->rb;
        case CPU8E_RI:
            return self->ri;
        case CPU8E_Z:
            return self->ula_state_z;
        case CPU8E_N:
            return self->ula_state_n;
        case CPU8E_C:
            return self->ula_state_c;
    }
}

c8word cpu8e_ula(cpu8e *self, cpu8e_ula_operation operation)
{
    unsigned long result;

    switch (operation)
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
    self->ula_state_c = (result >> 8) != 0;

    return (c8word)(result & 0xff);
}

void cpu8e_substep_00(cpu8e *self)
{
    // primeira fase: obtenção do opcode
    self->mar = self->pc;
    self->mdr = cpu8e_memory_get(self, self->mar);
    self->pc += 1;
    self->ri = self->mdr;

    // o próximo estado dependerá de se o opcode possui operando ou não.
    self->state = CPU8E_IS_MULTIWORD(self->ri) ? CPU8E_ST_01 : CPU8E_ST_10;
}

void cpu8e_substep_01(cpu8e *self)
{
    self->mar = self->pc;
    self->pc += 1;
    // se o endereçamento for direto, carrega o endereço do operando
    if (CPU8E_ADDRESSING_MODE(self->ri) == CPU8E_ADDRESSING_DIRECT) {
        self->mdr = cpu8e_memory_get(self, self->mar);
        self->mar = self->mdr;
    }
    self->state = CPU8E_ST_10;
}

void cpu8e_substep_10(cpu8e *self)
{
    // execução do opcode. aqui é que a mágina acontece :-)
    
    switch (CPU8E_OPCODE(self->ri))
    {
        case CPU8E_HLT:
            self->state = CPU8E_ST_HLT;
            break;
        case CPU8E_NOP:
            break;
        case CPU8E_NOT:
            self->ra = self->acc;
            self->acc = cpu8e_ula(self, CPU8E_ULA_NOT);
            break;
        case CPU8E_RET:
            self->sp += 1; // incrementa o stackpointer
            self->mar = self->sp; // endereça o último elemento da pilha
            self->mdr = cpu8e_memory_get(self, self->mar);
            self->pc = self->mdr;
            break;
        case CPU8E_JMP:
            self->pc = self->mar; // jump incondicional
            break;
        case CPU8E_JEQ:
            if (self->ula_state_z) {
                self->pc = self->mar;
            }
            break;
        case CPU8E_JGT:
            if (!self->ula_state_n && !self->ula_state_z) {
                self->pc = self->mar;
            }
            break;
        case CPU8E_JGE:
            if (!self->ula_state_n) {
                self->pc = self->mar;
            }
            break;
        case CPU8E_JCY:
            if (self->ula_state_c) {
                self->pc = self->mar;
            }
            break;
        case CPU8E_CAL:
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
        case CPU8E_SHL:
            // lê contagem para o shift;
            self->mdr = cpu8e_memory_get(self, self->mar);

            self->ra = self->acc;
            self->rb = self->mdr;
            self->acc = cpu8e_ula(self, CPU8E_ULA_SHL);
            break;
        case CPU8E_SHR:
            // lê a contagem para o shift;
            self->mdr = cpu8e_memory_get(self, self->mar);

            self->ra = self->acc;
            self->rb = self->mdr;
            self->acc = cpu8e_ula(self, CPU8E_ULA_SHR);
            break;
        case CPU8E_SRA:
            // lê a contagem para o shift;
            self->mdr = cpu8e_memory_get(self, self->mar);

            self->ra = self->acc;
            self->rb = self->mdr;
            self->acc = cpu8e_ula(self, CPU8E_ULA_SRA);
            break;
        case CPU8E_ROL:
            // lê a contagem para o rotate;
            self->mdr = cpu8e_memory_get(self, self->mar);

            self->ra = self->acc;
            self->rb = self->mdr;
            self->acc = cpu8e_ula(self, CPU8E_ULA_ROL);
            break;
        case CPU8E_ROR:
            // lê a contagem para o rotate;
            self->mdr = cpu8e_memory_get(self, self->mar);

            self->ra = self->acc;
            self->rb = self->mdr;
            self->acc = cpu8e_ula(self, CPU8E_ULA_ROR);
            break;
        case CPU8E_STO:
            // o operando é o acc
            self->mdr = self->acc;
            cpu8e_memory_set(self, self->mar, self->mdr);
            break;
        case CPU8E_LOD:
            // lê o operando
            self->mdr = self->mar;
            self->ra = self->mdr;
            // carrega A em ACC via ULA
            self->acc = cpu8e_ula(self, CPU8E_ULA_LOD);
            break;
        case CPU8E_CMP:
            // lê o segundo operando
            self->mdr = cpu8e_memory_get(self, self->mar);
            self->ra = self->acc;
            self->rb = self->mdr;
            // subtrai os operandos, sem alterar o ACC. essa operação altera um
            // daqueles bits da ula (que agora eu n lembro qual é), que será
            // utilizado depois pelo CMP. genial!
            cpu8e_ula(self, CPU8E_ULA_SUB);
            break;
        case CPU8E_ADD:
            // lê o segundo operando
            self->mdr = cpu8e_memory_get(self, self->mar);
            self->ra = self->acc;
            self->rb = self->mdr;
            // soma os operandos e guarda o resultado em ACC.
            self->acc = cpu8e_ula(self, CPU8E_ULA_ADD);
            break;
        case CPU8E_SUB:
            // lê o segundo operando
            self->mdr = cpu8e_memory_get(self, self->mar);
            self->ra = self->acc;
            self->rb = self->mdr;
            // subtrai os operandos e guarda o resultado em ACC.
            self->acc = cpu8e_ula(self, CPU8E_ULA_SUB);
            break;
        case CPU8E_AND:
            // lê o segundo operando
            self->mdr = cpu8e_memory_get(self, self->mar);
            self->ra = self->acc;
            self->rb = self->mdr;
            self->acc = cpu8e_ula(self, CPU8E_ULA_AND);
            break;
        case CPU8E_XOR:
            // lê o segundo operando
            self->mdr = cpu8e_memory_get(self, self->mar);
            self->ra = self->acc;
            self->rb = self->mdr;
            self->acc = cpu8e_ula(self, CPU8E_ULA_XOR);
            break;
        case CPU8E_ORL:
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
