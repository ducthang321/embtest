#include <stdio.h>
#include <math.h>
#include "findroot.h"

#define EPSILON 1e-6

float derivative(Token *postfix, float x) {
    float h = 1e-4;
    float fx_plus_h = evaluatePostfix(postfix, x + h);
    float fx_minus_h = evaluatePostfix(postfix, x - h);
    
    // Kiểm tra lỗi trong tính toán đạo hàm
    if (isnan(fx_plus_h) || isnan(fx_minus_h) || isinf(fx_plus_h) || isinf(fx_minus_h)) {
        return NAN;
    }
    return (fx_plus_h - fx_minus_h) / (2 * h);
}

float newtonRaphson(Token *postfix) {
    float x = 1.0; // Giá trị khởi tạo ban đầu
    float fx, dfx, x1;
    
    while (1) {
        fx = evaluatePostfix(postfix, x);
        dfx = derivative(postfix, x);

        // Kiểm tra giá trị không hợp lệ
        if (isnan(fx) || isnan(dfx) || isinf(fx) || isinf(dfx)) {
            return NAN;
        }

        // Kiểm tra đạo hàm gần 0 (tránh chia cho 0)
        if (fabs(dfx) < EPSILON) {
            return NAN; // Đạo hàm quá nhỏ, phương pháp không hội tụ
        }

        x1 = x - fx / dfx;

        // Kiểm tra điều kiện dừng: sai số giữa 2 lần lặp liên tiếp
        if (fabs(x1 - x) < EPSILON) {
            // Kiểm tra xem giá trị hàm tại x1 có đủ nhỏ không
            float fx1 = evaluatePostfix(postfix, x1);
            if (fabs(fx1) < EPSILON) {
                return x1; // Nghiệm hợp lệ
            }
            return NAN; // Sai số lặp nhỏ nhưng không phải nghiệm
        }

        // Kiểm tra giá trị hàm tại x1 để dừng sớm
        float fx1 = evaluatePostfix(postfix, x1);
        if (fabs(fx1) < EPSILON) {
            return x1; // Đã tìm được nghiệm
        }

        x = x1; // Cập nhật x cho lần lặp tiếp theo
    }

    return NAN; // Không bao giờ đến đây
}
