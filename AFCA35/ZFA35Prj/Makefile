# ============================================================================
# AFC-7 最终用户应用层 Makefile (完全独立、系统变量驱动版)
# ============================================================================

# 1. 🚨 独立安全拦截：确保最终用户配置了环境
ifndef ST_MP2_SDK_PATH
$(error "🚨 致命错误：ST_MP2_SDK_PATH 环境变量未找到！请确认 ~/.afc7_env 已正确配置，并重启了终端")
endif

# 2. 路径推导
override SDK_PATH := $(subst ~,$(HOME),$(ST_MP2_SDK_PATH))
SDK_BIN := $(SDK_PATH)/sysroots/x86_64-ostl_sdk-linux/usr/bin/aarch64-ostl-linux
CC      := $(SDK_BIN)/aarch64-ostl-linux-gcc
SYSROOT := $(SDK_PATH)/sysroots/cortexa35-ostl-linux

# 3. 🚨 独立构建目录：把产物关在自己的 build 下
TARGET_NAME := $(notdir $(CURDIR))
BUILD_DIR   := build
TARGET      := $(BUILD_DIR)/$(TARGET_NAME)

# 4. 纯净扫描：只找本目录下的源码
SRCS := $(patsubst ./%,%,$(shell find . -name "*.c" ! -path "./$(BUILD_DIR)/*"))
OBJS := $(addprefix $(BUILD_DIR)/, $(SRCS:.c=.o))

# 自动扫描本地 Include
RAW_INC_DIRS := $(shell find . -maxdepth 4 -name "*.h" -exec dirname {} + | sort -u)
INC_FLAGS := $(addprefix -I, $(RAW_INC_DIRS))

# 5. 🚨 独立链接逻辑：直接链接本地 Lib/ 目录下的静态库
LIB_DIR  := Lib
LIBS     := -L$(LIB_DIR) -lZFA35Lib -lpthread -lm
override CFLAGS += -g -O0 -Wall -pthread --sysroot=$(SYSROOT) -mcpu=cortex-a35+crc $(INC_FLAGS)

.PHONY: all clean

all: $(TARGET)
	@if [ -f $(TARGET) ]; then \
		echo "=================================================="; \
		echo " ✅ 应用编译成功！可执行文件已落盘: $(TARGET)"; \
		echo "=================================================="; \
	fi

$(TARGET): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
	@echo "🧹 [ZFA35Prj] 清理完成: 已删除本地 build 目录"