#pragma once

#include <d3d12.h>
#include <d3dx12.h>
#include <atlbase.h>
#include <DebugCheck.h>
#include <CommandAllocatorPool.h>

struct ICmdListSubRecorder
{
    virtual ~ICmdListSubRecorder() {}
    virtual void Record(ID3D12GraphicsCommandList* destination) = 0;
};

struct ICmdListRecorder
{
    virtual ~ICmdListRecorder() {}
    virtual CComPtr<ID3D12GraphicsCommandList> Record(ID3D12CommandAllocator* allocator) = 0;
};

class CmdListScheduler
{
public:
    CmdListScheduler(CComPtr<ID3D12Device> device, CComPtr<ID3D12CommandQueue> queue, uint64_t numFrames)
        : mNumFrames(numFrames)
        , mDevice(device)
        , mQueue(queue)
        , mAllocatorPool(device, numFrames)
        , mFrameIndex(0)
    {

    }
    void NewFrame(uint64_t frameIndex)
    {
        mFrameIndex = frameIndex;
    }
    void Submit(ICmdListRecorder* recorder)
    {
        ID3D12CommandAllocator* allocator = mAllocatorPool.GetAllocatorForFrame(mFrameIndex);
        CComPtr<ID3D12GraphicsCommandList> cmdList = recorder->Record(allocator);
        if (cmdList)
        {
            ID3D12CommandList* cmdLists[] = { cmdList };
            mQueue->ExecuteCommandLists(1, cmdLists);
        }
    }
private:
    CComPtr<ID3D12Device> mDevice;
    CComPtr<ID3D12CommandQueue> mQueue;
    CommandAllocatorPool mAllocatorPool;
    uint64_t mNumFrames;
    uint64_t mFrameIndex;
};

class CmdListRecorder : public ICmdListRecorder
{
private:
    CComPtr<ID3D12Device> mDevice;
    std::vector<ICmdListSubRecorder*> mRecorders;
public:
    CmdListRecorder(CComPtr<ID3D12Device> device, const std::vector<ICmdListSubRecorder*>& recorders)
        : mRecorders(recorders)
        , mDevice(device)
    {}
    // ICmdListRecorder
    CComPtr<ID3D12GraphicsCommandList> Record(ID3D12CommandAllocator* allocator) override
    {
        CComPtr<ID3D12GraphicsCommandList> cmdList;
        DCHECK_COM(mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator, nullptr, IID_PPV_ARGS(&cmdList)));
        for (ICmdListSubRecorder* recorder : mRecorders)
        {
            recorder->Record(cmdList);
        }
        cmdList->Close();
        return cmdList;
    }
};

template<typename T>
class LambdaSubRecorder : public ICmdListSubRecorder
{
public:
    LambdaSubRecorder(T func)
        : mFunc(func)
    {}
    //ICmdListSubRecorder
    void Record(ID3D12GraphicsCommandList* destination)
    {
        mFunc(destination);
    }
private:
    T mFunc;
};
