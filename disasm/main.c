#include <stdio.h>
#include <stdlib.h>

/**
 * Cada instrução pode ter um ou dois bytes, mas cada opcode tem apenas um
 * byte. O byte restante conterá (ou não) o parâmetro da instrução.
 *
 * -  Os bits 0-4 identificam o opcode.
 * -  O bit 5 não é utilizado.
 * -  O bit 6 indica endereçamento imediato (0) ou direto (1).
 * -  O bit 7 indica se a instrução possui uma ou duas palavras.
 */

/**
 * Esta é a lista de opcodes. Os valores são os valores do opcode com o bit 6
 * zerado.
 */
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

// identificaremos o opcode usando os bits 0-5 e 7.
#define OPCODE(b)               (b & 0xbf)

// o bit 6 indica o modo de endereçamento
#define ADDRESSING_MODE(b)      (b & 0x40)
#define ADDRESSING_IMMEDIATE    0
#define ADDREESSING_DIRECT      1

// o bit 7 indica se a instrução possui duas palavras
#define IS_MULTIWORD(b)         (b & 0x80)


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

int is_opcode(int opcode)
{
    return *get_opcode_mnemonic(opcode) != '\0';
}

int main(int argc, char *argv[])
{
    FILE *input;
    char disasm_buf[256];
    const char *disasm;

    if (argc < 2) {
        puts("Modo de usar: cpu-8e-disasm: [INPUT]");
        return EXIT_SUCCESS;
    }

    input = fopen(argv[1], "rb");
    if (input == NULL) {
        fprintf(stderr, "Falha ao carregar arquivo %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    while (1) {
        long int pc = ftell(input);
        int op = fgetc(input);
        int param;

        if (op == EOF) {
            break;
        }

        if (!is_opcode(op)) {
            disasm = "XXX";
        } else {
            disasm = get_opcode_mnemonic(op);
            if (IS_MULTIWORD(op)) {
                param = fgetc(input);
                if (param == EOF) {
                    fprintf(stderr, "O arquivo terminou inesperadamente\n");
                    return EXIT_FAILURE;
                }

                if (ADDRESSING_MODE(op) == ADDRESSING_IMMEDIATE) {
                    sprintf(disasm_buf, "%s %02x", disasm, param);
                } else {
                    sprintf(disasm_buf, "%s [%02x]", disasm, param);
                }

                disasm = disasm_buf;
            }
        }

        /* imprimir o PC e o opcode */
        printf("%05lx %02x ", pc, op);

        /* imprimir (ou não) o parâmetro */
        if (is_opcode(op) && IS_MULTIWORD(op)) {
            printf("%02x   ", param);
        } else {
            printf("     ", param);
        }

        /* imprimir o código desmontado */
        puts(disasm);
    }

    return EXIT_SUCCESS;
}
