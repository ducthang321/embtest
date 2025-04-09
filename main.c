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

typedef struct {
    Token *postfix;
    long double result;  // Sử dụng long double
} ThreadData;

void *findrootNewton(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    data->result = newtonRaphson(data->postfix);

    pthread_mutex_lock(&mutex);
    if (!found && !isnan(data->result)) {
        long double fx = evaluatePostfix(data->postfix, data->result);
        if (fabsl(fx) < 1e-10) {
            best_result = data->result;
            found = 1;
            printf("Newton-Raphson tìm được nghiệm: %.10Lf\n", best_result);
        }
    }
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

void *findrootBisection(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    data->result = bisectionMethod(data->postfix);

    pthread_mutex_lock(&mutex);
    if (!found && !isnan(data->result)) {
        long double fx = evaluatePostfix(data->postfix, data->result);
        if (fabsl(fx) < 1e-10) {
            best_result = data->result;
            found = 1;
            printf("Bisection tìm được nghiệm: %.10Lf\n", best_result);
        }
    }
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

void *findrootSecant(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    data->result = secantMethod(data->postfix);

    pthread_mutex_lock(&mutex);
    if (!found && !isnan(data->result)) {
        long double fx = evaluatePostfix(data->postfix, data->result);
        if (fabsl(fx) < 1e-10) {
            best_result = data->result;
            found = 1;
            printf("Secant tìm được nghiệm: %.10Lf\n", best_result);
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
    if (output != NULL) {
        printTokens(output);

        pthread_t threads[NUM_THREADS];
        ThreadData threadData[NUM_THREADS];

        clock_gettime(CLOCK_MONOTONIC, &start);

        // Chạy tất cả các phương pháp tìm nghiệm
        threadData[0].postfix = output;
        pthread_create(&threads[0], NULL, findrootNewton, (void *)&threadData[0]);

        threadData[1].postfix = output;
        pthread_create(&threads[1], NULL, findrootBisection, (void *)&threadData[1]);

        threadData[2].postfix = output;
        pthread_create(&threads[2], NULL, findrootSecant, (void *)&threadData[2]);

        // Chờ các luồng hoàn thành
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
    } else {
        printf("Lỗi khi chuyển đổi biểu thức!\n");
    }
    return 0;
}
