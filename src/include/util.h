#ifndef RAFAEL_UTIL_H
#define RAFAEL_UTIL_H

#include <stdbool.h>
#include <stdlib.h>

char *get_unlimited_input(char *buffer, bool *eof_flag);

void print_error(const char *message);

char *get_n_chars_from_utf_8_str(const char *string, size_t n);

char *read_file_to_string(const char *path);

#endif //RAFAEL_UTIL_H
