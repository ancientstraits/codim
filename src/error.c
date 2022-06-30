#include <setjmp.h>
#include "error.h"

static jmp_buf buf;

void error_jmp(Error err) {
	longjmp(buf, err);
}

Error error_get() {
	return setjmp(buf);
}

