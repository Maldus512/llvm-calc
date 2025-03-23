#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "parser.h"
#include "config.h"


static uint8_t       next_token_is(parser_t *parser, lexer_token_tag_t tag);
static parser_ast_t *parse_with_declaration(parser_t *parser);
static parser_ast_t *parse_expr(parser_t *parser);
static parser_ast_t *parse_term(parser_t *parser);
static parser_ast_t *parse_factor(parser_t *parser);
static parser_ast_t *parse_error(parser_t *parser, const char *source_reference,
                                 const lexer_token_tag_t *recovery_tokens, size_t num_recovery_tokens,
                                 const char *error_description);
static parser_ast_t *create_tagged(const char *source_reference, parser_ast_tag_t tag);
static parser_ast_t *create_with_declaration(const char *source_reference, const char **identifiers,
                                             parser_ast_t *expr);
static parser_ast_t *create_binary_operation(const char *source_reference, parser_operator_t op, parser_ast_t *left,
                                             parser_ast_t *right);


parser_ast_t *parser_parse(parser_t *parser) {
    return parse_with_declaration(parser);
}


int parser_get_int_value_from_source_reference(const char *string) {
    int value = 0;
    sscanf(string, "%i%*[^0123456789]", &value);
    return value;
}


uint8_t parser_compare_string_in_source_reference(const char *string1, const char *string2) {
    size_t i = 0;
    while (string1[i] != '\0' && lexer_is_letter(string1[i]) && string2[i] != '\0' && lexer_is_letter(string2[i])) {
        if (string1[i] != string2[i]) {
            return 0;
        }
        i++;
    }

    if ((string1[i] == '\0' || !lexer_is_letter(string1[i])) && (string2[i] == '\0' || !lexer_is_letter(string2[i]))) {
        return 1;
    } else {
        return 0;
    }
}


void parser_copy_string_from_source_reference(char *dest, const char *src) {
    size_t i = 0;
    while (src[i] != '\0' && lexer_is_letter(src[i])) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

static parser_ast_t *parse_with_declaration(parser_t *parser) {
    const char *source_reference = lexer_get_source_reference(&parser->lexer);

    const lexer_token_tag_t recovery_tokens[]   = {LEXER_TOKEN_TAG_R_PAREN};
    const size_t            num_recovery_tokens = sizeof(recovery_tokens) / sizeof(recovery_tokens[0]);

    if (!next_token_is(parser, LEXER_TOKEN_TAG_WITH)) {
        return parse_error(parser, source_reference, recovery_tokens, num_recovery_tokens, "Expected \"with\" keyword");
    }

    const char *identifiers[CONFIG_MAX_WITH_VARIABLES] = {0};
    size_t      num_identifiers                        = 0;

    lexer_token_t token;
    do {
        token = lexer_next(&parser->lexer);
        if (token.tag != LEXER_TOKEN_TAG_IDENTIFIER) {
            return parse_error(parser, source_reference, recovery_tokens, num_recovery_tokens, "Expected identifier");
        } else if (num_identifiers == CONFIG_MAX_WITH_VARIABLES) {
            return parse_error(parser, source_reference, recovery_tokens, num_recovery_tokens,
                               "Maximum number of bindable variables exceeded");
        }

        identifiers[num_identifiers++] = token.source_reference;

        token = lexer_next(&parser->lexer);
    } while (token.tag == LEXER_TOKEN_TAG_COMMA);

    if (token.tag != LEXER_TOKEN_TAG_COLON) {
        return parse_error(parser, source_reference, recovery_tokens, num_recovery_tokens, "Expected colon");
    }

    parser_ast_t *expr = parse_expr(parser);

    if (num_identifiers == 0) {
        return expr;
    } else {
        return create_with_declaration(source_reference, identifiers, expr);
    }
}


static parser_ast_t *parse_expr(parser_t *parser) {
    const char   *source_reference = lexer_get_source_reference(&parser->lexer);
    parser_ast_t *left             = parse_term(parser);

    lexer_token_t token = lexer_peek(&parser->lexer);
    while (token.tag == LEXER_TOKEN_TAG_PLUS || token.tag == LEXER_TOKEN_TAG_MINUS) {
        parser_operator_t op = token.tag == LEXER_TOKEN_TAG_PLUS ? PARSER_OPERATOR_PLUS : PARSER_OPERATOR_MINUS;
        lexer_next(&parser->lexer);
        parser_ast_t *right = parse_term(parser);

        left = create_binary_operation(source_reference, op, left, right);

        source_reference = lexer_get_source_reference(&parser->lexer);
        token            = lexer_peek(&parser->lexer);
    }

    return left;
}


static parser_ast_t *parse_term(parser_t *parser) {
    parser_ast_t *left             = parse_factor(parser);
    const char   *source_reference = lexer_get_source_reference(&parser->lexer);

    lexer_token_t token = lexer_peek(&parser->lexer);
    while (token.tag == LEXER_TOKEN_TAG_STAR || token.tag == LEXER_TOKEN_TAG_SLASH) {
        parser_operator_t op = token.tag == LEXER_TOKEN_TAG_STAR ? PARSER_OPERATOR_STAR : PARSER_OPERATOR_SLASH;
        lexer_next(&parser->lexer);
        parser_ast_t *right = parse_factor(parser);

        left = create_binary_operation(source_reference, op, left, right);

        source_reference = lexer_get_source_reference(&parser->lexer);
        token            = lexer_peek(&parser->lexer);
    }

    return left;
}


static parser_ast_t *parse_factor(parser_t *parser) {
    parser_ast_t *result           = NULL;
    const char   *source_reference = lexer_get_source_reference(&parser->lexer);

    const lexer_token_tag_t recovery_tokens[] = {
        LEXER_TOKEN_TAG_R_PAREN, LEXER_TOKEN_TAG_STAR,  LEXER_TOKEN_TAG_PLUS,
        LEXER_TOKEN_TAG_MINUS,   LEXER_TOKEN_TAG_SLASH,
    };
    const size_t num_recovery_tokens = sizeof(recovery_tokens) / sizeof(recovery_tokens[0]);

    switch (lexer_next(&parser->lexer).tag) {
        case LEXER_TOKEN_TAG_NUMBER:
            result = create_tagged(source_reference, PARSER_AST_TAG_NUMBER);
            break;

        case LEXER_TOKEN_TAG_IDENTIFIER:
            result = create_tagged(source_reference, PARSER_AST_TAG_IDENTIFIER);
            break;

        case LEXER_TOKEN_TAG_L_PAREN:
            result = parse_expr(parser);
            if (!next_token_is(parser, LEXER_TOKEN_TAG_R_PAREN)) {
                // TODO: memory leak in result
                result = parse_error(parser, source_reference, recovery_tokens, num_recovery_tokens,
                                     "Mismatched parentheses");
            }
            break;

        default:
            result = parse_error(parser, source_reference, recovery_tokens, num_recovery_tokens,
                                 "Expected number or identifier");
            break;
    }

    return result;
}


static parser_ast_t *create_tagged(const char *source_reference, parser_ast_tag_t tag) {
    parser_ast_t *with_declaration = (parser_ast_t *)malloc(sizeof(parser_ast_t));
    assert(with_declaration);
    with_declaration->tag              = tag;
    with_declaration->source_reference = source_reference;
    return with_declaration;
}


static parser_ast_t *create_binary_operation(const char *source_reference, parser_operator_t op, parser_ast_t *left,
                                             parser_ast_t *right) {
    parser_ast_t *binary_operation              = create_tagged(source_reference, PARSER_AST_TAG_BINARY_OPERATION);
    binary_operation->as.binary_operation.op    = op;
    binary_operation->as.binary_operation.left  = left;
    binary_operation->as.binary_operation.right = right;
    return binary_operation;
}


static parser_ast_t *create_with_declaration(const char *source_reference, const char **identifiers,
                                             parser_ast_t *expr) {
    parser_ast_t *with_declaration = create_tagged(source_reference, PARSER_AST_TAG_WITH_DECLARATION);
    memcpy(with_declaration->as.with_declaration.identifiers, identifiers,
           sizeof(with_declaration->as.with_declaration.identifiers));
    with_declaration->as.with_declaration.expr = expr;
    return with_declaration;
}


static parser_ast_t *parse_error(parser_t *parser, const char *source_reference,
                                 const lexer_token_tag_t *recovery_tokens, size_t num_recovery_tokens,
                                 const char *error_description) {
    parser_ast_t *error = (parser_ast_t *)malloc(sizeof(parser_ast_t));
    assert(error);
    error->tag                  = PARSER_AST_TAG_ERROR;
    error->source_reference     = source_reference;
    error->as.error.description = error_description;
    size_t reference_start      = parser->lexer.buffer_index;

    lexer_token_t token;
    uint8_t       is_recovery = 0;

    while (!is_recovery) {
        token = lexer_peek(&parser->lexer);
        if (token.tag == LEXER_TOKEN_TAG_TAG_EOI) {
            is_recovery = 1;
        } else {
            for (size_t i = 0; i < num_recovery_tokens; i++) {
                if (token.tag == recovery_tokens[i]) {
                    is_recovery = 1;
                    break;
                }
            }
        }

        if (is_recovery) {
            break;
        } else {
            lexer_next(&parser->lexer);
        }
    }

    error->as.error.reference_length = parser->lexer.buffer_index - reference_start;

    return error;
}


static uint8_t next_token_is(parser_t *parser, lexer_token_tag_t tag) {
    lexer_token_t token = lexer_next(&parser->lexer);
    return token.tag == tag;
}
