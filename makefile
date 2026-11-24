CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11 -g
LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

BUILD := build

# 递归查找所有 .c
SRCS := $(shell find . -name '*.c' -type f -not -path './$(BUILD)/*')

# 每个 .c 对应的可执行文件：build/去掉路径和.c后缀的名字
# 例如 ./tetris/tetrix.c -> build/tetrix
TARGETS := $(addprefix $(BUILD)/,$(notdir $(SRCS:.c=)))

all: $(TARGETS)

# 让 make 能在子目录里找到源文件
vpath %.c $(sort $(dir $(SRCS)))

# 任意 build/xxx 都从同名 xxx.c 编译，且依赖 build 目录存在
$(BUILD)/%: %.c | $(BUILD)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)

.PHONY: all clean run

TARGET ?= tetrix
run: $(BUILD)/$(TARGET)
	    LIBGL_ALWAYS_SOFTWARE=1 ./$(BUILD)/$(TARGET)
