#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>

using namespace std;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

SDL_Texture* dinoTexture = nullptr;
TTF_Font* font = nullptr;
SDL_Color textColor = { 255, 255, 255, 255 };
int highScore = 0;
int obstaclesCleared = 0;
SDL_Texture* backgroundTexture = nullptr;
SDL_Texture* obstacleTexture = nullptr;
SDL_Texture* gameOverTexture = nullptr;
SDL_Texture* startImageTexture = nullptr;

Mix_Music* bgMusic = nullptr;
Mix_Chunk* hitSound = nullptr;

SDL_Rect dinoRect;
vector<SDL_Rect> obstacles;

bool isJumping = false;
int dinoVelocityY = 0;
int score = 0;
bool gameOver = false;
bool gameStarted = false;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 400;
const int OBSTACLE_WIDTH = 50;
const int OBSTACLE_HEIGHT = 50;
const int OBSTACLE_SPACING_MIN = 200;
const int OBSTACLE_SPACING_MAX = 400;
const int GROUND_Y = 120;
const int JUMP_VELOCITY = -14;
const int GRAVITY = 1;
const int OBSTACLE_SPEED = 6;

bool initSDL();
SDL_Texture* loadTexture(const string& path);
void closeSDL();
void gameLoop();
void spawnObstacle();
void renderStartScreen();
void renderText(const string& message, int x, int y);

int main(int argc, char* argv[]) {
    if (!initSDL()) {
        return -1;
    }

    gameLoop();
    closeSDL();
    return 0;
}

bool initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) return false;

    window = SDL_CreateWindow("Dino Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) return false;

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) return false;

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) return false;

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) return false;

    dinoTexture = loadTexture("dino.png");
    backgroundTexture = loadTexture("background.png");
    obstacleTexture = loadTexture("obstacle.png");
    gameOverTexture = loadTexture("gameover.png");
    startImageTexture = loadTexture("startscreen.png");

    bgMusic = Mix_LoadMUS("bg_music.mp3");
    hitSound = Mix_LoadWAV("hit.wav");

    if (!bgMusic || !hitSound) return false;

    dinoRect = { 50, SCREEN_HEIGHT - GROUND_Y, 60, 50 };
    srand(static_cast<unsigned int>(time(nullptr)));

    if (TTF_Init() == -1) return false;

    font = TTF_OpenFont("OpenSans-Regular.ttf", 12);
    if (!font) return false;

    return true;
}

SDL_Texture* loadTexture(const string& path) {
    SDL_Texture* texture = IMG_LoadTexture(renderer, path.c_str());
    return texture;
}

void closeSDL() {
    Mix_FreeMusic(bgMusic);
    Mix_FreeChunk(hitSound);

    SDL_DestroyTexture(dinoTexture);
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(obstacleTexture);
    SDL_DestroyTexture(gameOverTexture);
    SDL_DestroyTexture(startImageTexture);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    Mix_Quit();
    IMG_Quit();
    SDL_Quit();
    TTF_CloseFont(font);
    TTF_Quit();
}

void spawnObstacle() {
    int spacing = OBSTACLE_SPACING_MIN + rand() % (OBSTACLE_SPACING_MAX - OBSTACLE_SPACING_MIN);
    int startX = obstacles.empty() ? SCREEN_WIDTH + spacing : obstacles.back().x + spacing;
    SDL_Rect newObstacle = { startX, SCREEN_HEIGHT - GROUND_Y, OBSTACLE_WIDTH, OBSTACLE_HEIGHT };
    obstacles.push_back(newObstacle);
}

void renderText(const string& message, int x, int y) {
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, message.c_str(), textColor);
    if (!textSurface) return;

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect textRect = { x, y, textSurface->w, textSurface->h };

    SDL_FreeSurface(textSurface);
    SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
    SDL_DestroyTexture(textTexture);
}

void renderStartScreen() {
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, backgroundTexture, nullptr, nullptr);
    SDL_RenderCopy(renderer, dinoTexture, nullptr, &dinoRect);
    if (startImageTexture) {
        SDL_Rect startRect = { SCREEN_WIDTH / 2 - 230, SCREEN_HEIGHT / 2 - 50, 500, 60 };
        SDL_RenderCopy(renderer, startImageTexture, nullptr, &startRect);
    }
    renderText("Highscore: " + to_string(highScore), 10, 10);
    renderText("Obstacles cleared: " + to_string(obstaclesCleared), 10, 25);
    SDL_RenderPresent(renderer);
}

void gameLoop() {
    bool quit = false;
    SDL_Event e;
    obstacles.clear();
    spawnObstacle();
    bool musicStarted = false;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }

            if (e.type == SDL_KEYDOWN) {
                if (!gameStarted && e.key.keysym.sym == SDLK_SPACE) {
                    gameStarted = true;
                    gameOver = false;
                    score = 0;
                    obstaclesCleared = 0;
                    dinoRect.x = 50;
                    dinoRect.y = SCREEN_HEIGHT - GROUND_Y;
                    obstacles.clear();
                    spawnObstacle();
                    isJumping = false;
                    dinoVelocityY = 0;
                    Mix_PlayMusic(bgMusic, -1);
                } else if (gameOver && e.key.keysym.sym == SDLK_RETURN) {
                    gameStarted = false;
                } else if (e.key.keysym.sym == SDLK_SPACE && gameStarted && !gameOver && !isJumping) {
                    isJumping = true;
                    dinoVelocityY = JUMP_VELOCITY;
                }
            }
        }

        if (!gameStarted) {
            renderStartScreen();
            continue;
        }

        if (!gameOver) {
            if (isJumping) {
                dinoRect.y += dinoVelocityY;
                dinoVelocityY += GRAVITY;
                if (dinoRect.y >= SCREEN_HEIGHT - GROUND_Y) {
                    dinoRect.y = SCREEN_HEIGHT - GROUND_Y;
                    isJumping = false;
                }
            }

            for (auto& obs : obstacles) {
                obs.x -= OBSTACLE_SPEED;
            }

            if (!obstacles.empty() && obstacles.front().x + OBSTACLE_WIDTH < 0) {
                obstacles.erase(obstacles.begin());
                score++;
                obstaclesCleared++;

                if (score > highScore) {
                    highScore = score;
                }
            }

            if (obstacles.empty() || obstacles.back().x < SCREEN_WIDTH - OBSTACLE_SPACING_MIN) {
                spawnObstacle();
            }

            for (auto& obs : obstacles) {
                SDL_Rect dinoHitbox = { dinoRect.x + 5, dinoRect.y + 5, dinoRect.w - 10, dinoRect.h - 10 };
                SDL_Rect obsHitbox = { obs.x + 5, obs.y + 5, obs.w - 10, obs.h - 10 };
                if (SDL_HasIntersection(&dinoHitbox, &obsHitbox)) {
                    gameOver = true;
                    Mix_HaltMusic();
                    Mix_PlayChannel(-1, hitSound, 0);
                }
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, backgroundTexture, nullptr, nullptr);
        SDL_RenderCopy(renderer, dinoTexture, nullptr, &dinoRect);

        for (auto& obs : obstacles) {
            SDL_RenderCopy(renderer, obstacleTexture, nullptr, &obs);
        }

        if (gameOver) {
            SDL_Rect goRect = { SCREEN_WIDTH / 2 - 240, SCREEN_HEIGHT / 2 - 50, 500, 60 };
            SDL_RenderCopy(renderer, gameOverTexture, nullptr, &goRect);
            renderText("Highscore: " + to_string(highScore), 10, 10);
            renderText("Obstacles cleared: " + to_string(obstaclesCleared), 10, 25);
        }

        if (gameStarted && !gameOver) {
            renderText("Highscore: " + to_string(highScore), 10, 10);
            renderText("Obstacles cleared: " + to_string(obstaclesCleared), 10, 25);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
}
