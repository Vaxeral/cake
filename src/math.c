#include "cake.h"

number_t math_computegroup(MathContext *ctx, MathGroup *group)
{
	switch (group->type) {
	case GROUP_NEGATE:
		return -math_computegroup(ctx, group->group);
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
		[MATH_DOUBLE_PLUS_MINUS] = "double +/-",
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
