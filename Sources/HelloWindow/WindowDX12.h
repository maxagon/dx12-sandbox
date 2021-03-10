#pragma once

#include <cstdint>
#include <vector>

#include <windows.h>
#include <d3d12.h>
#include <atlbase.h>
#include <dxgi1_4.h>

class WindowDX12
{
public:
    WindowDX12(HWND windowHandle, CComPtr<ID3D12Device> device, uint32_t backbufferCount);
    ~WindowDX12();
    ID3D12CommandQueue* GetRenderQueue() { return mRenderQueue.p; };
    ID3D12Resource* GetCurrentBackbuffer();
    uint32_t GetCurrentFrameIndex() { return mFrameIndex; };
    void SubmitNextFrame();
    void WaitForNextFrame();
    void ResizeBackbuffer();
    D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRtvHandle();
    const D3D12_VIEWPORT& GetViewport() { return mViewport; };
    const D3D12_RECT& GetWindowRect() { return mWindowRect; };

private:
    CComPtr<ID3D12CommandQueue> mRenderQueue;
    CComPtr<IDXGISwapChain3> mSwapChain;
    HANDLE mFenceEvent;
    CComPtr<ID3D12Fence> mFrameFence;
    uint64_t mFenceValue;
    std::vector<uint64_t> mFrameFenceValues;
    uint32_t mBackbufferCount;
    HWND mWindowHandle;
    uint32_t mFrameIndex;
    CComPtr<ID3D12DescriptorHeap> mRtvHeap;
    uint32_t mRtvDescriptorSize;
    D3D12_RECT mWindowRect;
    D3D12_VIEWPORT mViewport;
};
