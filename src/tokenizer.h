typedef enum symbol {
	CAKE_SUB,
	CAKE_ADD,
	CAKE_MUL,
	CAKE_DIV,
	CAKE_NUM,
	CAKE_END,
} Symbol;

typedef enum tokenizer_state {
	CAKE_TOKENIZE,
	CAKE_SKIP_WHITESPACE,
	CAKE_OVERFLOW,
} TokenizerState;

typedef struct {
	Symbol symbol;
	float number;
} Token;

#define TOKENS_SIZE 1024
#define BUFFER_SIZE 64

typedef struct tokenizer {
	char buffer[BUFFER_SIZE];
	size_t lenBuffer;
	char *expression;
	char *character;
	Token tokens[TOKENS_SIZE];
	size_t numTokens;
	TokenizerState state;
} Tokenizer;

void tokenizer_init(Tokenizer *tokenizer, char *expression);
char tokenizer_peek(Tokenizer *tokenizer);
char tokenizer_advance(Tokenizer *tokenizer);
void tokenizer_addtoken(Tokenizer *tokenizer, Token *token);
Token *tokenizer_gettoken(Tokenizer *tokenizer);
void tokenizer_putchar(Tokenizer *tokenizer, char character);
int tokenizer_tokenize(Tokenizer *tokenizer, char *expression);

