// ============================================================
// 模块：数据模型（实现）
// ============================================================
#include "model.h"
#include "config.h"

#include <algorithm>   // std::min
#include <climits>     // LLONG_MAX / LLONG_MIN

bool safe_add(long long a, long long b, long long &out) {
    if (b > 0 && a > LLONG_MAX - b) return false;   // 向上溢出
    if (b < 0 && a < LLONG_MIN - b) return false;   // 向下溢出
    out = a + b;
    return true;
}

Box::Box(std::string _id, std::vector<long long> _scores)
    : id(_id), scores(_scores), total(0), overflow(false) {
    long long sum = 0;
    for (long long s : _scores) {
        long long next = 0;
        if (!safe_add(sum, s, next)) {
            overflow = true;
            break;
        }
        sum = next;
    }
    total = overflow ? 0 : sum;
}

bool compare(const Box &a, const Box &b) {
    // 溢出的记录总计不可信（total 被置 0），无论什么方向都永远垫底，
    // 否则升序模式下 total=0 会让无效记录直接冲到第 1 名。
    if (a.overflow != b.overflow) return !a.overflow;

    // 先比总的
    if (a.total != b.total)
        return g_descending ? (a.total > b.total) : (a.total < b.total);

    // 如果 total 比不出来，按顺序逐项比较（上界取较小值，避免种类数不等时越界）
    size_t m = std::min(a.scores.size(), b.scores.size());
    for (size_t i = 0; i < m; i++) {
        if (a.scores[i] != b.scores[i])
            return g_descending ? (a.scores[i] > b.scores[i]) : (a.scores[i] < b.scores[i]);
    }

    // 前面都一样但项数不同：项数多的一方视为数据更完整，排在后（固定规则）
    if (a.scores.size() != b.scores.size()) return a.scores.size() > b.scores.size();

    // 实在不行就比 id，确保公平公正（UTF-8 字节序，非拼音序）
    return a.id < b.id;
}
