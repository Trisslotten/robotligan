#pragma once
#define WIN32_LEAN_MEAN
#ifndef MEMORYINFO_H
#define MEMORYINFO_H
#define NOMINMAX
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
/*
        En threadsafe singleton som låter en utvecklare debugga och optimisera
        minneshanteringen av applikationen
*/
class MemoryInfo {
 public:
  static MemoryInfo& GetInstance() {
    static MemoryInfo instance;
    return instance;
  }
  unsigned GetUsedRAM(unsigned short suffix = MB) {
    GetProcessMemoryInfo(GetCurrentProcess(), &pmc_, sizeof(pmc_));
    return (unsigned)(pmc_.WorkingSetSize >> suffix) - GetUsedVRAM();
  }
  /*
        Returnerar hur mycket VRAM processen använder sig utav
        via ett windows/directX api.
  */
  int GetUsedVRAM(unsigned short suffix = MB) {
    // Adapters innehåller alla renderings-enheter, både fysiska och virtuella
    curr_adapter_ = nullptr;
    adapters_ = nullptr;
    // Vi vet att vi enbart vill ha den första fysiska enheten, så vi skippar
    // att loopa
    dx_factory_->EnumAdapters1(0, &curr_adapter_);
    int ret = -1;
    DXGI_ADAPTER_DESC1 desc;
    curr_adapter_->GetDesc1(&desc);
    if (!adapters_ && desc.Flags == 0) {
      curr_adapter_->QueryInterface(IID_PPV_ARGS(&adapters_));
      // Få fram minnesinfo från adaptern.
      DXGI_QUERY_VIDEO_MEMORY_INFO info = {};
      adapters_->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL,
                                      &info);
      // Returnera mängden använt minne eller -1 om det går fel.
      ret = (int)(info.CurrentUsage >> suffix);
    } else {
      ret = -1;
    }
    dx_factory_->Release();
    adapters_->Release();
    return ret;
  }
  /*/
        Samma princip som ovan, fast returnera hur mycket minne som finns på
     enheten
  */
  int GetVRAMAmount(unsigned short suffix = MB) {
    curr_adapter_ = nullptr;
    adapters_ = nullptr;
    dx_factory_->EnumAdapters1(0, &curr_adapter_);
    DXGI_ADAPTER_DESC1 desc;
    curr_adapter_->GetDesc1(&desc);
    int ret = -1;
    if (!adapters_ && desc.Flags == 0) {
      curr_adapter_->QueryInterface(IID_PPV_ARGS(&adapters_));
      DXGI_QUERY_VIDEO_MEMORY_INFO info = {};
      adapters_->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL,
                                      &info);
      ret = (int)(info.Budget >> suffix);
    } else {
      ret = -1;
    }
    dx_factory_->Release();
    adapters_->Release();
    return ret;
  }

 private:
  MemoryInfo() { CreateDXGIFactory1(IID_PPV_ARGS(&dx_factory_)); }
  ~MemoryInfo() {
    dx_factory_->Release();
    adapters_->Release();
  }
  MemoryInfo(const MemoryInfo&) = delete;
  MemoryInfo& operator=(const MemoryInfo&) = delete;
  MemoryInfo(MemoryInfo&&) = delete;
  MemoryInfo& operator=(MemoryInfo&&) = delete;

  // DX-variabler för GPU information.
  IDXGIFactory4* dx_factory_ = nullptr;
  IDXGIAdapter3* adapters_ = nullptr;
  IDXGIAdapter1* curr_adapter_ = nullptr;
  PROCESS_MEMORY_COUNTERS pmc_ = {};
};
}  // namespace util
#endif  // !MEMORYINFO_H