#include <stdio.h>
#include <stdlib.h>
#include "8e.h"

int main(int argc, char *argv[])
{
    FILE *input;
    char disasm_buf[256];
    const char *disasm;

    if (argc < 2) {
        puts("Modo de usar: cpu-8e-disasm [INPUT]");
        return EXIT_SUCCESS;
    }

    input = fopen(argv[1], "rb");
    if (input == NULL) {
        fprintf(stderr, "Falha ao carregar arquivo \"%s\"\n", argv[1]);
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
