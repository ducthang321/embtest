#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "findroot.h"

#define EPSILON 1e-10  // Độ chính xác yêu cầu, giống Casio

// Khởi tạo seed cho hàm random (giữ nguyên để tương thích, dù không dùng trong Newton)
void initRandom() {
    static int initialized = 0;
    if (!initialized) {
        srand(time(NULL));
        initialized = 1;
    }
}

// Tính đạo hàm bằng sai phân trung tâm, giống cách Casio xấp xỉ
long double derivative(Token *postfix, long double x) {
    long double h = 1e-6;  // Bước nhỏ, tương tự Casio
    long double fx_plus_h = evaluatePostfix(postfix, x + h);
    long double fx_minus_h = evaluatePostfix(postfix, x - h);
    
    if (isnan(fx_plus_h) || isnan(fx_minus_h) || isinf(fx_plus_h) || isinf(fx_minus_h)) {
        return NAN;
    }
    return (fx_plus_h - fx_minus_h) / (2 * h);
}

// Phương pháp Newton-Raphson mô phỏng Casio fx-580
long double newtonRaphson(Token *postfix) {
    long double x = 1.0;  // Giá trị khởi tạo cố định như Casio
    printf("Newton-Raphson: Bắt đầu với giá trị khởi tạo x = %.10Lf\n", x);

    while (1) {  // Lặp vô hạn như Casio cho đến khi tìm nghiệm
        long double fx = evaluatePostfix(postfix, x);
        long double dfx = derivative(postfix, x);

        // Kiểm tra lỗi nghiêm trọng
        if (isnan(fx) || isnan(dfx) || isinf(fx) || isinf(dfx)) {
            printf("Newton-Raphson: Giá trị không hợp lệ tại x = %.10Lf, thử lại với x mới\n", x);
            x += 0.1;  // Tăng x một chút để thử lại
            continue;  // Tiếp tục lặp
        }

        // Tìm được nghiệm
        if (fabsl(fx) < EPSILON) {
            printf("Newton-Raphson: Tìm được nghiệm x = %.10Lf (f(x) = %.10Lf)\n", x, fx);
            return x;  // Dừng khi f(x) đủ nhỏ
        }

        // Xử lý đạo hàm nhỏ (Casio thường nhảy một bước nhỏ)
        if (fabsl(dfx) < EPSILON) {
            printf("Newton-Raphson: Đạo hàm quá nhỏ tại x = %.10Lf, nhảy bước nhỏ\n", x);
            x += 1e-6;  // Nhảy bước nhỏ giống Casio khi đạo hàm gần 0
            continue;
        }

        // Cập nhật x mới theo công thức Newton
        long double x1 = x - fx / dfx;

        // Kiểm tra giá trị mới
        if (isnan(x1) || isinf(x1)) {
            printf("Newton-Raphson: Giá trị lặp mới không hợp lệ tại x = %.10Lf, thử lại với x mới\n", x);
            x += 0.1;  // Tăng x một chút để thử lại
            continue;
        }

        x = x1;  // Cập nhật x và tiếp tục
        printf("Newton-Raphson: Lặp mới, x = %.10Lf\n", x);
    }

    return NAN;  // Không bao giờ đến đây do vòng lặp vô hạn
}

// Phương pháp chia đôi
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
    // Tập hợp các điểm để quét và chọn cặp khởi tạo
    long double points[] = {-100.0, -50.0, -10.0, -5.0, -1.0, -0.9, -0.5, -0.1, 0.0, 0.1, 0.5, 0.9, 1.0, 5.0, 10.0, 50.0, 100.0};
    int num_points = sizeof(points) / sizeof(points[0]);
    int max_iter = 1000; // Giới hạn số lần lặp cho mỗi cặp

    // Quét qua các điểm để tìm cặp khởi tạo tốt
    for (int i = 0; i < num_points - 1; i++) {
        for (int j = i + 1; j < num_points; j++) {
            long double x0 = points[i];
            long double x1 = points[j];
            long double f0 = evaluatePostfix(postfix, x0);
            long double f1 = evaluatePostfix(postfix, x1);

            // Bỏ qua nếu giá trị không hợp lệ
            if (isnan(f0) || isnan(f1) || isinf(f0) || isinf(f1)) {
                printf("Secant: Giá trị không hợp lệ tại x0 = %.10Lf, x1 = %.10Lf\n", x0, x1);
                continue;
            }

            // Ưu tiên cặp có dấu khác nhau hoặc giá trị nhỏ
            if (f0 * f1 < 0 || fabsl(f0) < 1e-5 || fabsl(f1) < 1e-5) {
                printf("Secant: Thử cặp khởi tạo (x0 = %.10Lf, x1 = %.10Lf)\n", x0, x1);
                int iter = 0;

                while (iter < max_iter) {
                    pthread_testcancel(); // Giữ điểm hủy để dừng luồng nếu cần

                    f0 = evaluatePostfix(postfix, x0);
                    f1 = evaluatePostfix(postfix, x1);

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
                    iter++;
                    printf("Secant: Lặp %d, x = %.10Lf\n", iter, x1);
                }
            }
        }
    }

    printf("Secant: Không tìm được nghiệm với các cặp khởi tạo đã thử\n");
    return NAN;
}

    initRandom();
    while (1) {
        pthread_testcancel();
        long double x0 = (long double)(rand() % 20001 - 10000);
        long double x1 = (long double)(rand() % 20001 - 10000);
        printf("Secant: Thử cặp khởi tạo ngẫu nhiên (x0 = %.10Lf, x1 = %.10Lf)\n", x0, x1);

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
