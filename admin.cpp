// ============================================================
// 模块：管理员功能（实现）
// ============================================================
#include "admin.h"
#include "config.h"
#include "data_store.h"
#include "display.h"
#include "file_utils.h"
#include "model.h"

#include <algorithm>   // std::sort
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// ==================== 新建文件（管理员内部功能） ====================
static void new_data_from_input() {
    int n;
    cout << u8"输入人数（或参数总数）：";
    cin >> n;

    // 人数与种类数必须为正，否则后面的循环空转、文件也不会生成
    if (!cin || n <= 0) {
        if (cin && !cin.eof()) { cin.clear(); cin.ignore(10000, '\n'); }
        cout << u8"人数必须是不小于 1 的整数，已取消本次新建。" << endl;
        return;
    }

    cout << u8"请输入数据种类数量：";
    int kinds;
    cin >> kinds;

    if (!cin || kinds <= 0) {
        if (cin && !cin.eof()) { cin.clear(); cin.ignore(10000, '\n'); }
        cout << u8"数据种类数必须是不小于 1 的整数，已取消本次新建。" << endl;
        return;
    }

    cout << u8"请按顺序输入：姓名（或编号）";
    print_header_prompt(kinds);

    vector<Box> students;

    for (int i = 0; i < n; i++) {
        string id;
        vector<long long> scores;

        cin >> id;
        for (int j = 0; j < kinds; j++) {
            long long score;
            cin >> score;
            scores.push_back(score);
        }

        // 输入中断（如 Ctrl+Z）时不再继续死等
        if (!cin) {
            cout << u8"输入中断，已取消本次新建，文件未被改动。" << endl;
            return;
        }

        Box box(id, scores);

        // 溢出记录退回重输，避免错误总计写入文件
        if (box.overflow) {
            cout << u8"第 " << i + 1 << u8" 条记录（" << id << u8"）的各项之和超出 long long 范围，"
                 << u8"请重新输入这一条。" << endl;
            i--;
            continue;
        }

        students.push_back(box);
    }

    sort(students.begin(), students.end(), compare);

    print_data_simply(students);

    save_data(students);
}

// ==================== 补写文件（管理员内部功能） ====================
static void append_data_from_input() {
    // 先检查文件里有没有有效数据
    bool has_valid_data = false;
    {
        FileStreamBuf check_buf(data_file, false);
        istream check_in(&check_buf);
        if (check_buf.is_open()) {
            // 与 load_data 保持同一种读法：整行取出首行、去掉 BOM 后再解析
            string first_line;
            if (getline(check_in, first_line)) {
                if (first_line.size() >= 3 &&
                    (unsigned char)first_line[0] == 0xEF &&
                    (unsigned char)first_line[1] == 0xBB &&
                    (unsigned char)first_line[2] == 0xBF) {
                    first_line.erase(0, 3);
                }
                if (!first_line.empty() && first_line.back() == '\r') first_line.pop_back();

                int check_n = 0, check_kinds = 0;
                istringstream hss(first_line);
                string extra;
                // 首行必须恰好是两个正数：n 与 kinds
                if ((hss >> check_n >> check_kinds) && !(hss >> extra)) {
                    has_valid_data = (check_n > 0 && check_kinds > 0);
                }
            }
        }
    }

    if (!has_valid_data) {
        cout << u8"未找到有效的数据文件，请先使用“1. 新建”创建数据。" << endl;
        return;
    }

    // 读取已有数据（strict=true：文件残缺时返回空，不覆盖原文件）
    bool ok = true;
    vector<Box> students = load_data(true, &ok);

    // 声明人数与实际条数不一致时，取消本次追加，避免静默丢数据
    if (!ok) {
        cout << u8"数据文件疑似残缺：记录条数不足，或某条记录的字段数异常（详见上方提示）。" << endl;
        cout << u8"为避免覆盖丢失原有数据，已取消本次补充写入。" << endl;
        cout << u8"请检查 " << data_file << u8"，或改用“1. 新建”重新录入。" << endl;
        return;
    }

    if (students.empty()) {
        cout << u8"当前文件中没有可追加的数据，请先使用“1. 新建”创建数据。" << endl;
        return;
    }

    // 沿用原有文件里的数据种类数量，保证格式一致
    int kinds = (int)students[0].scores.size();

    int add_count;
    cout << u8"输入需要补充的人数（或参数总数）：";
    cin >> add_count;

    if (!cin || add_count <= 0) {
        if (cin && !cin.eof()) { cin.clear(); cin.ignore(10000, '\n'); }
        cout << u8"补充人数必须是不小于 1 的整数，已取消本次补充写入，原文件未被改动。" << endl;
        return;
    }

    cout << u8"当前数据种类数量为 " << kinds << u8" 个，请按顺序输入：姓名（或编号）";
    print_header_prompt(kinds);

    for (int i = 0; i < add_count; i++) {
        string id;
        vector<long long> scores;

        cin >> id;
        for (int j = 0; j < kinds; j++) {
            long long score;
            cin >> score;
            scores.push_back(score);
        }

        if (!cin) {
            cout << u8"输入中断，已取消本次补充写入，原文件未被改动。" << endl;
            return;
        }

        Box box(id, scores);

        // 溢出当场提示重输，不写入错误总计
        if (box.overflow) {
            cout << u8"第 " << i + 1 << u8" 条记录（" << id << u8"）的各项之和超出 long long 范围，"
                 << u8"请重新输入这一条。" << endl;
            i--;
            continue;
        }

        students.push_back(box);
    }

    sort(students.begin(), students.end(), compare);

    print_data_simply(students);

    if (save_data(students)) {
        cout << u8"补充写入完成，数据已重新排序并保存到 " << data_file << u8"。" << endl;
    }
}

// ==================== 管理员入口 ====================
void admin() {

#if ADMIN_PASSWORD_ENABLE
    // 口令保护（默认关闭）：不改动下面任何业务逻辑，只在进入管理员前多问一句
    {
        cout << u8"请输入管理员口令：" << endl;
        string pwd;
        cin >> pwd;
        if (!cin || pwd != ADMIN_PASSWORD) {
            cout << u8"口令错误，已自动关闭程序。" << endl;
            return;
        }
    }
#endif

    // 选项同样要有输入保护，输错不再静默返回
    while (true) {
        cout << u8"输入操作类型" << endl;
        cout << u8"1. 新建\n2. 读取\n3. 补充写入\n4. 切换排序方向" << endl;
        cout << u8"（当前排序方向：" << order_name() << u8"）" << endl;

        int option = 0;
        cin >> option;

        // 输入不是数字、或按下 Ctrl+Z/Ctrl+D 时的处理
        if (!cin) {
            if (cin.eof()) {
                cout << u8"检测到输入结束，返回身份选择。" << endl;
                return;
            }
            cin.clear();
            cin.ignore(10000, '\n');
            cout << u8"输入无效（请输入 1、2、3 或 4），请重新输入！" << endl;
            continue;
        }

        if (option == 1) {
            new_data_from_input();
            return;
        }
        else if (option == 2) {
            // 只读路径：不写盘，残缺文件也只做提示，不会改动文件
            bool ok = true;
            vector<Box> students = load_data(false, &ok);
            if (!ok) {
                cout << u8"警告：数据文件存在问题（记录条数不足或字段数异常，详见上方提示），"
                     << u8"以下仅为已成功读取的部分，请检查 " << data_file << u8"。" << endl;
            }
            print_data_simply(students);
            return;
        }
        else if (option == 3) {
            append_data_from_input();
            return;
        }
        else if (option == 4) {
            // 切换方向：只改运行时口径，不写盘；切完立刻按新方向重排显示一遍
            g_descending = !g_descending;
            cout << u8"已切换排序方向，当前为：" << order_name() << endl;
            if (!g_descending) {
                cout << u8"注意：升序下第 1 档对应数值最小的一批记录。" << endl;
            }

            bool ok = true;
            vector<Box> students = load_data(false, &ok);
            if (!ok) {
                cout << u8"警告：数据文件疑似残缺，以下仅为已成功读取的部分。" << endl;
            }
            if (students.empty()) {
                cout << u8"（当前数据文件为空或不存在，暂无可显示的内容）" << endl;
            } else {
                print_data_simply(students);
            }
            continue;   // 回到菜单，可继续其他操作
        }
        else {
            cout << u8"输入无效（请输入 1、2、3 或 4），请重新输入！" << endl;
        }
    }
}
