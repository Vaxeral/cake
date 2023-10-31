#include "../src/cake.h"

static void print_group(MathContext *ctx, MathGroup *group)
{
	static const char *types[] = {
		[GROUP_ADD] = "+",
		[GROUP_SUBTRACT] = "-",
		[GROUP_MULTIPLY] = "*",
		[GROUP_DIVIDE] = "/",
	};
	if (types[group->type] != NULL) {
		printf("(");
		print_group(ctx, group->left);
		printf(" %s ", types[group->type]);
		print_group(ctx, group->right);
		printf(")");
		return;
	}
	switch (group->type) {
	case GROUP_NEGATE:
		printf("-");
		print_group(ctx, group->group);
		break;
	case GROUP_NUMBER:
		printf("%LF", group->value);
		break;
	default:
		break;
	}
}

static void print_token(MathToken *token)
{
	static char *tokenNames[] = {
		"null",

		"plus", "minus",
		"divide", "multiply",
		
		"and", "or", "xor",
		"mod",

		"floor", "ceil",
		"exp", "pow", "erfc",
		"sqrt", "cbrt", "root",
		"log10", "log", "ln",
		"sin", "cos",
		"tan", "cot",
		"sec", "csc",
		"sinh", "cosh", "tanh",
		"asinh", "acosh", "atanh",
		"gamma",

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

		"open_corner", "closed_corner",
		"open_curly", "closed_curly",
		"open_round", "closed_round",
		"raise", "lower",

		"keyword",
		"number",
		"variable",
	};
	printf("%s", tokenNames[token->type]);
}

int main(int argc, char *argv[])
{
	MathContext ctx;
	MathTokenizer tokenizer;
	MathGroup *group;

	(void) argc;
	(void) argv;

	const char *const text = "-(-3.1) * -(7 * (3 - 2) + 5)";
	printf("%s\n", text);

	memset(&ctx, 0, sizeof(ctx));
	memset(&tokenizer, 0, sizeof(tokenizer));
	if (!math_tokenize(&ctx, &tokenizer, text)) {
		printf("tokenizing failed: %s\n", math_error(&ctx));
		return -1;
	}

	for (size_t i = 0; i < tokenizer.numTokens; i++) {
		print_token(&tokenizer.tokens[i]);
		printf("\n");
	}

	if ((group = math_parsegroup(&ctx, &tokenizer)) == NULL) {
		printf("parsing failed: %s\n", math_error(&ctx));
		return -1;
	}
	print_group(&ctx, group);
	printf(" = %LF\n", math_computegroup(&ctx, group));
	return 0;
}
