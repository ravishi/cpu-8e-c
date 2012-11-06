#include <stdio.h>
#include <stdlib.h>
#include "8e.h"

int main(int argc, char *argv[])
{
    FILE *input;
    cpu8e *cpu;

    if (argc < 2) {
        fprintf(stderr, "Modo de usar: cpu-8e [INPUT]");
        return EXIT_FAILURE;
    }

    input = fopen(argv[1], "rb");
    if (input == NULL) {
        fprintf(stderr, "Falha ao carregar arquivo \"%s\"\n", argv[1]);
        return EXIT_FAILURE;
    }

    cpu = cpu8e_new_with_init();

    // preencher a memória da cpu com o conteúdo do arquivo
    fread(cpu->memory, cpu->memory_size, 1, input);

    // executar a CPU
    if (cpu8e_continue(cpu) != 0) {
        puts("Ocorreu um erro durante a execução");
    } else {
        puts("Arquivo executado. Em futuras versões do programa você poderá saber qual foi o resultado da execução. Por enquanto, contente-se em acreditar que a execução terminou.");
    }

    // dumpar o estado da CPU :-)
    puts("dump:");
    puts("=====");
    puts("");
    printf("PC  %02x\n", cpu->pc);
    printf("MAR %02x\n", cpu->mar);
    printf("MDR %02x\n", cpu->mdr);
    printf("ACC %02x\n", cpu->acc);
    puts("");

    {
        c8addr i;
        c8word *c = (c8word *) cpu->memory;
        for (i = 0; i < cpu->memory_size; ++i) {
            if (i % 8 == 0) {
                printf("%02x: ", i);
            }
            printf("%02x ", c[i]);
            if ((i+1) % 8 == 0) {
                printf("\n");
            }
        }
    }

    return EXIT_SUCCESS;
}
