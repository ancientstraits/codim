#ifndef C_LEX_H
#define C_LEX_H

typedef enum c_lex_ret {
	C_LEX_NONE,
	C_LEX_DELIM,
	C_LEX_STR,
	C_LEX_NUM,
	C_LEX_TYPE,
	C_LEX_OPERATOR,
	C_LEX_VAR,
	C_LEX_COMMENT,
} c_lex_ret;

c_lex_ret c_lex(char c);

#endif // !C_LEX_H