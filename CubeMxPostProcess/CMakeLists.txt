cmake_minimum_required(VERSION 3.22)
include(ExternalProject)

# In order to use CMake for cross-compiling
set(CMAKE_CXX_COMPILER_FORCED "true")

# ==========================================
# 🚨 动态项目名提取：自动适配外部文件夹名称
# ==========================================
get_filename_component(AUTO_PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(CMAKE_PROJECT_NAME ${AUTO_PROJECT_NAME})

# Enable compile commands for VS Code IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Core project settings
# project(${CMAKE_PROJECT_NAME})
project(${CMAKE_PROJECT_NAME} C CXX ASM)

# Include external project file
include("mx-generated.cmake")
