# 编译器和标志
CC = gcc

OPENSSL_INCLUDE = -Ithird_party/openssl/usr/local/include
LIBEV_INCLUDE 	= -Ithird_party/libev/include
LLHTTP_INCLUDE 	= -Ithird_party/llhttp/include

OPENSSL_LIB = -Lthird_party/openssl/usr/local/lib 
LIBEV_LIB 	= -Lthird_party/libev/lib
LLHTTP_LIB 	= -Lthird_party/llhttp/lib 

CFLAGS_DEBUG = -Wall -g -Iinclude $(OPENSSL_INCLUDE) $(LIBEV_INCLUDE) $(LLHTTP_INCLUDE) -fPIC
CFLAGS_RELEASE = -Wall -O2 -Iinclude $(OPENSSL_INCLUDE) $(LIBEV_INCLUDE) $(LLHTTP_INCLUDE) -fPIC

LDFLAGS = -Llib -L. $(OPENSSL_LIB) $(LLHTTP_LIB) $(LIBEV_LIB)
LDFLAGS += -lssl -lcrypto -lev -lllhttp

# 目录
SRC_DIR = src
EXAMPLE_DIR = examples
INCLUDE_DIR = include
BUILD_DIR = build
LIB_DIR = lib
BIN_DIR = bin

# 目标
LIBRARY_NAME_STATIC = $(LIB_DIR)/libxhttp.a
LIBRARY_NAME_SHARED = $(LIB_DIR)/libxhttp.so
TARGET_CLIENT = $(BIN_DIR)/client_example
TARGET_SERVER = $(BIN_DIR)/server_example

# 默认构建类型
BUILD_TYPE ?= debug

ifeq ($(BUILD_TYPE), debug)
    CFLAGS = $(CFLAGS_DEBUG)
else
    CFLAGS = $(CFLAGS_RELEASE)
endif

# 源文件
SRCS_LIB = $(wildcard $(SRC_DIR)/*.c)
OBJS_LIB = $(SRCS_LIB:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

SRCS_CLIENT = $(EXAMPLE_DIR)/client_example.c
SRCS_SERVER = $(EXAMPLE_DIR)/server_example.c

OBJS_CLIENT = $(SRCS_CLIENT:$(EXAMPLE_DIR)/%.c=$(BUILD_DIR)/%.o)
OBJS_SERVER = $(SRCS_SERVER:$(EXAMPLE_DIR)/%.c=$(BUILD_DIR)/%.o)

all: $(LIBRARY_NAME_STATIC) $(LIBRARY_NAME_SHARED) $(TARGET_CLIENT) $(TARGET_SERVER)

lib: $(LIBRARY_NAME_STATIC) $(LIBRARY_NAME_SHARED)

example: $(TARGET_CLIENT) $(TARGET_SERVER)

$(LIB_DIR):
	mkdir -p $(LIB_DIR)
$(BIN_DIR):
	mkdir -p $(BIN_DIR)
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# 静态库规则
$(LIBRARY_NAME_STATIC): $(OBJS_LIB)
	ar rcs $@ $(OBJS_LIB)

$(LIBRARY_NAME_SHARED): $(OBJS_LIB)
	$(CC) -shared -o $@ $(OBJS_LIB) $(LDFLAGS)


# 生成对象文件
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(EXAMPLE_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# 客户端
$(TARGET_CLIENT): $(BUILD_DIR) $(OBJS_CLIENT)
	$(CC) -o $@ $(OBJS_CLIENT) $(LDFLAGS) -lxhttp

# 服务器
$(TARGET_SERVER): $(BUILD_DIR) $(OBJS_SERVER) 
	$(CC) -o $@ $(OBJS_SERVER) $(LDFLAGS) -lxhttp

# 清理
clean:
	rm -f $(LIBRARY_NAME_STATIC) $(LIBRARY_NAME_SHARED) $(TARGET_CLIENT) $(TARGET_SERVER)
	rm -f $(BUILD_DIR)/*.o

.PHONY: all clean
