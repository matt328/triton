#pragma once

#include <string>

#if defined(_WIN32)
#include <windows.h>
#include <processthreadsapi.h>
#elif defined(__APPLE__) || defined(__linux__)
#include <pthread.h>
#include <array>
#endif

#if defined(_WIN32)
inline auto wideToUtf8(const std::wstring& wstr) -> std::string {
  if (wstr.empty())
    return {};

  int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
  if (size <= 0)
    return {};

  std::string result(size - 1, '\0'); // size includes null terminator
  WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, result.data(), size, nullptr, nullptr);
  return result;
}
#endif

inline auto getCurrentThreadName() -> std::string {
#if defined(_WIN32)

  PWSTR name = nullptr;
  HRESULT hr = GetThreadDescription(GetCurrentThread(), &name);
  if (SUCCEEDED(hr) && name) {
    std::wstring wname{name};
    LocalFree(name);
    return wideToUtf8(wname);
  }
  return {};
#elif defined(__APPLE__) || defined(__linux__)
  std::array<char, 64> name{};
  pthread_getname_np(pthread_self(), name.data(), name.size());
  return std::string{name.data()};
#else
  return {};
#endif
}

inline void setCurrentThreadName(const std::string& name) {
#if defined(_WIN32)
  // Windows 10, version 1607 and above
  HRESULT hr =
      SetThreadDescription(GetCurrentThread(), std::wstring(name.begin(), name.end()).c_str());
  (void)hr; // You might want to log or assert on failure
#elif defined(__APPLE__)
  pthread_setname_np(name.c_str());
#elif defined(__linux__)
  // Linux only allows up to 16 bytes including null terminator
  pthread_setname_np(pthread_self(), name.substr(0, 15).c_str());
#endif
}
