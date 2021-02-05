#include "include/functions.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>

#include "include/script_loader.h"
#include "include/statement.h"
#include "include/execution.h"
#include "include/terminate.h"
#include "include/args.h"
#include "include/vars.h"
#include "include/util.h"
#include "include/gc.h"

arg_t *ctrl_define(const arg_t *arg);

arg_t *fnc_return(const arg_t *args);

arg_t *fnc_null(const arg_t *args);

arg_t *fnc_print(const arg_t *arg);

arg_t *fnc_input(const arg_t *arg);

arg_t *fnc_to_str(const arg_t *arg);

arg_t *fnc_to_int(const arg_t *arg);

arg_t *fnc_to_real(const arg_t *args);

arg_t *fnc_sum(const arg_t *arg);

arg_t *fnc_mul(const arg_t *arg);

arg_t *fnc_get(const arg_t *args);

arg_t *fnc_set(const arg_t *args);

arg_t *fnc_less(const arg_t *arg);

arg_t *fnc_more(const arg_t *arg);

arg_t *fnc_less_or_equal(const arg_t *arg);

arg_t *fnc_more_or_equal(const arg_t *arg);

arg_t *fnc_equal(const arg_t *arg);

arg_t *fnc_not_equal(const arg_t *arg);

arg_t *ctrl_exec(const arg_t *arg);

arg_t *ctrl_if(const arg_t *arg);

arg_t *ctrl_while(const arg_t *arg);

arg_t *ctrl_case(const arg_t *arg);

arg_t *fnc_join(const arg_t *arg);

arg_t *fnc_exit(const arg_t *arg);

arg_t *fnc_repeat(const arg_t *arg);

arg_t *fnc_index(const arg_t *arg);

arg_t *fnc_len(const arg_t *arg);

arg_t *fnc_global(const arg_t *arg);

arg_t *ctrl_for(const arg_t *arg);

arg_t *fnc_raise(const arg_t *arg);

arg_t *fnc_struct(const arg_t *arg);

arg_t *fnc_import(const arg_t *arg);

arg_t *ctrl_lambda(const arg_t *arg);

arg_t *ctrl_run_lambda(const arg_t *args);

arg_t *fnc_split(const arg_t *args);

arg_t *fnc_replace(const arg_t *args);

arg_t *fnc_div(const arg_t *args);

const fnc_t predefined[] = {
		{"null",    fnc_null},
		{"ret",     fnc_return},
		{"print",   fnc_print},
		{"input",   fnc_input},
		{"str",     fnc_to_str},
		{"int",     fnc_to_int},
		{"real",    fnc_to_real},
		{"+",       fnc_sum},
		{"sum",     fnc_sum},
		{"*",       fnc_mul},
		{"mul",     fnc_mul},
		{"div",     fnc_div},
		{"/",       fnc_div},
		{"get",     fnc_get},
		{"set",     fnc_set},
		{"global",  fnc_global},
		{"<",       fnc_less},
		{">",       fnc_more},
		{"<=",      fnc_less_or_equal},
		{">=",      fnc_more_or_equal},
		{"==",      fnc_equal},
		{"<>",      fnc_not_equal},
		{"!=",      fnc_not_equal},
		{"join",    fnc_join},
		{"repeat",  fnc_repeat},
		{"replace", fnc_replace},
		{"split",   fnc_split},
		{"len",     fnc_len},
		{"index",   fnc_index},
		{"struct",  fnc_struct},
		{"exit",    fnc_exit},
		{"raise",   fnc_raise},
		{"import",  fnc_import},
		{"def",     ctrl_define,     FF_DO_NOT_EXECUTE_ARGS},
		{"lambda",  ctrl_lambda,     FF_DO_NOT_EXECUTE_ARGS},
		{"exec",    ctrl_exec,       FF_DO_NOT_EXECUTE_ARGS},
		{"run",     ctrl_run_lambda, FF_DO_NOT_EXECUTE_ARGS},
		{"if",      ctrl_if,         FF_DO_NOT_EXECUTE_ARGS},
		{"while",   ctrl_while,      FF_DO_NOT_EXECUTE_ARGS},
		{"for",     ctrl_for,        FF_DO_NOT_EXECUTE_ARGS},
		{"case",    ctrl_case,       FF_DO_NOT_EXECUTE_ARGS}
};

const fnc_t *get_predefined(const char *string) {
	for (int i = 0; i < sizeof(predefined) / sizeof(fnc_t); i++) {
		if (!strcmp(string, predefined[i].name)) {
			return &predefined[i];
		}
	}
	return NULL;
}

arg_t *ctrl_define(const arg_t *arg) {
	arg_t *ret = NULL;
	if (!args_match_pattern(arg, T_TOKEN | F_MULTIPLE, T_STATEMENT | F_MULTIPLE, F_END)) {
		EXCEPTION(ret, "define: invalid arguments")
	}
	set_var_globally(arg->string, arg->rc, copy_arg(arg->next, true));
err:
	return ret;
}

arg_t *ctrl_lambda(const arg_t *args) {
	arg_t *ret = NULL;
	if (!args_match_pattern(args, T_TOKEN | F_MULTIPLE | F_OPTIONAL, T_STATEMENT | F_MULTIPLE, F_END)) {
		EXCEPTION(ret, "lambda: invalid arguments")
	}
	ret = init_arg(T_LAMBDA | F_ORIGINAL);
	ret->lambda = copy_arg(args, true);
err:
	return ret;
}

arg_t *fnc_cond(const arg_t *arg) {
	arg_t *ret = NULL,
			*cnd = NULL;
	if (arg->type & T_STATEMENT) {
		cnd = execute(arg->statement);
		RETURN_IF_TERMINATE(ret, cnd)
	} else
		cnd = copy_arg(arg, true);
	if (!args_match_pattern(cnd, F_NUMBER | F_MULTIPLE | F_OPTIONAL, F_END)) {
		EXCEPTION(ret, "condition: invalid arguments")
	}

	arg_t *tmp = execute_inner_stm(fnc_to_real, cnd, true);
	add_arg(&ret, execute_inner_stm(fnc_sum, tmp, true));
	delete(tmp);
err:
	delete(cnd);
	return ret;
}

arg_t *ctrl_if(const arg_t *args) {
	arg_t *ret = NULL,
			*cnd = NULL;
	if (!args_match_pattern(args, F_BOOLEAN, T_STATEMENT | F_MULTIPLE, T_TOKEN | F_OPTIONAL,
	                        T_STATEMENT | F_MULTIPLE | F_OPTIONAL, F_END)) {
		EXCEPTION(ret, "if: invalid arguments")
	}

	cnd = execute_inner_stm(fnc_cond, args, false);
	RETURN_IF_TERMINATE(ret, cnd)

	if (cnd->floating != 0) {
		ret = execute_inner_stm(ctrl_exec, args->next, true);
		if (ret->type & F_TERMINATE)
			goto err;
	} else {
		arg_t else_tok = {T_TOKEN, "else"};
		const arg_t *else_statement = get_next_to_given(args->next, &else_tok);
		if (else_statement == NULL)
			goto err;
		ret = execute_inner_stm(ctrl_exec, else_statement, true);
		if (ret->type & F_TERMINATE)
			goto err;
	}
err:
	if (cnd != NULL)
		delete(cnd);
	return ret;
}

arg_t *ctrl_case(const arg_t *args) {
	arg_t *ret = NULL,
			*cnd = NULL;
	if (!args_match_pattern(args, T_STATEMENT | F_NUMBER | T_TOKEN | T_STRING, T_NULL)) {
		EXCEPTION(ret, "case: invalid arguments")
	}
	if (args->type & T_STATEMENT) {
		cnd = execute(args->statement);
		RETURN_IF_TERMINATE(ret, cnd)
	} else
		cnd = copy_arg(args, false);
	const arg_t *case_statement = get_next_to_given(args->next, cnd);
	if (case_statement == NULL) {
		arg_t default_tok = {T_TOKEN, "default"};
		case_statement = get_next_to_given(args->next, &default_tok);
	}
	if (case_statement != NULL) {
		while (case_statement->type != T_STATEMENT) {
			case_statement = case_statement->next;
			if (case_statement == NULL)
				goto err;
		}
		ret = execute_inner_stm(ctrl_exec, case_statement, true);
		if (ret->type & F_TERMINATE)
			goto err;
	}
err:
	if (cnd != NULL)
		delete(cnd);
	return ret;
}

arg_t *ctrl_while(const arg_t *args) {
	arg_t *ret = init_arg(T_INT),
			*cnd = NULL,
			*res = NULL;
	if (!args_match_pattern(args, F_BOOLEAN, T_STATEMENT | F_MULTIPLE, F_END)) {
		EXCEPTION(ret, "while: invalid arguments")
	}
	for (;; ret->value++) {
		cnd = execute_inner_stm(fnc_cond, args, false);
		RETURN_IF_TERMINATE(ret, cnd)
		if (cnd->floating == 0)
			goto err;
		delete(cnd);
		cnd = NULL;
		res = execute_inner_stm(ctrl_exec, args->next, true);
		RETURN_IF_TERMINATE(ret, res)
		delete(res);
		res = NULL;
	}
err:
	if (cnd != NULL)
		delete(cnd);
	if (res != NULL)
		delete(res);
	return ret;
}

arg_t *ctrl_for(const arg_t *args) {
	arg_t *ret = NULL;
	const arg_t *stm = NULL;
	arg_t *argv = NULL;
	if (args->type != T_TOKEN) {
		EXCEPTION(ret, "for: variable name not found");
	}
	const arg_t *var_name = args;
	if (args->next == NULL) {
		EXCEPTION(ret, "for: invalid arguments")
	}
	args = args->next;
	if (args->type == T_STATEMENT) {
		argv = execute(args->statement);
		RETURN_IF_TERMINATE(ret, argv);
	}
	for (const arg_t *arg = args; arg != NULL; arg = arg->next) {
		if (arg->type == T_STATEMENT) {
			stm = arg;
			break;
		} else if (args->type != T_STATEMENT)
			add_arg(&argv, copy_arg(arg, false));
	}
	if (stm == NULL) {
		EXCEPTION(ret, "for: statement not found");
	}
	for (const arg_t *arg = argv; arg != NULL; arg = arg->next) {
		if (arg->type == T_STATEMENT)
			goto err;
		set_var_globally(var_name->string, var_name->rc, copy_arg(arg, false));
		arg_t *res = execute_inner_stm(ctrl_exec, stm, true);
		RETURN_IF_TERMINATE(ret, res);
		add_arg(&ret, res);
	}
err:
	if (argv != NULL)
		delete(argv);
	return ret;
}

arg_t *fnc_set(const arg_t *args) {
	arg_t *ret = NULL;
	if (!args_match_pattern(args, F_HAVE_STR, T_NULL)) {
		EXCEPTION(ret, "set: invalid arguments")
	}
	ret = args->next != NULL ? copy_arg(args->next, true) : init_arg(T_NULL);
	add_var_to_context(args->string, args->rc, copy_arg(ret, true));
err:
	return ret;
}

arg_t *fnc_global(const arg_t *args) {
	arg_t *ret = NULL;
	if (!args_match_pattern(args, F_HAVE_STR, T_NULL)) {
		EXCEPTION(ret, "global: invalid arguments")
	}
	ret = args->next != NULL ? copy_arg(args->next, true) : init_arg(T_NULL);
	set_var_globally(args->string, args->rc, copy_arg(ret, true));
err:
	return ret;
}

arg_t *fnc_get(const arg_t *args) {
	arg_t *ret = NULL;
	if (!args_match_pattern(args, T_TOKEN | F_MULTIPLE, F_END)) {
		EXCEPTION(ret, "get: invalid arguments")
	}
	for (const arg_t *arg = args; arg != NULL; arg = arg->next)
		add_arg(&ret, get_var_copy(arg->string));
	for (const arg_t *arg = ret; arg != NULL; arg = arg->next) {
		if (arg->type == T_STATEMENT) {
			EXCEPTION(ret, "get: cannot return statement");
		}
	}
err:
	return ret;
}

arg_t *ctrl_exec(const arg_t *args) {
	arg_t *ret = NULL;
	for (const arg_t *arg = args; arg != NULL; arg = arg->next) {
		if (arg->type & T_STATEMENT) {
			arg_t *res = execute(arg->statement);
			RETURN_IF_TERMINATE(ret, res)
			add_arg(&ret, res);
		} else
			break;
	}
err:
	return ret;
}

arg_t *ctrl_run_lambda(const arg_t *args) {
	arg_t *ret = NULL,
	      *lambda = NULL;
	if (!args_match_pattern(args, T_LAMBDA | T_STATEMENT, T_NULL)) {
		EXCEPTION(ret, "run: invalid arguments")
	}
	if (args->type == T_LAMBDA)
		lambda = copy_arg(args, false);
	else { // if (args->type == T_STATEMENT)
		lambda = execute(args->statement);
		RETURN_IF_TERMINATE(ret, lambda);
		if (lambda->type != T_LAMBDA || lambda->next != NULL) {
			EXCEPTION(ret, "run: the first statement must return one lambda");
		}
	}
	ret = macro_processor(lambda->lambda, args->next);
err:
	if (lambda != NULL)
		delete(lambda);
	return ret;
}

arg_t *fnc_return(const arg_t *args) {
	return copy_arg(args, true);
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

arg_t *fnc_null(const arg_t *args) {
	return init_arg(T_NULL);
}

#pragma clang diagnostic pop

arg_t *fnc_to_str(const arg_t *args) {
	arg_t *ret = init_arg(T_STRING | F_ORIGINAL);
	size_t len = 1;
	for (const arg_t *arg = args; arg != NULL; arg = arg->next) {
		if (arg->type & F_HAVE_STR)
			len += strlen(arg->string);
		else if (arg->type & (F_NUMBER)) {
			if (get_real(arg) == 0)
				len++;
			else {
				len += ceil(log10(get_real(arg))) + 1;
				if (get_real(arg) < 0) len++;
			}
			if (arg->type & T_REAL)
				len += 7; // "%.6f"
		}
	}
	char *string = malloc(len * sizeof(char));
	char *cursor = string;
	for (const arg_t *arg = args; arg != NULL; arg = arg->next) {
		if (arg->type & F_HAVE_STR)
			cursor += sprintf(cursor, "%s", arg->string);
		else if (arg->type & T_INT)
			cursor += sprintf(cursor, "%d", arg->value);
		else if (arg->type & T_REAL)
			cursor += sprintf(cursor, "%.6f", arg->floating);
	}
	*cursor = '\0';
	ret->string = string;
	return ret;
}

arg_t *fnc_print(const arg_t *args) {
	arg_t *len = init_arg(T_INT);
	for (const arg_t *arg = args; arg != NULL; arg = arg->next) {
		if (arg->type & F_HAVE_STR) {
			len->value += (int) strlen(arg->string);
			printf("%s", arg->string);
		} else {
			arg_t *res = execute_inner_stm(fnc_to_str, arg, false);
			len->value += (int) strlen(res->string);
			printf("%s", res->string);
			delete(res);
		}
		if (arg->next != NULL)
			putchar(' ');
	}
	return len;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

arg_t *fnc_input(const arg_t *args) {
	arg_t *ret = init_arg(T_STRING | F_ORIGINAL);
	bool eof_flag = false;
	ret->string = get_unlimited_input(NULL, &eof_flag);
	if (!eof_flag)
		*(char *) (ret->string + strlen(ret->string) - 1) = '\0';
	return ret;
}

#pragma clang diagnostic pop

arg_t *fnc_split(const arg_t *args) {
	arg_t *ret = NULL;
	if (args_match_pattern(args, T_INT, T_STRING | F_MULTIPLE | F_OPTIONAL, F_END)) {
		for (arg_t *arg = args->next; arg != NULL; arg = arg->next) {
			for (const char *cursor = arg->string; *cursor != '\0';) {
				char *chars = get_n_chars_from_utf_8_str(cursor, args->value);
				if (chars == NULL) {
					EXCEPTION(ret, "split: the string is not valid utf-8");
				}
				arg_t *current = init_arg(T_STRING | F_ORIGINAL);
				current->string = chars;
				add_arg(&ret, current);
				cursor += strlen(chars);
			}
		}
	} else if (args_match_pattern(args, T_STRING | F_MULTIPLE, F_END)) {
		for (arg_t *arg = args->next; arg != NULL; arg = arg->next) {
			for (const char *cursor = arg->string; *cursor != '\0';) {
				char *occurrence = strstr(cursor, args->string);
				if (occurrence != cursor) {
					arg_t *current = init_arg(T_STRING | F_ORIGINAL);
					if (occurrence == NULL) {
						current->string = strdup(cursor);
						add_arg(&ret, current);
						break;
					}
					current->string = strndup(cursor, occurrence - cursor);
					add_arg(&ret, current);
				}
				cursor = occurrence + strlen(args->string);
			}
		}
	} else {
		EXCEPTION(ret, "split: invalid arguments");
	}
err:
	return ret;
}

arg_t *fnc_join(const arg_t *args) {
	arg_t *ret = NULL;
	for (const arg_t *arg = args->next; arg != NULL; arg = arg->next) {
		add_arg(&ret, copy_arg(arg, false));
		if (arg->next != NULL)
			add_arg(&ret, copy_arg(args, false));
	}
	return ret;
}

arg_t *fnc_repeat(const arg_t *args) {
	arg_t *ret = NULL;
	if (!args_match_pattern(args, T_INT, T_NULL)) {
		EXCEPTION(ret, "repeat: invalid arguments")
	}
	if (args->next == NULL)
		goto err;
	for (int i = 0; i < args->value; i++) {
		add_arg(&ret, copy_arg(args->next, true));
	}
err:
	return ret;
}

arg_t *fnc_replace(const arg_t *args) {
	arg_t *ret = NULL;
	if (args->type == T_NULL || args->next == NULL) {
		EXCEPTION(ret, "replace: invalid arguments")
	}
	const arg_t *remove = args;
	const arg_t *insert = args->next;
	for (const arg_t *arg = args->next->next; arg != NULL; arg = arg->next) {
		arg_t *current;
		if (compare(arg, remove) == 0) {
			current = copy_arg(insert, false);
		} else {
			current = copy_arg(arg, false);
		}
		add_arg(&ret, current);
	}
err:
	return ret;
}

arg_t *fnc_index(const arg_t *args) {
	arg_t *ret = NULL;
	arg_t *len = NULL;
	if (!args_match_pattern(args, T_INT, T_NULL)) {
		EXCEPTION(ret, "ret: invalid arguments");
	}
	if (args->next == NULL)
		goto err;
	len = execute_inner_stm(fnc_len, args->next, true);
	{
		int index = args->value;
		args = args->next;
		if (index < 0)
			index = len->value + index;
		if (index >= len->value)
			goto err;
		for (int i = 0; i < index; i++) {
			args = args->next;
			assert(args != NULL);
		}
	}
	ret = copy_arg(args, false);
err:
	if (len != NULL)
		delete(len);
	return ret;
}

arg_t *fnc_struct(const arg_t *args) {
	arg_t *ret = NULL;
	if (!args_match_pattern(args, T_TOKEN, T_NULL)) {
		EXCEPTION(ret, "struct: invalid arguments");
	}
	const arg_t *elem = get_next_to_given(args->next, args);
	for (; elem != NULL && elem->type != T_TOKEN; elem = elem->next) {
		add_arg(&ret, copy_arg(elem, false));
	}
err:
	return ret;
}

arg_t *fnc_len(const arg_t *args) {
	arg_t *ret = init_arg(T_INT);
	if (args->type == T_NULL)
		return ret;
	do {
		ret->value++;
	} while ((args = args->next) != NULL);
	return ret;
}

#define FNC_COMPARE(name, cmp) \
arg_t *name(const arg_t *args) { \
    arg_t *ret = init_arg(T_INT); \
    for (const arg_t *arg = args; arg->next != NULL; arg = arg->next) { \
        if (!(compare(arg, arg->next) cmp 0)) \
            return ret; \
    } \
    ret->value = 1; \
    return ret; \
}

FNC_COMPARE(fnc_less, <);

FNC_COMPARE(fnc_more, >);

FNC_COMPARE(fnc_less_or_equal, <=);

FNC_COMPARE(fnc_more_or_equal, >=);

FNC_COMPARE(fnc_equal, ==);

#undef FNC_COMPARE

arg_t *fnc_not_equal(const arg_t *args) {
	arg_t *ret = init_arg(T_INT);
	for (const arg_t *left = args; left->next != NULL; left = left->next)
		for (const arg_t *right = left->next; right != NULL; right = right->next)
			if (compare(left, right) == 0)
				return ret;
	ret->value = 1;
	return ret;
}


arg_t *fnc_to_int(const arg_t *args) {
	arg_t *ret = NULL;
	for (const arg_t *arg = args; arg != NULL; arg = arg->next) {
		arg_t *current = init_arg(T_INT);
		current->value = get_int(arg);
		add_arg(&ret, current);
	}
	return ret;
}

arg_t *fnc_to_real(const arg_t *args) {
	arg_t *ret = NULL;
	if (!args_match_pattern(args, F_NUMBER | T_STRING | F_MULTIPLE | F_OPTIONAL, F_END)) {
		EXCEPTION(ret, "real: invalid arguments");
	}
	for (const arg_t *arg = args; arg != NULL; arg = arg->next) {
		arg_t *current = init_arg(T_REAL);
		current->floating = get_real(arg);
		add_arg(&ret, current);
	}
err:
	return ret;
}

arg_t *fnc_sum(const arg_t *args) {
	arg_t *ret = NULL;
	if (args_match_pattern(args, T_INT | F_MULTIPLE | F_OPTIONAL, F_END)) {
		ret = init_arg(T_INT);
		for (const arg_t *arg = args; arg != NULL; arg = arg->next)
			ret->value += arg->value;
	} else if (args_match_pattern(args, F_NUMBER | F_MULTIPLE, F_END)) {
		ret = init_arg(T_REAL);
		for (const arg_t *arg = args; arg != NULL; arg = arg->next)
			ret->floating += get_real(arg);
	} else {
		EXCEPTION(ret, "sum: invalid arguments");
	}
err:
	return ret;
}

arg_t *fnc_mul(const arg_t *args) {
	arg_t *ret = NULL;
	if (args_match_pattern(args, T_INT | F_MULTIPLE, F_END)) {
		ret = init_arg(T_INT);
		ret->value = 1;
		for (const arg_t *arg = args; arg != NULL; arg = arg->next)
			ret->value *= arg->value;
	} else if (args_match_pattern(args, F_NUMBER | F_MULTIPLE, F_END)) {
		ret = init_arg(T_REAL);
		ret->floating = 1.f;
		for (const arg_t *arg = args; arg != NULL; arg = arg->next)
			ret->floating *= get_real(arg);
	} else {
		EXCEPTION(ret, "mul: invalid arguments");
	}
err:
	return ret;
}

arg_t *fnc_div(const arg_t *args) {
	arg_t *ret = NULL;
	if (!args_match_pattern(args, F_NUMBER | F_MULTIPLE, F_END)) {
		EXCEPTION(ret, "div: invalid arguments")
	}
	ret = copy_arg(args, false);
	for (const arg_t *arg = args->next; arg != NULL; arg = arg->next) {
		if (ret->type & arg->type & T_INT) {
			int divider = get_int(arg);
			if (divider != 0 && ret->value % divider == 0) {
				ret->value /= divider;
				continue;
			}
		}
		double divider = get_real(arg);
		if (divider == 0.f) {
			EXCEPTION(ret, "div: division by zero");
		}
		ret->floating = get_real(ret) / divider;
		ret->type = T_REAL;
	}
err:
	return ret;
}

arg_t *fnc_import(const arg_t *args) {
	arg_t *ret = NULL,
	      *res = NULL;
	if (!args_match_pattern(args, T_STRING | F_MULTIPLE, F_END)) {
		EXCEPTION(ret, "import: invalid arguments");
	}
	for (const arg_t *arg = args; arg != NULL; arg = arg->next) {
		res = load_file(arg->string);
		RETURN_IF_TERMINATE(ret, res);
		add_arg(&ret, res);
		res = NULL;
	}
err:
	if (res != NULL)
		delete(res);
	return ret;
}

arg_t *fnc_raise(const arg_t *args) {
	arg_t *ret = execute_inner_stm(fnc_to_str, args, true);
	ret->type = T_EXCEPTION;
	return ret;
}

arg_t *fnc_exit(const arg_t *args) {
	arg_t *tmp = execute_inner_stm(fnc_to_int, args, true);
	arg_t *ret = execute_inner_stm(fnc_sum, tmp, true);
	delete(tmp);
	ret->type = T_EXIT;
	return ret;
}
