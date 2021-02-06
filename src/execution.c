#include "include/statement.h"
#include "include/terminate.h"
#include "include/functions.h"
#include "include/vars.h"
#include "include/context.h"
#include "include/args.h"
#include "include/gc.h"

#include <stddef.h>
#include <assert.h>
#include <string.h>


arg_t *convert_args(const arg_t *arg, fnc_flags_t fnc_flags);

arg_t *macro_processor(const arg_t *macro, const arg_t *args);

arg_t *execute(const stm_t *stm) {

	arg_t *ret = NULL;
	arg_t *macro = NULL;
	const fnc_t *fnc = get_predefined(stm->name);
	if (fnc != NULL) {
		arg_t *args = convert_args(stm->args, fnc->flags);
		RETURN_IF_TERMINATE(ret, args)

		ret = fnc->func(args);
		delete(args);
	} else {
		macro = get_var_copy(stm->name);
		if (!args_match_pattern(macro, T_TOKEN | F_MULTIPLE | F_OPTIONAL, T_STATEMENT | F_MULTIPLE, F_END)) {
			EXCEPTION(ret, "No macro specified by %s", stm->name)
		}
		ret = macro_processor(macro, stm->args);
	}
err:
	if (macro != NULL)
		delete(macro);
	return safe_ret(ret);
}

arg_t *execute_inner_stm(fnc_ptr_t function, const arg_t *args, bool pass_next);

arg_t *convert_macro_args(const arg_t *args, const arg_t *macro);

extern arg_t *ctrl_exec(const arg_t *arg);

arg_t *macro_processor(const arg_t *macro_def, const arg_t *args) {
	init_context();
	arg_t *ret = NULL;
	arg_t *macro = convert_macro_args(args, macro_def);
	RETURN_IF_TERMINATE(ret, macro);
	ret = execute_inner_stm(ctrl_exec, macro, true);
err:
	delete(macro);
	delete_context();
	return ret;
}

arg_t *execute_inner_stm(fnc_ptr_t function, const arg_t *args, bool pass_next) {
	arg_t *copy = copy_arg(args, pass_next);
	arg_t *ret = function(copy);
	delete(copy);
	return safe_ret(ret);
}

arg_t *convert_argument(const arg_t *arg, fnc_flags_t fnc_flags);

arg_t *convert_args(const arg_t *arg, fnc_flags_t fnc_flags) {
	arg_t *ret = NULL;
	while (arg != NULL) {
		arg_t *current = convert_argument(arg, fnc_flags);
		RETURN_IF_TERMINATE(ret, current);
		add_arg(&ret, current);
		arg = arg->next;
	}
err:
	return safe_ret(ret);
}

arg_t *convert_argument(const arg_t *arg, fnc_flags_t fnc_flags) {
	assert(arg != NULL);
	if (arg->type & T_STATEMENT && (!(fnc_flags & FF_DO_NOT_EXECUTE_ARGS))) {
		return execute(arg->statement);
	} else if (arg->type & T_VAR) {
		arg_t *copy = get_var_copy(arg->string);
		for (arg_t *guess = copy; guess != NULL; guess = guess->next) {
			if (guess->type == T_STATEMENT) {
				add_exception(&copy, "function must not return a macro");
				break;
			}
		}
		return copy;
	} else {
		return copy_arg(arg, false);
	}
}

arg_t *convert_macro_args(const arg_t *args, const arg_t *macro) {
	arg_t *ret = NULL;
	arg_t *def = NULL;
	arg_t *additional = NULL;
	for (const arg_t *arg = args; arg != NULL; arg = arg->next) {
		if (arg->type == T_NULL)
			continue;
		arg_t *copy = copy_arg(arg, false);
		def = convert_args(copy, FF_NONE);
		delete(copy);
		RETURN_IF_TERMINATE(ret, def);
		if (macro->type & T_TOKEN) {
			add_var_to_context(macro->string, macro->rc, def);
			macro = macro->next;
		} else { //if (macro->type & T_STATEMENT) (should be guaranteed by ctrl_def)
			add_arg(&additional, def);
		}
		def = NULL;
	}
	ref_counter_t *rc_args = init_ref_counter();
	add_var_to_context(strdup("args"), rc_args,
	                   additional != NULL ? additional : init_arg(T_NULL));
	(*rc_args)--; //add_var_to_context increments the reference counter
	if (macro->type & T_STATEMENT) {
		ret = copy_arg(macro, true);
	} else {
		EXCEPTION(ret, "Too few arguments to macro call");
	}
err:
	if (def != NULL)
		delete(def);
	return ret;
}