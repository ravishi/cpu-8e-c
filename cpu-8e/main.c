#include <stdio.h>
#include <stdlib.h>
#include "8e.h"

/**
 * Executa um arquivo numa instância da CPU 8e. *file* deve ser um ponteiro
 * para um arquivo devidamente aberto no modo *rb*, com seu ponteiro
 * devidamente posicionado no início do arquivo. O arquivo não será fechado
 * pela função execfile.
 *
 * Retorna booleano indicando se o programa foi executado.
 */
int execfile(FILE *file)
{
    int result;
    size_t i;
    cpu8e *cpu;

    cpu = cpu8e_new_with_init();

    // reiniciar o ponteiro do arquivo
    fseek(file, 0, SEEK_SET);

    // preencher a memória da cpu com o conteúdo do arquivo
    fread(cpu->memory, cpu->memory_size, 1, file);

    // executar a CPU
    result = cpu8e_continue(cpu);
    cpu8e_destroy(cpu);
    cpu = NULL;
    return (!result);
}

int main(int argc, char *argv[])
{
    FILE *input;
    cpu8e *instance;

    if (argc < 2) {
        fprintf(stderr, "Modo de usar: cpu-8e [INPUT]");
        return EXIT_FAILURE;
    }

    input = fopen(argv[1], "rb");
    if (input == NULL) {
        fprintf(stderr, "Falha ao carregar arquivo \"%s\"\n", argv[1]);
        return EXIT_FAILURE;
    }

    if (!execfile(input)) {
        fprintf(stderr, "Ocorreu um erro durante a execução\n");
        return EXIT_FAILURE;
    }

    puts("Arquivo executado. Em futuras versões do programa você poderá saber qual foi o resultado da execução. Por enquanto, contente-se em acreditar que a execução terminou.");

    return EXIT_SUCCESS;
}
