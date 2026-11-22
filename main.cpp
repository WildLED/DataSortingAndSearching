// ============================================================
// 模块：程序入口（文件名设置、排序方向选择、身份选择）
// ============================================================
#include "config.h"
#include "admin.h"
#include "user_query.h"
#include "display.h"
#include "file_utils.h"

#include <iostream>
#include <string>

#ifdef _WIN32
    #include <windows.h>
#endif

using namespace std;

// ==================== 初始化数据文件名 ====================
// 从用户输入读取并初始化数据文件名（由 main 中的数据文件名输入段整合而来）
static void init_data_file() {
    cout << u8"请输入数据文件名（直接按回车使用默认的 programming.txt）：" << endl;
    cout << u8"（没有 .txt 后缀会自动补上，例如输入“成绩单”即使用 成绩单.txt）" << endl;
    string input_name;
    getline(cin, input_name);

    // 去掉首尾空白字符（空格、Tab等）
    while (!input_name.empty() && (input_name.front() == ' ' || input_name.front() == '\t')) {
        input_name.erase(input_name.begin());
    }
    while (!input_name.empty() && (input_name.back() == ' ' || input_name.back() == '\t')) {
        input_name.pop_back();
    }

    // 没输入（直接回车）就用默认文件名
    if (!input_name.empty()) {
        // 如果没有以.txt结尾，自动补上
        if (input_name.size() < 4 || input_name.substr(input_name.size() - 4) != ".txt") {
            input_name += ".txt";
        }
        data_file = input_name;
    }

    cout << u8"当前使用的数据文件：" << data_file << endl;

    // 输出txt文件地址
    cout << u8"txt文件地址（可导入，名称与类型需一致）：";
    cout << get_absolute_path(data_file) << endl;
}

// ==================== 排序方向选择 ====================
// 选一次即可，管理员菜单里还能用"4. 切换排序方向"随时改。
// 方向只影响排名口径，不写入数据文件
static void choose_sort_order() {
    cout << u8"请选择排序方向（序号）：" << endl;
    cout << u8"1. 降序：数值越大排名越靠前（适用于成绩、得分等“越大越好”的数据）" << endl;
    cout << u8"2. 升序：数值越小排名越靠前（适用于耗时、扣分、故障数等“越小越好”的数据）" << endl;
    cout << u8"（直接按回车使用默认的降序）" << endl;

    string line;
    getline(cin, line);

    // 去掉首尾空白，方便直接回车或误输空格
    while (!line.empty() && (line.front() == ' ' || line.front() == '\t')) line.erase(line.begin());
    while (!line.empty() && (line.back() == ' ' || line.back() == '\t')) line.pop_back();

    if (line.empty() || line == "1") {
        g_descending = true;
    } else if (line == "2") {
        g_descending = false;
    } else {
        cout << u8"输入无效，已使用默认的降序。" << endl;
        g_descending = true;
    }

    cout << u8"当前排序方向：" << order_name() << endl;

    // 升序时第 1 档会落在数值最小的一批，这里明确说清楚，避免看错
    if (!g_descending) {
        cout << u8"注意：升序下第 1 档对应数值最小的一批记录。" << endl;
    }
}

// 主函数，判断身份，操作还有查询
int main() {

    // UTF-8 的一些事
    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
    #endif

    // ==================== 自定义数据文件名 ====================
    init_data_file();

    // ==================== 选择排序方向 ====================
    // 放在选身份之前，管理员与一般用户共用同一个方向；
    // 管理员后续还能用菜单"4. 切换排序方向"再改。
    choose_sort_order();

    short int user_kind = 0;

    // 条件一直循环，输错了可以重复输入
    while (true) {
        cout << u8"请输入你的身份（序号）：" << endl << u8"1.管理员（老师等）" << endl;
        cout << u8"2.一般用户（家长等）" << endl;
        cin >> user_kind;

        // EOF（Ctrl+Z / Ctrl+D）时显式退出
        if (!cin) {
            if (cin.eof()) {
                cout << u8"检测到输入结束，程序将退出。" << endl;
                break;
            }
            cout << u8"输入无效，请重新输入！" << endl;
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }

        if (user_kind == 1) {          // admin的(1)
            admin();
            break;
        }
        else if (user_kind == 2) {     // user的(1)
            user();
            break;
        }
        else {
            cout << u8"输入无效，请重新输入！" << endl;
        }
    }

    // 在exe运行后不会立即关闭，方便复制，对照之类的
    over();

    return 0;
}

// =====================================================
// 西郊有密林，助君出重围。     ————《郊眠寺》
// =====================================================
// 赵灿作品 BY:Fufack（Fufack@outlook.com）
// =====================================================
