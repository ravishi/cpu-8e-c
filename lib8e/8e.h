#ifndef __8E_H__
#define __8E_H__

#include <stddef.h>

typedef unsigned char           c8word;
typedef size_t                  c8addr;

typedef struct cpu8e_s          cpu8e;
typedef struct cpu8e_tracer_s   cpu8e_tracer;

struct cpu8e_tracer_s
{
    void *data;
    void (*trace)(const cpu8e *, void *);
};

struct cpu8e_s
{
    void *memory;
    size_t memory_size;
    c8word mar;
    c8word mdr;
    c8word pc;
    c8word sp;
    c8word acc;
    c8word ra;
    c8word rb;
    c8word ri;
    c8word state;
    c8word ula_state_z;
    c8word ula_state_n;
    c8word ula_state_c;
    cpu8e_tracer tracer;
};

typedef enum
{
    CPU8E_MAR,
    CPU8E_MDR,
    CPU8E_PC,
    CPU8E_SP,
    CPU8E_ACC,
    CPU8E_RA,
    CPU8E_RB,
    CPU8E_RI,
    CPU8E_Z,
    CPU8E_N,
    CPU8E_C
} cpu8e_register;

/**
 * Cada instrução pode ter um ou dois bytes, mas cada opcode tem apenas um
 * byte. O byte restante conterá (ou não) o parâmetro da instrução.
 *
 * O byte principal da instrução é composto de:
 *
 * -  Os bits 0-4 identificam o opcode.
 * -  O bit 5 não é utilizado.
 * -  O bit 6 indica endereçamento imediato (0) ou direto (1).
 * -  O bit 7 indica se a instrução possui uma ou duas palavras.
 *
 * Sendo assim, identificaremos os opcodes a partir de seus bits 0-5 e 7.
 */

// Utilizaremos os bits 0-5 e 7 para identificar o opcode.
#define CPU8E_OPCODE(b)                (b & 0xbf)

// O bit 6 indica o modo de endereçamento de uma instrução.
#define CPU8E_ADDRESSING_DIRECT        1
#define CPU8E_ADDRESSING_IMMEDIATE     0
#define CPU8E_ADDRESSING_MODE(b)       (b >> 6) & 0x1

// O bit 7 indica se a instrução possui uma ou duas palavras
#define CPU8E_IS_MULTIWORD(b)          (b >> 7) & 0x1

// Esta é a lista dos identificadores dos opcodes, conforme a definição acima.
#define CPU8E_HLT 0x0
#define CPU8E_NOP 0x1
#define CPU8E_NOT 0x2
#define CPU8E_RET 0x3
#define CPU8E_JMP 0x84
#define CPU8E_JEQ 0x85
#define CPU8E_JGT 0x86
#define CPU8E_JGE 0x87
#define CPU8E_JCY 0x88
#define CPU8E_CAL 0x89
#define CPU8E_SHL 0x8a
#define CPU8E_SHR 0x8b
#define CPU8E_SRA 0x8c
#define CPU8E_ROL 0x8d
#define CPU8E_ROR 0x8e
#define CPU8E_STO 0x90
#define CPU8E_LOD 0x91
#define CPU8E_CMP 0x94
#define CPU8E_ADD 0x95
#define CPU8E_SUB 0x96
#define CPU8E_AND 0x9a
#define CPU8E_XOR 0x9b
#define CPU8E_ORL 0x9c

/**
 * Retorna booleano indicando se o byte passado é um opcode da cpu8e.
 */
int cpu8e_is_opcode(c8word byte);

/**
 * Retorna uma string contendo o mnemônico do opcode passado. Algo como "HLT"
 * para CPU8E_HLT, "ADD" para CPU8E_ADD e assim por diante. Se o byte passado não for
 * um opcode válido, retorna uma string vazia ("").
 */
const char *cpu8e_get_opcode_mnemonic(c8word byte);

/**
 * Cria e inicializa uma nova instância da CPU.
 */
cpu8e *cpu8e_new_with_init();

/**
 * Destrói uma instância da CPU, liberando a memória de seus membros.
 */
void cpu8e_destroy(cpu8e *cpu);

/**
 * Retorna o valor de um registrador da CPU.
 */
c8word  cpu8e_get_register(const cpu8e *self, cpu8e_register reg);

/**
 * Registra um tracer. *trace* deve ser uma função que recebe como parâmetro
 * uma instância da cpu8e e um ponteiro de dados. *data* é um ponteiro de dados
 * que será repassado para a função cada vez que ela for chamada.
 */
void cpu8e_tracer_set(cpu8e *self, void (*trace)(const cpu8e *, void *), void *data);
void cpu8e_tracer_unset(cpu8e *self);

/**
 * Continua a execução do programa em memória.
 */
int cpu8e_continue(cpu8e *cpu);

#endif // __8E_H__
