#ifndef RAFAEL_UTIL_H
#define RAFAEL_UTIL_H

#include <stdbool.h>

char *get_unlimited_input(char *buffer, bool *eof_flag);

void print_error(const char *message);

#endif //RAFAEL_UTIL_H
