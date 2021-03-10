#include "WindowDX12.h"

#include <d3dx12.h>
#include <DebugCheck.h>

WindowDX12::WindowDX12(HWND windowHandle, CComPtr<ID3D12Device> device, uint32_t backbufferCount)
    : mBackbufferCount(backbufferCount)
    , mWindowHandle(windowHandle)
    , mFrameFenceValues(backbufferCount)
    , mFenceValue(0)
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
    device->CreateFence(mFenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFrameFence));
    mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

    mRtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = mBackbufferCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    DCHECK_COM(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mRtvHeap)));

    for (uint32_t frameIndex = 0; frameIndex < mBackbufferCount; ++frameIndex)
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, mRtvDescriptorSize);
        ID3D12Resource* buffer = nullptr;
        mSwapChain->GetBuffer(frameIndex, IID_PPV_ARGS(&buffer));
        device->CreateRenderTargetView(buffer, nullptr, rtvHandle);
    }
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

void WindowDX12::WaitForNextFrame()
{
    if (mFrameFence->GetCompletedValue() < mFrameFenceValues[mFrameIndex])
    {
        DCHECK_COM(mFrameFence->SetEventOnCompletion(mFrameFenceValues[mFrameIndex], mFenceEvent));
        WaitForSingleObjectEx(mFenceEvent, INFINITE, FALSE);
    }
}

void WindowDX12::SubmitNextFrame()
{
    mRenderQueue->Signal(mFrameFence, ++mFenceValue);
    mFrameFenceValues[mFrameIndex] = mFenceValue;
    DCHECK_COM(mSwapChain->Present(1, 0));
    mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();
}

void WindowDX12::ResizeBackbuffer()
{
    GetClientRect(mWindowHandle, &mWindowRect);
    UINT sizeX = mWindowRect.right;
    UINT sizeY = mWindowRect.bottom;
    mSwapChain->ResizeBuffers(mBackbufferCount, sizeX, sizeY, DXGI_FORMAT_UNKNOWN, 0);
    mViewport = CD3DX12_VIEWPORT(0.0f, 0.0f, (float)sizeX, (float)sizeY, 0.0f, 1.0f);
}

D3D12_CPU_DESCRIPTOR_HANDLE WindowDX12::GetCurrentRtvHandle()
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), mFrameIndex, mRtvDescriptorSize);
}

