#include "include/context.h"
#include "include/repl.h"
#include "include/script_loader.h"
#include "include/util.h"
#include "include/gc.h"


int main(int argc, char *argv[]) {
	init_context();
	int ret_value = 0;
	if (argc == 1) {
		ret_value = repl();
	} else {
		for (int i = 1; i < argc; i++) {
			arg_t *res = load_file(argv[i]);
			if (res->type == T_EXCEPTION) {
				print_error(res->string);
			} else if (res->type == T_EXIT) {
				ret_value += res->value;
			}
			delete(res);
		}
	}
	delete_context();
	return ret_value;
}
