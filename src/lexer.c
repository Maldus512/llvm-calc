#include <string.h>
#include "lexer.h"


#define CHAR_AT(Lexer, Pos) ((Lexer)->buffer[(Pos)])
#define CHAR_CURRENT(Lexer) CHAR_AT(Lexer, (Lexer)->buffer_index)


static uint8_t       is_digit(char c);
static lexer_token_t create_token_here(lexer_t *lexer, lexer_token_tag_t tag);
static lexer_token_t create_token_at(lexer_t *lexer, lexer_token_tag_t tag, size_t buffer_index);


uint8_t lexer_token_is(lexer_token_t token, lexer_token_tag_t tag) {
    return token.tag == tag;
}


const char *lexer_get_source_reference(lexer_t *lexer) {
    // Also skip ahed the whitespace
    while (CHAR_CURRENT(lexer) != '\0' && lexer_is_whitespace(CHAR_CURRENT(lexer))) {
        lexer->buffer_index++;
    }
    return &lexer->buffer[lexer->buffer_index];
}


lexer_token_t lexer_peek(lexer_t *lexer) {
    size_t        buffer_index = lexer->buffer_index;
    lexer_token_t token        = lexer_next(lexer);
    lexer->buffer_index        = buffer_index;
    return token;
}


lexer_token_t lexer_next(lexer_t *lexer) {
    while (CHAR_CURRENT(lexer) != '\0' && lexer_is_whitespace(CHAR_CURRENT(lexer))) {
        lexer->buffer_index++;
    }

    if (CHAR_CURRENT(lexer) == '\0') {
        return create_token_here(lexer, LEXER_TOKEN_TAG_TAG_EOI);
    } else if (lexer_is_letter(CHAR_CURRENT(lexer))) {
        size_t end_index = lexer->buffer_index + 1;
        while (lexer_is_letter(CHAR_AT(lexer, end_index))) {
            end_index++;
        }

        uint8_t       is_with = memcmp("with", &lexer->buffer[lexer->buffer_index], end_index - lexer->buffer_index) == 0;
        lexer_token_t token   = create_token_here(lexer, is_with ? LEXER_TOKEN_TAG_WITH : LEXER_TOKEN_TAG_IDENTIFIER);
        lexer->buffer_index   = end_index;
        return token;
    } else if (is_digit(CHAR_CURRENT(lexer))) {
        size_t end_index = lexer->buffer_index + 1;
        while (is_digit(CHAR_AT(lexer, end_index))) {
            end_index++;
        }

        lexer_token_t token = create_token_here(lexer, LEXER_TOKEN_TAG_NUMBER);
        lexer->buffer_index = end_index;
        return token;
    } else {
        char c = CHAR_CURRENT(lexer);
        lexer->buffer_index++;

        switch (c) {
            case '+':
                return create_token_at(lexer, LEXER_TOKEN_TAG_PLUS, lexer->buffer_index - 1);
            case '-':
                return create_token_at(lexer, LEXER_TOKEN_TAG_MINUS, lexer->buffer_index - 1);
            case '*':
                return create_token_at(lexer, LEXER_TOKEN_TAG_STAR, lexer->buffer_index - 1);
            case '/':
                return create_token_at(lexer, LEXER_TOKEN_TAG_SLASH, lexer->buffer_index - 1);
            case '(':
                return create_token_at(lexer, LEXER_TOKEN_TAG_L_PAREN, lexer->buffer_index - 1);
            case ')':
                return create_token_at(lexer, LEXER_TOKEN_TAG_R_PAREN, lexer->buffer_index - 1);
            case ':':
                return create_token_at(lexer, LEXER_TOKEN_TAG_COLON, lexer->buffer_index - 1);
            case ',':
                return create_token_at(lexer, LEXER_TOKEN_TAG_COMMA, lexer->buffer_index - 1);
            default:
                return create_token_at(lexer, LEXER_TOKEN_TAG_UNKNOWN, lexer->buffer_index - 1);
        }
    }
}


uint8_t lexer_is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\f' || c == '\v' || c == '\r' || c == '\n';
}


uint8_t lexer_is_letter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}


static lexer_token_t create_token_at(lexer_t *lexer, lexer_token_tag_t tag, size_t buffer_index) {
    const char *source_reference = &lexer->buffer[buffer_index];
    return (lexer_token_t){.tag = tag, .source_reference = source_reference};
}


static lexer_token_t create_token_here(lexer_t *lexer, lexer_token_tag_t tag) {
    return create_token_at(lexer, tag, lexer->buffer_index);
}


static uint8_t is_digit(char c) {
    return c >= '0' && c <= '9';
}

