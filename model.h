// ============================================================
// 模块：数据模型（Box 结构 / 安全加法 / 排序规则）
// ============================================================
#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>

// 安全加法：long long 求和溢出是未定义行为，改为"加之前先检测"，溢出时返回 false
bool safe_add(long long a, long long b, long long &out);

// 一条数据记录
struct Box {
    std::string id;                  // 姓名（或编号）
    std::vector<long long> scores;   // 各项数据
    long long total;                 // 总计（构造时自动求和）
    bool overflow;                   // 总计是否溢出，此时 total 记 0

    Box(std::string _id, std::vector<long long> _scores);
};

// 排序规则：先按总计，再逐项比较，再按项数，最后按 id；方向由 g_descending 控制。
// 溢出记录无论方向都垫底；末尾的 id 兜底规则不随方向反转。
bool compare(const Box &a, const Box &b);

#endif // MODEL_H
