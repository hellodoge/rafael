#ifndef RAFAEL_FUNCTIONS_H
#define RAFAEL_FUNCTIONS_H

#include "statement.h"

typedef arg_t *(*fnc_ptr_t)(const arg_t *);

typedef enum fnc_flags {
	FF_NONE = 0,
	FF_DO_NOT_EXECUTE_ARGS = 0x1u,
} fnc_flags_t;

typedef struct function {
	char *name;
	fnc_ptr_t func;
	fnc_flags_t flags;
} fnc_t;

const fnc_t *get_predefined(const char *string);


#endif //RAFAEL_FUNCTIONS_H
