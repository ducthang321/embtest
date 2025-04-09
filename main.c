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
float best_result = 0.0;

typedef struct {
    Token *postfix;
    float result;
} ThreadData;

void *findrootNewton(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    data->result = newtonRaphson(data->postfix);
    pthread_exit(NULL);
}

void *findrootBisection(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    data->result = bisectionMethod(data->postfix);
    pthread_exit(NULL);
}

void *findrootSecant(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    data->result = secantMethod(data->postfix);
    pthread_exit(NULL);
}

int main() {
    struct timespec start, end;
    Token *output;
    // Biểu thức được hardcode thay vì nhập từ keypad
    char str[] = "(x^14-3*x^12+7*x^9)-(5*x^8+2*x^6)+(4*x^5-11*x^3+6*x^2)-(20*x-50)";

    printf("Biểu thức: %s\n", str);

    output = infixToPostfix(str);
    if (output != NULL) {
        printTokens(output);

        pthread_t threads[NUM_THREADS];
        ThreadData threadData[NUM_THREADS];

        clock_gettime(CLOCK_MONOTONIC, &start);

        // Khởi tạo các thread
        threadData[0].postfix = output;
        pthread_create(&threads[0], NULL, findrootNewton, (void *)&threadData[0]);

        threadData[1].postfix = output;
        pthread_create(&threads[1], NULL, findrootBisection, (void *)&threadData[1]);

        threadData[2].postfix = output;
        pthread_create(&threads[2], NULL, findrootSecant, (void *)&threadData[2]);

        // Chờ tất cả thread hoàn thành
        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_join(threads[i], NULL);
        }

        clock_gettime(CLOCK_MONOTONIC, &end);
        
        // Kiểm tra kết quả từ tất cả các phương pháp sau khi thread hoàn thành
        for (int i = 0; i < NUM_THREADS; i++) {
            if (!isnan(threadData[i].result)) {
                float fx = evaluatePostfix(output, threadData[i].result);
                if (fabs(fx) < 1e-4) {
                    best_result = threadData[i].result;
                    found = 1;
                    switch(i) {
                        case 0: printf("Newton-Raphson tìm được nghiệm: %f\n", best_result); break;
                        case 1: printf("Bisection tìm được nghiệm: %f\n", best_result); break;
                        case 2: printf("Secant tìm được nghiệm: %f\n", best_result); break;
                    }
                    break; // Dừng khi tìm thấy nghiệm đầu tiên
                }
            }
        }

        double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        printf("Thời gian tìm nghiệm: %f giây\n", elapsed);

        if (found) {
            float fx = evaluatePostfix(output, best_result);
            printf("Kết quả với nghiệm %.4f là: %.4f\n", best_result, fx);
            if (fabs(fx) > 1e-4) {
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
