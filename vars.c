#include "vars.h"

#include "context.h"
#include "args.h"
#include "gc.h"

#include <stdlib.h>
#include <string.h>

var_t *find_var_in_list(const char *name, var_t *vars);

void add_var(const char *name, ref_counter_t *rc, const arg_t *def, var_t **vars) {
	///safety: you shouldn't use def after calling this function
	///safety: vars must be the first element of context.vars
	var_t *current = find_var_in_list(name, *vars);
	if (current != NULL) {
		raw_forget((void*)current->name, current->name_rc);
		delete((arg_t*)current->def);
	} else {
		current = malloc(sizeof(var_t));
		if (*vars == NULL) {
			*vars = current;
		} else {
			var_t *var = *vars;
			for (; var->next != NULL; var = var->next);
			var->next = current;
		}
		current->next = NULL;
	}
	current->name = name;
	current->name_rc = rc;
	(*current->name_rc)++;
	current->def = def;
}

struct context_and_var find_var(const char *name);

void add_var_to_context(const char *name, ref_counter_t *rc, const arg_t *def) {
	///safety: you shouldn't use def after calling this function
	add_var(name, rc, def, &context->vars);
}

void set_var_globally(const char *name, ref_counter_t *rc, const arg_t* def) {
	///safety: you shouldn't use def after calling this function
    struct context_and_var context_var = find_var(name);
    if (context_var.vars != NULL) {
        delete((arg_t*) context_var.vars->def);
        context_var.vars->def = def;
        raw_forget((void*)context_var.vars->name, context_var.vars->name_rc);
        context_var.vars->name = name;
        context_var.vars->name_rc = rc;
	    (*context_var.vars->name_rc)++;
        return;
    }
    add_var(name, rc, def, &context_var.context->vars);
}

arg_t *get_var_copy(const char *name) {
	var_t *var = find_var(name).vars;
	if (var == NULL)
		return init_arg(T_NULL);
	return copy_arg(var->def, true);
}

struct context_and_var find_var(const char *name) {
    for (context_t *current = context;; current = current->context) {
        struct context_and_var ret = {current, NULL};
        ret.vars = find_var_in_list(name, current->vars);
        if (ret.vars != NULL || current->context == NULL)
            return ret;
    }
}

var_t *find_var_in_list(const char *name, var_t *vars) {
	for (; vars != NULL; vars = vars->next) {
		if (!strcmp(vars->name, name)) {
			return vars;
		}
	}
	return NULL;
}



