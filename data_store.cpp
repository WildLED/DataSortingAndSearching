// ============================================================
// 模块：数据存储（实现）
// ============================================================
#include "data_store.h"
#include "config.h"
#include "file_utils.h"

#include <iostream>
#include <ostream>
#include <sstream>
#include <algorithm>   // std::sort

using namespace std;

bool save_data(const vector<Box>& students) {
    size_t n_size = students.size();
    int n = (int)n_size;

    // 判断错误情况
    if (n == 0) {
        return false;
    }

    int kinds = (int)students[0].scores.size();

    // 各条记录的种类数应当一致，不一致时给出提示（仍以第一条为准，与原有行为相同）
    for (size_t i = 1; i < students.size(); i++) {
        if ((int)students[i].scores.size() != kinds) {
            cout << u8"提示：第 " << i + 1 << u8" 条记录的数据项数与其他记录不一致，"
                 << u8"保存时将按 " << kinds << u8" 项处理，请核对。" << endl;
            break;
        }
    }

    const string tmp_path = data_file + ".tmp";
    const string bak_path = data_file + ".bak";

    // 先写临时文件
    {
        FileStreamBuf out_buf(tmp_path, true);
        ostream out_txt_file(&out_buf);

        if (!out_buf.is_open()) {
            cout << u8"保存失败：无法创建临时文件 " << tmp_path << u8"，原数据未被改动。" << endl;
            return false;
        }

        // UTF-8（无 BOM）：不再写入 BOM 头，避免某些场景下误判编码
        // 注：load_data() 中对 BOM 的检测/回退逻辑保留，可兼容含 BOM 的旧文件

        out_txt_file << n << " " << kinds << endl;

        for (int i = 0; i < n; i++) {
            out_txt_file << i + 1 << " ";
            out_txt_file << students[i].id << " ";
            for (long long score : students[i].scores) {
                out_txt_file << score << " ";
            }
            out_txt_file << students[i].total;
            out_txt_file << endl;

            // 写入过程中出错：丢弃临时文件，原数据保持不变
            if (!out_txt_file) {
                out_txt_file.flush();
                out_buf.close();   // 删除前先关闭句柄
                remove_file(tmp_path);
                cout << u8"保存失败：写入临时文件时出错，原数据未被改动。" << endl;
                return false;
            }
        }

        // 刷盘并关闭：确保后面改名 .bak / 正式替换时临时文件已彻底写完、句柄已释放
        out_txt_file.flush();
        out_buf.close();
    }

    // 备份上一版
    remove_file(bak_path);
    bool have_bak = rename_file(data_file, bak_path);

    // 原子替换
    if (!rename_file(tmp_path, data_file)) {
        // 替换失败：把备份恢复回去，保证文件还在
        if (have_bak) rename_file(bak_path, data_file);
        remove_file(tmp_path);
        cout << u8"保存失败：无法替换原文件，已恢复原数据。" << endl;
        return false;
    }

    cout << u8"已保存到 " << get_absolute_path(data_file);
    if (have_bak) cout << u8"（上一版备份为 " << bak_path << u8"）";
    cout << endl;

    return true;
}

vector<Box> load_data(bool strict, bool* ok) {
    vector<Box> students;
    if (ok) *ok = false;

    FileStreamBuf in_buf(data_file, false);
    istream in(&in_buf);

    // 检查文件是否成功打开
    if (!in_buf.is_open()) {
        return students;
    }

    // ==================== 首行：n 与 kinds ====================
    string header_line;
    if (!getline(in, header_line)) {
        // 空文件（或首行不存在）：视为无数据，不算错误
        if (ok) *ok = true;
        return students;
    }

    // UTF-8的一些事：BOM（EF BB BF）只可能出现在文件最开头，直接在首行上去掉
    if (header_line.size() >= 3 &&
        (unsigned char)header_line[0] == 0xEF &&
        (unsigned char)header_line[1] == 0xBB &&
        (unsigned char)header_line[2] == 0xBF) {
        header_line.erase(0, 3);
    }

    // 去掉行尾可能的 \r（CRLF 时由文本模式转换，这里再兜一层）
    if (!header_line.empty() && header_line.back() == '\r') header_line.pop_back();

    int n = 0, kinds = 0;
    {
        istringstream hss(header_line);
        string extra;
        // 首行必须恰好是 n 与 kinds 两个数：读不出、有多余内容、或不是正数都视为无数据
        if (!(hss >> n >> kinds) || (hss >> extra) || n <= 0 || kinds <= 0) {
            if (ok) *ok = true;
            return students;
        }
    }

    bool broken = false;

    // 本行应有的字段数：序号 + 姓名 + kinds 项数据 + 总计
    const int expect_fields = kinds + 3;

    for (int i = 0; i < n; i++) {
        string line;

        if (!getline(in, line)) {
            cout << u8"提示：数据文件在第 " << i + 1 << u8" 条记录处提前结束，"
                 << u8"开头声明 " << n << u8" 条，实际只读到 " << i << u8" 条。" << endl;
            broken = true;
            if (strict) students.clear();   // 严格模式：残缺即空
            break;
        }

        if (!line.empty() && line.back() == '\r') line.pop_back();

        // 统计这一行实际有几个字段（只用于报错时告诉管理员差了多少）
        int actual_fields = 0;
        {
            istringstream counter(line);
            string token;
            while (counter >> token) actual_fields++;
        }

        // 行内解析：序号、姓名、kinds 项数据、总计
        istringstream lss(line);

        int temp;
        string id;
        vector<long long> scores;
        long long temp_total = 0;   // temp吞序号，temp_total吞总分（用不上）

        bool line_bad = false;

        if (!(lss >> temp >> id)) line_bad = true;

        if (!line_bad) {
            for (int j = 0; j < kinds; j++) {
                long long score;
                if (!(lss >> score)) { line_bad = true; break; }
                scores.push_back(score);
            }
        }

        if (!line_bad && !(lss >> temp_total)) line_bad = true;

        // 解析完还要确认这一行刚好用完，行尾不能还剩下东西
        if (!line_bad) {
            string extra;
            if (lss >> extra) line_bad = true;   // 多写了字段
        }

        if (line_bad) {
            cout << u8"提示：第 " << i + 1 << u8" 条记录字段数异常，应为 "
                 << expect_fields << u8" 个（序号、姓名、" << kinds
                 << u8" 项数据、总计），实际为 " << actual_fields << u8" 个。" << endl;
            cout << u8"      该文件可能已被手工修改而发生错位，后续记录已无法正确对应。" << endl;
            broken = true;
            if (strict) students.clear();   // 严格模式：错位即空，不返回错位数据
            break;
        }

        Box box(id, scores);

        // 再核验一下
        if (!box.overflow && box.total != temp_total) {
            cout << u8"提示：第 " << i + 1 << u8" 条记录（" << id << u8"）文件中的总计为 "
                 << temp_total << u8"，按各项重算应为 " << box.total << u8"，已按重算值处理。"
                 << endl;
        }

        students.push_back(box);
    }

    if (ok) *ok = !broken;

    // 加载即有序，保证后续 i+1 就是真实名次
    sort(students.begin(), students.end(), compare);

    return students;
}
