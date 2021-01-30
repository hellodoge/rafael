#ifndef RAFAEL_CONTEXT_H
#define RAFAEL_CONTEXT_H

#include "vars.h"

typedef struct context_and_var context_t;

struct context_and_var {
	context_t *context;
	var_t *vars;
};

extern context_t *context;

void init_context(void);

void delete_context(void);


#endif //RAFAEL_CONTEXT_H
