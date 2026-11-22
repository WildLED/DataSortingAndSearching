// ============================================================
// 模块：显示输出（按任意键、表头、结果列表）
// ============================================================
#ifndef DISPLAY_H
#define DISPLAY_H

#include <vector>
#include "model.h"

// 按任意键结束（Windows 用 _getch，其他平台改终端属性读一个字符）
void over();

// 结果表头（数据种类数超过 30 时自动压缩，避免刷屏）
void print_header(int kinds);

// 录入时的列名提示（与 print_header 同源，避免两处写法不一致）
void print_header_prompt(int kinds);

// 输出排序好的数据
void print_data_simply(const std::vector<Box>& students);

#endif // DISPLAY_H
