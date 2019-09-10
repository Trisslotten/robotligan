#pragma once
#define WIN32_LEAN_MEAN
#ifndef MEMORYINFO_H
#define MEMORYINFO_H
#include <windows.h>
#include <Psapi.h>
#include <atlbase.h>
#include <dxgi1_4.h>
#include <iostream>
namespace util {
enum {
  KB = 10,
  MB = 20,
  GB = 30,
};
class memoryinfo {
 public:
  static memoryinfo& getInstance() {
    static memoryinfo instance;
    return instance;
  }
  unsigned getRamUsage(unsigned short suffix = MB) {
    GetProcessMemoryInfo(GetCurrentProcess(), &PMC_, sizeof(PMC_));
    return (unsigned)(PMC_.WorkingSetSize >> suffix);
  }
  int getUsedVRAM(unsigned short suffix = MB) {
    if (Curr_Adapter_ || Adapters_) {
      Dx_factory_->Release();
      Adapters_->Release();
    }
    Curr_Adapter_ = nullptr;
    Adapters_ = nullptr;
    Dx_factory_->EnumAdapters1(0, &Curr_Adapter_);
    DXGI_ADAPTER_DESC1 desc;
    Curr_Adapter_->GetDesc1(&desc);
    if (!Adapters_ && desc.Flags == 0) {
      Curr_Adapter_->QueryInterface(IID_PPV_ARGS(&Adapters_));
      DXGI_QUERY_VIDEO_MEMORY_INFO info = {};
      Adapters_->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL,
                                      &info);
      return (int)(info.CurrentUsage >> suffix);
    } else {
      return -1;
    }
  }
  int getVRAMAmount(unsigned short suffix = MB) {
    if (Curr_Adapter_ || Adapters_) {
      Dx_factory_->Release();
      Adapters_->Release();
    }
    Curr_Adapter_ = nullptr;
    Adapters_ = nullptr;
    Dx_factory_->EnumAdapters1(0, &Curr_Adapter_);
    DXGI_ADAPTER_DESC1 desc;
    Curr_Adapter_->GetDesc1(&desc);
    if (!Adapters_ && desc.Flags == 0) {
      Curr_Adapter_->QueryInterface(IID_PPV_ARGS(&Adapters_));
      DXGI_QUERY_VIDEO_MEMORY_INFO info = {};
      Adapters_->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL,
                                      &info);
      return (int)(info.Budget >> suffix);
    } else {
      return -1;
    }
  }

 private:
  memoryinfo() { CreateDXGIFactory1(IID_PPV_ARGS(&Dx_factory_)); }
  ~memoryinfo() {
    Dx_factory_->Release();
    Adapters_->Release();
  }
  memoryinfo(const memoryinfo&) = delete;
  memoryinfo& operator=(const memoryinfo&) = delete;
  memoryinfo(memoryinfo&&) = delete;
  memoryinfo& operator=(memoryinfo&&) = delete;

  IDXGIFactory4* Dx_factory_ = nullptr;
  IDXGIAdapter3* Adapters_ = nullptr;
  IDXGIAdapter1* Curr_Adapter_ = nullptr;
  PROCESS_MEMORY_COUNTERS PMC_ = {};
};
}  // namespace util
#endif  // !MEMORYINFO_H