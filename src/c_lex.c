#include <stdio.h>
#include <ctype.h>

#include "c_lex.h"

static int is_operator(char c) {
    switch (c) {
    case '+':
    case '-':
    case '/':
    case '*':
    case '^':
    case '&':
    case '|':
        return 1;
    default:
        return 0;
    }
}

static int is_delim(char c) {
    switch (c) {
    case ';':
    case '{':
    case '}':
    case '(':
    case ')':
    case '[':
    case ']':
    case '<':
    case '>':
        return 1;
    default:
        return 0;
    }
}

c_lex_ret c_lex(char c) {
    static c_lex_ret last_char = C_LEX_NONE;
    static int in_string = 0;

    static enum {COM_SIN, COM_MUL} comment_type = COM_SIN;
    static int in_comment = 0;

    if (c == '\"') {
        in_string = !in_string;
        return C_LEX_STR;
    }

    if (in_string)
        return C_LEX_STR;

    if (isdigit(c)) {
        // This will make sure that variables with digits
        // in their names are lexed only as variables.
        if (last_char != C_LEX_VAR)
            last_char = C_LEX_NUM;
        return last_char;
    }
    if (isalpha(c))
        return C_LEX_VAR;
    
    if (is_operator(c))
        return C_LEX_OPERATOR;
    
    if (is_delim(c))
        return C_LEX_DELIM;

    return C_LEX_NONE;
}
