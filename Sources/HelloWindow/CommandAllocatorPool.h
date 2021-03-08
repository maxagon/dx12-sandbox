#pragma once

#include <cstdint>
#include <vector>

#include <d3d12.h>
#include <atlbase.h>

class CommandAllocatorPool
{
public:
    CommandAllocatorPool(CComPtr<ID3D12Device> device, uint32_t backbufferCount);
    ID3D12CommandAllocator* GetAllocatorForFrame(uint32_t frameIndex);
private:
    CComPtr<ID3D12Device> mDevice;
    std::vector<CComPtr<ID3D12CommandAllocator>> mAllocators;
};
