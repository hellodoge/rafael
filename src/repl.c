#include "include/parser.h"
#include "include/context.h"
#include "include/execution.h"
#include "include/util.h"
#include "include/gc.h"

#include <malloc.h>
#include <string.h>
#include <assert.h>

void show_return(const arg_t *args);

const char *input_expression();

int repl(void) {
	int exit_code = 1;
	for (bool exit = false; exit == false;) {
		const char *buffer = input_expression();
		if (buffer == NULL)
			return 0;
		stm_t *stm = parse(&buffer);
		if (stm == NULL)
			return 0;
		if (stm->args->type == T_EXCEPTION) {
			print_error(stm->args->string);
			continue;
		}
		arg_t *ret = execute(stm);
		if (ret->type == T_EXIT) {
			exit_code = ret->value;
			exit = true;
		}
		if (ret->type == T_EXCEPTION) {
			print_error(ret->string);
		}
		if (!(ret->type & F_TERMINATE))
			show_return(ret);
		delete(ret);
		delete_stm(stm);
	}
	return exit_code;
}

const char *input_expression() {
	char *buffer = NULL;
	slice_t expr = {NULL, NULL};
	static bool eof_flag = false;
	for (; eof_flag != true;) {
		if (expr.start == NULL) {
			if (buffer != NULL) {
				free(buffer);
				buffer = NULL;
			}
			printf("--> ");
		} else if (expr.end == NULL) {
			printf("... ");
		} else
			break;
		buffer = get_unlimited_input(buffer, &eof_flag);
		expr = get_expression(buffer);
	}
	return buffer;
}

void show_return(const arg_t *args) {
	if (args->type == T_NULL)
		return;
	if (args->next != NULL)
		putchar('(');
	for (const arg_t *arg = args; arg != NULL; arg = arg->next) {
		assert(!(arg->type & (T_STATEMENT | T_VAR | F_TERMINATE)));
		if (arg->type & T_STRING) {
			printf("\"%s\"", arg->string);
		} else if (arg->type & T_INT) {
			printf("%d", arg->value);
		} else if (arg->type & T_REAL) {
			printf("%f", arg->floating);
		} else if (arg->type & T_TOKEN) {
			printf("%s", arg->string);
		}
		if (arg->next != NULL)
			printf(", ");
	}
	if (args->next != NULL)
		putchar(')');
	putchar('\n');
}


