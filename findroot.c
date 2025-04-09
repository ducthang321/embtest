#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "findroot.h"

#define EPSILON 1e-10  // Độ chính xác yêu cầu, giống Casio

// Khởi tạo seed cho hàm random
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

// Phương pháp dây cung (cải thiện dựa trên code cũ)
long double secantMethod(Token *postfix) {
    // Mở rộng tập hợp cặp khởi tạo cố định, bao gồm các vùng gần nghiệm đã biết
    long double initial_pairs[][2] = {
        {-1.0, -0.8}, {-0.9, -0.7}, {-0.1, 0.0}, {0.0, 0.1}, {0.9, 1.0}, // Gần x = 0 và x = -0.9
        {-2.0, -1.0}, {1.0, 2.0}, {-5.0, -4.0}, {4.0, 5.0}, // Các khoảng trung bình
        {-10.0, -9.0}, {9.0, 10.0}, {-100.0, -99.0}, {99.0, 100.0} // Các khoảng lớn
    };
    int num_pairs = sizeof(initial_pairs) / sizeof(initial_pairs[0]);
    int max_iter = 1000; // Giới hạn số lần lặp
    int max_stagnation = 10; // Số lần lặp tối đa mà giá trị không cải thiện

    // Phần 1: Thử các cặp khởi tạo cố định
    for (int pair_idx = 0; pair_idx < num_pairs; pair_idx++) {
        pthread_testcancel();
        long double x0 = initial_pairs[pair_idx][0];
        long double x1 = initial_pairs[pair_idx][1];
        printf("Secant: Thử cặp khởi tạo (x0 = %.10Lf, x1 = %.10Lf)\n", x0, x1);

        int iter = 0;
        int stagnation_count = 0;
        long double last_x1 = x1;

        while (iter < max_iter) {
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

            // Kiểm tra stagnation
            if (fabsl(x1 - last_x1) < EPSILON) {
                stagnation_count++;
                if (stagnation_count >= max_stagnation) {
                    printf("Secant: Giá trị lặp không cải thiện tại x0 = %.10Lf, x1 = %.10Lf\n", x0, x1);
                    break;
                }
            } else {
                stagnation_count = 0;
            }
            last_x1 = x1;

            if (fabsl(f1 - f0) < EPSILON) {
                printf("Secant: Mẫu số quá nhỏ tại x0 = %.10Lf, x1 = %.10Lf\n", x0, x1);
                break;
            }

            long double x2 = x1 - f1 * (x1 - x0) / (f1 - f0);
            if (isnan(x2) || isinf(x2)) {
                printf("Secant: Giá trị lặp mới không hợp lệ tại x0 = %.10Lf, x1 = %.10Lf\n", x0, x1);
                break;
            }

            // Giới hạn x2 trong khoảng [x0, x1] để tránh dao động
            long double min_x = x0 < x1 ? x0 : x1;
            long double max_x = x0 < x1 ? x1 : x0;
            if (x2 < min_x || x2 > max_x) {
                printf("Secant: Giá trị lặp x2 = %.10Lf nằm ngoài khoảng [%.10Lf, %.10Lf], điều chỉnh\n", x2, min_x, max_x);
                x2 = (min_x + max_x) / 2; // Lấy điểm giữa
            }

            x0 = x1;
            x1 = x2;
            iter++;
            printf("Secant: Lặp %d, x = %.10Lf\n", iter, x1);
        }
    }

    // Phần 2: Thử cặp ngẫu nhiên nếu các cặp cố định không tìm được nghiệm
    printf("Secant: Không tìm được nghiệm với các cặp cố định, thử cặp ngẫu nhiên\n");
    initRandom();
    int max_attempts = 10; // Số lần thử ngẫu nhiên tối đa
    for (int attempt = 0; attempt < max_attempts; attempt++) {
        pthread_testcancel();
        // Chọn ngẫu nhiên trong khoảng nhỏ hơn để tránh giá trị lớn
        long double x0 = (long double)(rand() % 201 - 100); // [-100, 100]
        long double x1 = (long double)(rand() % 201 - 100);
        printf("Secant: Thử cặp khởi tạo ngẫu nhiên (x0 = %.10Lf, x1 = %.10Lf)\n", x0, x1);

        int iter = 0;
        int stagnation_count = 0;
        long double last_x1 = x1;

        while (iter < max_iter) {
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

            // Kiểm tra stagnation
            if (fabsl(x1 - last_x1) < EPSILON) {
                stagnation_count++;
                if (stagnation_count >= max_stagnation) {
                    printf("Secant: Giá trị lặp không cải thiện tại x0 = %.10Lf, x1 = %.10Lf\n", x0, x1);
                    break;
                }
            } else {
                stagnation_count = 0;
            }
            last_x1 = x1;

            if (fabsl(f1 - f0) < EPSILON) {
                printf("Secant: Mẫu số quá nhỏ tại x0 = %.10Lf, x1 = %.10Lf\n", x0, x1);
                break;
            }

            long double x2 = x1 - f1 * (x1 - x0) / (f1 - f0);
            if (isnan(x2) || isinf(x2)) {
                printf("Secant: Giá trị lặp mới không hợp lệ tại x0 = %.10Lf, x1 = %.10Lf\n", x0, x1);
                break;
            }

            // Giới hạn x2 trong khoảng [x0, x1]
            long double min_x = x0 < x1 ? x0 : x1;
            long double max_x = x0 < x1 ? x1 : x0;
            if (x2 < min_x || x2 > max_x) {
                printf("Secant: Giá trị lặp x2 = %.10Lf nằm ngoài khoảng [%.10Lf, %.10Lf], điều chỉnh\n", x2, min_x, max_x);
                x2 = (min_x + max_x) / 2;
            }

            x0 = x1;
            x1 = x2;
            iter++;
            printf("Secant: Lặp %d, x = %.10Lf\n", iter, x1);
        }
    }

    printf("Secant: Không tìm được nghiệm với các cặp khởi tạo đã thử\n");
    return NAN;
}
