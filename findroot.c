#include <stdio.h>
#include <math.h>
#include "findroot.h"

#define EPSILON 1e-6
// Bỏ #define MAX_ITER 1000 vì không cần nữa

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

    // Vòng lặp while sẽ không bao giờ đến đây do đã có điều kiện dừng
    return NAN;
}

float bisectionMethod(Token *postfix) {
    float a = -10.0, b = 10.0; // Khoảng ban đầu
    float fa = evaluatePostfix(postfix, a);
    float fb = evaluatePostfix(postfix, b);

    // Kiểm tra điều kiện tồn tại nghiệm trong khoảng
    if (fa * fb >= 0) return NAN;

    while (1) {
        float c = (a + b) / 2;
        float fc = evaluatePostfix(postfix, c);

        // Kiểm tra giá trị không hợp lệ
        if (isnan(fc) || isinf(fc)) {
            return NAN;
        }

        // Điều kiện dừng: giá trị hàm tại c hoặc độ dài khoảng đủ nhỏ
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
    }

    return NAN; // Không bao giờ đến đây
}

float secantMethod(Token *postfix) {
    float x0 = 0.0, x1 = 1.0; // Giá trị khởi tạo ban đầu

    while (1) {
        float f0 = evaluatePostfix(postfix, x0);
        float f1 = evaluatePostfix(postfix, x1);

        // Kiểm tra giá trị không hợp lệ
        if (isnan(f0) || isnan(f1) || isinf(f0) || isinf(f1)) {
            return NAN;
        }

        // Kiểm tra mẫu số
        if (fabs(f1 - f0) < EPSILON) {
            return NAN; // Mẫu số quá nhỏ
        }

        float x2 = x1 - f1 * (x1 - x0) / (f1 - f0);

        // Kiểm tra giá trị không hợp lệ
        if (isnan(x2) || isinf(x2)) {
            return NAN;
        }

        // Kiểm tra điều kiện dừng: sai số giữa 2 lần lặp
        if (fabs(x2 - x1) < EPSILON) {
            float fx2 = evaluatePostfix(postfix, x2);
            if (fabs(fx2) < EPSILON) {
                return x2;
            }
            return NAN;
        }

        // Kiểm tra giá trị hàm tại x2
        float fx2 = evaluatePostfix(postfix, x2);
        if (fabs(fx2) < EPSILON) {
            return x2;
        }

        x0 = x1;
        x1 = x2;
    }

    return NAN; // Không bao giờ đến đây
}
