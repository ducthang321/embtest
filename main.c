#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include "postfix.h"
#include "findroot.h"

#define NUM_THREADS 3

typedef struct {
    Token *postfix;
    double result;  // Sử dụng double thay vì float để tăng độ chính xác
    int valid;
    char method_name[20]; // Tên phương pháp để in thông báo
} ThreadData;

void *findrootNewton(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    strcpy(data->method_name, "Newton-Raphson");
    double result = newtonRaphson(data->postfix);
    data->result = result;
    data->valid = !isnan(result) && !isinf(result);
    pthread_exit(NULL);
}

void *findrootBisection(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    strcpy(data->method_name, "Bisection");
    double result = bisectionMethod(data->postfix);
    data->result = result;
    data->valid = !isnan(result) && !isinf(result);
    pthread_exit(NULL);
}

void *findrootSecant(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    strcpy(data->method_name, "Secant");
    double result = secantMethod(data->postfix);
    data->result = result;
    data->valid = !isnan(result) && !isinf(result);
    pthread_exit(NULL);
}

int main() {
    struct timespec start, end;
    Token *output = NULL;
    char str[MAX];

    printf("Nhập biểu thức (ví dụ: x^2-4 hoặc (x^2+1)/(x-1)): ");
    if (fgets(str, MAX, stdin) == NULL) {
        printf("Lỗi khi đọc input!\n");
        return 1;
    }
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

    pthread_t threads[NUM_THREADS];
    ThreadData threadData[NUM_THREADS] = {0};

    clock_gettime(CLOCK_MONOTONIC, &start);

    // Khởi tạo các thread
    for (int i = 0; i < NUM_THREADS; i++) {
        threadData[i].postfix = output;
        switch(i) {
            case 0:
                if (pthread_create(&threads[i], NULL, findrootNewton, &threadData[i]) != 0) {
                    printf("Lỗi tạo thread Newton!\n");
                    free(output);
                    return 1;
                }
                break;
            case 1:
                if (pthread_create(&threads[i], NULL, findrootBisection, &threadData[i]) != 0) {
                    printf("Lỗi tạo thread Bisection!\n");
                    free(output);
                    return 1;
                }
                break;
            case 2:
                if (pthread_create(&threads[i], NULL, findrootSecant, &threadData[i]) != 0) {
                    printf("Lỗi tạo thread Secant!\n");
                    free(output);
                    return 1;
                }
                break;
        }
    }

    // Chờ tất cả thread hoàn thành
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            printf("Lỗi khi chờ thread %d!\n", i);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    // Kiểm tra kết quả
    int found = 0;
    double best_result = 0.0;  // Sử dụng double thay vì float
    for (int i = 0; i < NUM_THREADS; i++) {
        if (threadData[i].valid) {
            double fx = evaluatePostfix(output, threadData[i].result);  // Sử dụng double
            if (isnan(fx) || isinf(fx)) {
                printf("%s: Giá trị hàm tại x = %.10f không hợp lệ (NaN/Inf)\n", threadData[i].method_name, threadData[i].result);
                continue;
            }
            if (fabs(fx) < 1e-10) {  // Điều chỉnh EPSILON từ 1e-6 thành 1e-10
                best_result = threadData[i].result;
                found = 1;
                printf("%s tìm được nghiệm: %.10f\n", threadData[i].method_name, best_result);
                break;
            } else {
                printf("%s: Giá trị hàm tại x = %.10f là %.10f (không phải nghiệm)\n", threadData[i].method_name, threadData[i].result, fx);
            }
        } else {
            printf("%s: Không hội tụ (kết quả không hợp lệ)\n", threadData[i].method_name);
        }
    }

    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Thời gian tìm nghiệm: %.6f giây\n", elapsed);

    if (found) {
        double fx = evaluatePostfix(output, best_result);  // Sử dụng double
        printf("Kết quả với nghiệm %.10f là: %.10f\n", best_result, fx);
        if (fabs(fx) > 1e-10) {  // Điều chỉnh EPSILON từ 1e-6 thành 1e-10
            printf("Cảnh báo: Giá trị tại nghiệm không đủ gần 0!\n");
        }
    } else {
        printf("Không tìm được nghiệm hợp lệ!\n");
    }

    free(output);
    return 0;
}
