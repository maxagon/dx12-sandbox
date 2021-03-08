#include "CommandAllocatorPool.h"

#include <DebugCheck.h>

CommandAllocatorPool::CommandAllocatorPool(CComPtr<ID3D12Device> device, uint32_t backbufferCount)
    : mAllocators(backbufferCount)
{
    for (size_t allocIndex = 0; allocIndex < mAllocators.size(); ++allocIndex)
    {
        DCHECK_COM(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mAllocators[allocIndex])));
    }
}

ID3D12CommandAllocator* CommandAllocatorPool::GetAllocatorForFrame(uint32_t frameIndex)
{
    DCHECK_COM(mAllocators[frameIndex]->Reset());
    return mAllocators[frameIndex];
}
