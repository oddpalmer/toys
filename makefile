CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11 -g
LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# 递归查找根目录和所有子目录下的 .c 文件
SRCS := $(shell find . -name '*.c' -type f)

# 每个 .c 对应的可执行文件：去掉路径和 .c 后缀，放在根目录
# 例如 ./games/tetris.c -> tetris
TARGETS := $(notdir $(SRCS:.c=))

# 默认编译所有
all: $(TARGETS)

# 通用规则：任意目标都从同名 .c 编译
# 用 vpath 让 make 能找到子目录里的源文件
vpath %.c $(sort $(dir $(SRCS)))

%: %.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGETS)

.PHONY: all clean run run-test

# 运行指定程序，例如：make run TARGET=tetris
TARGET ?= game
run: $(TARGET)
	LIBGL_ALWAYS_SOFTWARE=1 ./$(TARGET)

run-test: test
	LIBGL_ALWAYS_SOFTWARE=1 ./test
