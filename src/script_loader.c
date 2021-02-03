#include "include/parser.h"
#include "include/statement.h"
#include "include/args.h"
#include "include/execution.h"
#include "include/terminate.h"
#include "include/gc.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

struct working_directory_list { //LIFO
	char *path;
	struct working_directory_list *prev;
};

struct working_directory_list *working_directory = NULL;

void set_working_directory(char *path_to_file);

void remove_working_directory(void);

char *concat_with_working_directory(const char *relative_path);

char *concat_with_standard_lib_directory(const char *relative_path);

arg_t *load_file(const char *relative_path) {
	arg_t *ret = NULL;

	char *path;
	if (working_directory != NULL)
		path = concat_with_working_directory(relative_path);
	else
		path = strdup(relative_path);
	if (access(path, F_OK) != 0) {
		free(path);
		path = concat_with_standard_lib_directory(relative_path);
		if (path == NULL) {
			EXCEPTION(ret, "File %s doesn't seem to exist (cannot obtain standard library)", path);
		} if (access(path, F_OK) != 0) {
			EXCEPTION(ret, "File %s doesn't seem to exist", path);
		}
	}
	set_working_directory(path);

	FILE *fp = fopen(path, "r");
	if (fp == NULL) {
		EXCEPTION(ret, "Error opening file %s", relative_path);
	}
	char *buffer = NULL;
	{
		size_t len;
		if (getdelim(&buffer, &len, '\0', fp) == -1) {
			fclose(fp);
			EXCEPTION(ret, "Failed reading file %s", relative_path);
		}
	}
	fclose(fp);
	for (char *text = buffer;;) {
		stm_t *stm = parse((const char **) &text);
		if (stm == NULL)
			break;
		if (stm->args->type & F_TERMINATE) {
			ret = copy_arg(stm->args, false);
			if (stm->name != NULL) {
				delete_stm(stm);
			} else {
				delete((arg_t *) stm->args);
				free(stm);
			}
			break;
		}
		arg_t *res = execute(stm);
		delete_stm(stm);
		if (res->type & F_TERMINATE) {
			delete(ret);
			ret = res;
			break;
		} else if (res->type == T_NULL) {
			delete(res);
		} else {
			add_arg(&ret, res);
		}
	}
	free(buffer);
err:
	if (working_directory != NULL)
		remove_working_directory();
	free(path);
	return ret != NULL ? ret : init_arg(T_NULL);
}

typedef struct working_directory_list working_directory_list_t;

void set_working_directory(char *path_to_file) {
	/// safety: path_to_file must be valid
	/// if path_to_file doesn't contain slashes, it won't set working_directory
	char *path;
	{
		char *slash = strrchr(path_to_file, '/');
		char *backslash = strrchr(path_to_file, '\\');
		char *last_slash = slash > backslash ? slash : backslash;
		if (last_slash == NULL) return;
		path = strndup(path_to_file, last_slash - path_to_file + 1);
	}
	struct working_directory_list *current = malloc(sizeof(working_directory_list_t));
	current->prev = working_directory;
	current->path = path;
	working_directory = current;
}

void remove_working_directory(void) {
	assert(working_directory != NULL);
	struct working_directory_list *current = working_directory;
	working_directory = working_directory->prev;
	free(current->path);
	free(current);
}

char *concat_with_working_directory(const char *relative_path) {
	assert(working_directory != NULL);
	char *path = malloc((strlen(working_directory->path) + strlen(relative_path) + 1) * sizeof(char));
	*path = '\0';
	strcat(strcat(path, working_directory->path), relative_path);
	return path;
}

char *concat_with_standard_lib_directory(const char *relative_path) {
#ifndef _WIN32
#define LIB_FOLDER "/.local/lib/rafael/"
	char *basedir = getenv("HOME");
	if (basedir == NULL)
		return NULL;
#else
#define LIB_FOLDER "/rafael/lib/"
	char *basedir = getenv("localappdata");
	if (basedir == NULL)
		return NULL;
#endif
	char *path = malloc((strlen(basedir) + strlen(relative_path)) * sizeof(char) + sizeof(LIB_FOLDER));
	*path = '\0';
	return strcat(strcat(strcat(path, basedir), LIB_FOLDER), relative_path);
#undef LIB_FOLDER
}