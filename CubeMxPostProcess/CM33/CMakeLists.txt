cmake_minimum_required(VERSION 3.22)

set(TFM_BUILD_DIR "" CACHE INTERNAL "")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

message("Build type: " ${CMAKE_BUILD_TYPE})

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(STM32_MCU_FLAGS "-mcpu=cortex-m33 -mfpu=fpv5-sp-d16 -mfloat-abi=hard")
set(STM32_LINKER_SCRIPT stm32mp2xx_DDR_m33_ns.ld)
set(STM32_LINKER_OPTION  )

# ==========================================
# 🚨 动态项目名：向上追溯获取根目录名
# ==========================================
get_filename_component(ROOT_DIR_PATH ${CMAKE_CURRENT_SOURCE_DIR}/.. ABSOLUTE)
get_filename_component(AUTO_PROJECT_NAME ${ROOT_DIR_PATH} NAME)
set(CMAKE_PROJECT_NAME ${AUTO_PROJECT_NAME})
# project(${CMAKE_PROJECT_NAME})
project(${CMAKE_PROJECT_NAME} C CXX ASM)

add_executable(${CMAKE_PROJECT_NAME})

include("mx-generated.cmake")

# ==============================================================================
# 🛠️ 调试/发布模式自动化宏定义 (位置：目标定义后，用户自定义区前)
# ==============================================================================
# 将变量转为大写后再比较，彻底规避正则 (?i) 报错问题
string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_UPPER)
if(BUILD_TYPE_UPPER STREQUAL "DEBUG")
    # 调试模式：开启硬件断点陷阱。PUBLIC 确保所有依赖此目标的源文件都能看到宏
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC __AFC7_DEBUG_MODE__)
    message(STATUS ">>>> [AFC7] Build Mode: Debug (bkpt #0 enabled)")
else()
    # 发布模式（Release）：禁用调试宏，保证连续运行不崩溃
    message(STATUS ">>>> [AFC7] Build Mode: Release (Ready for mission)")
endif()

set(CMAKE_CXX_COMPILER_FORCED "true")

list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

if((CMAKE_C_STANDARD EQUAL 90) OR (CMAKE_C_STANDARD EQUAL 99))
    message(ERROR "Generated code requires C11 or higher")
endif()

# ==============================================================================
# 用户自定义高级配置区 (M33 核心专属，重新生成不会被覆盖)
# ==============================================================================
target_link_libraries(${CMAKE_PROJECT_NAME} Utilities)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../Utilities/CoproSync
    ${CMAKE_CURRENT_SOURCE_DIR}/../Utilities/ResourcesManager
)

# 确保 res_mgr.c 唯一存在于主工程中，并补齐漏掉的汇编启动文件
set(STARTUP_FILE ${CMAKE_CURRENT_SOURCE_DIR}/Core/Startup/startup_stm32mp257dxx_m33.s)
# 🚨 核心修复：强行按着 CMake 的头，告诉它这个小写 .s 文件必须用汇编器(ASM)编译！
set_source_files_properties(${STARTUP_FILE} PROPERTIES LANGUAGE ASM)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../Utilities/ResourcesManager/res_mgr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/OPENAMP/rsc_table.c
    ${STARTUP_FILE}
)

# 🚨 优雅的环境变量驱动修复：读取系统环境中的路径配置，解耦代码与畸形路径
target_include_directories(OpenAMP PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../Middlewares/Third_Party/OpenAMP/libmetal/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../Middlewares/Third_Party/OpenAMP/open-amp/lib/include
)