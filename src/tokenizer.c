#include "tokenizer.h"
#include <stdio.h>

number_t ld_parse(byte_t *bytes, int len)
{
	number_t integer, fraction;

	bool foundSeparator;

	foundSeparator = false;
	int i;
	for (i = 0; i < len; i++)
		if (bytes[i] == '.') {
			foundSeparator = true;
			break;
		}

	fraction = 0;
	integer = 0;
	if (!foundSeparator) {
		for (int j = 0; j < len; j++) {
			int digit = bytes[j] - '0';
			integer *= 10;
			integer += digit;
		}
	} else {
		for (int j = 0; j < i; j++) {
			int digit = bytes[j] - '0';
			integer *= 10;
			integer += digit;
		}
		for (int j = len - 1; j > i; j--) {
			int digit = bytes[j] - '0';
			fraction += digit;
			fraction /= 10;
		}
	}
	return integer + fraction;
}

byte_t *utf8_next(byte_t *bytes, int *len)
{
	byte_t byte;
	byte = *bytes;
	if ((byte & 0x80) == 0)
		*len = 1;
	else if ((byte & 0xE0) == 0xC0)
		*len = 2;
	else if ((byte & 0xF0) == 0xE0)
		*len = 3;
	else if ((byte & 0xF8) == 0xF0)
		*len = 4;
	return bytes += *len;
}

void init(Tokenizer *tokenizer)
{
	memset(tokenizer, 0, sizeof(*tokenizer));
}

void puttoken(Tokenizer *tokenizer, Token *token)
{
	if (tokenizer->nTokens > TOKENS_SIZE - 1)
		return;
	tokenizer->tokens[tokenizer->nTokens++] = *token;
}

#define ARRLEN(a) (sizeof(a)/sizeof(*(a)))

enum type iskeyword(byte_t *name, size_t len)
{
	static char *keywords[] = {
		[TOKEN_SIN] = "sin",
		[TOKEN_COS] = "cos",
		[TOKEN_FLOOR] = "floor",
		[TOKEN_CEIL] = "ceil",
		[TOKEN_EXP] = "exp",
		[TOKEN_POW] = "pow",
		[TOKEN_SQRT] = "sqrt",
		[TOKEN_CBRT] = "cbrt",
		[TOKEN_ROOT] = "root",
		[TOKEN_LOG] = "log",
		[TOKEN_LN] = "ln",
		[TOKEN_LOG10] = "log10",
		[TOKEN_ERFC] = "erfc",
		[TOKEN_TAN] = "tan",
		[TOKEN_COT] = "cot",
		[TOKEN_SEC] = "sec",
		[TOKEN_CSC] = "csc",
		[TOKEN_SINH] = "sinh",
		[TOKEN_COSH] = "cosh",
		[TOKEN_TANH] = "tanh",
		[TOKEN_ASINH] = "asinh",
		[TOKEN_ACOSH] = "acosh",
		[TOKEN_ATANH] = "atanh",
		[TOKEN_GAMMA] = "gamma",
		[TOKEN_PI] = "PI",
		[TOKEN_E] = "E",
		[TOKEN_AND] = "and",
		[TOKEN_OR] = "or",
		[TOKEN_XOR] = "xor",
		[TOKEN_MOD] = "mod"
	};

	for (size_t i = 0; i < ARRLEN(keywords); i++) {
		if (keywords[i] != NULL
				&& memcmp(keywords[i], name, len) == 0) {
			return i;
		}
	}
	return TOKEN_INVALID;
}

/* bytes must be null terminated. */
void tokenize(Tokenizer *tokenizer, byte_t *bytes)
{
	static byte_t utf8[][4] = {
		[TOKEN_PI] = { 0xcf, 0x80 },
		[TOKEN_GAMMA] = { 0xce, 0xb3 },
		[TOKEN_DEGREE] = { 0xc2, 0xb0 },
	};
	bool inVariable;

	Token token;
	token.lenName = 0;

	tokenizer->bytesAhead = bytes;
	while (token.type != TOKEN_END) {
		token.type = TOKEN_INVALID;

		int lenUtf8Seq;
		tokenizer->bytes = tokenizer->bytesAhead;
		tokenizer->bytesAhead = utf8_next(tokenizer->bytesAhead,
			&lenUtf8Seq);

		memcpy(&token.name[token.lenName], tokenizer->bytes, lenUtf8Seq);
		token.lenName += lenUtf8Seq;

		if (lenUtf8Seq == 1) {
			byte_t *utf8Seq;
			
			switch (tokenizer->bytes[0]) {
			case '+': token.type = TOKEN_ADD; break;
			case '-': token.type = TOKEN_SUBTRACT; break;
			case '/': token.type = TOKEN_DIVIDE; break;
			case '*': token.type = TOKEN_MULTIPLY; break;
			case '(': token.type = TOKEN_OPENING_PARENTHESIS; break;
			case ')': token.type = TOKEN_CLOSING_PARENTHESIS; break;
			case '%': token.type = TOKEN_PERCENT; break;
			case '!': token.type = TOKEN_BANG; break;
			case '^': token.type = TOKEN_CARRET; break;
			case '0': case '1': case '2':
			case '3': case '4': case '5':
			case '6': case '7': case '8':
			case '9': case '.':
				utf8Seq = utf8_next(tokenizer->bytes, &lenUtf8Seq);
				if (inVariable)
					goto letter;
				if (lenUtf8Seq > 1 || !(isdigit(utf8Seq[0]) || utf8Seq[0] == '.')) {
					token.type = TOKEN_NUMBER;
					token.value = ld_parse(token.name, token.lenName);
				}
				break;
			case 'A' ... 'Z':
			case 'a' ... 'z':
			letter:
				inVariable = true;
				utf8Seq = utf8_next(tokenizer->bytes, &lenUtf8Seq);
				if (lenUtf8Seq > 1 || !isalnum(utf8Seq[0])) {
					enum type type = iskeyword(token.name, token.lenName);
					token.type = (type == TOKEN_INVALID) ? TOKEN_VARIABLE : type;
					if (token.type == TOKEN_PI)
						token.value = M_PI;
					if (token.type == TOKEN_E)
						token.value = M_E;
					inVariable = false;
				}
				break;
			case '\0':
				token.type = TOKEN_END;
				break;
			default:
				token.lenName = 0;
				break;
			}
		} else {
			if (memcmp(tokenizer->bytes, utf8[TOKEN_PI], lenUtf8Seq) == 0) {
				token.type = TOKEN_PI;
				token.value = M_PI;
			}
			if (memcmp(tokenizer->bytes, utf8[TOKEN_GAMMA], lenUtf8Seq) == 0)
				token.type = TOKEN_GAMMA;
			if (memcmp(tokenizer->bytes, utf8[TOKEN_DEGREE], lenUtf8Seq) == 0)
				token.type = TOKEN_DEGREE;
		}

		if (token.type != TOKEN_INVALID) {
			token.name[token.lenName] = '\0';
			puttoken(tokenizer, &token);
			token.lenName = 0;
		}
	}
}
