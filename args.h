#ifndef RAFAEL_ARGS_H
#define RAFAEL_ARGS_H

#include "statement.h"

#include <stdbool.h>

arg_t *init_arg(type_t type);

arg_t *safe_ret(arg_t *args);

void add_arg(arg_t **first, const arg_t *arg);

arg_t *copy_arg(const arg_t *arg, bool copy_next);

void add_exception(arg_t **arg, const char *format, ...);

const arg_t *get_next_to_given(const arg_t *hay, const arg_t *needle);

bool every_arg_has_a_type(const arg_t *args, type_t type);

bool args_match_pattern(const arg_t *args, ...);

double compare(const arg_t *left, const arg_t *right);

int get_int(const arg_t *arg);

double get_real(const arg_t *arg);


#endif //RAFAEL_ARGS_H
