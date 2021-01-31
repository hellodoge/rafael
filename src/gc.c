#include "include/gc.h"

#include "include/statement.h"
#include "include/context.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

ref_counter_t *init_ref_counter() {
	ref_counter_t *rc = malloc(sizeof(ref_counter_t));
	*rc = 1;
	return rc;
}

void delete(arg_t *args) {
	for (arg_t *arg = args, *next = NULL; arg != NULL; arg = next) {
		next = arg->next;
		if (arg->type & F_ALLOCATED) {
			forget(arg);
		}
		free(arg);
	}
}

void delete_stm(stm_t *stm) {
	raw_forget((void *) stm->name, stm->name_rc);
	delete((arg_t *) stm->args);
	free(stm);
}

void delete_vars(var_t *var) {
	for (var_t *c = var, *next = NULL; c != NULL; c = next) {
		next = c->next;
		delete((arg_t *) c->def);
		raw_forget((void *) (c->name), c->name_rc);
		free(c);
	}
}

void forget(arg_t *arg) {
	assert(arg->type & F_ALLOCATED);
	assert(arg->content != NULL);
	assert(arg->rc != NULL);
	assert(*arg->rc > 0);
	(*arg->rc)--;
	if (*arg->rc == 0) {
		free(arg->rc);
		if (arg->type & T_STATEMENT) {
			delete_stm((stm_t *) arg->statement);
		} else {
			free(arg->content);
		}
	}
	arg->rc = NULL;
	arg->content = NULL;
}

void raw_forget(void *data, ref_counter_t *rc) {
	assert(data != NULL);
	assert(rc != NULL);
	assert(*rc > 0);
	(*rc)--;
	if (*rc == 0) {
		free(rc);
		free(data);
	}
}