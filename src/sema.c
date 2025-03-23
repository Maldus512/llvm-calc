#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "sema.h"


static void    insert_binding(sema_t *sema, const char *variable);
static uint8_t find_binding(sema_t *sema, const char *variable);


uint8_t sema_check(sema_t *sema, parser_ast_t *ast) {
    switch (ast->tag) {
        case PARSER_AST_TAG_IDENTIFIER:
            if (!find_binding(sema, ast->source_reference)) {
                char name[CONFIG_MAX_VARIABLE_NAME_LENGTH] = {0};
                parser_copy_string_from_source_reference(name, ast->source_reference);
                fprintf(stderr, "Undeclared variable %s\n", name);
                return 0;
            } else {
                return 1;
            }

        case PARSER_AST_TAG_BINARY_OPERATION:
            return sema_check(sema, ast->as.binary_operation.left) & sema_check(sema, ast->as.binary_operation.right);

        case PARSER_AST_TAG_WITH_DECLARATION:
            for (size_t i = 0; i < CONFIG_MAX_WITH_VARIABLES; i++) {
                if (ast->as.with_declaration.identifiers[i] != NULL) {
                    insert_binding(sema, ast->as.with_declaration.identifiers[i]);
                } else {
                    break;
                }
            }
            return sema_check(sema, ast->as.with_declaration.expr);

        case PARSER_AST_TAG_ERROR: {
            char source[128] = {0};
            memcpy(source, ast->source_reference, ast->as.error.reference_length);
            fprintf(stderr, "Error:\n%s\n^%s\n", source, ast->as.error.description);
            return 0;
        }

        default:
            return 1;
    }
}


static void insert_binding(sema_t *sema, const char *variable) {
    for (size_t i = 0; i < CONFIG_MAX_WITH_VARIABLES; i++) {
        if (sema->bindings[i] == NULL) {
            sema->bindings[i] = variable;
            return;
        }
    }

    assert(0);
}


static uint8_t find_binding(sema_t *sema, const char *variable) {
    for (size_t i = 0; i < CONFIG_MAX_WITH_VARIABLES; i++) {
        if (sema->bindings[i] != NULL && parser_compare_string_in_source_reference(sema->bindings[i], variable)) {
            return 1;
        }
    }

    return 0;
}
