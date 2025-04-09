#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "findroot.h"

#define EPSILON 1e-10  // Độ chính xác yêu cầu

// Biến toàn cục từ main.c
extern int stop_all;

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

// Phương pháp Newton-Raphson với nhiều giá trị khởi tạo
long double newtonRaphson(Token *postfix) {
    long double initial_guesses[] = {1.0, -1.0, 0.0, 2.0, -2.0, 5.0, -5.0, 10.0, -10.0, 100.0, -100.0, 1000.0, -1000.0};
    int num_guesses = sizeof(initial_guesses) / sizeof(initial_guesses[0]);
    
    for (int guess_idx = 0; guess_idx < num_guesses; guess_idx++) {
        if (stop_all) return NAN;  // Dừng nếu đã tìm được nghiệm
        long double x = initial_guesses[guess_idx];
        printf("Newton-Raphson: Thử giá trị khởi tạo x = %.10Lf\n", x);

        while (1) {  // Lặp vô hạn cho đến khi tìm được nghiệm
            if (stop_all) return NAN;  // Dừng nếu đã tìm được nghiệm
            long double fx = evaluatePostfix(postfix, x);
            long double dfx = derivative(postfix, x);

            if (isnan(fx) || isnan(dfx) || isinf(fx) || isinf(dfx)) {
                printf("Newton-Raphson: Giá trị không hợp lệ tại x = %.10Lf (giá trị khởi tạo %.10Lf)\n", x, initial_guesses[guess_idx]);
                break;
            }

            if (fabsl(fx) < EPSILON) {
                return x;  // Tìm được nghiệm
            }

            if (fabsl(dfx) < EPSILON) {
                printf("Newton-Raphson: Đạo hàm quá nhỏ tại x = %.10Lf (giá trị khởi tạo %.10Lf)\n", x, initial_guesses[guess_idx]);
                break;
            }

            long double x1 = x - fx / dfx;

            if (isnan(x1) || isinf(x1)) {
                printf("Newton-Raphson: Giá trị lặp mới không hợp lệ tại x = %.10Lf (giá trị khởi tạo %.10Lf)\n", x, initial_guesses[guess_idx]);
                break;
            }

            x = x1;
        }
    }

    // Nếu không tìm được nghiệm với các giá trị khởi tạo cố định, thử ngẫu nhiên
    initRandom();
    while (1) {
        if (stop_all) return NAN;  // Dừng nếu đã tìm được nghiệm
        long double x = (long double)(rand() % 20001 - 10000);  // Giá trị ngẫu nhiên từ -10000 đến 10000
        printf("Newton-Raphson: Thử giá trị khởi tạo ngẫu nhiên x = %.10Lf\n", x);

        while (1) {
            if (stop_all) return NAN;  // Dừng nếu đã tìm được nghiệm
            long double fx = evaluatePostfix(postfix, x);
            long double dfx = derivative(postfix, x);

            if (isnan(fx) || isnan(dfx) || isinf(fx) || isinf(dfx)) {
                printf("Newton-Raphson: Giá trị không hợp lệ tại x = %.10Lf (giá trị khởi tạo ngẫu nhiên)\n", x);
                break;
            }

            if (fabsl(fx) < EPSILON) {
                return x;
            }

            if (fabsl(dfx) < EPSILON) {
                printf("Newton-Raphson: Đạo hàm quá nhỏ tại x = %.10Lf (giá trị khởi tạo ngẫu nhiên)\n", x);
                break;
            }

            long double x1 = x - fx / dfx;

            if (isnan(x1) || isinf(x1)) {
                printf("Newton-Raphson: Giá trị lặp mới không hợp lệ tại x = %.10Lf (giá trị khởi tạo ngẫu nhiên)\n", x);
                break;
            }

            x = x1;
        }
    }

    return NAN;  // Không bao giờ đến đây do vòng lặp vô hạn
}

// Phương pháp chia đôi với khoảng ngẫu nhiên
long double bisectionMethod(Token *postfix) {
    initRandom();
    while (1) {  // Lặp vô hạn cho đến khi tìm được nghiệm
        if (stop_all) return NAN;  // Dừng nếu đã tìm được nghiệm
        // Tạo khoảng ngẫu nhiên
        long double a = (long double)(rand() % 20001 - 10000);  // Từ -10000 đến 10000
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
            if (stop_all) return NAN;  // Dừng nếu đã tìm được nghiệm
            long double c = (a + b) / 2;
            long double fc = evaluatePostfix(postfix, c);

            if (isnan(fc) || isinf(fc)) {
                printf("Bisection: Giá trị không hợp lệ tại x = %.10Lf trong khoảng [%.10Lf, %.10Lf]\n", c, a, b);
                break;
            }

            if (fabsl(fc) < EPSILON || fabsl(b - a) < EPSILON) {
                return c;  // Tìm được nghiệm
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

    return NAN;  // Không bao giờ đến đây do vòng lặp vô hạn
}

// Phương pháp dây cung với nhiều cặp giá trị khởi tạo
long double secantMethod(Token *postfix) {
    long double initial_pairs[][2] = {{0.0, 1.0}, {-1.0, 1.0}, {1.0, 2.0}, {-2.0, -1.0}, {5.0, 6.0}, {-5.0, -4.0}, {10.0, 11.0}, {-10.0, -9.0}, {100.0, 101.0}, {-100.0, -99.0}};
    int num_pairs = sizeof(initial_pairs) / sizeof(initial_pairs[0]);

    for (int pair_idx = 0; pair_idx < num_pairs; pair_idx++) {
        if (stop_all) return NAN;  // Dừng nếu đã tìm được nghiệm
        long double x0 = initial_pairs[pair_idx][0];
        long double x1 = initial_pairs[pair_idx][1];
        printf("Secant: Thử cặp khởi tạo (x0 = %.10Lf, x1 = %.10Lf)\n", x0, x1);

        while (1) {
            if (stop_all) return NAN;  // Dừng nếu đã tìm được nghiệm
            long double f0 = evaluatePostfix(postfix, x0);
            long double f1 = evaluatePostfix(postfix, x1);

            if (isnan(f0) || isnan(f1) || isinf(f0) || isinf(f1)) {
                printf("Secant: Giá trị không hợp lệ tại x0 = %.10Lf, x1 = %.10Lf\n", x0, x1);
                break;
            }

            if (fabsl(f1) < EPSILON) {
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

    // Nếu không tìm được nghiệm với các cặp khởi tạo cố định, thử ngẫu nhiên
    initRandom();
    while (1) {
        if (stop_all) return NAN;  // Dừng nếu đã tìm được nghiệm
        long double x0 = (long double)(rand() % 20001 - 10000);
        long double x1 = (long double)(rand() % 20001 - 10000);
        printf("Secant: Thử cặp khởi tạo ngẫu nhiên (x0 = %.10Lf, x1 = %.10Lf)\n", x0, x1);

        while (1) {
            if (stop_all) return NAN;  // Dừng nếu đã tìm được nghiệm
            long double f0 = evaluatePostfix(postfix, x0);
            long double f1 = evaluatePostfix(postfix, x1);

            if (isnan(f0) || isnan(f1) || isinf(f0) || isinf(f1)) {
                printf("Secant: Giá trị không hợp lệ tại x0 = %.10Lf, x1 = %.10Lf\n", x0, x1);
                break;
            }

            if (fabsl(f1) < EPSILON) {
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

    return NAN;  // Không bao giờ đến đây do vòng lặp vô hạn
}
