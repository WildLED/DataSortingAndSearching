// ============================================================
// 模块：显示输出（实现）
// ============================================================
#include "display.h"
#include "config.h"

#include <iostream>

using namespace std;

#ifndef _WIN32
    #include <termios.h>
    #include <unistd.h>
#else
    #include <conio.h>   // Windows上用_getch()
#endif

void over() {
    cout << u8"————————————————————————————————————————————" << endl;
    cout << u8"按任意键以结束程序…………" << endl;

    #ifdef _WIN32
        _getch();
    #else
        // 设置终端为非规范模式，读取一个字符，实现"按任意键结束"
        struct termios old_tio, new_tio;
        tcgetattr(STDIN_FILENO, &old_tio);
        new_tio = old_tio;
        new_tio.c_lflag &= (~ICANON & ~ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
        getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
    #endif
}

void print_header(int kinds) {
    cout << u8"排名|姓名（或编号）";
    if (kinds <= 30) {
        for (int i = 1; i <= kinds; i++) {
            cout << u8"|数据" << i;
        }
        cout << u8"|总计";
        cout << "\n";
    } else {
        // if可以避免输出过长的标头，好看点>^<
        cout << u8"|数据1|数据2|数据3|数据4|……|数据" << kinds << u8"|总计";
        cout << endl;
    }
}

void print_header_prompt(int kinds) {
    if (kinds <= 30) {
        for (int i = 1; i <= kinds; i++) {
            cout << u8"|数据" << i;
        }
        cout << "\n";
    } else {
        cout << u8"|数据1|数据2|数据3|数据4|……|数据" << kinds;
        cout << endl;
    }
}

void print_data_simply(const vector<Box>& students) {
    if (students.empty()) {
        cout << u8"未查到数据";
        return;
    }

    int kinds = (int)students[0].scores.size();

    // 输出排序结果
    cout << u8"结果如下：" << endl;
    cout << u8"（排序方向：" << order_name() << u8"）" << endl;
    print_header(kinds);

    for (size_t i = 0; i < students.size(); i++) {
        cout << i + 1 << " " << students[i].id << " ";
        for (long long score : students[i].scores) {
            cout << score << " ";
        }
        cout << students[i].total;

        // 溢出明确标出来
        if (students[i].overflow) cout << u8"  [总计溢出，请检查该项数据]";
        cout << endl;
    }
}
