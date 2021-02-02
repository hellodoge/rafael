#ifndef RAFAEL_EXECUTION_H
#define RAFAEL_EXECUTION_H

#include "statement.h"
#include "functions.h"

#include <stdbool.h>

arg_t *execute(const stm_t *stm);

arg_t *execute_inner_stm(fnc_ptr_t function, const arg_t *args, bool pass_next);

arg_t *macro_processor(const arg_t *macro_def, const arg_t *args); //lambdas

#endif //RAFAEL_EXECUTION_H
