// ============================================================
// 模块：数据存储（保存 / 读取，含备份与严格解析）
// 数据文件格式：第一行为 n（人数）与 kinds（数据种类数），
//              其后每行依次为：序号、姓名、各项数据、总计
// ============================================================
#ifndef DATA_STORE_H
#define DATA_STORE_H

#include <vector>
#include "model.h"

// 保存：写 .tmp → 原文件改名 .bak → .tmp 原子替换；任一步失败都不破坏原文件。
bool save_data(const std::vector<Box>& students);

// 读取：strict=true 且文件残缺时返回空（用于写入路径，避免覆盖丢数据）；
// strict=false 时返回已成功读取的部分。ok 报告是否完整。返回前统一排序。
std::vector<Box> load_data(bool strict = false, bool* ok = nullptr);

#endif // DATA_STORE_H
