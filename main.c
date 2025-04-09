#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include "postfix.h"
#include "findroot.h"

// Bỏ #define NUM_THREADS vì không cần thread nữa

int main() {
    struct timespec start, end;
    Token *output = NULL;
    char str[MAX];  // Sử dụng MAX từ postfix.h

    printf("Nhập biểu thức (ví dụ: x^2-4 hoặc (x^2+1)/(x-1)): ");
    if (fgets(str, MAX, stdin) == NULL) {
        printf("Lỗi khi đọc input!\n");
        return 1;
    }
    // Xóa ký tự xuống dòng
    str[strcspn(str, "\n")] = '\0';

    if (strlen(str) == 0) {
        printf("Biểu thức rỗng!\n");
        return 1;
    }

    printf("Biểu thức đã nhập: %s\n", str);

    output = infixToPostfix(str);
    if (output == NULL) {
        printf("Lỗi khi chuyển đổi biểu thức sang postfix!\n");
        return 1;
    }

    printTokens(output);

    clock_gettime(CLOCK_MONOTONIC, &start);

    // Gọi trực tiếp phương pháp Newton-Raphson
    float result = newtonRaphson(output);
    int found = 0;
    float best_result = 0.0;

    clock_gettime(CLOCK_MONOTONIC, &end);

    // Kiểm tra kết quả
    if (!isnan(result) && !isinf(result)) {
        float fx = evaluatePostfix(output, result);
        if (!isnan(fx) && !isinf(fx) && fabs(fx) < 1e-6) {
            best_result = result;
            found = 1;
            printf("Newton-Raphson tìm được nghiệm: %.6f\n", best_result);
        }
    }

    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Thời gian tìm nghiệm: %.6f giây\n", elapsed);

    if (found) {
        float fx = evaluatePostfix(output, best_result);
        printf("Kết quả với nghiệm %.6f là: %.6f\n", best_result, fx);
        if (fabs(fx) > 1e-6) {
            printf("Cảnh báo: Giá trị tại nghiệm không đủ gần 0!\n");
        }
    } else {
        printf("Không tìm được nghiệm hợp lệ!\n");
    }

    free(output);
    return 0;
}
