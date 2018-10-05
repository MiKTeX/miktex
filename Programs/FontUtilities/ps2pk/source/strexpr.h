/*
 * File:    strexpr.h
 * Purpose: calculate value from algebraic string expression
 * Version: 1.0 (Nov 1995)
 * Author:  Piet Tutelaers
 */

/*
 * Evaluate the algebraic expression contained in <expression>. The
 * value will be assigned to <result>. When errors are encountered
 * non-zero (if program is not aborted) is returned otherwise zero. 
 */
int strexpr(int *result, char *expression);
