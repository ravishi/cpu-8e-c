#include <stdio.h>
#include <stdlib.h>
#include "8e.h"

typedef struct {
    c8word pc;
    c8word mar;
    c8word mdr;
    c8word a;
    c8word b;
    c8word ri;
    c8word ula_state_z;
    c8word ula_state_n;
    c8word ula_state_c;
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
    te->mar = cpu->mar;
    te->mdr = cpu->mdr;
    te->ri = cpu->ri;
    te->ula_state_z = cpu->ula_state_z;
    te->ula_state_n = cpu->ula_state_n;
    te->ula_state_c = cpu->ula_state_c;
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

    // imprimir a memória da CPU antes da execução
    puts("Memória:");
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

    cpu8e_continue(cpu);

    puts("");
    puts("Trace:");
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
            printf(disasm);

            // alinhar
            {
                int i;
                for (i = 16 - strlen(disasm); i > 0; i--)
                {
                    printf(" ");
                }
            }

            // valores dos registradores
            printf("PC=%02x MAR=%02x MDR=%02x RI=%02x Z=%x N=%x C=%x\n",
                    t->pc, t->mar, t->mdr, t->ri, t->ula_state_z,
                    t->ula_state_n, t->ula_state_c);
        }
    }

    return EXIT_SUCCESS;
}
