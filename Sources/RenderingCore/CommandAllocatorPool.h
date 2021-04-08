#pragma once

#include <cstdint>
#include <vector>

#include <d3d12.h>
#include <atlbase.h>
#include <DebugCheck.h>

class CommandAllocatorPool
{
private:
    CComPtr<ID3D12Device> mDevice;
    std::vector<CComPtr<ID3D12CommandAllocator>> mAllocators; 
public:
    CommandAllocatorPool(CComPtr<ID3D12Device> device, uint32_t backbufferCount)
        : mAllocators(backbufferCount)
    {
        for (size_t allocIndex = 0; allocIndex < mAllocators.size(); ++allocIndex)
        {
            DCHECK_COM(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mAllocators[allocIndex])));
        }
    }
    ID3D12CommandAllocator* GetAllocatorForFrame(uint32_t frameIndex)
    {
        DCHECK_COM(mAllocators[frameIndex]->Reset());
        return mAllocators[frameIndex];
    }
};
