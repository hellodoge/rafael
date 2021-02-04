#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <stdint.h>

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

#undef DEFAULT_BUFFER_SIZE

char *get_n_chars_from_utf_8_str(const char *string, size_t n) {
	/// returns NULL is string is not valid UTF-8
	assert(string != NULL);
	size_t size = 0;
	for (size_t i = 0; i < n; i++) {
		if (*(string + size) == '\0')
			break;
		const uint8_t *bytes = (uint8_t *) string + size;
		uint8_t mask = 1u << 6u | 1u << 7u;
		size_t size_of_char = 1;
		while ((*bytes & mask) == mask) {
			mask = (mask >> 1u) | 1u << 7u;
			size_of_char++;
		}
		for (size_t j = 1; j < size_of_char; j++) {
			if ((*(bytes + j) & (1u << 6u | 1u << 7u)) != 1u << 7u)
				return NULL;
		}
		if (size_of_char == 1 && (*bytes & 1u << 7u) == 1u << 7u)
			return NULL;
		size += size_of_char;
	}
	return strndup(string, size);
}

void print_error(const char *message) {
	fprintf(stderr, "Error! %s\n", message);
}

