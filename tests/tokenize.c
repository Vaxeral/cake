#include "../src/cake.h"

int main(int argc, char *argv[])
{
	static char *tokenNames[] = {
		"null",

		"add",
		"subtract",
		"divide",
		"multiply",

		"percent",
		"bang",
		"degrees",

		"element_of",
		"intersection",
		"union",
		"real_numbers",
		"complex_numbers",
		"integers",
		"natural_numbers",
		"maps_to",
		"subset_of",
		"implies",

		"open_corner",
		"closed_corner",
		"open_curly",
		"closed_curly",
		"open_round",
		"closed_round",
		"raise",
		"lower",

		"keyword",
		"number",
		"variable",
	};
	const char *const text = "log10log10((((PI / 2) and 4";
	MathContext ctx;
	MathTokenizer tokenizer;

	(void) argc;
	(void) argv;
	printf("%s\n", text);

	memset(&ctx, 0, sizeof(ctx));
	memset(&tokenizer, 0, sizeof(tokenizer));
	if (!math_tokenize(&ctx, &tokenizer, text)) {
		printf("tokenizing failed\n");
		return -1;
	}

	for (size_t i = 0; i < tokenizer.numTokens; i++) {
		const MathToken token = tokenizer.tokens[i];
		printf("%s\n", tokenNames[token.type]);
	}
	return 0;
}
