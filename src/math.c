#include "cake.h"

number_t math_computegroup(MathContext *ctx, MathGroup *group)
{
	switch (group->type) {
	case GROUP_NUMBER:
		return group->value;
	case GROUP_ADD:
		return math_computegroup(ctx, group->left) +
			math_computegroup(ctx, group->right);
	case GROUP_SUBTRACT:
		return math_computegroup(ctx, group->left) -
			math_computegroup(ctx, group->right);
	case GROUP_MULTIPLY:
		return math_computegroup(ctx, group->left) *
			math_computegroup(ctx, group->right);
	case GROUP_DIVIDE:
		return math_computegroup(ctx, group->left) /
			math_computegroup(ctx, group->right);
	default:
		return 0;
	}
 }

struct math_parser {
	MathContext *ctx;
	MathTokenizer tokens;
};

static MathGroup *parse_expression(struct math_parser *parser, int precedence)
{
	static const struct {
		enum math_token_type tokenType;
		enum math_group_type groupType;
		int precedence;
	} precedences[] = {
		{ TOKEN_PLUS, GROUP_ADD, 1 },
		{ TOKEN_MINUS, GROUP_SUBTRACT, 1 },

		{ TOKEN_MULTIPLY, GROUP_MULTIPLY, 2 },
		{ TOKEN_DIVIDE, GROUP_DIVIDE, 2 },
	};

	MathToken token;
	MathGroup *group;

	group = malloc(sizeof(*group));
	if (group == NULL) {
		parser->ctx->error = MATH_MEMORY;
		parser->ctx->errorNumber = errno;
		goto err;
	}

	token = *parser->tokens.tokens;
	switch(token.type) {
	case TOKEN_NUMBER:
		group->type = GROUP_NUMBER;
		group->value = token.value;
		break;
	default:
		goto err;
	}
	parser->tokens.tokens++;
	parser->tokens.numTokens--;
	if (parser->tokens.numTokens == 0)
		return group;
	token = *parser->tokens.tokens;

	for(size_t i = 0; i < ARRLEN(precedences); ) {
		MathGroup *right;
		MathGroup *parent;

		if (precedences[i].tokenType != token.type) {
			i++;
			continue;
		}
		if (precedences[i].precedence <= precedence)
			break;
		parent = malloc(sizeof(*parent));
		if (parent == NULL) {
			parser->ctx->error = MATH_MEMORY;
			parser->ctx->errorNumber = errno;
			goto err;
		}
		parent->type = precedences[i].groupType;
		parent->left = group;
		parent->right = NULL;
		group = parent;

		parser->tokens.tokens++;
		parser->tokens.numTokens--;
		if (parser->tokens.numTokens == 0)
			goto err;
		right = parse_expression(parser, precedences[i].precedence);
		if (right == NULL)
			goto err;
		group->right = right;
		if (parser->tokens.numTokens == 0)
			break;
		token = *parser->tokens.tokens;
		/* go again */
		i = 0;
	}
	return group;

err:
	free(group);
	return NULL;
}


bool math_parsegroup(MathContext *ctx, MathTokenizer *tokenizer)
{
	struct math_parser parser;
	MathGroup *group;

	parser.ctx = ctx;
	parser.tokens = *tokenizer;
	group = parse_expression(&parser, 0);
	if (group != NULL)
		printf("%LF\n", math_computegroup(ctx, group));
	return group != NULL;
}

number_t math_computefunction(MathContext *ctx, MathFunction *func)
{
	number_t (*funcZero)(MathContext *ctx);
	number_t (*funcSingle)(MathContext *ctx, number_t);
	number_t (*funcDouble)(MathContext *ctx, number_t, number_t);
	number_t (*funcTriple)(MathContext *ctx, number_t, number_t, number_t);

	if (func->group == NULL) {
		const number_t *const args =
			&ctx->locals[ctx->numLocals - func->numParameters];
		/* system function */
		switch (func->numParameters) {
		case 0:
			funcZero = func->system;
			return (*funcZero)(ctx);
		case 1:
			funcSingle = func->system;
			return (*funcSingle)(ctx, args[0]);
		case 2:
			funcDouble = func->system;
			return (*funcDouble)(ctx, args[0], args[1]);
		case 3:
			funcTriple = func->system;
			return (*funcTriple)(ctx, args[0], args[1], args[2]);
		}
		return 0;
	}
	return math_computegroup(ctx, func->group);
}

size_t math_pushlocal(MathContext *ctx, number_t value)
{
	number_t *newLocals;

	newLocals = realloc(ctx->locals, sizeof(*ctx->locals) *
			(ctx->numLocals + 1));
	if (newLocals == NULL)
		return (size_t) -1;
	ctx->locals = newLocals;
	ctx->locals[ctx->numLocals] = value;
	return ctx->numLocals++;
}

bool math_poplocal(MathContext *ctx)
{
	if (ctx->numLocals == 0)
		return false;
	ctx->numLocals--;
	return true;
}

bool math_setlocal(MathContext *ctx, size_t addr, number_t value)
{
	if (addr >= ctx->numLocals)
		return false;
	ctx->locals[addr] = value;
	return true;
}

char *math_error(MathContext *ctx)
{
	static const char *errors[] = {
		[MATH_SUCCESS] = "success",
		[MATH_MEMORY] = "failed allocating memory",
		[MATH_INVALID_UTF8] = "the utf8 sequence is invalid",
		[MATH_INVALID_TOKEN] = "the token is invalid",
		[MATH_HANGING_OPERATOR] = "the operator is hanging at the end",
	};
	static char error[1024];

	if (ctx->errorNumber == 0) {
		strcpy(error, errors[ctx->error]);
	} else {
		sprintf(error, "%s: %s", errors[ctx->error],
				strerror(ctx->errorNumber));
	}
	return error;
}

bool math_tokenize(MathContext *ctx, MathTokenizer *tokenizer, const char *text)
{
	static const char *keywords[] = {
		"floor", "ceil",
		"exp", "pow",
		"erfc",
		"sqrt", "cbrt", "root",
		"log10", "log", "ln",
		"sin", "cos", "tan",
		"cot", "sec", "csc",
		"sinh", "cosh", "tanh",
		"asinh", "acosh", "atanh",
		"gamma",

		"and", "or", "xor", "mod"
	};
	static const enum math_token_type asciSymbols[] = {
		['+'] = TOKEN_PLUS,
		['-'] = TOKEN_MINUS,
		['/'] = TOKEN_DIVIDE,
		['*'] = TOKEN_MULTIPLY,
		['%'] = TOKEN_PERCENT,
		['!'] = TOKEN_BANG,
		['^'] = TOKEN_RAISE,
		['_'] = TOKEN_LOWER,
		['('] = TOKEN_OPEN_ROUND,
		[')'] = TOKEN_CLOSED_ROUND,
		['{'] = TOKEN_OPEN_CURLY,
		['}'] = TOKEN_CLOSED_CURLY,
		['['] = TOKEN_OPEN_CORNER,
		[']'] = TOKEN_CLOSED_CORNER,

		['0' ... '9'] = TOKEN_NUMBER,
	};
	static const struct {
		wchar_t symbol;
		enum math_token_type type;
	} specialSymbols[] = {
		{ L'°', TOKEN_DEGREES },
		{ L'∈', TOKEN_ELEMENT_OF },
		{ L'∩', TOKEN_INTERSECTION },
		{ L'∪', TOKEN_UNION },
		{ L'ℝ', TOKEN_REAL_NUMBERS },
		{ L'ℂ', TOKEN_COMPLEX_NUMBERS },
		{ L'ℤ', TOKEN_INTEGERS },
		{ L'ℕ', TOKEN_NATURAL_NUMBERS },
		{ L'↦', TOKEN_MAPS_TO },
		{ L'⊆', TOKEN_SUBSET_OF },
		{ L'⇒', TOKEN_IMPLIES },
	};
	size_t lenText;
	wchar_t wch;
	MathToken token;
	MathToken *newTokens;

	lenText = strlen(text);
	while (text[tokenizer->position] != '\0') {
		size_t len;

		/* skip all space */
		if (isspace(text[tokenizer->position])) {
			tokenizer->position++;
			continue;
		}

		token.position = tokenizer->position;

		/* get asci symbols */
		const size_t index = (unsigned char) text[tokenizer->position];
		if (index < ARRLEN(asciSymbols) &&
				asciSymbols[index] != TOKEN_NULL) {
			token.type = asciSymbols[index];
			if (token.type == TOKEN_NUMBER) {
				char *end;

				token.type = TOKEN_NUMBER;
				token.value =
					strtold(&text[tokenizer->position],
							&end);
				len = end - &text[tokenizer->position];
			} else {
				len = 1;
			}
			goto end;
		}

		/* check for keyword */
		for (size_t i = 0; i < ARRLEN(keywords); i++) {
			len = strlen(keywords[i]);
			if (strncmp(keywords[i], &text[tokenizer->position],
					MIN(lenText - tokenizer->position, len))
					== 0) {
				token.type = TOKEN_KEYWORD;
				strcpy(token.word, keywords[i]);
				goto end;
			}
		}

		/* convert to wchar_t */
		len = mbrtowc(&wch, &text[tokenizer->position],
				lenText - tokenizer->position, NULL);
		if (len == (size_t) -1 || len == (size_t) -2) {
			/* invalid utf8 */
			ctx->error = MATH_INVALID_UTF8;
			ctx->errorNumber = errno;
			return false;
		}

		if (iswalpha(wch)) {
			token.type = TOKEN_VARIABLE;
			memcpy(token.word, &text[tokenizer->position], len);
			token.word[len] = '\0';
			goto end;
		}

		/* check for special symbols */
		for (size_t i = 0; i < ARRLEN(specialSymbols); i++)
			if (specialSymbols[i].symbol == wch) {
				token.type = specialSymbols[i].type;
				goto end;
			}

		/* invalid token */
		ctx->error = MATH_INVALID_TOKEN;
		ctx->errorNumber = 0;
		return false;

	end:
		newTokens = realloc(tokenizer->tokens,
				sizeof(*tokenizer->tokens) *
				(tokenizer->numTokens + 1));
		if (newTokens == NULL) {
			/* memory error */
			ctx->error = MATH_MEMORY;
			ctx->errorNumber = errno;
			return false;
		}
		tokenizer->tokens = newTokens;
		tokenizer->tokens[tokenizer->numTokens++] = token;
		tokenizer->position += len;

		/* safety so that your pc doesn't crash
		 * when there is a bug
		 */
		if (len == 0)
			return false;
	}
	return true;
}

void math_freetokenizer(MathContext *ctx, MathTokenizer *tokenizer)
{
	(void) ctx;
	free(tokenizer->tokens);
}
