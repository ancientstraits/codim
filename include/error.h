#ifndef CODIM_ERR_H
#define CODIM_ERR_H

// `error.h` adds error handling utilities.

#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <errno.h>

#define ASSERT(cond, ...) if (!(cond)) DIE(__VA_ARGS__);

#define LOG(...) do { \
	fprintf(stderr, "log %s:%d (%s()): ", __FILE__, __LINE__, __func__); \
	fprintf(stderr, __VA_ARGS__); \
	fputc('\n', stderr); \
} while (0)

#define DIE(err, ...) do { \
	LOG(__VA_ARGS__); \
	ERROR_JMP(err); \
} while (0)

#define DIE_ERRNO(err) DIE(err, "%s\n", strerror(errno))

#define ERROR_JMP(err) longjmp(errbuf, err)
#define ERROR_GET() setjmp(errbuf)

typedef enum Error {
	ERROR_NONE,
	// Fatal allocation error
	ERROR_ALLOC,

	// For errors relating to IO
	ERROR_OUTPUT,

	// For errors relating to graphics initialization
	ERROR_GFX,
} Error;

extern jmp_buf errbuf;

#endif // !CODIM_ERR_H
