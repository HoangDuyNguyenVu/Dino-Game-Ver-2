#include<iostream>
#include<SDL.h>
#include<SDL_image.h>
using namespace std;
const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 500;
const char* WINDOW_TITLE = "Dự án đầu tiên =))";
void SDL_init(SDL_Window* &window,SDL_Renderer* &renderer);
void quit_SDL(SDL_Window* &window,SDL_Renderer* &renderer);
void waitforEvent();


int main(int argc, char* argv[])
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_init(window,renderer);
    waitforEvent();
    quit_SDL(window,renderer);
    cout<<"Fighting !!";
    return 0;
}
//Hàm khai báo cửa sổ + bút vẽ
void SDL_init(SDL_Window* &window,SDL_Renderer* &renderer){
     window = SDL_CreateWindow(WINDOW_TITLE,SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,SCREEN_WIDTH,SCREEN_HEIGHT,SDL_WINDOW_SHOWN);
     renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
     SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
     SDL_RenderSetLogicalSize(renderer,SCREEN_WIDTH,SCREEN_HEIGHT);
}
//Hàm giải phóng bộ nhớ
void quit_SDL(SDL_Window* &window,SDL_Renderer* &renderer){
     SDL_DestroyWindow(window);
     SDL_DestroyRenderer(renderer);
     SDL_Quit();
}
//Hàm đợi keydown để không tắt luôn
void waitforEvent()
{    SDL_Event e;
     while (true) {
        if ( SDL_WaitEvent(&e) != 0 &&(e.type == SDL_KEYDOWN || e.type == SDL_QUIT)){
               return;
        }
        SDL_Delay(100);
    }
}


