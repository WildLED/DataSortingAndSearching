// ============================================================
// 模块：跨平台文件工具（实现）
// ============================================================
#include "file_utils.h"
#include "encoding.h"

#include <cstdlib>   // std::remove / std::rename / realpath（非 Windows）

#ifdef _WIN32
    #include <windows.h>
#else
    #include <climits>
    #include <unistd.h>   // getcwd
#endif

// ==================== 构造：打开底层 FILE* ====================
FileStreamBuf::FileStreamBuf(const std::string& path, bool for_write)
    : fp_(nullptr), for_write_(for_write) {
    #ifdef _WIN32
        std::wstring w = utf8_to_wide(path);
        if (!w.empty()) fp_ = _wfopen(w.c_str(), for_write ? L"w" : L"r");
    #else
        fp_ = fopen(path.c_str(), for_write ? "w" : "r");
    #endif
    // 读缓冲初始为空，第一次读时再真正取数据
    setg(gbuf_, gbuf_, gbuf_);
}

// ==================== 显式关闭 ====================
void FileStreamBuf::close() {
    if (!fp_) return;
    if (for_write_) fflush(fp_);   // 读流不刷，避免对只读 FILE* 调 fflush 的 UB
    fclose(fp_);
    fp_ = nullptr;                 // 置空，重复调用与 is_open() 都不会出错
}

// ==================== 读：缓冲区空了就从文件补一批 ====================
FileStreamBuf::int_type FileStreamBuf::underflow() {
    if (!fp_) return traits_type::eof();
    if (gptr() < egptr()) return traits_type::to_int_type(*gptr());
    size_t got = fread(gbuf_, 1, sizeof(gbuf_), fp_);
    if (got == 0) return traits_type::eof();
    setg(gbuf_, gbuf_, gbuf_ + got);
    return traits_type::to_int_type(*gptr());
}

// ==================== 写：单字符 ====================
FileStreamBuf::int_type FileStreamBuf::overflow(int_type c) {
    if (!fp_) return traits_type::eof();
    if (c != traits_type::eof()) {
        if (fputc((char)traits_type::to_char_type(c), fp_) == EOF)
            return traits_type::eof();
    }
    return c;
}

// ==================== 写：整块 ====================
std::streamsize FileStreamBuf::xsputn(const char* s, std::streamsize n) {
    if (!fp_ || n <= 0) return 0;
    return (std::streamsize)fwrite(s, 1, (size_t)n, fp_);
}

// ==================== 刷盘 ====================
int FileStreamBuf::sync() {
    return (fp_ && fflush(fp_) == 0) ? 0 : -1;
}

// ==================== 定位 ====================
FileStreamBuf::pos_type FileStreamBuf::seekoff(off_type off, std::ios_base::seekdir dir,
                                              std::ios_base::openmode which) {
    (void)which;
    if (!fp_) return pos_type(off_type(-1));

    if (dir == std::ios_base::beg) {
        if (fseek(fp_, (long)off, SEEK_SET) != 0) return pos_type(off_type(-1));
    } else if (dir == std::ios_base::cur) {
        off_type adjust = off - (off_type)(egptr() - gptr());
        if (fseek(fp_, (long)adjust, SEEK_CUR) != 0) return pos_type(off_type(-1));
    } else {
        if (fseek(fp_, (long)off, SEEK_END) != 0) return pos_type(off_type(-1));
    }

    setg(gbuf_, gbuf_, gbuf_);   // 定位后原有缓冲作废
    long pos = ftell(fp_);
    return (pos < 0) ? pos_type(off_type(-1)) : pos_type(off_type(pos));
}

FileStreamBuf::pos_type FileStreamBuf::seekpos(pos_type pos, std::ios_base::openmode which) {
    return seekoff(off_type(pos), std::ios_base::beg, which);
}

// ==================== 删除 / 改名 / 绝对路径 ====================
bool remove_file(const std::string& path) {
    #ifdef _WIN32
        return _wremove(utf8_to_wide(path).c_str()) == 0;
    #else
        return std::remove(path.c_str()) == 0;
    #endif
}

bool rename_file(const std::string& from, const std::string& to) {
    #ifdef _WIN32
        return _wrename(utf8_to_wide(from).c_str(), utf8_to_wide(to).c_str()) == 0;
    #else
        return std::rename(from.c_str(), to.c_str()) == 0;
    #endif
}

std::string get_absolute_path(const std::string& path) {
    #ifdef _WIN32
        std::wstring wpath = utf8_to_wide(path);
        if (wpath.empty()) return "";

        wchar_t buffer[MAX_PATH];
        DWORD ret = GetFullPathNameW(wpath.c_str(), MAX_PATH, buffer, nullptr);
        if (ret == 0) return "";

        if (ret <= MAX_PATH) return wide_to_utf8(std::wstring(buffer, (size_t)ret));

        std::wstring big((size_t)ret, L'\0');
        ret = GetFullPathNameW(wpath.c_str(), (DWORD)big.size(), &big[0], nullptr);
        if (ret == 0 || ret > big.size()) return "";
        return wide_to_utf8(std::wstring(big, (size_t)ret));
    #else
        char resolved[PATH_MAX];
        if (realpath(path.c_str(), resolved) != nullptr) {
            return std::string(resolved);
        }
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != nullptr) {
            return std::string(cwd) + "/" + path;
        }
        return "";
    #endif
}
