#ifndef RAFAEL_EXECUTION_H
#define RAFAEL_EXECUTION_H

#include "statement.h"
#include "functions.h"

#include <stdbool.h>

arg_t *execute(const stm_t *stm);

arg_t *execute_inner_stm(fnc_ptr_t function, const arg_t *args, bool pass_next);

#endif //RAFAEL_EXECUTION_H
