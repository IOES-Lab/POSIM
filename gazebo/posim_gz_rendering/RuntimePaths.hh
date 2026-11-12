// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <filesystem>
#include <stdexcept>
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace posim
{
namespace
{
std::filesystem::path LibraryDirectory()
{
#ifdef _WIN32
  HMODULE module;
  char path[MAX_PATH];
  if (
    GetModuleHandleExA(
      GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
      reinterpret_cast<LPCSTR>(&LibraryDirectory), &module) &&
    GetModuleFileNameA(module, path, MAX_PATH))
  {
    return std::filesystem::path(path).parent_path();
  }
#else
  Dl_info info;
  if (dladdr(reinterpret_cast<const void *>(&LibraryDirectory), &info))
  {
    return std::filesystem::path(info.dli_fname).parent_path();
  }
#endif
  throw std::runtime_error("underwater_library_directory_unavailable");
}
}  // namespace
}  // namespace posim
