#include <SDL.h>

#include <DebugCheck.h>

int main(int argc, char *argv[])
{
    DCHECK(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) == 0);

    constexpr int32_t screenSizeX = 1024;
    constexpr int32_t screenSizeY = 768;
    SDL_Window* window = SDL_CreateWindow(
        "hello_sdl2",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        screenSizeX, screenSizeY,
        SDL_WINDOW_SHOWN
        );
    DCHECK(window);

    SDL_Surface* screenSurface = SDL_GetWindowSurface(window);

    SDL_FillRect(screenSurface, nullptr, SDL_MapRGB(screenSurface->format, 0xFF, 0x00, 0x0));
    SDL_UpdateWindowSurface(window);
    SDL_Delay(2000);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
