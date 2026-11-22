// ============================================================
// 模块：配置（实现）
// ============================================================
#include "config.h"

std::string data_file = "programming.txt";
bool g_descending = true;

#if ADMIN_PASSWORD_ENABLE
    const char* ADMIN_PASSWORD = "123456";   // 示例口令，请自行修改
#endif

const char* order_name() {
    return g_descending ? u8"降序（数值越大越靠前）" : u8"升序（数值越小越靠前）";
}
