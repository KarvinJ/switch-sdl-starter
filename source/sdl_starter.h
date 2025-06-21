#pragma once

#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

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

typedef struct
{
    SDL_Texture *texture;
    SDL_Rect bounds;
} Sprite;

int startSDLSystems(SDL_Window *window, SDL_Renderer *renderer);

Sprite loadSprite(SDL_Renderer *renderer, const char *filePath, int positionX, int positionY);

Mix_Chunk *loadSound(const char *filePath);

Mix_Music *loadMusic(const char *filePath);

void updateTextureText(SDL_Texture *&texture, const char *text, TTF_Font *&fontSquare, SDL_Renderer *renderer);

void stopSDLSystems();