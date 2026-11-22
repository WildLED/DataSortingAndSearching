// ============================================================
// 模块：一般用户查询（实现）
// ============================================================
#include "user_query.h"
#include "config.h"
#include "data_store.h"
#include "display.h"
#include "model.h"

#include <algorithm>   // equal_range
#include <iostream>
#include <vector>

using namespace std;

void user() {
    int n, kinds;

    // 查询循环：每次输入后重新读取数据文件，输错不结束
    while (true) {
        cout << u8"请输入查询信息（姓名或编号等）：" << endl;
        cout << u8"（提示：输入“exit”或“退出”可结束查询）" << endl;
        string search_id;
        cin >> search_id;

        // 输入流出问题时的保护（例如按下了Ctrl+Z/Ctrl+D）
        if (!cin) {
            if (cin.eof()) {
                cout << u8"检测到输入结束，退出查询。" << endl;
                break;
            }
            cin.clear();
            cin.ignore(10000, '\n');
            cout << u8"输入读取失败，请重新输入！" << endl;
            continue;
        }

        // 主动退出查询
        if (search_id == "exit" || search_id == "退出") {
            cout << u8"已结束查询。" << endl;
            break;
        }

        // 每次输入后都重新读取数据文件（自动使用自定义文件名）
        bool ok = true;
        vector<Box> students = load_data(true, &ok);
        n = (int)students.size();

        if (!ok) {
            cout << u8"数据文件存在问题（记录条数不足或字段数异常，详见上方提示），"
                 << u8"为避免给出错误排名，已停止本次查询。" << endl;
            cout << u8"请检查 " << data_file << u8"，或让管理员重新新建数据。" << endl;
            continue;
        }

        // 数据文件为空或不存在时：不结束程序，回到循环重新输入
        if (n == 0) {
            cout << u8"数据文件为空或不存在，无法查询！" << endl;
            cout << u8"请先让管理员使用“1. 新建”创建数据，稍后再输入。" << endl;
            continue;
        }

        kinds = (int)students[0].scores.size();

        // 把所有同名（同编号）的记录都找出来，不再只返回第一条
        vector<int> hits;
        for (int i = 0; i < n; i++) {
            if (students[i].id == search_id) {
                hits.push_back(i);
            }
        }

        // 没查到：不结束程序，回到循环开头重新输入
        if (hits.empty()) {
            cout << u8"未找到该信息！请核验后重新输入！" << endl;
            cout << u8"你的输入：" << search_id << endl;
            continue;
        }

        if (hits.size() > 1) {
            cout << u8"注意：存在 " << hits.size() << u8" 条同名（同编号）记录，以下全部列出，"
                 << u8"建议改用唯一编号查询。" << endl;
        }

        for (size_t k = 0; k < hits.size(); k++) {
            int i = hits[k];

            // 并列记录共享同一名次（compare 已保证全序，正常情况下 tied == 1）
            // 总计溢出的记录改为"集体并列垫底"，不分出虚假的先后：
            // 先找到第一条溢出记录（已排序，非溢出在前、溢出在后），
            // 让全部溢出记录共用这个名次，并把并列人数如实标出来。
            int rank = i + 1;
            int tied = 1;

            if (students[i].overflow) {
                int overflow_start = n;   // 初值取 n，即"没有溢出记录"时的兜底
                for (int j = 0; j < n; j++) {
                    if (students[j].overflow) { overflow_start = j; break; }
                }
                rank = overflow_start + 1;      // 所有溢出记录共用这一个名次
                tied = n - overflow_start;      // 并列人数 = 垫底区的总条数
            } else {
                auto range = equal_range(students.begin(), students.end(), students[i], compare);
                rank = (int)(range.first - students.begin()) + 1;
                tied = (int)(range.second - range.first);
            }

            cout << u8"查询成功！" << endl;

            // 标头（与管理员读取共用同一个函数，格式永远一致）
            cout << u8"结果如下：" << endl;
            print_header(kinds);

            cout << rank << " " << students[i].id << " ";
            for (long long score : students[i].scores) {
                cout << score << " ";
            }
            cout << students[i].total;
            if (students[i].overflow) cout << u8"  [总计溢出，请检查该项数据]";
            cout << endl;

            // 排名百分比：这里改成"超越比例"，名次越好比例越高
            double q = (n - rank) * 100.000000 / n;
            cout << '"' << students[i].id << '"' << u8"的排名超过了全体成员的 ";
            cout << q << "%" << u8"（百分比越高，排名越好）" << endl;
            cout << u8"具体内容（排名/总人数）:";
            cout << "|" << rank << "/" << n << "|";
            if (tied > 1) cout << u8"（并列 " << tied << u8" 人）";
            cout << endl;

            // 当 kinds 在 2~9 时划分档位；溢出记录不参与分档
            if (kinds >= 2 && kinds <= 9 && students[i].overflow) {
                cout << u8"——————————————【不予分档】——————————————" << endl;
                cout << u8"（该记录总计溢出，已排在最末，名次与百分比均不反映真实水平）" << endl;
            }
            else if (kinds >= 2 && kinds <= 9) {
                // 统一按名次四等分
                int b1 = (n + 3) / 4;      // 前 1/4 的上界  = ceil(n/4)
                int b2 = (n + 1) / 2;      // 前 2/4 的上界  = ceil(n/2)
                int b3 = (3 * n + 3) / 4;  // 前 3/4 的上界  = ceil(3n/4)

                // 措辞保持中性（第N档）：排序方向可切换，"优"在升序下容易看反
                const char* g = (rank <= b1) ? u8"第1档"
                              : (rank <= b2) ? u8"第2档"
                              : (rank <= b3) ? u8"第3档"
                                             : u8"第4档";

                cout << u8"——————————————【" << g << u8"】——————————————" << endl;
                cout << u8"（共 4 档，按当前排序方向由前往后划分）" << endl;

                if (!g_descending) {
                    cout << u8"（当前为升序：第 1 档对应数值最小的一批）" << endl;
                }
            }
        }

        break;
    }
}
