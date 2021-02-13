#include "include/context.h"
#include "include/repl.h"
#include "include/script_loader.h"
#include "include/util.h"
#include "include/gc.h"
#include "include/options.h"

#include <stdio.h>

int main(int argc, const char *argv[]) {
	int ret_value = 0;
	flags_t flags = F_NULL;
	if (argc == 1) {
		init_context();
		ret_value = repl();
		delete_context();
	} else {
		for (int i = 1; i < argc; i++) {
			if (*argv[i] == '-') {
				flags |= get_flag(argv[i]);
				if (flags & F_ERR) {
					printf("rafael: invalid option: %s", argv[i]);
					return -1;
				}
				if (flags & F_HELP) {
					print_help();
					return 0;
				}
			} else {
				if (flags & F_TEST) {
					fprintf(stderr, "Running test: %s ", argv[i]);
				}
				arg_t *res = load_file(argv[i]);
				if (flags & F_TEST) {
					fprintf(stderr, "%c%s%c[0m", 27,
							res->type != T_EXCEPTION ?
							"[32mPASSED\n" : "[31mFAILED ", 27);
					if (res->type == T_EXCEPTION)
						fprintf(stderr, "(%s)\n", res->string);
				} else {
					if (res->type == T_EXCEPTION) {
						print_error(res->string);
					} else if (res->type == T_EXIT) {
						ret_value += res->value;
					}
				}
				delete(res);
			}
		}
	}
	return ret_value;
}
