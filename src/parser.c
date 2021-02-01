#include "include/parser.h"

#include "include/terminate.h"
#include "include/args.h"
#include "include/gc.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

slice_t get_token(const char *str, const char *delim);

char *slice_to_str(slice_t slice);

arg_t *process_expression(const char **text);

arg_t *process_string(const char **text);

arg_t *process_number(slice_t arg);

arg_t *process_token(slice_t arg);

arg_t *process_variable(slice_t var);


stm_t *parse(const char **textPtr) {
	const char *text = *textPtr;

	stm_t *stm = malloc(sizeof(stm_t));
	memset(stm, 0, sizeof(stm_t));

	slice_t expr = get_expression(text);
	if (expr.start == NULL) {
		if (strchr(text, ')') == NULL) {
			free(stm);
			return NULL;
		} else {
			EXCEPTION(stm->args, "Parser: Single closing parenthesis");
		}
	}
	slice_t name = get_token(expr.start + 1, "( \t\n)");
	{
		char *open_parenthesis = strchr(expr.start + 1, '(');
		char *close_parenthesis = strchr(expr.start + 1, ')');
		char *closer_parenthesis = open_parenthesis < close_parenthesis ?
		                           open_parenthesis : close_parenthesis;
		if (name.start == NULL ||
		    (closer_parenthesis != NULL && name.start > closer_parenthesis)) {
			EXCEPTION(stm->args, "Parser: Name of expression not found");
		}
	}
	stm->name = slice_to_str(name);
	stm->name_rc = init_ref_counter();
	text = name.end + 1;

	if (expr.end == NULL) {
		EXCEPTION(stm->args, "Parser: End of expression %s not found", stm->name);
	}
	for (;;) {
		slice_t arg_slice = get_token(text, "\t\n )");
		if (arg_slice.end == NULL || arg_slice.start >= expr.end)
			break;
		text = arg_slice.start;
		arg_t *arg = NULL;
		switch (*arg_slice.start) {
			case '(':
				arg = process_expression(&text);
				break;
			case '\'':
			case '\"':
				arg = process_string(&text);
				break;
			case '&':
				arg = process_variable(arg_slice);
				text = arg_slice.end + 1;
				break;
			default:
				if (isdigit(*arg_slice.start) ||
				    *arg_slice.start == '-')
					arg = process_number(arg_slice);
				else
					arg = process_token(arg_slice);
				text = arg_slice.end + 1;
				break;
		}
		if (arg->type & F_ALLOCATED) {
			assert(arg->content != NULL);
			arg->rc = init_ref_counter();
		}
		RETURN_IF_TERMINATE(stm->args, arg);
		add_arg((arg_t **) &stm->args, arg);
	}
	*textPtr = expr.end + 1;
err:
	if (*textPtr != expr.end + 1)
		*textPtr = ++text;
	stm->args = safe_ret((arg_t *) stm->args);
	return stm;
}


arg_t *process_expression(const char **text) {
	arg_t *ret = NULL;
	stm_t *inner_stm = parse(text);
	RETURN_IF_TERMINATE(ret, inner_stm->args);
	ret = init_arg(T_STATEMENT);
	ret->statement = inner_stm;
	return ret;
err:
	free(inner_stm);
	return ret;
}

arg_t *parse_string(slice_t slice);

slice_t get_string(const char *text);

arg_t *process_string(const char **text) {
	arg_t *ret = NULL;
	slice_t string = get_string(*text);
	if (string.end == NULL) {
		EXCEPTION(ret, "End of string not found");
	}
	ret = parse_string(string);
	*text = string.end + 2;
err:
	return ret;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err34-c"

arg_t *process_number(slice_t arg) {
	arg_t *ret = init_arg(T_INT);
	for (const char *current = arg.start; current <= arg.end; current += 1) {
		if (*current == '.') {
			ret->type = T_REAL;
			ret->floating = atof(arg.start);
			return ret;
		}
	}
	ret->value = atoi(arg.start);
	return ret;
}

#pragma clang diagnostic pop

arg_t *process_token(slice_t arg) {
	arg_t *ret = init_arg(T_TOKEN);
	ret->string = slice_to_str(arg);
	return ret;
}

arg_t *process_variable(slice_t var) {
	arg_t *ret = NULL;
	if (var.start == var.end) {
		EXCEPTION(ret, "Empty variable name");
	}
	var.start++;
	ret = init_arg(T_VAR);
	ret->string = slice_to_str(var);
err:
	return ret;
}

arg_t *parse_string(slice_t slice) {
	const char escape_seq[][2] = {
			{'b',  '\b'},
			{'n',  '\n'},
			{'r',  '\r'},
			{'t',  '\t'},
			{'\\', '\\'},
			{'\"', '\"'},
			{'\'', '\''}
	};
	arg_t *stringArg = NULL;
	char *string = NULL;
	if (slice.start == NULL || slice.end == NULL) {
		EXCEPTION(stringArg, "Parser: String not properly ended");
	}
	size_t len = 0;
	for (const char *i = slice.start; i < slice.end; i++)
		if (*i != '\\' || (i > slice.start && *(i - 1) == '\\'))
			len++;
	string = malloc(len + 2);
	*(string + len + 1) = '\0';
	char *cursor = string;
	for (const char *i = slice.start; i <= slice.end; i++) {
		if (*i == '\\') {
			i++;
			bool found = false;
			for (size_t j = 0; j < sizeof(escape_seq) / sizeof(escape_seq[0]); j++) {
				if (*i == escape_seq[j][0]) {
					found = true;
					*cursor++ = escape_seq[j][1];
				}
			}
			if (!found) {
				EXCEPTION(stringArg, "Parser: Escape sequence \\%c not found", *i);
			}
		} else
			*cursor++ = *i;
	}
	stringArg = init_arg(T_STRING);
	stringArg->string = string;
	return stringArg;
err:
	if (string != NULL)
		free(string);
	return stringArg;
}

slice_t get_string(const char *text) {
	slice_t string = {};
	text = strpbrk(text, "'\"");
	if (text == NULL)
		return string;
	const char start = *text++;
	string.start = text;
	for (; (text = strpbrk(text, "\\'\"")) != NULL;) {
		if (*text == start) {
			string.end = text - 1;
			return string;
		}
		if (*(++text) != '\0' && *(text - 1) == '\\')
			text++;
	}
	return string;
}

slice_t get_expression(const char *text) {
	int i = 0;
	slice_t expr = {};
	do {
		text = strpbrk(text, "(\'\")");
		if (text == NULL) return expr;
		if (*text == '(') {
			if (i == 0)
				expr.start = text;
			i++;
		} else if (*text == '\'' || *text == '\"') {
			slice_t string = get_string(text);
			if (string.end == NULL)
				return expr;
			text = string.end + 1;
		} else i--; // *text == ')'
		text++;
	} while (i > 0);
	if (i < 0) return expr;
	expr.end = text - 1;
	return expr;
}

slice_t get_token(const char *str, const char *delim) {
	slice_t token = {0x0, 0x0};
	str += strspn(str, delim);
	if (*str == '\0') return token;
	token.start = str;
	token.end = strpbrk(str, delim) - 1;
	return token;
}

char *slice_to_str(slice_t slice) {
	if (slice.start == 0 || slice.end == 0)
		return 0;
	char *str = malloc(slice.end - slice.start + 2);
	strncpy(str, slice.start, slice.end - slice.start + 1);
	*(str + (slice.end - slice.start + 1)) = '\0';
	return str;
}
