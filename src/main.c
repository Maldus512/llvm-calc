#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "parser.h"
#include "codegen.h"
#include "sema.h"


static void compile(const char *source);


int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Calc, the expression compiler\n\nUsage:\n\t%s input\n\n", argv[0]);
        return 1;
    } else {
        FILE *f = fopen(argv[1], "r");
        if (!f) {
            fprintf(stderr, "Could not open file %s: %s\n", argv[1], strerror(errno));
            exit(1);
        }

        char buffer[256] = {0};
        int  res         = fread(buffer, 1, sizeof(buffer), f);
        if (res < 0) {
            fprintf(stderr, "Could not read file %s: %s\n", argv[1], strerror(errno));
            fclose(f);
            exit(1);
        }
        fclose(f);

        compile(buffer);
    }
    return 0;
}


static void compile(const char *source) {
    parser_t      parser = {.lexer = {.buffer = source, .buffer_index = 0}};
    parser_ast_t *ast    = parser_parse(&parser);

    sema_t sema = {0};
    if (sema_check(&sema, ast)) {
        codegen_compile(ast);
    } else {
        fprintf(stderr, "\nCompilation failed\n");
    }
}
