#include <memory>

#include <SDL.h>
#include <DebugCheck.h>
#include <d3d12.h>

#include "WindowDX12.h"

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

    auto dx12Window = std::make_shared<WindowDX12>(GetActiveWindow(), device, 4);

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
        dx12Window->SubmitNextFrame();
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
