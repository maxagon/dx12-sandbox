#include "WindowDX12.h"

#include <DebugCheck.h>

WindowDX12::WindowDX12(HWND windowHandle, CComPtr<ID3D12Device> device, uint32_t backbufferCount)
    : mBackbufferCount(backbufferCount)
    , mWindowHandle(windowHandle)
    , mFenceValues(backbufferCount)
    , mFrameIndex(0)
{
    D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
    commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    DCHECK_COM(device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&mRenderQueue)));

    CComPtr<IDXGIFactory3> dxgiFactory;
    DCHECK_COM(CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory)));

    DXGI_SWAP_CHAIN_DESC swapChainDesc{};
    swapChainDesc.BufferCount = backbufferCount;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.Windowed = true;
    swapChainDesc.OutputWindow = mWindowHandle;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    CComPtr<IDXGISwapChain> swapChain1;
    DCHECK_COM(dxgiFactory->CreateSwapChain(mRenderQueue.p, &swapChainDesc, &swapChain1));
    DCHECK_COM(swapChain1->QueryInterface(IID_PPV_ARGS(&mSwapChain)));

    ResizeBackbuffer();

    mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();
    device->CreateFence(mFenceValues[mFrameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFrameFence));
    mFenceValues[mFrameIndex]++;
    mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

}

WindowDX12::~WindowDX12()
{
    CloseHandle(mFenceEvent);
}

ID3D12Resource* WindowDX12::GetCurrentBackbuffer()
{
    ID3D12Resource* backBuffer = nullptr;
    DCHECK_COM(mSwapChain->GetBuffer(mSwapChain->GetCurrentBackBufferIndex(), IID_PPV_ARGS(&backBuffer)));
    return backBuffer;
}

void WindowDX12::SubmitNextFrame()
{
    DCHECK_COM(mSwapChain->Present(1, 0));
    mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();
    uint64_t fenceValue = mFenceValues[mFrameIndex];
    mRenderQueue->Signal(mFrameFence, fenceValue);
    if (mFrameFence->GetCompletedValue() < fenceValue)
    {
        DCHECK_COM(mFrameFence->SetEventOnCompletion(fenceValue, mFenceEvent));
        WaitForSingleObjectEx(mFenceEvent, INFINITE, FALSE);
    }
    mFenceValues[mFrameIndex] = fenceValue + 1;
}

void WindowDX12::ResizeBackbuffer()
{
    RECT windowRect{};
    GetClientRect(mWindowHandle, &windowRect);
    UINT sizeX = windowRect.right;
    UINT sizeY = windowRect.left;
    mSwapChain->ResizeBuffers(mBackbufferCount, sizeX, sizeY, DXGI_FORMAT_UNKNOWN, 0);
}
