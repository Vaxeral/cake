enum math_token_type {
	TOKEN_NULL,

	TOKEN_ADD,
	TOKEN_SUBTRACT,
	TOKEN_DIVIDE,
	TOKEN_MULTIPLY,

	TOKEN_PERCENT,
	TOKEN_BANG,
	TOKEN_DEGREES,

	TOKEN_ELEMENT_OF,
	TOKEN_INTERSECTION,
	TOKEN_UNION,
	TOKEN_REAL_NUMBERS,
	TOKEN_COMPLEX_NUMBERS,
	TOKEN_INTEGERS,
	TOKEN_NATURAL_NUMBERS,
	TOKEN_MAPS_TO,
	TOKEN_SUBSET_OF,
	TOKEN_IMPLIES,

	TOKEN_OPEN_CORNER,
	TOKEN_CLOSED_CORNER,
	TOKEN_OPEN_CURLY,
	TOKEN_CLOSED_CURLY,
	TOKEN_OPEN_ROUND,
	TOKEN_CLOSED_ROUND,
	TOKEN_RAISE,
	TOKEN_LOWER,

	TOKEN_KEYWORD,
	TOKEN_NUMBER,
	TOKEN_VARIABLE,
};

enum math_group_type {
	GROUP_NULL,
	GROUP_NUMBER,
	GROUP_FUNCTION,
	GROUP_OPERATOR,
};

typedef struct math_group {
	enum math_group_type type;
	struct {
		struct math_group *left;
		struct math_group *right;
	};
	union {
		number_t value;
		enum math_token_type operator;
		struct {
			char name[256];
			size_t numParameters;
		};
	};
} MathGroup;

typedef struct math_variable {
	char name[256];
	MathGroup *group;
} MathVariable;

typedef struct math_function {
	char name[256];
	char (*parameters)[256];
	size_t numParameters;
	MathGroup *group;
	void *system;
} MathFunction;

typedef struct math_token {
	size_t position;
	enum math_token_type type;
	union {
		number_t value;
		char word[8];
	};
} MathToken;

typedef struct math_tokenizer {
	MathToken *tokens;
	size_t numTokens;
	size_t position;
} MathTokenizer;

enum math_error {
	MATH_SUCCESS,

	MATH_MEMORY,
	MATH_INVALID_UTF8,
	MATH_INVALID_TOKEN,
	MATH_INVALID_CALL,
};

typedef struct math_context {
	MathVariable *variables;
	size_t numVariables;
	number_t *locals;
	size_t numLocals;
	MathFunction *functions;
	size_t numFunctions;
	MathGroup *group;
	enum math_error error;
	int errorNumber;
} MathContext;

number_t math_computefunction(MathContext *ctx, MathFunction *func);
number_t math_computevariable(MathContext *ctx, MathVariable *var);

size_t math_pushlocal(MathContext *ctx, number_t value);
bool math_poplocal(MathContext *ctx);
bool math_setlocal(MathContext *ctx, size_t addr, number_t value);

char *math_error(MathContext *ctx);

bool math_tokenize(MathContext *ctx, MathTokenizer *tokenizer, const char *text);
void math_freetokenizer(MathContext *ctx, MathTokenizer *tokenizer);
bool math_parsegroup(MathContext *ctx, MathTokenizer *tokenizer);

