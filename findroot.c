#include <stdio.h>
#include <math.h>
#include "findroot.h"

#define EPSILON 1e-6
#define MAX_ATTEMPTS 1000000 // Giới hạn số lần lặp để tránh vòng lặp vô hạn

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
    float initial_guesses[] = {1.0, -1.0, 0.0, 2.0, -2.0}; // Nhiều giá trị khởi tạo
    int num_guesses = sizeof(initial_guesses) / sizeof(initial_guesses[0]);
    
    for (int guess_idx = 0; guess_idx < num_guesses; guess_idx++) {
        float x = initial_guesses[guess_idx];
        int iteration = 0;

        while (iteration < MAX_ATTEMPTS) {
            float fx = evaluatePostfix(postfix, x);
            float dfx = derivative(postfix, x);

            // Kiểm tra giá trị không hợp lệ
            if (isnan(fx) || isnan(dfx) || isinf(fx) || isinf(dfx)) {
                printf("Lỗi: Giá trị không hợp lệ (NaN/Inf) tại x = %.6f với giá trị khởi tạo %.6f\n", x, initial_guesses[guess_idx]);
                break;
            }

            // Kiểm tra giá trị hàm tại x
            if (fabs(fx) < EPSILON) {
                return x; // Tìm được nghiệm
            }

            // Kiểm tra đạo hàm gần 0
            if (fabs(dfx) < EPSILON) {
                printf("Cảnh báo: Đạo hàm quá nhỏ tại x = %.6f với giá trị khởi tạo %.6f\n", x, initial_guesses[guess_idx]);
                break;
            }

            float x1 = x - fx / dfx;

            // Kiểm tra giá trị mới
            if (isnan(x1) || isinf(x1)) {
                printf("Lỗi: Giá trị lặp mới không hợp lệ tại x = %.6f với giá trị khởi tạo %.6f\n", x, initial_guesses[guess_idx]);
                break;
            }

            x = x1;
            iteration++;
        }

        // Nếu vượt quá số lần lặp tối đa
        if (iteration >= MAX_ATTEMPTS) {
            printf("Cảnh báo: Vượt quá số lần lặp tối đa với giá trị khởi tạo %.6f\n", initial_guesses[guess_idx]);
        }
    }

    // Nếu không tìm được nghiệm với bất kỳ giá trị khởi tạo nào
    return NAN;
}
