# 1. 目标名
TARGET := $(notdir $(CURDIR))

# 2. 【核心改动】全递归源码扫描
# 使用 find 命令抓取当前目录下（含所有子目录）的所有 .c 文件
SRCS := $(shell find . -name "*.c")

# 3. 将 .c 列表转换为 .o 对象列表
# 这样 make 就能知道哪些文件需要执行“从 .c 到 .o”的规则
OBJS := $(SRCS:.c=.o)

# 4. 头文件路径：自动扫描所有包含 .h 的目录
RAW_INC_DIRS := $(shell find . -maxdepth 4 -name "*.h" -exec dirname {} + | sort -u)
ifneq ($(RAW_INC_DIRS),)
    INC_FLAGS := $(addprefix -I, $(RAW_INC_DIRS))
endif

# 5. 编译参数：强制调试模式
override CFLAGS += -g -O0 -Wall $(INC_FLAGS) -pthread
LIBS     := -lpthread -lm
INCLUDES := -I. $(INC_FLAGS)

# --- 6. 核心感应：模式切换 (保持逻辑) ---
DRV_DIR  := ZfA35DevLib
LIB_DIR  := Lib
INC_DIR  := Include

ifneq ($(wildcard $(DRV_DIR)/.*),)
    MODE := "CORE_SOURCE_MODE"
    # 源码模式下，SRCS 已经通过第 2 步的全扫描包含进去了
else ifneq ($(wildcard $(LIB_DIR)/.*),)
    MODE := "PRODUCT_LIB_MODE"
    LIBS += -L$(LIB_DIR) -lzflibA35
    INCLUDES += -I$(INC_DIR)
else
    MODE := "BASE_HARDWARE_TEST"
endif

# --- 7. 编译规则 (分步执行) ---
.PHONY: all clean

# 最终链接：将所有的 .o 文件链接成 TARGET
all: $(TARGET)
	@echo "---------------------------------------"
	@echo " Project : $(TARGET)"
	@echo " Mode    : $(MODE)"
	@echo " Objects : $(words $(OBJS)) files scanned"
	@echo "---------------------------------------"

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# 模式规则：如何将每一个 .c 编译成对应的 .o
# -c 表示只编译不链接，$< 是源文件，$@ 是目标文件
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(TARGET)
	find . -name "*.o" -type f -delete