#include <stdio.h>
#include <math.h>
#include "findroot.h"

#define EPSILON 1e-6
#define MAX_ATTEMPTS 10000

float derivative(Token *postfix, float x) {
    float h = 1e-4;
    float fx_plus_h = evaluatePostfix(postfix, x + h);
    float fx_minus_h = evaluatePostfix(postfix, x - h);
    
    if (isnan(fx_plus_h) || isnan(fx_minus_h) || isinf(fx_plus_h) || isinf(fx_minus_h)) {
        return NAN;
    }
    return (fx_plus_h - fx_minus_h) / (2 * h);
}

float newtonRaphson(Token *postfix) {
    float initial_guesses[] = {1.0, -1.0, 0.0, 2.0, -2.0, 5.0, -5.0, 10.0, -10.0};
    int num_guesses = sizeof(initial_guesses) / sizeof(initial_guesses[0]);
    
    for (int guess_idx = 0; guess_idx < num_guesses; guess_idx++) {
        float x = initial_guesses[guess_idx];
        int iteration = 0;

        while (iteration < MAX_ATTEMPTS) {
            float fx = evaluatePostfix(postfix, x);
            float dfx = derivative(postfix, x);

            if (isnan(fx) || isnan(dfx) || isinf(fx) || isinf(dfx)) {
                printf("Newton-Raphson: Giá trị không hợp lệ tại x = %.6f (giá trị khởi tạo %.6f)\n", x, initial_guesses[guess_idx]);
                break;
            }

            if (fabs(fx) < EPSILON) {
                return x;
            }

            if (fabs(dfx) < EPSILON) {
                printf("Newton-Raphson: Đạo hàm quá nhỏ tại x = %.6f (giá trị khởi tạo %.6f)\n", x, initial_guesses[guess_idx]);
                break;
            }

            float x1 = x - fx / dfx;

            if (isnan(x1) || isinf(x1)) {
                printf("Newton-Raphson: Giá trị lặp mới không hợp lệ tại x = %.6f (giá trị khởi tạo %.6f)\n", x, initial_guesses[guess_idx]);
                break;
            }

            x = x1;
            iteration++;
        }

        if (iteration >= MAX_ATTEMPTS) {
            printf("Newton-Raphson: Vượt quá số lần lặp tối đa với giá trị khởi tạo %.6f\n", initial_guesses[guess_idx]);
        }
    }

    return NAN;
}

float bisectionMethod(Token *postfix) {
    float intervals[][2] = {{-1000.0, 1000.0}, {-100.0, 100.0}, {-10.0, 10.0}, {-1.0, 1.0}};
    int num_intervals = sizeof(intervals) / sizeof(intervals[0]);

    for (int interval_idx = 0; interval_idx < num_intervals; interval_idx++) {
        float a = intervals[interval_idx][0];
        float b = intervals[interval_idx][1];
        float fa = evaluatePostfix(postfix, a);
        float fb = evaluatePostfix(postfix, b);

        if (isnan(fa) || isnan(fb) || isinf(fa) || isinf(fb)) {
            printf("Bisection: Giá trị không hợp lệ tại khoảng [%.6f, %.6f]\n", a, b);
            continue;
        }

        if (fa * fb >= 0) {
            printf("Bisection: Không có nghiệm trong khoảng [%.6f, %.6f]\n", a, b);
            continue;
        }

        int iteration = 0;
        while (iteration < MAX_ATTEMPTS) {
            float c = (a + b) / 2;
            float fc = evaluatePostfix(postfix, c);

            if (isnan(fc) || isinf(fc)) {
                printf("Bisection: Giá trị không hợp lệ tại x = %.6f trong khoảng [%.6f, %.6f]\n", c, a, b);
                break;
            }

            if (fabs(fc) < EPSILON || fabs(b - a) < EPSILON) {
                return c;
            }

            if (fa * fc < 0) {
                b = c;
                fb = fc;
            } else {
                a = c;
                fa = fc;
            }
            iteration++;
        }

        if (iteration >= MAX_ATTEMPTS) {
            printf("Bisection: Vượt quá số lần lặp tối đa trong khoảng [%.6f, %.6f]\n", a, b);
        }
    }

    return NAN;
}

float secantMethod(Token *postfix) {
    float initial_pairs[][2] = {{0.0, 1.0}, {-1.0, 1.0}, {1.0, 2.0}, {-2.0, -1.0}, {5.0, 6.0}, {-5.0, -4.0}};
    int num_pairs = sizeof(initial_pairs) / sizeof(initial_pairs[0]);

    for (int pair_idx = 0; pair_idx < num_pairs; pair_idx++) {
        float x0 = initial_pairs[pair_idx][0];
        float x1 = initial_pairs[pair_idx][1];
        int iteration = 0;

        while (iteration < MAX_ATTEMPTS) {
            float f0 = evaluatePostfix(postfix, x0);
            float f1 = evaluatePostfix(postfix, x1);

            if (isnan(f0) || isnan(f1) || isinf(f0) || isinf(f1)) {
                printf("Secant: Giá trị không hợp lệ tại x0 = %.6f, x1 = %.6f\n", x0, x1);
                break;
            }

            if (fabs(f1) < EPSILON) {
                return x1;
            }

            if (fabs(f1 - f0) < EPSILON) {
                printf("Secant: Mẫu số quá nhỏ tại x0 = %.6f, x1 = %.6f\n", x0, x1);
                break;
            }

            float x2 = x1 - f1 * (x1 - x0) / (f1 - f0);

            if (isnan(x2) || isinf(x2)) {
                printf("Secant: Giá trị lặp mới không hợp lệ tại x0 = %.6f, x1 = %.6f\n", x0, x1);
                break;
            }

            x0 = x1;
            x1 = x2;
            iteration++;
        }

        if (iteration >= MAX_ATTEMPTS) {
            printf("Secant: Vượt quá số lần lặp tối đa với cặp khởi tạo (%.6f, %.6f)\n", initial_pairs[pair_idx][0], initial_pairs[pair_idx][1]);
        }
    }

    return NAN;
}
