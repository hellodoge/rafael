#include "include/context.h"

#include "include/gc.h"

#include <stdlib.h>

context_t *context = NULL;

void init_context(void) {
	context_t *new_context = malloc(sizeof(context_t));
	new_context->context = context;
	new_context->vars = NULL;
	context = new_context;
}

void delete_context(void) {
	context_t *global = context->context;
	delete_vars(context->vars);
	free(context);
	context = global;
}