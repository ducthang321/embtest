#ifndef FINDROOT_H
#define FINDROOT_H

#include "postfix.h"

// Khai báo các hàm trả về long double
long double newtonRaphson(Token *postfix);
long double bisectionMethod(Token *postfix);
long double secantMethod(Token *postfix);

#endif
