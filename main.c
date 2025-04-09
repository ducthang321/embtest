#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <pigpio.h>
#include "postfix.h"
#include "findroot.h"

#define NUM_THREADS 3
#define ROWS 7
#define COLS 4
#define MAX 100

// Định nghĩa các chân GPIO cho bàn phím
int rowPins[ROWS] = {17, 18, 27, 22, 23, 24, 25};
int colPins[COLS] = {8, 7, 1, 4};

// Bàn phím 7x4
char keymap[ROWS][COLS] = {
    {'1', '2', '3', '+'},
    {'4', '5', '6', '-'},
    {'7', '8', '9', '*'},
    {'0', '.', '/', '^'},
    {'(', ')', 'x', 'E'}, // 'E' để xác nhận
    {'B', '\0', '\0', '\0'}, // 'B' để xóa ký tự cuối
    {'\0', '\0', '\0', '\0'}
};

// Biến toàn cục
int found = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t threads[NUM_THREADS];
long double best_result = 0.0;

typedef struct {
    Token *postfix;
    long double result;
    int thread_id;
} ThreadData;

// Hàm quét bàn phím
char scanKeypad() {
    for (int r = 0; r < ROWS; r++) {
        gpioWrite(rowPins[r], PI_LOW);
        for (int c = 0; c < COLS; c++) {
            if (gpioRead(colPins[c]) == PI_LOW) {
                while (gpioRead(colPins[c]) == PI_LOW) time_sleep(0.01); // Debounce
                gpioWrite(rowPins[r], PI_HIGH);
                return keymap[r][c];
            }
        }
        gpioWrite(rowPins[r], PI_HIGH);
    }
    return '\0';
}

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
    int idx = 0;
    char ch;

    // Khởi tạo pigpio
    if (gpioInitialise() < 0) {
        printf("Lỗi: Không thể khởi tạo pigpio!\n");
        return 1;
    }

    // Thiết lập các chân GPIO
    for (int i = 0; i < ROWS; i++) {
        gpioSetMode(rowPins[i], PI_OUTPUT);
        gpioWrite(rowPins[i], PI_HIGH);
    }
    for (int i = 0; i < COLS; i++) {
        gpioSetMode(colPins[i], PI_INPUT);
        gpioSetPullUpDown(colPins[i], PI_PUD_UP);
    }

    // Nhập biểu thức từ bàn phím
    printf("Nhập biểu thức bằng bàn phím 7x4 (ấn 'E' để xác nhận, 'B' để xóa ký tự cuối):\n");
    printf("Hiện tại: ");
    str[0] = '\0';

    while (1) {
        ch = scanKeypad();
        if (ch == '\0') continue;
        if (ch == 'E') {
            str[idx] = '\0';
            printf("\nBiểu thức đã nhập: %s\n", str);
            break;
        } else if (ch == 'B') {
            if (idx > 0) {
                idx--;
                str[idx] = '\0';
                printf("\rHiện tại: %s  ", str);
                fflush(stdout);
            }
        } else if (idx < MAX - 1) {
            str[idx++] = ch;
            str[idx] = '\0';
            printf("\rHiện tại: %s", str);
            fflush(stdout);
        }
    }

    // Chuyển đổi sang postfix
    output = infixToPostfix(str);
    if (output == NULL) {
        printf("Lỗi khi chuyển đổi biểu thức!\n");
        gpioTerminate();
        return 1;
    }

    printTokens(output);

    // Tạo dữ liệu cho các luồng
    ThreadData threadData[NUM_THREADS];

    clock_gettime(CLOCK_MONOTONIC, &start);

    // Chạy các phương pháp tìm nghiệm song song
    threadData[0].postfix = output;
    threadData[0].thread_id = 0;
    if (pthread_create(&threads[0], NULL, findrootNewton, (void *)&threadData[0]) != 0) {
        printf("Lỗi khi tạo thread Newton-Raphson!\n");
        free(output);
        gpioTerminate();
        return 1;
    }

    threadData[1].postfix = output;
    threadData[1].thread_id = 1;
    if (pthread_create(&threads[1], NULL, findrootBisection, (void *)&threadData[1]) != 0) {
        printf("Lỗi khi tạo thread Bisection!\n");
        free(output);
        gpioTerminate();
        return 1;
    }

    threadData[2].postfix = output;
    threadData[2].thread_id = 2;
    if (pthread_create(&threads[2], NULL, findrootSecant, (void *)&threadData[2]) != 0) {
        printf("Lỗi khi tạo thread Secant!\n");
        free(output);
        gpioTerminate();
        return 1;
    }

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

    // Giải phóng tài nguyên
    free(output);
    pthread_mutex_destroy(&mutex);
    gpioTerminate();
    return 0;
}
