#ifndef __8E_H__
#define __8E_H__

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
#define OPCODE(b)               (b & 0xbf)

// O bit 6 indica o modo de endereçamento de uma instrução.
#define ADDRESSING_DIRECT       1
#define ADDRESSING_IMMEDIATE    0
#define ADDRESSING_MODE(b)      (b & 0x40)

// O bit 7 indica se a instrução possui uma ou duas palavras
#define IS_MULTIWORD(b)         (b & 0x80)

// Esta é a lista dos identificadores dos opcodes, conforme a definição acima.
#define HLT 0x0
#define NOP 0x1
#define NOT 0x2
#define RET 0x3
#define JMP 0x84
#define JEQ 0x85
#define JGT 0x86
#define JGE 0x87
#define JCY 0x88
#define CAL 0x89
#define SHL 0x8a
#define SHR 0x8b
#define SRA 0x8c
#define ROL 0x8d
#define ROR 0x8e
#define STO 0x90
#define LOD 0x91
#define CMP 0x94
#define ADD 0x95
#define SUB 0x96
#define AND 0x9a
#define XOR 0x9b
#define ORL 0x9c


const char *get_opcode_mnemonic(int opcode);

int is_opcode(int opcode);

#endif // __8E_H__
