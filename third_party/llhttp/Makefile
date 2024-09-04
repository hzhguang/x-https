# 编译器和标志
CC = gcc
CFLAGS = -Wall -Iinclude -fPIC
AR = ar
ARFLAGS = rcs
LIBRARY_NAME_STATIC = libllhttp.a
LIBRARY_NAME_SHARED = libllhttp.so

# 目录
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
LIB_DIR = lib

# 源文件
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# 规则
all: $(LIBRARY_NAME_STATIC) $(LIBRARY_NAME_SHARED)

# 创建构建目录
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# 编译源文件
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# 创建静态库
$(LIBRARY_NAME_STATIC): $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

# 创建动态库
$(LIBRARY_NAME_SHARED): $(OBJS)
	$(CC) -shared -o $@ $(OBJS)

# 安装库文件
install: $(LIBRARY_NAME_STATIC) $(LIBRARY_NAME_SHARED)
	mkdir -p $(LIB_DIR)
	cp $(LIBRARY_NAME_STATIC) $(LIB_DIR)
	cp $(LIBRARY_NAME_SHARED) $(LIB_DIR)
	cp $(SRC_DIR)/*.h $(INCLUDE_DIR)  # 复制头文件

# 清理
clean:
	rm -f $(BUILD_DIR)/*.o $(LIBRARY_NAME_STATIC) $(LIBRARY_NAME_SHARED)

.PHONY: all install clean
