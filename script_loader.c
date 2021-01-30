#include "parser.h"
#include "statement.h"
#include "args.h"
#include "execution.h"
#include "terminate.h"
#include "gc.h"

#include <stdio.h>
#include <string.h>


arg_t *load_file(const char *path) {
	arg_t *ret = NULL;
	FILE *fp = fopen(path, "r");
	if (fp == NULL) {
		EXCEPTION(ret, "Error opening file %s", path);
	}
	char *buffer = NULL;
	{
		size_t len;
		if (getdelim(&buffer, &len, '\0', fp) == -1) {
			fclose(fp);
			EXCEPTION(ret, "Failed reading file %s", path);
		}
	}
	fclose(fp);
	for (char *text = buffer;;) {
		stm_t *stm = parse((const char **) &text);
		if (stm == NULL)
			break;
		if (stm->args->type & F_TERMINATE) {
			ret = copy_arg(stm->args, false);
			delete_stm(stm);
			break;
		}
		arg_t *res = execute(stm);
		delete_stm(stm);
		if (res->type & F_TERMINATE) {
			delete(ret);
			ret = res;
			break;
		} else if (res->type == T_NULL) {
			delete(res);
		} else {
			add_arg(&ret, res);
		}
	}
	free(buffer);
err:
	return ret != NULL ? ret : init_arg(T_NULL);
}