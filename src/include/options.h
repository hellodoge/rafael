#ifndef RAFAEL_OPTIONS_H
#define RAFAEL_OPTIONS_H

typedef enum flags {
	F_NULL = 0u,
	F_ERR  = 1u,
	F_HELP = 1u << 2u,
	F_TEST = 1u << 3u,
} flags_t;

flags_t get_flag(const char *arg);

void print_help(void);

#endif //RAFAEL_OPTIONS_H
