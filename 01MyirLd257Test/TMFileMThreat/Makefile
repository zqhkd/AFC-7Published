# ============================================================================
# AFC-7 标准 Makefile (系统环境变量驱动版)
# ============================================================================

# 1. 🚨 安全卫士：拦截未配置系统环境的报错
ifndef ST_MP2_SDK_PATH
$(error "🚨 致命错误：ST_MP2_SDK_PATH 环境变量未找到！请确认 ~/.afc7_env 已正确配置，并重启了 WSL")
endif

# 兼容处理：防止部分用户在 .afc7_env 里手写了波浪号 ~
override SDK_PATH := $(subst ~,$(HOME),$(ST_MP2_SDK_PATH))

# 2. 严格复刻物理路径 (基于环境变量动态推导)
SDK_BIN := $(SDK_PATH)/sysroots/x86_64-ostl_sdk-linux/usr/bin/aarch64-ostl-linux
CC      := $(SDK_BIN)/aarch64-ostl-linux-gcc
SYSROOT := $(SDK_PATH)/sysroots/cortexa35-ostl-linux

# 3. 头文件路径自动扫描
RAW_INC_DIRS := $(shell find . -maxdepth 4 -name "*.h" -exec dirname {} + | sort -u)
ifneq ($(RAW_INC_DIRS),)
    INC_FLAGS := $(addprefix -I, $(RAW_INC_DIRS))
endif

# 4. 编译参数
override CFLAGS += -g -O0 -Wall $(INC_FLAGS) -pthread --sysroot=$(SYSROOT) -mcpu=cortex-a35+crc
LIBS     := -lpthread -lm
INCLUDES := -I. $(INC_FLAGS)

# 5. 核心构建逻辑
TARGET_NAME := $(notdir $(CURDIR))
BUILD_DIR   := build
TARGET      := $(BUILD_DIR)/$(TARGET_NAME)

# 全递归扫描源码
SRCS        := $(patsubst ./%,%,$(shell find . -name "*.c" ! -path "./$(BUILD_DIR)/*"))
OBJS        := $(addprefix $(BUILD_DIR)/, $(SRCS:.c=.o))

.PHONY: all clean display_info

# 🚨 核心改动：增加物理实体确证逻辑（双保险）
all: display_info $(TARGET)
	@if [ -f $(TARGET) ]; then \
		echo ""; \
		echo "=================================================="; \
		echo " ✅ 编译大功告成！(双重校验通过)"; \
		echo " 🎯 实体文件已确证落盘: $(TARGET)"; \
		echo " 🚀 您现在可以安全地进行部署或按下 F5 启动联机调试了！"; \
		echo "=================================================="; \
		echo ""; \
	else \
		echo ""; \
		echo "=================================================="; \
		echo " ❌ 致命异常：构建过程未报错，但未能找到物理目标文件！"; \
		echo " 🔍 请检查链接器是否将 $(TARGET) 生成到了其他位置。"; \
		echo "=================================================="; \
		echo ""; \
		exit 1; \
	fi
	
display_info:
	@echo "--------------------------------------------------"
	@echo "🚀 开始编译目标: $(TARGET_NAME)"
	@echo "📂 SDK 路径: $(SDK_PATH)"
	@echo "🛠️ 交叉编译器: $(CC)"
	@echo "--------------------------------------------------"

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
	@echo "🧹 清理完成: 已移除 $(BUILD_DIR) 目录"