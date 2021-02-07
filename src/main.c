#include "include/context.h"
#include "include/repl.h"
#include "include/script_loader.h"
#include "include/util.h"
#include "include/gc.h"


int main(int argc, char *argv[]) {
	int ret_value = 0;
	if (argc == 1) {
		init_context();
		ret_value = repl();
		delete_context();
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
	return ret_value;
}
