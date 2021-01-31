#ifndef RAFAEL_VARS_H
#define RAFAEL_VARS_H

#include "statement.h"

typedef struct variable {
	const char *name;
	ref_counter_t *name_rc;
	const arg_t *def;
	struct variable *next;
} var_t;

arg_t *get_var_copy(const char *name);

void set_var_globally(const char *name, ref_counter_t *rc, const arg_t *def);

void add_var_to_context(const char *name, ref_counter_t *rc, const arg_t *def);

#endif //RAFAEL_VARS_H
