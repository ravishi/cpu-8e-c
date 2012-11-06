#include <stdio.h>
#include <stdlib.h>
#include "8e.h"

typedef struct {
    c8word pc;
    c8word a;
    c8word b;
} tracer_entry;

typedef struct {
    tracer_entry *trace;
    size_t idx;
    size_t size;
} tracer;

tracer *tracer_new_with_init(size_t size)
{
    tracer *self = malloc(sizeof(tracer));
    self->trace = malloc(sizeof(tracer_entry) * size);
    self->size = size;
    self->idx = 0;
    return self;
}

void trace(const cpu8e *cpu, void *data)
{
    tracer *t = (tracer *) data;
    tracer_entry *te;

    if (t->idx >= t->size) {
        t->size *= 2;
        t->trace = realloc(t->trace, sizeof(tracer_entry) * t->size);
    }

    te = t->trace + t->idx;
    t->idx += 1;

    te->pc = cpu->pc;
    te->a = cpu8e_memory_get(cpu, te->pc);
    if (IS_MULTIWORD(te->a)) {
        te->b = cpu8e_memory_get(cpu, te->pc + 1);
    }
}


int main(int argc, char *argv[])
{
    FILE *input;
    cpu8e *cpu;
    tracer *tracer;

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
    tracer = tracer_new_with_init(512);

    cpu8e_tracer_set(cpu, trace, tracer);

    // preencher a memória da cpu com o conteúdo do arquivo
    fread(cpu->memory, cpu->memory_size, 1, input);

    // executar a CPU
    if (cpu8e_continue(cpu) != 0) {
        puts("Ocorreu um erro durante a execução");
    } else {
        puts("Arquivo executado. Em futuras versões do programa você poderá saber qual foi o resultado da execução. Por enquanto, contente-se em acreditar que a execução terminou.");
    }

    // imprimir o trace
    {
        size_t i;
        char disasm_buf[256];
        const char *disasm;

        for (i = 0; i < tracer->idx; i++)
        {
            tracer_entry *t = tracer->trace + i;
            if (!is_opcode(t->a)) {
                disasm = "XXX";
            } else {
                disasm = get_opcode_mnemonic(t->a);
                if (IS_MULTIWORD(t->a)) {
                    if (ADDRESSING_MODE(t->a) == ADDRESSING_IMMEDIATE) {
                        sprintf(disasm_buf, "%s %02x", disasm, t->b);
                    } else {
                        sprintf(disasm_buf, "%s [%02x]", disasm, t->b);
                    }
                    disasm = disasm_buf;
                }
            }

            /* imprimir o PC e o opcode */
            printf("%05lx %02x ", t->pc, t->a);

            /* imprimir (ou não) o parâmetro */
            if (is_opcode(t->a) && IS_MULTIWORD(t->a)) {
                printf("%02x   ", t->b);
            } else {
                printf("     ");
            }

            /* imprimir o código desmontado */
            puts(disasm);
        }
    }

    return EXIT_SUCCESS;
}
