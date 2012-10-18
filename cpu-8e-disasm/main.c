#include <stdio.h>
#include <stdlib.h>

#define HLT 0x0
#define NOP 0x1
#define NOT 0x2
#define RET 0x3
#define JMP 0x84    // 0xc4

#define OPCODE(x)           (x & 0xbf)
#define ADDRESSING_MODE(x)  (x & 0x40)

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
    int c, i;
    const char *s;
    char params[256];

    if (argc < 2) {
        puts("Modo de usar: cpu-8e-disasm: [INPUT]");
        return EXIT_SUCCESS;
    }

    input = fopen(argv[1], "rb");
    if (input == NULL) {
        fprintf(stderr, "Falha ao carregar arquivo %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    i = 0;
    while (EOF != (c = fgetc(input))) {
        params[0] = '\0';

        switch (OPCODE(c)) {
            case HLT:
                s = "HLT";
                break;
            case NOP:
                s = "NOP";
                break;
            case RET:
                s = "RET";
                break;
            case JMP: {
                    int address = fgetc(input);
                    s = "JMP";
                    if (ADDRESSING_MODE(c) == ADDR_IMMEDIATE) {
                        sprintf(params, "%02x", address);
                    } else {
                        sprintf(params, "[%02x]", address);
                    }
                    break;
                }
            default:
                s = "UNK";
                break;
        }

        printf("%02i %02x %s %s\n", i, c, s, params);
        i += 1;
    }
}
