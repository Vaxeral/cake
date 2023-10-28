#include "../src/cake.h"

int main(int argc, char *argv[])
{
	const char *const text = "3 * 7 + 5";
	MathContext ctx;
	MathTokenizer tokenizer;

	static char *tokenNames[] = {
		"null",

		"plus",
		"minus",
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

	(void) argc;
	(void) argv;
	printf("%s\n", text);

	memset(&ctx, 0, sizeof(ctx));
	memset(&tokenizer, 0, sizeof(tokenizer));
	if (!math_tokenize(&ctx, &tokenizer, text)) {
		printf("tokenizing failed: %s\n", math_error(&ctx));
		return -1;
	}

	for (size_t i = 0; i < tokenizer.numTokens; i++) {
		const MathToken token = tokenizer.tokens[i];
		printf("%s\n", tokenNames[token.type]);
	}

	if (!math_parsegroup(&ctx, &tokenizer)) {
		printf("parsing failed: %s\n", math_error(&ctx));
		return -1;
	}
	
	return 0;
}
