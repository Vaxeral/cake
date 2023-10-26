#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <math.h>

enum type {
	TOKEN_SUBTRACT,
	TOKEN_ADD,
	TOKEN_DIVIDE,
	TOKEN_MULTIPLY,
	TOKEN_OPENING_PARENTHESIS,
	TOKEN_CLOSING_PARENTHESIS,
	TOKEN_CARRET,

	TOKEN_PERCENT,
	TOKEN_BANG,
	TOKEN_DEGREE,

	TOKEN_NUMBER,
	TOKEN_VARIABLE,

	TOKEN_PI,
	TOKEN_E,

	TOKEN_AND,
	TOKEN_OR,
	TOKEN_XOR,
	TOKEN_MOD,

	TOKEN_SIN,
	TOKEN_COS,
	TOKEN_FLOOR,
	TOKEN_CEIL,
	TOKEN_EXP,
	TOKEN_POW,
	TOKEN_SQRT,
	TOKEN_CBRT,
	TOKEN_ROOT,
	TOKEN_LOG,
	TOKEN_LN,
	TOKEN_LOG10,
	TOKEN_ERFC,
	TOKEN_TAN,
	TOKEN_COT,
	TOKEN_SEC,
	TOKEN_CSC,
	TOKEN_SINH,
	TOKEN_COSH,
	TOKEN_TANH,
	TOKEN_ASINH,
	TOKEN_ACOSH,
	TOKEN_ATANH,
	TOKEN_GAMMA,

	TOKEN_END,
	TOKEN_INVALID,
};

typedef long double number_t;
typedef unsigned char byte_t;

#define NAME_SIZE 32

typedef struct token {
	enum type type;
	number_t value;
	byte_t name[NAME_SIZE];
	size_t lenName;
} Token;

#define TOKENS_SIZE 256

typedef struct tokenizer {
	Token tokens[TOKENS_SIZE];
	size_t nTokens;
	byte_t *bytes;
	byte_t *bytesAhead;
} Tokenizer;

void init(Tokenizer *tokenizer);
void tokenize(Tokenizer *tokenizer, byte_t *bytes);
number_t ld_parse(byte_t *bytes, int len);
