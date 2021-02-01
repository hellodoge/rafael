#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define DEFAULT_BUFFER_SIZE 16

char *get_unlimited_input(char *buffer, bool *eof_flag) {
	size_t current_size;
	char *cursor;
	if (buffer == NULL) {
		buffer = malloc(DEFAULT_BUFFER_SIZE * sizeof(char));
		current_size = DEFAULT_BUFFER_SIZE;
		cursor = buffer;
	} else {
		current_size = strlen(buffer) + 1;
		cursor = buffer + current_size - 1;
	}
	for (;;) {
		int current = getchar();
		if (current == EOF) {
			if (eof_flag != NULL)
				*eof_flag = true;
			break;
		}
		*cursor = (char) current;
		cursor++;
		if (cursor >= buffer + current_size) {
			current_size += DEFAULT_BUFFER_SIZE;
			buffer = realloc(buffer, current_size);
			cursor = buffer + current_size - DEFAULT_BUFFER_SIZE;
		}
		if (current == '\n')
			break;
	}
	*cursor = '\0';
	buffer = realloc(buffer, cursor - buffer + 1);
	return buffer;
}

void print_error(const char *message) {
	fprintf(stderr, "Error! %s\n", message);
}


#undef DEFAULT_BUFFER_SIZE
