#include "sdl_starter.h"
#include <time.h>
#include <switch.h>
#include <iostream>

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;
SDL_GameController *controller = nullptr;

bool isGamePaused = false;
int isGameRunning = true;
int trail = 0;
int wait = 15;

Sprite playerSprite;
Sprite switchlogoSprite;

const int PLAYER_SPEED = 600;

int logoVelocityX = 250;
int logoVelocityY = 250;

int colorIndex = 0;
int soundIndex = 0;

Mix_Music *music = nullptr;
Mix_Chunk *sounds[4] = {nullptr};

SDL_Texture *pauseGameTexture = nullptr;
SDL_Rect pauseGameBounds;

SDL_Texture *scoreTexture = nullptr;
SDL_Rect scoreBounds;

int score = 0;

TTF_Font *font = nullptr;

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

void handleEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            isGameRunning = false;
            break;
        }

        if (event.type == SDL_JOYBUTTONDOWN)
        {
            if (event.jbutton.button == JOY_MINUS)
            {
                isGameRunning = false;
                break;
            }

            if (event.jbutton.button == JOY_PLUS)
            {
                isGamePaused = !isGamePaused;
                Mix_PlayChannel(-1, sounds[0], 0);
            }

            if (event.jbutton.button == JOY_A)
            {
                if (wait > 0)
                    wait--;
            }

            if (event.jbutton.button == JOY_X)
            {
                if (wait < 100)
                    wait++;
            }

            if (event.jbutton.button == JOY_B)
            {
                trail = !trail;
            }
        }
    }
}

int rand_range(int min, int max)
{
    return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

void update(float deltaTime)
{
    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP) && playerSprite.bounds.y > 0)
    {
        playerSprite.bounds.y -= PLAYER_SPEED * deltaTime;
    }

    else if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN) && playerSprite.bounds.y < SCREEN_HEIGHT - playerSprite.bounds.h)
    {
        playerSprite.bounds.y += PLAYER_SPEED * deltaTime;
    }

    else if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT) && playerSprite.bounds.x > 0)
    {
        playerSprite.bounds.x -= PLAYER_SPEED * deltaTime;
    }

    else if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) && playerSprite.bounds.x < SCREEN_WIDTH - playerSprite.bounds.w)
    {
        playerSprite.bounds.x += PLAYER_SPEED * deltaTime;
    }

    if (switchlogoSprite.bounds.x + switchlogoSprite.bounds.w > SCREEN_WIDTH || switchlogoSprite.bounds.x < 0)
    {
        logoVelocityX *= -1;
        colorIndex = rand_range(0, 4);
        soundIndex = rand_range(0, 3);

        Mix_PlayChannel(-1, sounds[soundIndex], 0);
    }

    if (switchlogoSprite.bounds.y + switchlogoSprite.bounds.h > SCREEN_HEIGHT || switchlogoSprite.bounds.y < 0)
    {
        logoVelocityY *= -1;
        colorIndex = rand_range(0, 4);
        soundIndex = rand_range(0, 3);

        Mix_PlayChannel(-1, sounds[soundIndex], 0);
    }

    if (SDL_HasIntersection(&playerSprite.bounds, &switchlogoSprite.bounds))
    {
        logoVelocityX *= -1;
        logoVelocityY *= -1;

        colorIndex = rand_range(0, 4);
        soundIndex = rand_range(0, 3);

        Mix_PlayChannel(-1, sounds[soundIndex], 0);

        score++;

        std::string scoreString = "SCORE: " + std::to_string(score);

        updateTextureText(scoreTexture, scoreString.c_str(), font, renderer);
    }

    // set position and bounce on the walls
    switchlogoSprite.bounds.x += logoVelocityX * deltaTime;
    switchlogoSprite.bounds.y += logoVelocityY * deltaTime;
}

void renderSprite(Sprite &sprite)
{
    SDL_RenderCopy(renderer, sprite.texture, NULL, &sprite.bounds);
}

void render()
{
    if (!trail)
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
        SDL_RenderClear(renderer);
    }

    SDL_SetTextureColorMod(switchlogoSprite.texture, colors[colorIndex].r, colors[colorIndex].g, colors[colorIndex].b);
    renderSprite(switchlogoSprite);

    renderSprite(playerSprite);

    if (isGamePaused)
    {
        // put text on screen
        SDL_RenderCopy(renderer, pauseGameTexture, NULL, &pauseGameBounds);
    }

    SDL_QueryTexture(scoreTexture, NULL, NULL, &scoreBounds.w, &scoreBounds.h);
    scoreBounds.x = SCREEN_WIDTH / 2 - pauseGameBounds.w / 2;
    scoreBounds.y = scoreBounds.h / 2;
    SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreBounds);

    SDL_RenderPresent(renderer);
}

int main(int argc, char **argv)
{
    romfsInit();
    chdir("romfs:/");

    window = SDL_CreateWindow("sdl2 switch starter", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (startSDLSystems(window, renderer) > 0)
    {
        return 1;
    }
  
    //the control is always connected in a handheld console.
    controller = SDL_GameControllerOpen(0);
   
    playerSprite = loadSprite(renderer, "sprites/alien_1.png", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2);
    switchlogoSprite = loadSprite(renderer, "sprites/switch.png", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

    // load font from romfs
    font = TTF_OpenFont("fonts/LeroyLetteringLightBeta01.ttf", 36);

    // render text as texture
    updateTextureText(scoreTexture, "SCORE: 0", font, renderer);

    updateTextureText(pauseGameTexture, "GAME PAUSED", font, renderer);

    SDL_QueryTexture(pauseGameTexture, NULL, NULL, &pauseGameBounds.w, &pauseGameBounds.h);
    pauseGameBounds.x = SCREEN_WIDTH / 2 - pauseGameBounds.w / 2;
    pauseGameBounds.y = 200;

    // load music and sounds from files
    sounds[0] = loadSound("sounds/pop1.wav");
    sounds[1] = loadSound("sounds/pop2.wav");
    sounds[2] = loadSound("sounds/pop3.wav");
    sounds[3] = loadSound("sounds/pop4.wav");

    music = loadMusic("music/background.ogg");

    Mix_PlayMusic(music, -1);

    srand(time(NULL));

    Uint32 previousFrameTime = SDL_GetTicks();
    Uint32 currentFrameTime = previousFrameTime;
    float deltaTime = 0.0f;

    while (isGameRunning && appletMainLoop())
    {
        currentFrameTime = SDL_GetTicks();
        deltaTime = (currentFrameTime - previousFrameTime) / 1000.0f;
        previousFrameTime = currentFrameTime;

        handleEvents();

        if (!isGamePaused)
        {
            update(deltaTime);
        }

        render();

        SDL_Delay(wait);
    }

    // clean up your textures when you are done with them
    SDL_DestroyTexture(switchlogoSprite.texture);
    SDL_DestroyTexture(pauseGameTexture);

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

    stopSDLSystems();
    Mix_CloseAudio();
    romfsExit();

    return 0;
}