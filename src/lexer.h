#ifndef LEXER_H_INCLUDED
#define LEXER_H_INCLUDED


#include <stdint.h>
#include <stdlib.h>


typedef enum {
    LEXER_TOKEN_TAG_TAG_EOI = 0,
    LEXER_TOKEN_TAG_UNKNOWN,
    LEXER_TOKEN_TAG_IDENTIFIER,
    LEXER_TOKEN_TAG_NUMBER,
    LEXER_TOKEN_TAG_COMMA,
    LEXER_TOKEN_TAG_COLON,
    LEXER_TOKEN_TAG_PLUS,
    LEXER_TOKEN_TAG_MINUS,
    LEXER_TOKEN_TAG_STAR,
    LEXER_TOKEN_TAG_SLASH,
    LEXER_TOKEN_TAG_L_PAREN,
    LEXER_TOKEN_TAG_R_PAREN,
    LEXER_TOKEN_TAG_WITH,
} lexer_token_tag_t;

typedef struct {
    lexer_token_tag_t tag;
    const char       *source_reference;
} lexer_token_t;

typedef struct {
    const char *buffer;
    size_t      buffer_index;
} lexer_t;


uint8_t       lexer_token_is(lexer_token_t token, lexer_token_tag_t tag);
lexer_token_t lexer_next(lexer_t *lexer);
lexer_token_t lexer_peek(lexer_t *lexer);
const char   *lexer_get_source_reference(lexer_t *lexer);
uint8_t       lexer_is_whitespace(char c);
uint8_t       lexer_is_letter(char c);


#endif
