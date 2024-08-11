#include <time.h>
#include <unistd.h>

#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <switch.h>

// some switch buttons
#define JOY_A 0
#define JOY_B 1
#define JOY_X 2
#define JOY_Y 3
#define JOY_PLUS 10
#define JOY_MINUS 11
#define JOY_LEFT 12
#define JOY_UP 13
#define JOY_RIGHT 14
#define JOY_DOWN 15

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;

Mix_Music *music = NULL;
Mix_Chunk *sounds[4] = {NULL};

SDL_Rect helloworld_rect;

SDL_Color colors[] = {
    {128, 128, 128, 0}, // gray
    {255, 255, 255, 0}, // white
    {255, 0, 0, 0},     // red
    {0, 255, 0, 0},     // green
    {0, 0, 255, 0},     // blue
    {255, 255, 0, 0},   // brown
    {0, 255, 255, 0},   // cyan
    {255, 0, 255, 0},   // purple
};

int quitGame = 0;
int trail = 0;
int wait = 25;

void handleEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
            quitGame = 1;

        // use joystick
        if (event.type == SDL_JOYBUTTONDOWN)
        {
            if (event.jbutton.button == JOY_UP)
                if (wait > 0)
                    wait--;
            if (event.jbutton.button == JOY_DOWN)
                if (wait < 100)
                    wait++;

            if (event.jbutton.button == JOY_PLUS)
                quitGame = 1;

            if (event.jbutton.button == JOY_B)
                trail = !trail;
        }
    }
}

void updateTextureText(SDL_Texture *&texture, const char *text, TTF_Font *&fontSquare, SDL_Renderer *renderer)
{
    SDL_Color fontColor = {255, 255, 255};

    if (fontSquare == nullptr)
    {
        printf("TTF_OpenFont fontSquare: %s\n", TTF_GetError());
    }

    SDL_Surface *surface = TTF_RenderUTF8_Blended(fontSquare, text, fontColor);
    if (surface == nullptr)
    {
        printf("TTF_OpenFont: %s\n", TTF_GetError());
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Unable to create text surface! SDL Error: %s\n", SDL_GetError());
        exit(3);
    }

    SDL_DestroyTexture(texture);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == nullptr)
    {
        printf("TTF_OpenFont: %s\n", TTF_GetError());
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Unable to create texture from surface! SDL Error: %s\n", SDL_GetError());
    }

    SDL_FreeSurface(surface);
}

typedef struct
{
    SDL_Texture *texture;
    SDL_Rect textureBounds;
} Sprite;

Sprite loadSprite(SDL_Renderer *renderer, const char *filePath, int positionX, int positionY)
{
    SDL_Rect textureBounds = {positionX, positionY, 0, 0};

    SDL_Texture *texture = IMG_LoadTexture(renderer, filePath);

    if (texture != nullptr)
    {
        SDL_QueryTexture(texture, NULL, NULL, &textureBounds.w, &textureBounds.h);
    }

    Sprite sprite = {texture, textureBounds};

    return sprite;
}

Mix_Music *loadMusic(const char *filePath)
{
    Mix_Music *music = nullptr;

    music = Mix_LoadMUS(filePath);
    if (music == nullptr)
    {
        printf("Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
    }

    return music;
}

Mix_Chunk *loadSound(const char *filePath)
{
    Mix_Chunk *sounds = nullptr;

    sounds = Mix_LoadWAV(filePath);
    if (sounds == nullptr)
    {
        printf("Failed to load scratch sounds effect! SDL_mixer Error: %s\n", Mix_GetError());
    }

    return sounds;
}

void renderSprite(Sprite sprite)
{
    SDL_RenderCopy(renderer, sprite.texture, NULL, &sprite.textureBounds);
}

int rand_range(int min, int max)
{
    return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

int main(int argc, char **argv)
{
    romfsInit();
    chdir("romfs:/");

    window = SDL_CreateWindow("sdl2 switch starter", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    Mix_Init(MIX_INIT_OGG);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    SDL_InitSubSystem(SDL_INIT_JOYSTICK);
    SDL_JoystickEventState(SDL_ENABLE);
    SDL_JoystickOpen(0);

    SDL_InitSubSystem(SDL_INIT_AUDIO);
    Mix_AllocateChannels(5);
    Mix_OpenAudio(48000, AUDIO_S16, 2, 4096);

    // load font from romfs
    TTF_Font *font = TTF_OpenFont("data/LeroyLetteringLightBeta01.ttf", 36);

    SDL_Texture *helloworld_tex = nullptr;
    
    // render text as texture
    updateTextureText(helloworld_tex, "Hello, world!", font, renderer);

    SDL_QueryTexture(helloworld_tex, NULL, NULL, &helloworld_rect.w, &helloworld_rect.h);
    helloworld_rect.x = SCREEN_WIDTH / 2 - helloworld_rect.w / 2;
    helloworld_rect.y = SCREEN_HEIGHT / 2 - helloworld_rect.h / 2;

    // no need to keep the font loaded
    TTF_CloseFont(font);

    // load music and sounds from files
    sounds[0] = loadSound("data/pop1.wav");
    sounds[1] = loadSound("data/pop2.wav");
    sounds[2] = loadSound("data/pop3.wav");
    sounds[3] = loadSound("data/pop4.wav");

    music = loadMusic("data/background.ogg");

    Mix_PlayMusic(music, -1);

    Sprite switchlogo_tex = loadSprite(renderer, "data/switch.png", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

    srand(time(NULL));

    int velocityX = rand_range(1, 5);
    int velocityY = rand_range(1, 5);

    int colorIndex = 0, soundIndex = 0;

    colorIndex = rand_range(0, 7);

    while (!quitGame && appletMainLoop())
    {
        handleEvents();

        if (switchlogo_tex.textureBounds.x + switchlogo_tex.textureBounds.w > SCREEN_WIDTH)
        {
            switchlogo_tex.textureBounds.x = SCREEN_WIDTH - switchlogo_tex.textureBounds.w;

            velocityX = -rand_range(1, 5);
            colorIndex = rand_range(0, 4);
            soundIndex = rand_range(0, 3);

            Mix_PlayChannel(-1, sounds[soundIndex], 0);
        }

        if (switchlogo_tex.textureBounds.x < 0)
        {
            switchlogo_tex.textureBounds.x = 0;

            velocityX = rand_range(1, 5);
            colorIndex = rand_range(0, 4);
            soundIndex = rand_range(0, 3);

            Mix_PlayChannel(-1, sounds[soundIndex], 0);
        }

        if (switchlogo_tex.textureBounds.y + switchlogo_tex.textureBounds.h > SCREEN_HEIGHT)
        {
            switchlogo_tex.textureBounds.y = SCREEN_HEIGHT - switchlogo_tex.textureBounds.h;

            velocityY = -rand_range(1, 5);
            colorIndex = rand_range(0, 4);
            soundIndex = rand_range(0, 3);

            Mix_PlayChannel(-1, sounds[soundIndex], 0);
        }

        if (switchlogo_tex.textureBounds.y < 0)
        {
            switchlogo_tex.textureBounds.y = 0;

            velocityY = rand_range(1, 5);
            colorIndex = rand_range(0, 4);
            soundIndex = rand_range(0, 3);

            Mix_PlayChannel(-1, sounds[soundIndex], 0);
        }

        // set position and bounce on the walls
        switchlogo_tex.textureBounds.x += velocityX;
        switchlogo_tex.textureBounds.y += velocityY;

        if (!trail)
        {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
            SDL_RenderClear(renderer);
        }

        SDL_SetTextureColorMod(switchlogo_tex.texture, colors[colorIndex].r, colors[colorIndex].g, colors[colorIndex].b);
        renderSprite(switchlogo_tex);

        // put text on screen
        SDL_RenderCopy(renderer, helloworld_tex, NULL, &helloworld_rect);

        SDL_RenderPresent(renderer);
        SDL_Delay(wait);
    }

    // clean up your textures when you are done with them
    SDL_DestroyTexture(switchlogo_tex.texture);
    SDL_DestroyTexture(helloworld_tex);

    // stop sounds and free loaded data
    Mix_HaltChannel(-1);
    Mix_FreeMusic(music);

    for (soundIndex = 0; soundIndex < 4; soundIndex++)
    {
        if (sounds[soundIndex])
        {
            Mix_FreeChunk(sounds[soundIndex]);
        }
    }

    IMG_Quit();
    Mix_CloseAudio();
    TTF_Quit();
    Mix_Quit();
    SDL_Quit();
    romfsExit();
    return 0;
}
