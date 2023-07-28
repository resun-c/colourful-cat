#include <string.h>

char *C_KW_LIST[14] = {
    "break",
    "else",
    "switch",
    "case",
    "return",
    "short",
    "continue",
    "for",
    "default",
    "goto",
    "sizeof",
    "do",
    "if",
    "while"
};

char *C_TYPES_LIST[19] = {
    "auto",
    "char",
    "const",
    "double",
    "enum",
    "extern",
    "float",
    "int",
    "long",
    "register",
    "short",
    "signed",
    "static",
    "struct",
    "typedef",
    "union",
    "void",
    "volatile",
    "unsigned"
};

int C_KW_LIST_LEN = 13;
int C_TYPES_LIST_LEN = 18;

int isckeyword (char *keyw)
{
    int i;
    for (i = 0; i < C_KW_LIST_LEN; i++)
    if (strcmp(C_KW_LIST[i], keyw) == 0)
        return 1;
    return 0;
}

int isctype (char *ctyp)
{
    int i;
    for (i = 0; i < C_TYPES_LIST_LEN; i++)
    if (strcmp(C_TYPES_LIST[i], ctyp) == 0)
        return 1;
    return 0;
}