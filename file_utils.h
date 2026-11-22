// ============================================================
// 模块：跨平台文件工具
//   - FileStreamBuf：自实现 streambuf（Windows _wfopen / 其他 fopen），
//     解决中文路径问题，对外仍可用 >> << 语法。仅标准 C++11，无编译器扩展。
//   - remove_file / rename_file / get_absolute_path：宽字符版封装。
// ============================================================
#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <string>
#include <streambuf>
#include <cstdio>

class FileStreamBuf : public std::streambuf {
    FILE* fp_;
    bool  for_write_;    // 记住打开方式，供 close() / 析构时判断要不要 fflush
    char  gbuf_[4096];   // 读缓冲区

public:
    explicit FileStreamBuf(const std::string& path, bool for_write);
    ~FileStreamBuf() { close(); }

    // 显式关闭：删除/改名之前必须调用，否则 Windows 下无法操作已打开的文件。可重复调用。
    void close();
    bool is_open() const { return fp_ != nullptr; }

protected:
    int_type underflow() override;
    int_type overflow(int_type c) override;
    std::streamsize xsputn(const char* s, std::streamsize n) override;
    int sync() override;
    pos_type seekoff(off_type off, std::ios_base::seekdir dir,
                     std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override;
    pos_type seekpos(pos_type pos, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override;
};

// 删除文件（替代 std::remove 的文件版本）
bool remove_file(const std::string& path);
// 重命名文件（替代 std::rename）
bool rename_file(const std::string& from, const std::string& to);
// 返回文件绝对路径，用于提示用户导入位置
std::string get_absolute_path(const std::string& path);

#endif // FILE_UTILS_H
