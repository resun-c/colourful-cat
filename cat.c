#include <stdio.h>
#include <ctype.h>
#include "keyword.c.h"
#include "keyword.py.h"

#define		MAXWORD				1024
#define		BUFSIZ				2048
#define		RED					"\x1B[38;2;255;0;0m"
#define		GRN					"\x1B[38;2;0;255;0m"
#define		YEL					"\x1B[38;2;255;255;0m"
#define		BLU					"\x1B[38;2;153;230;255m"
#define		MAG					"\x1B[38;2;255;0;255m"
#define		CYN					"\x1B[38;2;0;255;255m"
#define		WHT					"\x1B[38;2;255;255;255m"
#define		BRN					"\x1B[38;2;168;96;40m"
#define		DEFAULT				"\x1B[39m"
#define		RESET				"\x1B[0m"

#define		KEYWORD_COLOR		"\x1B[38;2;230;128;230m"
#define		FNAME_COLOR			"\x1B[38;2;247;247;110m"
#define		NAME_COLOR			"\x1B[38;2;77;210;255m"
#define		CAP_NAME_COLOR		"\x1B[38;2;0;230;172m"
#define		TYPENAME_COLOR		"\x1B[38;2;0;230;172m"
#define		STRING_COLOR		"\x1B[38;2;255;128;128m" // "\x1B[38;2;213;101;48m"
#define		PUNCT_COLOR			WHT	//"\x1B[38;2;255;128;128m"
#define		DIGIT_COLOR			"\x1B[38;2;179;255;217m"
#define		OTHERS_COLOR		DEFAULT
#define		COMMENT_COLOR		"\x1B[38;2;45;134;89m"


enum token_types {
	NAME,
	KEYWORD,
	TYPENAME,
	STRING,
	C_CHAR,
	DIGIT,
	PARENS,
		LPAREN,
		RPAREN,
	CURLY,
		LCURLY,
		RCURLY,
	BRACKET,
		LBRACKET,
		RBRACKET,
	PUNCT,
	SPACE,
	COMMENT,
	TYPES,
	OTHERS,
	C_PREPROCESSOR_INI,
};
int tokentype = 0;

char *BRAC_COLORS[] = {
	"\x1B[38;2;230;128;230m",
	"\x1B[38;2;0;230;172m",
	YEL,
	"\x1B[38;2;153;230;255m",
	MAG
};

int BRAC_COLORS_LEN = 4;

int ENCOUNTERED_BRACES[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void filecopy(FILE *ifp, FILE *ofp);
// int getftok(FILE *fp, char *word, int lim);
int getftoken(FILE *fp, char *token, int lim);
int get_brac_color_index(int brace_type);
int is_keyword(char *s);
int is_typename(char *s);
int is_all_caps(char *s);


int C_FILE = 0, PY_FILE = 0;

main (int argc, char *argv[])
{
	FILE *fp;
	char *prog = argv[0];			/* program name for errors */

	if (argc == 1)				/* no args: copy standard input  */
		filecopy(stdin, stdout);
	else
		while (--argc > 0)
			if ((fp = fopen(*++argv, "r")) == NULL) {
				fprintf(stderr, "%s: can't open %s\n", prog, *argv);
				exit(1);
			} else {
				if (strstr(*argv, ".c")) {
					C_FILE = 1;
					PY_FILE = 0;
				} else if (strstr(*argv, ".py")) {
					C_FILE = 0;
					PY_FILE = 1;
				}
				filecopy(fp, stdout);
				fclose(fp);
			}
	
	if (ferror(stdout)) {
		fprintf(stderr, "%s: error writing stdout\n", prog);
		exit(2);
	}
	exit(0);
}

int is_keyword(char *s)
{
	if (C_FILE)
		return isckeyword(s);

	if (PY_FILE)
		return ispykeyword(s) || ispysoftkeyword(s);
}
int is_typename(char *s)
{
	if (C_FILE)
		return isctype(s);

	if (PY_FILE)
		return ispytype(s);
}

int is_all_caps(char *s)
{
	for (; *s != '\0'; )
		if (islower(*s++))
			return 0;
	return 1;
}

/* filecopy: copy file ifp to file ofp */
void filecopy(FILE *ifp, FILE *ofp)
{
	int c, i;
    char word[MAXWORD], *word1, buf[BUFSIZ];

	while (getftoken(ifp, word, MAXWORD) != EOF) {

switch_token:
		switch (tokentype) {
			case NAME:
        		// fprintf(stderr, "\t%s\t", word);
        		if (is_keyword(word)) {
				    fprintf(ofp, "%s%s%s", KEYWORD_COLOR, word, RESET);
					break;
        		} else if (is_typename(word)) {
					fprintf(ofp, "%s%s%s", TYPENAME_COLOR, word, RESET);
					break;
				} else if (is_all_caps(word)) {
					fprintf(ofp, "%s%s%s", CAP_NAME_COLOR, word, RESET);
					break;
				} else if (isupper(word[0])) {
					fprintf(ofp, "%s%s%s", CAP_NAME_COLOR, word, RESET);
					break;
				}


				strcpy(buf, word);
				while (getftoken(ifp, word, MAXWORD) == SPACE) {
					strcat(buf, word);
				}
				if (tokentype == LPAREN) {
					fprintf(ofp, "%s%s%s", FNAME_COLOR, buf, RESET);
					word1 = BRAC_COLORS[get_brac_color_index(tokentype)];
        			fprintf(ofp, "%s%s%s", word1, word, RESET);
					break;
				} else {
					fprintf(ofp, "%s%s%s", NAME_COLOR, buf, RESET);
					goto switch_token;
				}
				break;
			case DIGIT:
				fprintf(ofp, "%s%s%s", DIGIT_COLOR, word, RESET);
				break;
			case STRING:
			case C_CHAR:
				fprintf(ofp, "%s%s%s", STRING_COLOR, word, RESET);
				break;
			case LPAREN: case LCURLY: case LBRACKET:
			case RPAREN: case RCURLY: case RBRACKET:
				word1 = BRAC_COLORS[get_brac_color_index(tokentype)];
				fprintf(ofp, "%s%s%s", word1, word, RESET);
				break;
			case PUNCT:
				fprintf(ofp, "%s%s%s", PUNCT_COLOR, word, RESET);
				break;
			case OTHERS: case SPACE:
				fprintf(ofp, "%s%s%s", OTHERS_COLOR, word, RESET);
				break;
			case COMMENT:
				fprintf(ofp, "%s%s%s", COMMENT_COLOR, word, RESET);
				break;
			case C_PREPROCESSOR_INI:
				strcpy(buf, word);
				getftoken(ifp, word, MAXWORD);
				if (tokentype == NAME) {
				    fprintf(ofp, "%s%s%s%s", KEYWORD_COLOR, buf, word, RESET);
				} else {
					fprintf(ofp, "%s%s%s", PUNCT_COLOR, buf, RESET);
					goto switch_token;
				}
				break;
			default:
				fprintf(ofp, "%c", tokentype);
				break;
		}
	}

}

int get_comment(int c, FILE *fp, char *token, int lim)
{
	char *p = token;
	if (C_FILE) {
		if (c == '/') {
			*p++ = c;
			c = getc(fp);
			if (c == '*') {
				*p++ = c;
				*p = getc(fp);	/* read one char further so that is's not / the * just added */
				for (; (c = getc(fp)) != EOF && --lim > 1; ) {
					if (*p == '*' && c == '/') {
						*++p = c;
						*++p = '\0';
						return tokentype = COMMENT;
					}
					*++p = c;
				}
			} else if (c == '/') {
				*p++ = c;
				while (((c = getc(fp)) != '\n' && c != EOF)  && --lim > 1) {
					*p++ = c;
				}
				*p++ = '\n';
				*p = '\0';
				return tokentype = COMMENT;
			}
			ungetc(c, fp);
			tokentype = PUNCT;
		}
	} else if (PY_FILE) {
		if (c == '#') {
			*p++ = c;
			while (((c = getc(fp)) != '\n' && c != EOF)  && --lim > 1) {
				*p++ = c;
			}
			*p++ = '\n';
			*p = '\0';
			return tokentype = COMMENT;
		}
	}

	return NULL;
}


int getftoken(FILE *fp, char *token, int lim)
{
	int c, is_comment, py_ml_str = 0;
	char *p = token;

	c = getc(fp);
	if (c == EOF)
		return tokentype = EOF;

	if ((is_comment = get_comment(c, fp, token, lim)) != NULL)
		return is_comment;

	if (isspace(c)) {
		*p++ = c;
		*p = '\0';
		tokentype = SPACE;
	} else if (isdigit(c)) {
		for (*p++ = c; isdigit(c = getc(fp)) && --lim > 1; )
			*p++ = c;
		*p = '\0';
		ungetc(c, fp);
		tokentype = DIGIT;
	} else if (isalpha(c) || c == '_') {
		for (*p++ = c; isalnum(c = getc(fp)) || c == '_' && --lim > 1; )
			*p++ = c;
		*p = '\0';
		ungetc(c, fp);
		tokentype = NAME;
	} else if (c == '\"') {
		*p = c;
		if (PY_FILE) {
			*++p = getc(fp);
			c = getc(fp);
			if (*p == '\"' && c == '\"') {
				py_ml_str = 1;
				*++p = c;
			} else {
				ungetc(c, fp);
				ungetc(*p, fp);
				p--;
			}
		}
		while((c = getc(fp)) != EOF && --lim > 3) {
			if (c == '\"') {
				if (PY_FILE && py_ml_str) {
					*++p = c;
					*++p = getc(fp);
					c = getc(fp);
					if (*p == '\"' && c == '\"') {
						*++p = c;
						*++p = '\0';
						return tokentype = STRING;
					}
				} else {
					break;
				}
			}

			if (c == '\\') {		/* if \ inside quote read the next char */
				*++p = c;
				c = getc(fp);
			}
			*++p = c;
		}
		if (c == EOF) {
			fprintf(stderr, "unexpected end of double quote.\n");
			*++p = '\0';
			return tokentype = STRING;
		}
		*++p = c;
		*++p = '\0';
		tokentype = STRING;
	} else if(c == '\'') {
		*p = c;
		if (PY_FILE) {
			*++p = getc(fp);
			c = getc(fp);
			if (*p == '\'' && c == '\'') {
				py_ml_str = 1;
				*++p = c;
			} else {
				ungetc(c, fp);
				ungetc(*p, fp);
				p--;
			}
		}
		while ((c = getc(fp)) != EOF && --lim > 3) {
			if (c == '\'') {
				if (PY_FILE && py_ml_str) {
					*++p = c;
					*++p = getc(fp);
					c = getc(fp);
					if (*p == '\'' && c == '\'') {
						*++p = c;
						*++p = '\0';
						return tokentype = STRING;
					}
				} else {
					break;
				}
			}

			if (c == '\\') {		/* if \ inside quote read the next char */
				*++p = c;
				c = getc(fp);
			}
			*++p = c;
		}
		if (c == EOF) {
			fprintf(stderr, "unexpected end of single quote.\n");
			*++p = '\0';
			return tokentype = STRING;
		}
		*++p = c;
		*++p = '\0';
		tokentype = C_CHAR;
	} else if(c == '(') {
		*p++ = c;
		*p = '\0';
		tokentype = LPAREN;
	} else if(c == ')') {
		*p++ = c;
		*p = '\0';
		tokentype = RPAREN;
	} else if (c== '{') {
		*p++ = c;
		*p = '\0';
		tokentype = LCURLY;
	} else if (c== '}') {
		*p++ = c;
		*p = '\0';
		tokentype = RCURLY;
	} else if (c == '[') {
		*p++ = c;
		*p = '\0';
		tokentype = LBRACKET;
	} else if (c == ']') {
		*p++ = c;
		*p = '\0';
		tokentype = RBRACKET;
	} else if (c == '#') {
		*p++ = c;
		*p = '\0';
		tokentype = C_PREPROCESSOR_INI;
	} else if (ispunct(c)) {
		*p++ = c;
		*p = '\0';
		tokentype = PUNCT;
	} else {
		*p++ = c;
		*p = '\0';
		tokentype = OTHERS;
	}

	return tokentype;
}

int get_brac_color_index(int brace_type)
{
	int color_index = 0;
	

	switch (brace_type) {
		case LPAREN:
			color_index = ENCOUNTERED_BRACES[PARENS] % BRAC_COLORS_LEN;
			ENCOUNTERED_BRACES[PARENS]++;
			break;
		case RPAREN:
			ENCOUNTERED_BRACES[PARENS]--;
			color_index = ENCOUNTERED_BRACES[PARENS] % BRAC_COLORS_LEN;
			break;
		case LCURLY:
			color_index = ENCOUNTERED_BRACES[CURLY] % BRAC_COLORS_LEN;
			ENCOUNTERED_BRACES[CURLY]++;
			break;
		case RCURLY:
			ENCOUNTERED_BRACES[CURLY]--;
			color_index = ENCOUNTERED_BRACES[CURLY] % BRAC_COLORS_LEN;
			break;
		case LBRACKET:
			color_index = ENCOUNTERED_BRACES[BRACKET] % BRAC_COLORS_LEN;
			ENCOUNTERED_BRACES[BRACKET]++;
			break;
		 case RBRACKET:
			ENCOUNTERED_BRACES[BRACKET]--;
			color_index = ENCOUNTERED_BRACES[BRACKET] % BRAC_COLORS_LEN;
			break;
		default:
			break;
	}

	return color_index;
}