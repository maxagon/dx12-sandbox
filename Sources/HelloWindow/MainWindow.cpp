#include <memory>

#include <SDL.h>
#include <DebugCheck.h>
#include <d3d12.h>

#include "WindowDX12.h"
#include "CommandAllocatorPool.h"

int SDL_main(int argc, char *argv[])
{
    DCHECK(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) == 0);

    constexpr int32_t screenSizeX = 1024;
    constexpr int32_t screenSizeY = 768;
    SDL_Window* window = SDL_CreateWindow(
        "hello",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        screenSizeX, screenSizeY,
        SDL_WINDOW_SHOWN
        );
    DCHECK(window);


    CComPtr<ID3D12Device> device;
    DCHECK_COM(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));

    static constexpr uint32_t backbufferCount = 2;
    auto dx12Window = std::make_shared<WindowDX12>(GetActiveWindow(), device, backbufferCount);
    auto allocatorPool = std::make_shared<CommandAllocatorPool>(device, backbufferCount);

    while (true)
    {
        SDL_Event sdlEvent{};
        if (SDL_PollEvent(&sdlEvent))
        {
            if (sdlEvent.type == SDL_QUIT)
            {
                break;
            }
        }
        dx12Window->WaitForNextFrame();

        CComPtr<ID3D12Device4> device4;
        DCHECK_COM(device->QueryInterface(IID_PPV_ARGS(&device4)));

        ID3D12CommandAllocator* cmdAllocForFrame = allocatorPool->GetAllocatorForFrame(dx12Window->GetCurrentFrameIndex());

        CComPtr<ID3D12GraphicsCommandList> cmdList;
        device4->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&cmdList));
        cmdList->Reset(cmdAllocForFrame, nullptr);

        cmdList->RSSetScissorRects(1, &dx12Window->GetWindowRect());
        cmdList->RSSetViewports(1, &dx12Window->GetViewport());
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = dx12Window->GetCurrentRtvHandle();
        cmdList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);
        float clearRGB[] = { 1.0f, 0.3f, 0.1f, 1.0f };
        cmdList->ClearRenderTargetView(rtvHandle, clearRGB, 0, nullptr);
        DCHECK_COM(cmdList->Close());

        ID3D12CommandList* commandLists[] = { cmdList.p };
        dx12Window->GetRenderQueue()->ExecuteCommandLists(1, commandLists);

        dx12Window->SubmitNextFrame();
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
