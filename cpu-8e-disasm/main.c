#include <stdio.h>
#include <stdlib.h>

#define HLT 0x0
#define NOP 0x1
#define NOT 0x2
#define RET 0x3
#define JMP 0x84    // 0xc4

#define OPCODE(x)           (x & 0xbf)
#define ADDRESSING_MODE(x)  (x & 0x40)
#define IS_MULTIWORD(x)     (x & 0x80)

#define ADDR_IMMEDIATE  0
#define ADDR_DIRECT     1


/**
 * Cada instrução pode ter um ou dois bytes, mas cada opcode tem apenas um
 * byte. O byte restante conterá (ou não) o parâmetro da instrução.
 *
 * Os bits 0-4 identificam o opcode.
 * O bit 5 não é utilizado.
 * O bit 6 indica endereçamento imediato (0) ou direto (1).
 * O bit 7 indica se a instrução possui uma ou duas palavras.
 */

int main(int argc, char *argv[])
{
    FILE *input;
    char *disasm, disasm_buf[256];

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

        if (IS_MULTIWORD(op)) {
            param = fgetc(input);
            if (param == EOF) {
                fprintf(stderr, "O arquivo terminou inesperadamente\n");
                return EXIT_FAILURE;
            }
        }

        switch (OPCODE(op)) {
            case HLT:
                disasm = "HLT";
                break;
            case NOP:
                disasm = "NOP";
                break;
            case RET:
                disasm = "RET";
                break;
            case JMP: {
                    if (ADDRESSING_MODE(op) == ADDR_IMMEDIATE) {
                        sprintf(disasm_buf, "JMP %02x", param);
                    } else {
                        sprintf(disasm_buf, "JMP [%02x]", param);
                    }
                    disasm = disasm_buf;
                    break;
                }
            default:
                disasm = "...";
                break;
        }

        /* imprimir o PC e o opcode */
        printf("%05lx %02x ", pc, op);

        /* imprimir (ou não) o parâmetro */
        if (IS_MULTIWORD(op)) {
            printf("%02x   ", param);
        } else {
            printf("     ", param);
        }

        /* imprimir o código desmontado */
        puts(disasm);
    }
}
