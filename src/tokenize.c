#include "cake.h"

bool math_tokenize(MathContext *ctx, MathTokenizer *tokenizer, const char *text)
{
	static const struct {
		const char *word;
		enum math_token_type type;
 	} keywords[] = {
		{ "floor", TOKEN_FLOOR }, { "ceil", TOKEN_CEIL },
		{ "exp", TOKEN_EXP }, { "pow", TOKEN_POW },
		{ "erfc", TOKEN_ERFC },
		{ "sqrt", TOKEN_SQRT }, { "cbrt", TOKEN_CBRT }, { "root", TOKEN_ROOT },
		{ "log10", TOKEN_LOG10 }, { "log", TOKEN_LOG }, { "ln", TOKEN_LN },
		{ "sin", TOKEN_SIN }, { "cos", TOKEN_COS }, { "tan", TOKEN_TAN },
		{ "cot", TOKEN_COT }, { "sec", TOKEN_SEC }, { "csc", TOKEN_CSC },
		{ "sinh", TOKEN_SINH }, { "cosh", TOKEN_COSH }, { "tanh", TOKEN_TANH },
		{ "asinh", TOKEN_ASINH }, { "acosh", TOKEN_ACOSH }, { "atanh", TOKEN_ATANH },
		{ "gamma", TOKEN_GAMMA },

		{ "and", TOKEN_AND }, { "or", TOKEN_OR }, { "xor", TOKEN_XOR }, { "mod", TOKEN_MOD }
	};
	static const enum math_token_type asciSymbols[] = {
		['+'] = TOKEN_PLUS, ['-'] = TOKEN_MINUS,
		['/'] = TOKEN_DIVIDE, ['*'] = TOKEN_MULTIPLY,
		['%'] = TOKEN_PERCENT,
		['!'] = TOKEN_BANG,
		['^'] = TOKEN_RAISE, ['_'] = TOKEN_LOWER,

		['('] = TOKEN_OPEN_ROUND, [')'] = TOKEN_CLOSED_ROUND,
		['{'] = TOKEN_OPEN_CURLY, ['}'] = TOKEN_CLOSED_CURLY,
		['['] = TOKEN_OPEN_CORNER, [']'] = TOKEN_CLOSED_CORNER,

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
			len = strlen(keywords[i].word);
			if (strncmp(keywords[i].word, &text[tokenizer->position],
					MIN(lenText - tokenizer->position, len))
					== 0) {
				token.type = keywords[i].type;
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
