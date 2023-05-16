#include <stdio.h>
#include <stdlib.h>
#include "error.h"
#define GASSERT(cond, ...) ASSERT(cond, ERROR_GAPEDIT, __VA_ARGS__);
