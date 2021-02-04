#ifndef RAFAEL_UTIL_H
#define RAFAEL_UTIL_H

#include <stdbool.h>
#include <stdlib.h>

char *get_unlimited_input(char *buffer, bool *eof_flag);

void print_error(const char *message);

char *get_n_chars_from_utf_8_str(const char *string, size_t n);

#endif //RAFAEL_UTIL_H
