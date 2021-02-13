#ifndef RAFAEL_STATEMENT_H
#define RAFAEL_STATEMENT_H

typedef int ref_counter_t;

typedef struct statement stm_t;
typedef struct argument arg_t;

typedef enum arg_types_and_flags {
	T_NULL		= 0u,
	T_STATEMENT	= 0x1u,
    T_EXCEPTION	= 0x1u << 1u,
	T_INT		= 0x1u << 2u,
	T_REAL		= 0x1u << 3u,
	T_STRING	= 0x1u << 4u,
	T_VAR		= 0x1u << 5u,
	T_TOKEN		= 0x1u << 6u,
	T_EXIT      = 0x1u << 7u,
	T_LAMBDA    = 0x1u << 8u,

	F_TERMINATE = T_EXCEPTION | T_EXIT,
	F_NUMBER	= T_INT | T_REAL,
	F_HAVE_STR	= T_STRING | T_VAR | T_TOKEN | T_EXCEPTION,
	F_ALLOCATED	= T_STATEMENT | F_HAVE_STR | T_LAMBDA,

	F_ORIGINAL	= 0x1u << 9u, //used and dropped by init_arg

	F_MULTIPLE  = 0x1u << 10u, //used by args_match_pattern
	F_END       = 0x1u << 11u,
	F_OPTIONAL  = 0x1u << 12u,
} type_t;

struct statement {
	const char *name;
	ref_counter_t *name_rc;
	const arg_t *args;
};

struct argument {
	type_t type;
	union {
		void *content;
		const char *string;
		const stm_t *statement;
		const arg_t *lambda;
		int value;
		double floating;
	};
	ref_counter_t *rc;
	arg_t *next;
};

#endif //RAFAEL_STATEMENT_H
