/*
 * File:    strexpr.c
 * Purpose: calculate value from algebraic string expression
 *          (Stripped down from expr.c from J.T. Conklin <jtc@netbsd.org>.)
 * Version: 1.0 (Nov 1995)
 * Author:  Piet Tutelaers
 */

#include <stdio.h>	/* sprintf() */
#include <stdlib.h>	/* free(), malloc() */
#include <string.h>	/* strchr(), strcpy() */
#include <ctype.h>	/* isdigit(), isspace() */
#include "basics.h"	/* fatal() */

const char *OPERATORS = "+-*/%()";

enum token {
	ADD, SUB, MUL, DIV, MOD, LP, RP, OPERAND, EOI
};

struct val {
	enum {
		integer,
		string
	} type;

	union {
		char           *s;
		int             i;
	} u;
};

enum token	token;
struct val     *tokval;
char           *pexpr;

struct val *
make_int(int i)
{
	struct val     *vp;

	vp = (struct val *) malloc(sizeof(*vp));
	if (vp == NULL) fatal("expression error\n");
	vp->type = integer;
	vp->u.i = i;
	return vp;
}


struct val *
make_str(char *s)
{
	char operand[32]; int i;

	struct val     *vp;

	vp = (struct val *) malloc(sizeof(*vp));
	if (vp == NULL) fatal("expression error\n");

        pexpr = s; i = 0;
        while (1) {
	   if (isspace(*pexpr) || *pexpr == '\0') break;
	   if (strchr(OPERATORS, *pexpr) != NULL) break;
	   if (i == 30) fatal("operand too large\n");
	   operand[i++] = *pexpr++;
	}
	if (i == 0) fatal("invalid operand\n");
	operand[i] = '\0';
	vp->u.s = malloc(i+1);
	if (vp->u.s == NULL) fatal("Out of memory\n");
	strcpy(vp->u.s, operand);
	vp->type = string;
	return vp;
}


void
free_value(struct val *vp)
{
	if (vp->type == string)
		free(vp->u.s);
	free(vp);
}


/* determine if vp is an integer; if so, return it's value in *r */
int
is_integer(struct val *vp, int *r)
{
	char           *s;
	int             neg;
	int             i;

	if (vp->type == integer) {
		*r = vp->u.i;
		return 1;
	}

	/*
	 * POSIX.2 defines an "integer" as an optional unary minus 
	 * followed by digits.
	 */
	s = vp->u.s;
	i = 0;

	neg = (*s == '-');
	if (neg)
		s++;

	while (*s) {
		if (!isdigit(*s))
			return 0;

		i *= 10;
		i += *s - '0';

		s++;
	}

	if (neg)
		i *= -1;

	*r = i;
	return 1;
}


/* coerce to vp to an integer */
int
to_integer(struct val *vp)
{
	int             r;

	if (vp->type == integer)
		return 1;

	if (is_integer(vp, &r)) {
		free(vp->u.s);
		vp->u.i = r;
		vp->type = integer;
		return 1;
	}

	return 0;
}


/* coerce to vp to an string */
void
to_string(struct val *vp)
{
	char           *tmp;

	if (vp->type == string)
		return;

	tmp = malloc(25);
	if (tmp == NULL) fatal("Out of memory\n");
	sprintf(tmp, "%d", vp->u.i);
	vp->type = string;
	vp->u.s = tmp;
}

void
nexttoken()
{
	char           *i;	/* index */

	while (isspace(*pexpr)) pexpr++;

	if (*pexpr == '\0') {
		token = EOI;
		return;
	}

	if ((i = strchr(OPERATORS, *pexpr)) != NULL) {
		pexpr++;
		token = i - OPERATORS;
		return;
	}
	tokval = make_str(pexpr);
	token = OPERAND;
	return;
}

struct val *
eval6()
{
	struct val *eval3(void);
	struct val     *v;

	if (token == OPERAND) {
		nexttoken();
		return tokval;

	} else if (token == LP) {
		nexttoken();
		v = eval3();

		if (token != RP)
		   fatal("missing parenthesis in expression\n");
		nexttoken();
		return v;
	} else fatal("expression error\n");
	/* NOTREACHED */
}

/* Parse and evaluate multiplication and division expressions */
struct val *
eval4()
{
	struct val     *l, *r;
	enum token	op;

	l = eval6();
	while ((op = token) == MUL || op == DIV || op == MOD) {
		nexttoken();
		r = eval6();

		if (!to_integer(l) || !to_integer(r)) {
			fatal("non-numeric argument\n");
		}

		if (op == MUL) {
			l->u.i *= r->u.i;
		} else {
			if (r->u.i == 0) {
				fatal("division by zero\n");
			}
			if (op == DIV) {
				l->u.i /= r->u.i;
			} else {
				l->u.i %= r->u.i;
			}
		}

		free_value(r);
	}

	return l;
}

/* Parse and evaluate addition and subtraction expressions */
struct val *
eval3()
{
	struct val     *l, *r;
	enum token	op;

	l = eval4();
	while ((op = token) == ADD || op == SUB) {
		nexttoken();
		r = eval4();

		if (!to_integer(l) || !to_integer(r)) {
			fatal("non-numeric argument\n");
		}

		if (op == ADD) {
			l->u.i += r->u.i;
		} else {
			l->u.i -= r->u.i;
		}

		free_value(r);
	}

	return l;
}

/*
 * Evaluate the algebraic expression contained in <expression>. The
 * value will be assigned to <result>. When errors are encountered
 * non-zero is returned (if program is not aborted) otherwise zero. 
 */
int strexpr(int *result, char *expression) {
	struct val     *vp;

	pexpr = expression;

        nexttoken();
	vp = eval3();

	if (token != EOI) return 1;

	if (vp->type == integer || to_integer(vp)) {
		*result = vp->u.i;
		return 0;
	}

        return 1;
}
