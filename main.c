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

// Biến toàn cục cho luồng
int found = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
long double best_result = 0.0;
pthread_t threads[NUM_THREADS];  // Lưu trữ các thread để hủy khi cần

typedef struct {
    Token *postfix;
    long double result;  // Sử dụng long double
    int thread_id;       // ID của thread để xác định thread nào tìm được nghiệm
} ThreadData;

void *findrootNewton(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    long double result = newtonRaphson(data->postfix);

    pthread_mutex_lock(&mutex);
    if (!found && !isnan(result)) {
        long double fx = evaluatePostfix(data->postfix, result);
        if (fabsl(fx) < 1e-10) {
            best_result = result;
            found = 1;
            printf("Newton-Raphson tìm được nghiệm: %.10Lf\n", best_result);
            // Hủy các thread khác
            for (int i = 0; i < NUM_THREADS; i++) {
                if (i != data->thread_id) {
                    pthread_cancel(threads[i]);
                }
            }
        }
    }
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

void *findrootBisection(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    long double result = bisectionMethod(data->postfix);

    pthread_mutex_lock(&mutex);
    if (!found && !isnan(result)) {
        long double fx = evaluatePostfix(data->postfix, result);
        if (fabsl(fx) < 1e-10) {
            best_result = result;
            found = 1;
            printf("Bisection tìm được nghiệm: %.10Lf\n", best_result);
            // Hủy các thread khác
            for (int i = 0; i < NUM_THREADS; i++) {
                if (i != data->thread_id) {
                    pthread_cancel(threads[i]);
                }
            }
        }
    }
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

void *findrootSecant(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    long double result = secantMethod(data->postfix);

    pthread_mutex_lock(&mutex);
    if (!found && !isnan(result)) {
        long double fx = evaluatePostfix(data->postfix, result);
        if (fabsl(fx) < 1e-10) {
            best_result = result;
            found = 1;
            printf("Secant tìm được nghiệm: %.10Lf\n", best_result);
            // Hủy các thread khác
            for (int i = 0; i < NUM_THREADS; i++) {
                if (i != data->thread_id) {
                    pthread_cancel(threads[i]);
                }
            }
        }
    }
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

int main() {
    struct timespec start, end;
    Token *output;
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
        printf("Lỗi khi chuyển đổi biểu thức!\n");
        return 1;
    }

    printTokens(output);

    ThreadData threadData[NUM_THREADS];

    clock_gettime(CLOCK_MONOTONIC, &start);

    // Chạy tất cả các phương pháp tìm nghiệm song song
    threadData[0].postfix = output;
    threadData[0].thread_id = 0;
    pthread_create(&threads[0], NULL, findrootNewton, (void *)&threadData[0]);

    threadData[1].postfix = output;
    threadData[1].thread_id = 1;
    pthread_create(&threads[1], NULL, findrootBisection, (void *)&threadData[1]);

    threadData[2].postfix = output;
    threadData[2].thread_id = 2;
    pthread_create(&threads[2], NULL, findrootSecant, (void *)&threadData[2]);

    // Chờ các luồng hoàn thành (hoặc bị hủy)
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Thời gian tìm nghiệm: %.6f giây\n", elapsed);

    if (found) {
        long double fx = evaluatePostfix(output, best_result);
        printf("Kết quả với nghiệm %.10Lf là: %.10Lf\n", best_result, fx);
        if (fabsl(fx) > 1e-10) {
            printf("Cảnh báo: Giá trị tại nghiệm không đủ gần 0 (có thể không phải nghiệm chính xác)!\n");
        }
    } else {
        printf("Không tìm được nghiệm hợp lệ!\n");
    }

    free(output);
    return 0;
}
