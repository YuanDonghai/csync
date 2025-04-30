# 编译器
CC = gcc

# 编译器选项
CFLAGS = -O2 -I./lib -Wall -Wextra -g -ljson-c -lpthread -luuid -lrsync -lssl -lcrypto
# 目标可执行文件
TARGET = csync

# 源文件
SRC_DIR = .
LIB_DIR = lib

# 获取所有源文件
SRCS = $(wildcard $(SRC_DIR)/*.c)
LIB_SRCS = $(wildcard $(LIB_DIR)/*.c)

# 生成对象文件列表
OBJS = $(SRCS:.c=.o)
LIB_OBJS = $(LIB_SRCS:.c=.o)

# 默认目标
all: $(TARGET)

# 链接对象文件生成可执行文件
$(TARGET): $(OBJS) $(LIB_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# 编译源文件为对象文件
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 清理生成的文件
clean:
	rm -f $(OBJS) $(LIB_OBJS) $(TARGET)

.PHONY: all clean