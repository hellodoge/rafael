#include "include/args.h"

#include "include/statement.h"
#include "include/gc.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>


arg_t *init_arg(type_t type) {
	arg_t *arg = malloc(sizeof(arg_t));
	memset(arg, 0, sizeof(arg_t));
	arg->type = type;
	if (type & F_ORIGINAL) {
		assert(type & F_ALLOCATED);
		arg->rc = init_ref_counter();
		arg->type &= ~F_ORIGINAL;
	}
	return arg;
}

arg_t *put_args_in_order(arg_t *args) {
	/// removes all null args
	/// (except the last one if there are no other types)
	/// if at least one F_TERMINATE is passed, put_args_in_order returns only it
	if (args == NULL)
		return init_arg(T_NULL);
	if (args->next == NULL)
		return args;
	for (arg_t *arg = args, *prev = NULL, *next; arg != NULL; arg = next) {
		next = arg->next;
		if (arg->type & F_TERMINATE) {
			arg_t *ret = copy_arg(arg, false);
			arg->type = T_NULL;
			delete(args);
			return ret;
		} else if (arg->type == T_NULL && (next != NULL || prev != NULL)) {
			free(arg);
			if (prev != NULL)
				prev->next = next;
			else
				args = next;
		} else
			prev = arg;
	}
	return args;
}


void add_arg(arg_t **first, const arg_t *arg) {
	/// safety: shouldn't use arg after this call
	assert(first != NULL);
	assert(arg != NULL);
	if (*first == NULL) {
		*first = (arg_t *) arg;
	} else {
		arg_t *prev = *first;
		while (prev->next != NULL)
			prev = prev->next;
		prev->next = (arg_t *) arg;
	}
}

arg_t *copy_arg(const arg_t *arg, bool copy_next) {
	assert(arg != NULL);
	arg_t *copy = malloc(sizeof(arg_t));
	*copy = *arg;
	if (arg->type & F_ALLOCATED) {
		if (arg->rc != NULL)
			(*arg->rc)++;
	}
	if (arg->next != NULL && copy_next)
		copy->next = copy_arg(arg->next, true);
	else
		copy->next = NULL;
	return copy;
}

const arg_t *search_arg_in_list(const arg_t *hay, const arg_t *needle) {
	/// searches by content of needle (string/number)
	for (; hay != NULL; hay = hay->next) {
		if (hay->type & needle->type & F_HAVE_STR) {
			if (!strcmp(hay->string, needle->string))
				return hay;
		} else if (hay->type & F_NUMBER) {
			if (get_real(hay) == get_real(needle))
				return hay;
		}
	}
	return NULL;
}

const arg_t *get_next_to_given(const arg_t *hay, const arg_t *needle) {
	const arg_t *arg = search_arg_in_list(hay, needle);
	if (arg != NULL)
		return arg->next;
	return NULL;
}

double compare(const arg_t *left, const arg_t *right) {
	if (left->type & right->type & F_HAVE_STR)
		return (double) strcmp(left->string, right->string);
	else if (left->type & F_NUMBER && right->type & F_NUMBER)
		return get_real(left) - get_real(right);
	else
		return NAN;
}


bool args_match_pattern(const arg_t *args, ...) {
	va_list data_types;
	va_start(data_types, args);
	type_t type = T_NULL;
	for (;;) {
		if (args != NULL && args->type == T_NULL)
			args = args->next;
		if (!(type & F_MULTIPLE && args != NULL && type & args->type))
			type = va_arg(data_types, type_t);
		if (type == T_NULL || (args == NULL && type == F_END)) {
			va_end(data_types);
			return true;
		}
		if (type & F_OPTIONAL && (args == NULL || !(type & args->type)))
			continue;
		if (args == NULL || !(args->type & type)) {
			va_end(data_types);
			return false;
		}
		args = args->next;
	}
}

const char *find_unknown_keywords(const arg_t *args, ...) {
	va_list allowed;

	for (const arg_t *arg = args; arg != NULL; arg = arg->next) {
		if (arg->type != T_TOKEN)
			continue;
		va_start(allowed, args);
		const char *guess;
		bool is_known = false;
		while ((guess = va_arg(allowed, const char *)) != NULL) {
			if (strcmp(guess, arg->string) == 0) {
				is_known = true;
				break;
			}
		}
		va_end(allowed);
		if (!is_known)
			return arg->string;
	}
	return NULL;
}

void add_exception(arg_t **arg, const char *format, ...) {
	va_list args;

	va_start(args, format);
	size_t size = vsnprintf(0, 0, format, args);
	char *str = malloc(size + 1);
	va_end(args);

	va_start(args, format);
	vsprintf(str, format, args);
	va_end(args);

	arg_t *exc = init_arg(T_EXCEPTION | F_ORIGINAL);
	exc->string = str;
	add_arg(arg, exc);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err34-c" //atoi atof

int get_int(const arg_t *arg) {
	if (arg->type & F_HAVE_STR) {
		return atoi(arg->string);
	} else if (arg->type & T_INT) {
		return arg->value;
	} else if (arg->type & T_REAL)
		return (int) arg->floating;
	return 0;
}

double get_real(const arg_t *arg) {
	if (arg->type & T_INT) {
		return (double) arg->value;
	} else if (arg->type & T_REAL) {
		return arg->floating;
	} else if (arg->type & F_HAVE_STR)
		return atof(arg->string);
	return 0;
}

#pragma clang diagnostic pop
