#ifndef POSTFIX_H
#define POSTFIX_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAX 100

typedef enum {S_START, S_OPERAND, S_OPERATOR, S_OPEN, S_CLOSE, S_ERROR, S_END} state_t;

typedef enum {
    OPERAND,
    OPERATOR,
    VARIABLE
} TokenType;

typedef struct {
    TokenType type;
    union {
        long double operand;  // Sử dụng long double
        char operator;
        long double variable;
    } value;
} Token;

int isOperator(char c);
int precedence(char op);
Token *infixToPostfix(char* myFunction);
long double evaluatePostfix(Token *postfix, long double x_value);  // Sử dụng long double
void printTokens(Token *output);

#endif // POSTFIX_H
