#include "cake.h"

void tokenizer_init(Tokenizer *tokenizer, char *expression)
{
	memset(tokenizer, 0, sizeof(*tokenizer));
	tokenizer->expression = expression;
	tokenizer->character = expression;
}

char tokenizer_peek(Tokenizer *tokenizer)
{
	char *character;
	if (*tokenizer->character == '\0')
		return -1;
	character = tokenizer->character;
	character++;
	return *character;
}

char tokenizer_advance(Tokenizer *tokenizer)
{
	char character;
	if (*tokenizer->character == '\0')
		return '\0';
	character = *tokenizer->character;
	tokenizer->character++;
	return character;
}

void tokenizer_addtoken(Tokenizer *tokenizer, Token *token)
{
	if (tokenizer->numTokens + 1 > TOKENS_SIZE) {
		tokenizer->state = CAKE_OVERFLOW;
		return;
	}
	tokenizer->tokens[tokenizer->numTokens++] = *token;
}

Token *tokenizer_gettoken(Tokenizer *tokenizer)
{
	if (tokenizer->numTokens == 0)
		return NULL;
	return &tokenizer->tokens[tokenizer->numTokens - 1];
}

void tokenizer_putchar(Tokenizer *tokenizer, char character)
{
	if (tokenizer->lenBuffer + 1 > BUFFER_SIZE) {
		tokenizer->state = CAKE_OVERFLOW;
		return;
	}
	tokenizer->buffer[tokenizer->lenBuffer++] = character;
}

int tokenizer_tokenize(Tokenizer *tokenizer, char *expression)
{
	char character;
	Token *token;
	Token *lastToken;

	tokenizer_init(tokenizer, expression);
	
	while (1) {
		token = tokenizer_gettoken(tokenizer);
		switch (tokenizer->state) {
		case CAKE_TOKENIZE:
			character = tokenizer_advance(tokenizer);
			if (lastToken != NULL && 
					lastToken->symbol == CAKE_NUM &&
						token->symbol != CAKE_NUM) {
				tokenizer->buffer[tokenizer->lenBuffer++] = '\0';
				lastToken->number = atof(tokenizer->buffer);
				tokenizer->lenBuffer = 0;
			}
			if (token != NULL && token->symbol == CAKE_END)
				goto end;
			switch (character) {
			case '+': tokenizer_addtoken(tokenizer, &(Token){CAKE_ADD}); break;
			case '-': tokenizer_addtoken(tokenizer, &(Token){CAKE_SUB}); break;
			case '/': tokenizer_addtoken(tokenizer, &(Token){CAKE_DIV}); break;
			case '*': tokenizer_addtoken(tokenizer, &(Token){CAKE_MUL}); break;
			case '0': case '1': case '2':
			case '3': case '4': case '5':
			case '6': case '7': case '8':
			case '9': case '.':
				if (token == NULL ||
						token != NULL && token->symbol != CAKE_NUM)
					tokenizer_addtoken(tokenizer, &(Token){CAKE_NUM});
				tokenizer_putchar(tokenizer, character);
				break;
			case '\0':
				tokenizer_addtoken(tokenizer, &(Token){CAKE_END});
				break;
			}
			break;
		case CAKE_OVERFLOW:
			return 1;
			break;
		case CAKE_SKIP_WHITESPACE:
			character = tokenizer_peek(tokenizer);
			if (!isspace(character))
				tokenizer->state = CAKE_TOKENIZE;
			tokenizer_advance(tokenizer);
			break;
		}
		lastToken = token;
	}
end:
	return 0;
}
