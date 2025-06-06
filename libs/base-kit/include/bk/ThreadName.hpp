#pragma once

#include <string>

#if defined(_WIN32)
#include <windows.h>
#include <processthreadsapi.h>
#elif defined(__APPLE__) || defined(__linux__)
#include <pthread.h>
#include <array>
#endif

inline auto getCurrentThreadName() -> std::string {
#if defined(_WIN32)
  wchar_t name[64] = {};
  HRESULT hr = GetThreadDescription(GetCurrentThread(), &name);
  if (SUCCEEDED(hr) && name) {
    std::wstring wname{name};
    LocalFree(name);
    return std::string(wname.begin(), wname.end());
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
