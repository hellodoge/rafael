#ifndef RAFAEL_PARSER_H
#define RAFAEL_PARSER_H

#include <stdlib.h>

#include "statement.h"

typedef struct slice {
	const char *start;
	const char *end;
} slice_t;

stm_t *parse(const char **textPtr);

slice_t get_expression(const char *text);



#endif //RAFAEL_PARSER_H
