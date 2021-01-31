#ifndef RAFAEL_TERMINATE_H
#define RAFAEL_TERMINATE_H

/// must be a label err in the function

#define EXCEPTION(ret, desc, ...) \
{ \
	add_exception((arg_t**)(&(ret)), (desc), ##__VA_ARGS__); \
	goto err; \
}

#define RETURN_IF_TERMINATE(ret, arg) \
if (arg->type & F_TERMINATE) { \
	add_arg((arg_t**)(&(ret)), copy_arg((arg), true)); \
	goto err; \
}

#endif //RAFAEL_TERMINATE_H
