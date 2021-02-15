#ifndef RAFAEL_SCRIPT_LOADER_H
#define RAFAEL_SCRIPT_LOADER_H

#include "statement.h"

#include <stdbool.h>

arg_t *load_file(const char *relative_path, bool search_in_lib_dir);

#endif //RAFAEL_SCRIPT_LOADER_H
