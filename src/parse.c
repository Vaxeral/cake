#include "cake.h"

struct math_parser {
	MathContext *ctx;
	MathTokenizer tokens;
};

struct math_operator {
	enum math_token_type tokenType;
	enum math_group_type groupType;
	int precedence;
};

static const struct math_operator *get_operator(enum math_token_type type)
{
	static const struct math_operator precedences[] = {
		{ TOKEN_AND, GROUP_AND, 1 },
		{ TOKEN_OR, GROUP_OR, 1 },
		{ TOKEN_XOR, GROUP_XOR, 1 },

		{ TOKEN_PLUS, GROUP_ADD, 2 },
		{ TOKEN_MINUS, GROUP_SUBTRACT, 2 },

		{ TOKEN_MOD, GROUP_MOD, 3 },

		{ TOKEN_MULTIPLY, GROUP_MULTIPLY, 4 },
		{ TOKEN_DIVIDE, GROUP_DIVIDE, 4 },
	};

	for(size_t i = 0; i < ARRLEN(precedences); i++)
		if (precedences[i].tokenType == type)
			return &precedences[i];
	return NULL;

}

static bool parser_consumetoken(struct math_parser *parser)
{
	if (parser->tokens.numTokens == 0)
		return false;
	parser->tokens.tokens++;
	parser->tokens.numTokens--;
	return true;
}

static bool parser_peektoken(struct math_parser *parser, MathToken *token)
{
	if (parser->tokens.numTokens == 0)
		return false;
	*token = *parser->tokens.tokens;
	return true;
}

static MathGroup *parse_expression(struct math_parser *parser, int precedence)
{
	MathToken token;
	MathGroup *group = NULL, *parent, *right;
	const struct math_operator *opr;

	if (!parser_peektoken(parser, &token))
		return NULL;
	switch (token.type) {
	case TOKEN_PLUS:
		parser_consumetoken(parser);
		break;
	case TOKEN_MINUS:
		group = malloc(sizeof(*group));
		if (group == NULL) {
			math_seterror(parser->ctx, MATH_MEMORY, errno);
			goto err;
		}
		group->type = GROUP_NEGATE;
		parser_consumetoken(parser);
		break;
	default:
	}

	parent = group;
	if (!parser_peektoken(parser, &token)) {
		math_seterror(parser->ctx, MATH_HANGING_OPERATOR, 0);
		goto err;
	}
	switch(token.type) {
	case TOKEN_PLUS:
	case TOKEN_MINUS:
		math_seterror(parser->ctx, MATH_DOUBLE_PLUS_MINUS, 0);
		goto err;
	case TOKEN_NUMBER:
		group = malloc(sizeof(*group));
		if (group == NULL) {
			math_seterror(parser->ctx, MATH_MEMORY, errno);
			goto err;
		}
		group->type = GROUP_NUMBER;
		group->value = token.value;
		break;
	case TOKEN_OPEN_ROUND:
		parser_consumetoken(parser);
		group = parse_expression(parser, 0);
		break;
	default:
		math_seterror(parser->ctx, MATH_INVALID_TOKEN, 0);
		goto err;
	}
	if (parent != NULL) {
		parent->group = group;
		group = parent;
	}
	parser_consumetoken(parser);
	if (!parser_peektoken(parser, &token) ||
			token.type == TOKEN_CLOSED_ROUND)
		return group;

	while ((opr = get_operator(token.type)) != NULL) {
		if (opr->precedence <= precedence)
			break;
		parent = malloc(sizeof(*parent));
		if (parent == NULL) {
			parser->ctx->error = MATH_MEMORY;
			parser->ctx->errorNumber = errno;
			goto err;
		}
		parent->type = opr->groupType;
		parent->left = group;
		parent->right = NULL;
		group = parent;

		parser_consumetoken(parser);
		if (!parser_peektoken(parser, &token)) {
			math_seterror(parser->ctx, MATH_HANGING_OPERATOR, 0);
			goto err;
		}
		right = parse_expression(parser, opr->precedence);
		if (right == NULL)
			goto err;
		group->right = right;
		if (!parser_peektoken(parser, &token))
			break;
	}
	return group;

err:
	free(group);
	/* TODO: call a function to deallocate even the sub groups */
	return NULL;
}


MathGroup *math_parsegroup(MathContext *ctx, MathTokenizer *tokenizer)
{
	struct math_parser parser;

	parser.ctx = ctx;
	parser.tokens = *tokenizer;
	return parse_expression(&parser, 0);
}
