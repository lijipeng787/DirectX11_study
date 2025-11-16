#include "Logger.h"

#include <iostream>
#include <sstream>

namespace Logger {
namespace {
std::string current_module_ = "[Unknown]";
Level min_level_ = Level::Warning; // Default: only Warning and Error

const char *GetLevelString(Level level) {
  switch (level) {
  case Level::Error:
    return "[ERROR]";
  case Level::Warning:
    return "[WARNING]";
  case Level::Info:
    return "[INFO]";
  case Level::Debug:
    return "[DEBUG]";
  default:
    return "[UNKNOWN]";
  }
}

std::wstring GetLevelStringW(Level level) {
  switch (level) {
  case Level::Error:
    return L"[ERROR]";
  case Level::Warning:
    return L"[WARNING]";
  case Level::Info:
    return L"[INFO]";
  case Level::Debug:
    return L"[DEBUG]";
  default:
    return L"[UNKNOWN]";
  }
}

// Extract filename from file path
std::string ExtractFileName(const std::string &filepath) {
  size_t pos = filepath.find_last_of("/\\");
  if (pos != std::string::npos) {
    return filepath.substr(pos + 1);
  }
  return filepath;
}

} // namespace

void SetModule(const std::string &module_name) {
  // If file path is passed, extract filename
  current_module_ = "[" + ExtractFileName(module_name) + "]";
}

const std::string &GetModule() { return current_module_; }

void SetMinLevel(Level level) { min_level_ = level; }

Level GetMinLevel() { return min_level_; }

void Log(Level level, const std::string &message) {
  if (level > min_level_) {
    return; // Skip messages below minimum level
  }
  std::cerr << current_module_ << " " << GetLevelString(level) << " " << message
            << std::endl;
}

void LogError(const std::string &message) { Log(Level::Error, message); }

void LogWarning(const std::string &message) { Log(Level::Warning, message); }

void LogInfo(const std::string &message) { Log(Level::Info, message); }

void LogDebug(const std::string &message) {
#ifdef _DEBUG
  Log(Level::Debug, message);
#endif
}

void Log(Level level, const std::wstring &message) {
  if (level > min_level_) {
    return; // Skip messages below minimum level
  }
  std::wcerr << current_module_.c_str() << L" " << GetLevelStringW(level)
             << L" " << message << std::endl;
}

void LogError(const std::wstring &message) { Log(Level::Error, message); }

void LogWarning(const std::wstring &message) { Log(Level::Warning, message); }

void LogInfo(const std::wstring &message) { Log(Level::Info, message); }

void LogDebug(const std::wstring &message) {
#ifdef _DEBUG
  Log(Level::Debug, message);
#endif
}

} // namespace Logger
