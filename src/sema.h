#ifndef SEMA_H_INCLUDED
#define SEMA_H_INCLUDED


#include "parser.h"
#include "config.h"


typedef struct {
    const char *bindings[CONFIG_MAX_WITH_VARIABLES];
} sema_t;


uint8_t sema_check(sema_t *sema, parser_ast_t *ast);


#endif
