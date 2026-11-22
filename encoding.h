// ============================================================
// 模块：编码转换（UTF-8 <-> UTF-16，仅 Windows 需要）
// 本程序全程用 UTF-8 表示字符串（含中文文件名），但 Windows 内部路径是 UTF-16。
// ANSI 版 API 会按系统代码页（简体中文为 GBK）解释 UTF-8 字节，
// 导致路径乱码、中文文件打不开，因此 Windows 下一律先转 UTF-16 走宽字符 API。
// ============================================================
#ifndef ENCODING_H
#define ENCODING_H

#include <string>

#ifdef _WIN32
    std::wstring utf8_to_wide(const std::string& s);
    std::string  wide_to_utf8(const std::wstring& w);
#endif

#endif // ENCODING_H
