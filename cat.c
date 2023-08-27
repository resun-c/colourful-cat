#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "keyword.c.h"
#include "keyword.py.h"

#define		MAXWORD				1024
#define		BUFSIZE				2048
#define		WHT					"\x1B[38;2;255;255;255m"
#define		DEFAULT				"\x1B[39m"
#define		RESET				"\x1B[0m"

/* colours for different kind of tokens */
#define		KEYWORD_COLOR		"\x1B[38;2;230;128;230m"
#define		FNAME_COLOR			"\x1B[38;2;247;247;110m"
#define		NAME_COLOR			"\x1B[38;2;77;163;255m"
#define		CAP_NAME_COLOR		"\x1B[38;2;0;230;172m"
#define		TYPENAME_COLOR		"\x1B[38;2;0;230;172m"
#define		STRING_COLOR		"\x1B[38;2;255;128;128m"
#define		PUNCT_COLOR			WHT
#define		DIGIT_COLOR			"\x1B[38;2;179;255;217m"
#define		OTHERS_COLOR		DEFAULT
#define		COMMENT_COLOR		"\x1B[38;2;45;134;89m"


/* all types of topkens */
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

/* global variable used by multiple functions */
int tokentype = 0;

/* selected colours for braces */
char *BRAC_COLORS[] = {
	"\x1B[38;2;240;173;48m",
	"\x1B[38;2;255;79;85m",
	"\x1B[38;2;77;210;255m"
};

/* length of BRAC_COLORS  */
int BRAC_COLOURS_LEN = 3;


/* tracking of encountered braces*/
int ENCOUNTERED_BRACES[] = {
	0,
	0,
	0,
	0,
	0,
	0,
	0,			/* PARENS */
	0,
	0,
	0,			/* CURLY */
	0,
	0,
	0			/* BRACKET */
	/* others are kept empty to match the index of PARAN, CURLY, BRACKET from enum token_types */
};

void filecopy(FILE *ifp, FILE *ofp);
int getftoken(FILE *fp, char *token, int lim);
int get_brac_color_index(int brace_type);
int is_keyword(char *s);
int is_typename(char *s);
int is_all_caps(char *s);

/* flag for different kind of file */
int C_FILE = 0, PY_FILE = 0;

int main (int argc, char *argv[])
{
	FILE *fp;
	char *prog = argv[0];			/* program name for errors */

	if (argc == 1) {				/* no args: copy standard input  */
		filecopy(stdin, stdout);
	} else {						/* print argument files */
		while (--argc > 0)			/* loop through arguments */
			if ((fp = fopen(*++argv, "r")) != NULL) {		/* file successfully opened */
				if (strstr(*argv, ".c")) {					/* check if .c file */
					C_FILE = 1;								/* set C_FILE flag*/
					PY_FILE = 0;							/* unset PY_FILE flag*/
				} else if (strstr(*argv, ".py")) {			/* check if .py file */
					C_FILE = 0;								/* unset C_FILE flag*/
					PY_FILE = 1;							/* set PY_FILE flag*/
				}
				filecopy(fp, stdout);						/* print file's content in the stdout */
				fclose(fp);
			} else {										/* unable to open file */
				fprintf(stderr, "%s: can't open %s\n", prog, *argv);
				exit(1);
			}
	}

	if (ferror(stdout)) {									/* error in stdout */
		fprintf(stderr, "%s: error writing stdout\n", prog);
		exit(2);
	}
	exit(0);
}
/* check if it's a keyword based on file type */
int is_keyword(char *s)
{
	if (C_FILE)
		return isckeyword(s);

	if (PY_FILE)
		return ispykeyword(s) || ispysoftkeyword(s);

	return 0;
}
/* check if it's a type name based on file type */
int is_typename(char *s)
{
	if (C_FILE)
		return isctype(s);

	if (PY_FILE)
		return ispytype(s);

	return 0;
}
/* check if a string has all capital laters */
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
    char word[MAXWORD], *word1, buf[BUFSIZE];

	/*  if a C file or Python file add colous to the content */
	if (C_FILE || PY_FILE) {
		while (getftoken(ifp, word, MAXWORD) != EOF) {			/* get each token */

			switch_token:
			switch (tokentype) {
				case NAME:										/* it's a string without any spaces */
    	    		if (is_keyword(word)) {						/* it's a keyword */
					    fprintf(ofp, "%s%s%s", KEYWORD_COLOR, word, RESET);
						break;
    	    		} else if (is_typename(word)) {				/* it's a typename */
						fprintf(ofp, "%s%s%s", TYPENAME_COLOR, word, RESET);
						break;
					} else if (is_all_caps(word)) {				/* it's an all capital string */
						fprintf(ofp, "%s%s%s", CAP_NAME_COLOR, word, RESET);
						break;
					} else if (isupper(word[0])) {				/* it's a uppercased string */
						fprintf(ofp, "%s%s%s", CAP_NAME_COLOR, word, RESET);
						break;
					}

					/* store the word to check the next token */
					strcpy(buf, word);
					/* ignore all the spaces after the word */
					while (getftoken(ifp, word, MAXWORD) == SPACE) {
						strcat(buf, word);
					}
					/* if the next token is left parenthesis, the stored word was a function name */
					if (tokentype == LPAREN) {
						/* apply function name colours to the stored word */
						fprintf(ofp, "%s%s%s", FNAME_COLOR, buf, RESET);
						/* get a colour for the parenthesis */
						word1 = BRAC_COLORS[get_brac_color_index(tokentype)];
    	    			fprintf(ofp, "%s%s%s", word1, word, RESET);
						break;
					} else {									/* not a function name */
						fprintf(ofp, "%s%s%s", NAME_COLOR, buf, RESET);		/* print the stored word */
						goto switch_token;									/* switch the next token */
					}
					break;
				case DIGIT:													/* digits */
					fprintf(ofp, "%s%s%s", DIGIT_COLOR, word, RESET);
					break;
				case STRING:												/* strings */
				case C_CHAR:												/* c char */
					fprintf(ofp, "%s%s%s", STRING_COLOR, word, RESET);
					break;
				case LPAREN: case LCURLY: case LBRACKET:					/* bracket */
				case RPAREN: case RCURLY: case RBRACKET:
					word1 = BRAC_COLORS[get_brac_color_index(tokentype)];	/* get a matching colour for brace */
					fprintf(ofp, "%s%s%s", word1, word, RESET);
					break;
				case PUNCT:													/* punctuation or operator */
					fprintf(ofp, "%s%s%s", PUNCT_COLOR, word, RESET);
					break;
				case OTHERS: case SPACE:									/* other and spaces */
					fprintf(ofp, "%s%s%s", OTHERS_COLOR, word, RESET);
					break;
				case COMMENT:												/* comments */
					fprintf(ofp, "%s%s%s", COMMENT_COLOR, word, RESET);
					break;
				case C_PREPROCESSOR_INI:									/* C preprocessors */
					/* store the word to check the next token */
					strcpy(buf, word);
					/* get the next toekn */
					getftoken(ifp, word, MAXWORD);
					if (tokentype == NAME) {								/* the next token is a sting, it's a preprocessors */
					    fprintf(ofp, "%s%s%s%s", KEYWORD_COLOR, buf, word, RESET);
					} else {												/* it's a punctuation or operator */
						fprintf(ofp, "%s%s%s", PUNCT_COLOR, buf, RESET);
						goto switch_token;
					}
					break;
				default:													/* for default print it */
					fprintf(ofp, "%c", tokentype);
					break;
			}
		}
	} else {																/* not a C file or Python file */
		while ((c = getc(ifp)) != EOF)
		putc(c, ofp);
	}

}

/* check if the char received from file starts a comment.
if it starts a comment then read until the end of the
comment and return it as token
*/
int get_comment(int c, FILE *fp, char *token, int lim)
{
	char *p = token;
	if (C_FILE) {								/* for C file */
		if (c == '/') {							/* if forward slash */
			*p++ = c;
			c = getc(fp);
			if (c == '*') {						/* and the next char is asterisk */
				*p++ = c;
				*p = getc(fp);					/* read one char further so that is's not / the * just added */
				for (; (c = getc(fp)) != EOF && --lim > 1; ) {				/* read until the End Of File or the limit is crossed */
					if (*p == '*' && c == '/') {			/* encountered a forward slash and the previous char is asterisk */
						*++p = c;
						*++p = '\0';
						return tokentype = COMMENT;		/* set tokentype to comment and return */
					}
					*++p = c;
				}
			} else if (c == '/') {						/* if another forward slash */
				*p++ = c;
				while (((c = getc(fp)) != '\n' && c != EOF)  && --lim > 1) {		/* read until the newline or End Of File */
					*p++ = c;
				}
				*p++ = '\n';
				*p = '\0';
				return tokentype = COMMENT;				/* set tokentype to comment and return */
			}
			ungetc(c, fp);								/* neither asterisk nor forward slash unread the next char */
			tokentype = PUNCT;							/* set tokentype to punctuation and return */
		}
	} else if (PY_FILE) {								/* for Python file */
		if (c == '#') {									/* if sharp character */
			*p++ = c;
			while (((c = getc(fp)) != '\n' && c != EOF)  && --lim > 1) {	/* read until the newline or End Of File */
				*p++ = c;
			}
			*p++ = '\n';
			*p = '\0';
			return tokentype = COMMENT;					/* set tokentype to comment and return */
		}
	}

	return -1;
}

/* read a token from file and store it in token and return it's type */
int getftoken(FILE *fp, char *token, int lim)
{
	int c, is_comment, py_ml_str = 0;
	char *p = token;

	c = getc(fp);	/* get a char */

	if (c == EOF)	/* End Of File, return; */
		return tokentype = EOF;

	/* check if it starts a comment */
	if ((is_comment = get_comment(c, fp, token, lim)) >= 0)
		return is_comment;

	if (isspace(c)) {						/* it's space */
		*p++ = c;
		*p = '\0';
		tokentype = SPACE;
	} else if (isdigit(c)) {				/* it starts a number */
		for (*p++ = c; isdigit(c = getc(fp)) && --lim > 1; )				/* read until the end of the number */
			*p++ = c;
		*p = '\0';
		ungetc(c, fp);
		tokentype = DIGIT;
	} else if (isalpha(c) || c == '_') {	/* it starts a name */
		for (*p++ = c; isalnum(c = getc(fp)) || (c == '_' && --lim > 1); )	/* read until the end of the number */
			*p++ = c;
		*p = '\0';
		ungetc(c, fp);
		tokentype = NAME;
	} else if (c == '\"') {					/* it starts a string */
		*p = c;
		if (PY_FILE) {						/* for python file check for multiline string */
			*++p = getc(fp);
			c = getc(fp);
			if (*p == '\"' && c == '\"') {	/* the next two char is also quote */
				py_ml_str = 1;				/* set py_ml_str flag */
				*++p = c;
			} else {
				ungetc(c, fp);
				ungetc(*p, fp);
				p--;
			}
		}
		while((c = getc(fp)) != EOF && --lim > 3) {		/* read until the End Of File  or limit crosses; save 3 rooms for terminating */
			if (c == '\"') {
				if (PY_FILE && py_ml_str) {				/* for python file if py_ml_str is set, check for multiline string */
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

			if (c == '\\') {				/* if backward slash inside quote read the next char */
				*++p = c;
				c = getc(fp);
			}
			*++p = c;
		}
		if (c == EOF) {						/* encounters End Of File */
			fprintf(stderr, "unexpected end of double quote.\n");
			*++p = '\0';
			return tokentype = STRING;
		}
		*++p = c;
		*++p = '\0';
		tokentype = STRING;
	} else if(c == '\'') {					/* it starts a string */
		*p = c;
		if (PY_FILE) {						/* for python file check for multiline string */
			*++p = getc(fp);
			c = getc(fp);
			if (*p == '\'' && c == '\'') {	/* the next two char is also apostrophe  */
				py_ml_str = 1;				/* set py_ml_str flag */
				*++p = c;
			} else {
				ungetc(c, fp);
				ungetc(*p, fp);
				p--;
			}
		}
		while ((c = getc(fp)) != EOF && --lim > 3) {	/* read until the End Of File  or limit crosses; save 3 rooms for terminating */
			if (c == '\'') {
				if (PY_FILE && py_ml_str) {				/* for python file if py_ml_str is set, check for multiline string */
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

			if (c == '\\') {				/* if backward slash inside quote read the next char */
				*++p = c;
				c = getc(fp);
			}
			*++p = c;
		}
		if (c == EOF) {						/* encounters End Of File */
			fprintf(stderr, "unexpected end of single quote.\n");
			*++p = '\0';
			return tokentype = STRING;
		}
		*++p = c;
		*++p = '\0';
		tokentype = C_CHAR;
	} else if(c == '(') {					/* left parenthesis */
		*p++ = c;
		*p = '\0';
		tokentype = LPAREN;
	} else if(c == ')') {					/* right parenthesis */
		*p++ = c;
		*p = '\0';
		tokentype = RPAREN;
	} else if (c== '{') {					/* left curly brace */
		*p++ = c;
		*p = '\0';
		tokentype = LCURLY;
	} else if (c== '}') {					/* right curly brace */
		*p++ = c;
		*p = '\0';
		tokentype = RCURLY;
	} else if (c == '[') {					/* left square brace */
		*p++ = c;
		*p = '\0';
		tokentype = LBRACKET;
	} else if (c == ']') {					/* right square brace */
		*p++ = c;
		*p = '\0';
		tokentype = RBRACKET;
	} else if (c == '#' && C_FILE) {		/* if sharp sign and it's a C file */
		*p++ = c;
		*p = '\0';
		tokentype = C_PREPROCESSOR_INI;		/* token type is C_PREPROCESSOR_INI */
	} else if (ispunct(c)) {				/* punctuation mark */
		*p++ = c;
		*p = '\0';
		tokentype = PUNCT;
	} else {								/* others */
		*p++ = c;
		*p = '\0';
		tokentype = OTHERS;
	}

	return tokentype;
}

/* return index for the colour matching brace
when a left brace is encountered:
return the reminder of the
`total number of that type of brace`
devidev by the `number of colours`
then add 1 to the `total number of that type of brace`

when right brace is encountered:
substract 1 from the `total number of that type of brace`
return the reminder of the
`total number of that type of brace`
devidev by the `number of colours`

 */
int get_brac_color_index(int brace_type)
{
	int color_index = 0;
	

	switch (brace_type) {
		case LPAREN:			/* left parenthesis */
			/*				number of parenthesis    % number of colours */
			color_index = ENCOUNTERED_BRACES[PARENS] % BRAC_COLOURS_LEN;
			ENCOUNTERED_BRACES[PARENS]++;									/* increase the number of parenthesis */
			break;
		case RPAREN:
			ENCOUNTERED_BRACES[PARENS]--;									/* decrease the number of parenthesis */
			/*				number of parenthesis    % number of colours */
			color_index = ENCOUNTERED_BRACES[PARENS] % BRAC_COLOURS_LEN;
			break;
		case LCURLY:
			/*				number of curly braces  % number of colours */
			color_index = ENCOUNTERED_BRACES[CURLY] % BRAC_COLOURS_LEN;
			ENCOUNTERED_BRACES[CURLY]++;									/* increase the number of curly brace */
			break;
		case RCURLY:
			ENCOUNTERED_BRACES[CURLY]--;									/* decrease the number of curly brace */
			/*				number of curly braces  % number of colours */
			color_index = ENCOUNTERED_BRACES[CURLY] % BRAC_COLOURS_LEN;
			break;
		case LBRACKET:
			/*				number of square braces   % number of colours */
			color_index = ENCOUNTERED_BRACES[BRACKET] % BRAC_COLOURS_LEN;
			ENCOUNTERED_BRACES[BRACKET]++;									/* increase the number of square brace */
			break;
		 case RBRACKET:
			ENCOUNTERED_BRACES[BRACKET]--;									/* decrease the number of square brace */
			/*				number of square braces   % number of colours */
			color_index = ENCOUNTERED_BRACES[BRACKET] % BRAC_COLOURS_LEN;
			break;
		default:
			break;
	}

	return color_index;
}