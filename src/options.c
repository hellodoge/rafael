#include "include/options.h"

#include <string.h>
#include <stdio.h>

typedef struct option {
	const char *string;
	const char shortcut;
	flags_t flags;
	const char *desc;
} option_t;

const option_t options[] = {
		{"help", 'h', F_HELP, "Display this information and exit"},
		{"test", 't', F_TEST, "Run files as tests"}
};

flags_t get_flag(const char *arg) {
	if (*arg != '-')
		return F_ERR;
	if (*(arg + 1) == '-') {
		for (int i = 0; i < sizeof(options) / sizeof(option_t); i++) {
			if (strcmp(arg + 2, options[i].string) == 0)
				return options[i].flags;
		}
	} else {
		for (int i = 0; i < sizeof(options) / sizeof(option_t); i++) {
			if (*(arg + 1) == options[i].shortcut)
				return options[i].flags;
		}
	}
	return F_ERR;
}

void print_help(void) {
	for (int i = 0; i < sizeof(options) / sizeof(option_t); i++) {
		printf("-%c, --%-6s %s", options[i].shortcut, options[i].string, options[i].desc);
		if (i + 1 < sizeof(options) / sizeof(option_t))
			putchar('\n');
	}
}