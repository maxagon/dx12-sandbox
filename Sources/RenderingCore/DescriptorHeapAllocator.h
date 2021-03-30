#pragma once

#include "BlockAllocator.h"

#include <DebugCheck.h>
#include <d3d12.h>
#include <atlbase.h>

class DescriptorHeapAllocator
{
public:
    DescriptorHeapAllocator(ID3D12Device* device, uint64_t descriptorsInBlock,
        uint64_t numOfBlocks, D3D12_DESCRIPTOR_HEAP_TYPE heapType, D3D12_DESCRIPTOR_HEAP_FLAGS heapFlags)
        : mVirtualBlockAllocator(descriptorsInBlock)
        , mVirtualBlockSize(numOfBlocks)
    {
        mHeapBlockSize = device->GetDescriptorHandleIncrementSize(heapType) * descriptorsInBlock;
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
        heapDesc.Type = heapType;
        heapDesc.Flags = heapFlags;
        heapDesc.NumDescriptors = descriptorsInBlock * numOfBlocks;
        device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mDescriptorHeap));
        mCpuHeapStart = mDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
        if (heapFlags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
        {
            mGpuHeapStart = mDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
        }
    }

    ID3D12DescriptorHeap* GetHeap() { return mDescriptorHeap.p; }

    D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(uint64_t block)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle;
        handle.ptr = mCpuHeapStart.ptr + block * mHeapBlockSize;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(uint64_t block)
    {
        DCHECK(mGpuHeapStart.ptr != 0);
        D3D12_CPU_DESCRIPTOR_HANDLE handle;
        handle.ptr = mGpuHeapStart.ptr + block * mHeapBlockSize;
    }

    bool Allocate(uint64_t numOfBlocks, uint64_t* output)
    {
        mVirtualBlockAllocator.AllocateBlocks(numOfBlocks, output);
    }

    void Deallocate(uint64_t blockStart, uint64_t numOfBlocks)
    {
        mVirtualBlockAllocator.FreeBlocks(numOfBlocks, blockStart);
    }

private:
    BlockAllocator mVirtualBlockAllocator;
    uint64_t mVirtualBlockSize;
    uint64_t mHeapBlockSize;
    CComPtr<ID3D12DescriptorHeap> mDescriptorHeap;
    D3D12_CPU_DESCRIPTOR_HANDLE mCpuHeapStart;
    D3D12_GPU_DESCRIPTOR_HANDLE mGpuHeapStart{};
};
