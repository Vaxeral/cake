#include "cake.h"

char *tokenNames[] = {
	[CAKE_SUB] = "SUB",
	[CAKE_ADD] = "ADD",
	[CAKE_MUL] = "MUL",
	[CAKE_DIV] = "DIV",
	[CAKE_NUM] = "NUM",
	[CAKE_END] = "END",
};

main(void)
{
	Tokenizer tokenizer;
	tokenizer_tokenize(&tokenizer, "4 + 5 * 6");
	for (int i = 0; i < tokenizer.numTokens; i++) {
		Token *token = &tokenizer.tokens[i];
		if (token->symbol == CAKE_NUM) {
			printf("Token: %s", tokenNames[token->symbol]);
			printf(" Value: %f\n", token->number);
		} else {
			printf("Token: %s\n", tokenNames[token->symbol]);
		}
	}
}
