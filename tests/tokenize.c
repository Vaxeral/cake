#include "tokenizer.h"
#include <stdlib.h>
#include <stdio.h>

Tokenizer tokenizer;

int main(int argc, char *argv[])
{
	(void) argc;
	(void) argv;
	static char *tokenNames[] = {
		[TOKEN_SUBTRACT] = "SUBTRACT",
		[TOKEN_ADD] = "ADD",
		[TOKEN_DIVIDE] = "DIVIDE",
		[TOKEN_MULTIPLY] = "MULTIPLY",
		[TOKEN_OPENING_PARENTHESIS] = "OPENING_PARENTHESIS",
		[TOKEN_CLOSING_PARENTHESIS] = "CLOSING_PARENTHESIS",
		[TOKEN_PERCENT] = "PERCENT",
		[TOKEN_BANG] = "BANG",
		[TOKEN_DEGREE] = "DEGREE",
		[TOKEN_NUMBER] = "NUMBER",
		[TOKEN_VARIABLE] = "VARIABLE",
		[TOKEN_CARRET] = "CARRET",
		[TOKEN_PI] = "PI",
		[TOKEN_E] = "E",
		[TOKEN_SIN] = "SIN",
		[TOKEN_COS] = "COS",
		[TOKEN_FLOOR] = "FLOOR",
		[TOKEN_CEIL] = "CEIL",
		[TOKEN_EXP] = "EXP",
		[TOKEN_POW] = "POW",
		[TOKEN_SQRT] = "SQRT",
		[TOKEN_CBRT] = "CBRT",
		[TOKEN_ROOT] = "ROOT",
		[TOKEN_LOG] = "LOG",
		[TOKEN_LN] = "LN",
		[TOKEN_LOG10] = "LOG10",
		[TOKEN_ERFC] = "ERFC",
		[TOKEN_TAN] = "TAN",
		[TOKEN_COT] = "COT",
		[TOKEN_SEC] = "SEC",
		[TOKEN_CSC] = "CSC",
		[TOKEN_SINH] = "SINH",
		[TOKEN_COSH] = "COSH",
		[TOKEN_TANH] = "TANH",
		[TOKEN_ASINH] = "ASINH",
		[TOKEN_ACOSH] = "ACOSH",
		[TOKEN_ATANH] = "ATANH",
		[TOKEN_GAMMA] = "GAMMA",
		[TOKEN_AND] = "AND",
		[TOKEN_OR] = "OR",
		[TOKEN_XOR] = "XOR",
		[TOKEN_MOD] = "MOD",
		[TOKEN_END] = "END",
	};

	char *text = "log10log10((((PI / 2) and 4";
	byte_t *utf8 = (byte_t *)text;

	printf("%s\n", utf8);

	tokenize(&tokenizer, utf8);

	for (size_t i = 0; i < tokenizer.nTokens; i++) {
		Token token = tokenizer.tokens[i];
		printf("%s\n", tokenNames[token.type]);
	}
	return 0;
}
