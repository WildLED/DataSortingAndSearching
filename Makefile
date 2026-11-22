# Data Sorting and Searching —— 数据排序与查找
# 用法：make            （Linux / macOS，g++ 或 Clang）
#       make mingw      （Windows MinGW / MSYS2，产出 programming.exe）
#       make clean
CXX ?= g++
CXXSTD := -std=c++11
CXXFLAGS ?= -Wall -Wextra -O2 $(CXXSTD)
TARGET := programming

SRCS := main.cpp admin.cpp user_query.cpp data_store.cpp model.cpp display.cpp file_utils.cpp encoding.cpp config.cpp
OBJS := $(SRCS:.cpp=.o)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

mingw: $(TARGET).exe
$(TARGET).exe: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f $(OBJS) $(TARGET) $(TARGET).exe

.PHONY: clean mingw
