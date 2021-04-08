#pragma once

#include <memory>

#include <d3d12.h>
#include <d3dx12.h>
#include <atlbase.h>
#include <DebugCheck.h>

struct IResourceUploader
{
    virtual ~IResourceUploader() {}
    virtual void UploadImmidiate(ID3D12Resource* uploadDst, void* srcData, size_t srcDataSize) = 0;
};

static CComPtr<ID3D12Resource> CreateUploadResource(ID3D12Device* device, void* data, uint64_t size)
{
    auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(size);
    CComPtr<ID3D12Resource> resource;
    DCHECK_COM(device->CreateCommittedResource(
        &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource
    )));

    void* resourceData = nullptr;
    DCHECK_COM(resource->Map(0, nullptr, &resourceData));
    memcpy(resourceData, data, size);
    resource->Unmap(0, nullptr);
    return resource;
}

class SimpleResourceUploader : public IResourceUploader
{
private:
    CComPtr<ID3D12CommandQueue> mCopyQueue;
    CComPtr<ID3D12CommandAllocator> mCmdAllocator;
    CComPtr<ID3D12GraphicsCommandList> mCmdList;
    CComPtr<ID3D12Fence> mFence;
    uint64_t mFenceValue;
    CComPtr<ID3D12Device4> mDevice;
    HANDLE mEvent;
public:
    SimpleResourceUploader(CComPtr<ID3D12Device4> device)
        : mDevice(device)
        , mFenceValue(0)
    {
        D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
        commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
        DCHECK_COM(mDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&mCopyQueue)));
        DCHECK_COM(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&mCmdAllocator)));
        DCHECK_COM(mDevice->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_COPY, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&mCmdList)));
        DCHECK_COM(mDevice->CreateFence(mFenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));
        mEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    }
    void UploadImmidiate(ID3D12Resource* uploadDst, void* srcData, size_t srcDataSize) override
    {
        auto uploadResource = CreateUploadResource(mDevice, srcData, srcDataSize);
        mCmdList->Reset(mCmdAllocator, nullptr);
        mCmdList->CopyResource(uploadDst, uploadResource);
        DCHECK_COM(mCmdList->Close());
        ID3D12CommandList* uploadLists[] = { mCmdList.p };
        mCopyQueue->ExecuteCommandLists(_countof(uploadLists), uploadLists);
        DCHECK_COM(mCopyQueue->Signal(mFence, ++mFenceValue));

        DCHECK_COM(mFence->SetEventOnCompletion(mFenceValue, mEvent));
        WaitForSingleObject(mEvent, INFINITE);
        DCHECK_COM(mCmdAllocator->Reset());
    }
    ~SimpleResourceUploader()
    {
        CloseHandle(mEvent);
    }
};