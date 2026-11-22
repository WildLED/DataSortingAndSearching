#!/bin/sh
# Data Sorting and Searching —— 一键编译脚本（Linux / macOS / MinGW）
# 用法：sh build.sh [g++|clang++]
cd "$(dirname "$0")" || exit 1
CXX="${1:-g++}"
set -e
$CXX -std=c++11 -Wall -Wextra -O2 \
  main.cpp admin.cpp user_query.cpp data_store.cpp model.cpp display.cpp \
  file_utils.cpp encoding.cpp config.cpp \
  -o programming
echo "编译完成：$(pwd)/programming"
