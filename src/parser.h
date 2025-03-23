#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED


#include <stdint.h>
#include <stdlib.h>
#include "lexer.h"
#include "config.h"


typedef struct {
    lexer_t lexer;
} parser_t;

typedef enum {
    PARSER_OPERATOR_PLUS = 0,
    PARSER_OPERATOR_MINUS,
    PARSER_OPERATOR_STAR,
    PARSER_OPERATOR_SLASH,
} parser_operator_t;

typedef enum {
    PARSER_AST_TAG_NUMBER = 0,
    PARSER_AST_TAG_IDENTIFIER,
    PARSER_AST_TAG_BINARY_OPERATION,
    PARSER_AST_TAG_WITH_DECLARATION,
    PARSER_AST_TAG_ERROR,
} parser_ast_tag_t;

typedef struct parser_ast {
    parser_ast_tag_t tag;
    const char      *source_reference;

    union {
        struct {
            parser_operator_t  op;
            struct parser_ast *left;
            struct parser_ast *right;
        } binary_operation;
        struct {
            const char        *identifiers[CONFIG_MAX_WITH_VARIABLES];
            struct parser_ast *expr;
        } with_declaration;
        struct {
            const char *description;
            size_t      reference_length;
        } error;
    } as;
} parser_ast_t;


parser_ast_t *parser_parse(parser_t *parser);
void          parser_copy_string_from_source_reference(char *dest, const char *src);
uint8_t       parser_compare_string_in_source_reference(const char *string1, const char *string2);
int           parser_get_int_value_from_source_reference(const char *string);

#endif
