// ============================================================
// 模块：配置（全局数据文件名 / 排序方向 / 管理员口令开关）
// 项目：Data Sorting and Searching（数据排序与查找）
// 作者：赵灿（Fufack@outlook.com）  指导老师：李怀伦
// Encoding: UTF-8(No BOM)   C++最低支持版本：C++11
// ============================================================
#ifndef CONFIG_H
#define CONFIG_H

#include <string>

// ==================== 管理员口令（默认关闭） ====================
// 口令保护会改变既有交互流程（每次启动都要多输一次），因此默认不启用。
// 需要时把下面的 0 改成 1 并修改口令字符串即可，其余代码无需改动。
#define ADMIN_PASSWORD_ENABLE 0
#if ADMIN_PASSWORD_ENABLE
    extern const char* ADMIN_PASSWORD;   // 示例口令，请自行修改
#endif

// ==================== 全局数据文件名 ====================
// 默认是 programming.txt，程序启动时可自定义
extern std::string data_file;

// ==================== 排序方向 ====================
// true  = 降序：数值越大越靠前（成绩等"越大越好"）
// false = 升序：数值越小越靠前（耗时、扣分、故障数等"越小越好"）
// 只影响排名口径，不写进数据文件，避免改变文件格式导致旧文件不兼容。
extern bool g_descending;

// 把当前排序方向转成中文，用于各处提示
const char* order_name();

#endif // CONFIG_H
