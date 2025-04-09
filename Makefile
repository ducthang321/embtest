# Makefile cho chương trình tìm nghiệm

# Compiler để sử dụng
CC = gcc

# Cờ compiler
CFLAGS = -Wall -g -O2

# Cờ linker
LDFLAGS = -lm -pthread -lwiringPi

# Tên file thực thi cuối cùng
TARGET = rootfinder

# Các file nguồn
SOURCES = main.c postfix.c findroot.c

# Các file object
OBJECTS = $(SOURCES:.c=.o)

# Các file header
HEADERS = postfix.h findroot.h

# Quy tắc mặc định
all: $(TARGET)

# Quy tắc để tạo file thực thi
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Quy tắc để biên dịch các file .c thành .o
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Quy tắc để làm sạch
clean:
	rm -f $(OBJECTS) $(TARGET)

# Quy tắc để chạy chương trình
run: $(TARGET)
	./$(TARGET)

# Phony targets
.PHONY: all clean run
