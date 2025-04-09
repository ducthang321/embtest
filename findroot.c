#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "findroot.h"

#define EPSILON 1e-10  // Độ chính xác yêu cầu

// Khởi tạo seed cho hàm random
void initRandom() {
    static int initialized = 0;
    if (!initialized) {
        srand(time(NULL));
        initialized = 1;
    }
}

// Tính đạo hàm bằng sai phân trung tâm với bước nhỏ hơn
long double derivative(Token *postfix, long double x) {
    long double h = 1e-6;  // Bước nhỏ để tăng độ chính xác
    long double fx_plus_h = evaluatePostfix(postfix, x + h);
    long double fx_minus_h = evaluatePostfix(postfix, x - h);

    if (isnan(fx_plus_h) || isnan(fx_minus_h) || isinf(fx_plus_h) || isinf(fx_minus_h)) {
        return NAN;
    }
    return (fx_plus_h - fx_minus_h) / (2 * h);
}

// Thuật toán mô phỏng tìm nghiệm kiểu "shift solve" của Casio
long double newtonRaphson(Token *postfix) {
    long double x = 1.0;  // Giá trị khởi tạo
    printf("Newton-Raphson (Casio-style): Bắt đầu với giá trị khởi tạo x = %.10Lf\n", x);

    while (1) {
        pthread_testcancel();
        long double fx = evaluatePostfix(postfix, x);
        long double dfx = derivative(postfix, x);

        if (isnan(fx) || isnan(dfx) || isinf(fx) || isinf(dfx)) {
            printf("Newton-Raphson: Giá trị không hợp lệ tại x = %.10Lf\n", x);
            break;
        }

        if (fabsl(fx) < EPSILON) {
            printf("Newton-Raphson: Tìm được nghiệm x = %.10Lf (f(x) = %.10Lf)\n", x, fx);
            return x;
        }

        long double delta;
        if (fabsl(dfx) < EPSILON) {
            delta = 1e-4;  // Dời x nhẹ nếu đạo hàm quá nhỏ
        } else {
            delta = fx / dfx;
        }

        long double x1 = x - delta;

        if (isnan(x1) || isinf(x1)) {
            printf("Newton-Raphson: Giá trị lặp mới không hợp lệ tại x = %.10Lf\n", x);
            break;
        }

        if (fabsl(x1 - x) < EPSILON) {
            printf("Newton-Raphson: Tìm được nghiệm x = %.10Lf (f(x) = %.10Lf)\n", x1, evaluatePostfix(postfix, x1));
            return x1;
        }

        x = x1;
    }

    return NAN;
}

// Phương pháp chia đôi với khoảng ngẫu nhiên
long double bisectionMethod(Token *postfix) {
    initRandom();
    while (1) {
        pthread_testcancel();
        long double a = (long double)(rand() % 20001 - 10000);
        long double b = (long double)(rand() % 20001 - 10000);
        if (a > b) {
            long double temp = a;
            a = b;
            b = temp;
        }
        printf("Bisection: Thử khoảng ngẫu nhiên [%.10Lf, %.10Lf]\n", a, b);

        long double fa = evaluatePostfix(postfix, a);
        long double fb = evaluatePostfix(postfix, b);

        if (isnan(fa) || isnan(fb) || isinf(fa) || isinf(fb)) {
            printf("Bisection: Giá trị không hợp lệ tại khoảng [%.10Lf, %.10Lf]\n", a, b);
            continue;
        }

        if (fa * fb >= 0) {
            printf("Bisection: Không có nghiệm trong khoảng [%.10Lf, %.10Lf]\n", a, b);
            continue;
        }

        while (1) {
            pthread_testcancel();
            long double c = (a + b) / 2;
            long double fc = evaluatePostfix(postfix, c);

            if (isnan(fc) || isinf(fc)) {
                printf("Bisection: Giá trị không hợp lệ tại x = %.10Lf trong khoảng [%.10Lf, %.10Lf]\n", c, a, b);
                break;
            }

            if (fabsl(fc) < EPSILON || fabsl(b - a) < EPSILON) {
                printf("Bisection: Tìm được nghiệm x = %.10Lf (f(x) = %.10Lf)\n", c, fc);
                return c;
            }

            if (fa * fc < 0) {
                b = c;
                fb = fc;
            } else {
                a = c;
                fa = fc;
            }
        }
    }
    return NAN;
}

// Phương pháp dây cung
long double secantMethod(Token *postfix) {
    long double initial_pairs[][2] = {{0.0, 1.0}, {-1.0, 1.0}, {1.0, 2.0}, {-2.0, -1.0}};
    int num_pairs = sizeof(initial_pairs) / sizeof(initial_pairs[0]);

    for (int i = 0; i < num_pairs; i++) {
        pthread_testcancel();
        long double x0 = initial_pairs[i][0];
        long double x1 = initial_pairs[i][1];
        printf("Secant: Thử cặp (x0 = %.10Lf, x1 = %.10Lf)\n", x0, x1);

        while (1) {
            pthread_testcancel();
            long double f0 = evaluatePostfix(postfix, x0);
            long double f1 = evaluatePostfix(postfix, x1);

            if (isnan(f0) || isnan(f1) || isinf(f0) || isinf(f1)) {
                printf("Secant: Giá trị không hợp lệ tại x0 = %.10Lf, x1 = %.10Lf\n", x0, x1);
                break;
            }

            if (fabsl(f1) < EPSILON) {
                printf("Secant: Tìm được nghiệm x = %.10Lf (f(x) = %.10Lf)\n", x1, f1);
                return x1;
            }

            if (fabsl(f1 - f0) < EPSILON) {
                printf("Secant: Mẫu số quá nhỏ tại x0 = %.10Lf, x1 = %.10Lf\n", x0, x1);
                break;
            }

            long double x2 = x1 - f1 * (x1 - x0) / (f1 - f0);

            if (isnan(x2) || isinf(x2)) {
                printf("Secant: Giá trị lặp mới không hợp lệ tại x0 = %.10Lf, x1 = %.10Lf\n", x0, x1);
                break;
            }

            x0 = x1;
            x1 = x2;
        }
    }

    return NAN;
}
