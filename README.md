# colourful-cat

`cat` _file ..._

Colourful Cat is a command-line program written in C that prints C and Python codes colourfully in the terminal.

I prefer to cat files in the terminal rather than opening them in a text editor. However, it is hard to find something from a large source file when printed in a single colour. So, I wrote a cat that prints C and Python files colourfully! Like VS Code, it uses different colours to represent functions, variables, operators, comments, etc.. These colours make each element of the file distinguishable and easy to locate. This representation makes it easier to find something from a large source file from the terminal.

#### description of the files:
`cat.c`: This file interacts with files and the terminal. It takes file names as arguments and if they are .c or .py files then it applies colours to them. Otherwise, it prints them with the default colour. The function `getftoken(FILE *fp, char *token, int lim)` takes a file (`fp`), a buffer (`token`) and a limit (`lim`) of the buffer. It reads each similar type of character from the given file and stores them in the buffer as a token. Then it decides the type of that token and returns that type. There are predefined token types defined in `token_types` `enum`. There are also predefined colours of different kind of tokens, such as keyword, types, function names, variable names, name that has all capital letters, strings, punctuation name, digits, comments, and others. For C and Python files, it reads one token at a time. Then based on the type of that token it applies colours to it. If it finds a name (word) it gets the next non-space token and if that's a left parenthesis it applies the colour of the function name to that name. For braces, it passes the type of the brace to a function called `get_brac_color_index(int brace_type)` that returns a matching colour for that brace based on already encountered braces. When `getftoken` encounters the start of a comment or string it reads until the end of it and puts it into the `token` buffer and returns the token type comment or string.

`keyword.c.h`: This file contains functions to check if a given string is a reserved C keyword/type or not. It has two functions `isckeyword` and `isctype`. They match a string against reserved keywords/types and return 1 if matches, and 0 otherwise.

`keyword.py.h`: This file contains functions to check if a given string is a reserved Python keyword/soft keyword/type or not. It is a modified version of the cpython/Lib/keyword.py file, which is a Python library to interacts with Python keywords and types.  It has three functions `ispykeyword`, `ispysoftkeyword` and `ispytype`. They match a string against reserved keywords/soft keywords/types and return 1 if matches, and 0 otherwise.

Happy Coding!