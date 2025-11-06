#pragma once

#include <iostream>
#include <string>

// Unified error handling and logging system
namespace Logger {
// Log level
enum class Level { Error, Warning, Info, Debug };

// Set module name (for log prefix)
void SetModule(const std::string &module_name);

// Get current module name
const std::string &GetModule();

// Set minimum output level (default: Warning)
// Only messages with level >= min_level will be output
void SetMinLevel(Level level);
Level GetMinLevel();

// Log functions (narrow string)
void Log(Level level, const std::string &message);
void LogError(const std::string &message);
void LogWarning(const std::string &message);
void LogInfo(const std::string &message);
void LogDebug(const std::string &message);

// Log functions (wide string)
void Log(Level level, const std::wstring &message);
void LogError(const std::wstring &message);
void LogWarning(const std::wstring &message);
void LogInfo(const std::wstring &message);
void LogDebug(const std::wstring &message);

// Convenience macros, automatically set module name
#define LOG_ERROR(msg)                                                         \
  Logger::SetModule(__FILE__);                                                 \
  Logger::LogError(msg)
#define LOG_WARNING(msg)                                                       \
  Logger::SetModule(__FILE__);                                                 \
  Logger::LogWarning(msg)
#define LOG_INFO(msg)                                                          \
  Logger::SetModule(__FILE__);                                                 \
  Logger::LogInfo(msg)
#define LOG_DEBUG(msg)                                                         \
  Logger::SetModule(__FILE__);                                                 \
  Logger::LogDebug(msg)

} // namespace Logger
