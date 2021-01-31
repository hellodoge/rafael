#ifndef RAFAEL_GC_H
#define RAFAEL_GC_H

#include "statement.h"
#include "vars.h"

#include <stdbool.h>

void delete(arg_t *args);

void delete_stm(stm_t *stm);

void delete_vars(var_t *var);

ref_counter_t *init_ref_counter();

void forget(arg_t *arg);

void raw_forget(void *data, ref_counter_t *rc);

#endif //RAFAEL_GC_H
