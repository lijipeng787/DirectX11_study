#pragma once

#include <iostream>
#include <string>

// 统一的错误处理和日志系统
namespace Logger {
// 日志级别
enum class Level { Error, Warning, Info, Debug };

// 设置模块名称（用于日志前缀）
void SetModule(const std::string &module_name);

// 获取当前模块名称
const std::string &GetModule();

// 日志函数（窄字符串）
void Log(Level level, const std::string &message);
void LogError(const std::string &message);
void LogWarning(const std::string &message);
void LogInfo(const std::string &message);
void LogDebug(const std::string &message);

// 日志函数（宽字符串）
void Log(Level level, const std::wstring &message);
void LogError(const std::wstring &message);
void LogWarning(const std::wstring &message);
void LogInfo(const std::wstring &message);
void LogDebug(const std::wstring &message);

// 便捷宏，自动设置模块名称
#define LOG_ERROR(msg) Logger::SetModule(__FILE__); Logger::LogError(msg)
#define LOG_WARNING(msg) Logger::SetModule(__FILE__); Logger::LogWarning(msg)
#define LOG_INFO(msg) Logger::SetModule(__FILE__); Logger::LogInfo(msg)
#define LOG_DEBUG(msg) Logger::SetModule(__FILE__); Logger::LogDebug(msg)

} // namespace Logger

