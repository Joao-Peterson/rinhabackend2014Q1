#ifndef _UTILS_HEADER_
#define _UTILS_HEADER_

#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>

// parse id and action from path: "/jdhuiasda/32847239/action"
char *parseIdAction(char *path, int64_t *idOut);

// parse json transaction
bool parseTransa(char *transa, int64_t *valor_out, char *tipo_out, char **descricao_out);

#endif