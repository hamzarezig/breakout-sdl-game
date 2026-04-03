#include <SDL3/SDL.h>

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define WINDOW_TITLE "Breakout Game"
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800

#define PADDLE_HEIGHT 50.0
#define PADDLE_WIDTH 300.0
#define PADDLE_SPEED 10.0

typedef struct{
    SDL_Window *window;
    SDL_Renderer *renderer;
} Game;
typedef struct{
    bool up;
    bool down;
    bool right;
    bool left;
} Directions;

typedef struct {
    SDL_FRect rect;
    Directions directions;
} Paddle ;



void game_cleanup(Game *game,int exit_status);
bool sdl_initialize(Game *game);
void paddle_update(Paddle *paddle);
void paddle_draw(Game *game,Paddle *paddle);

void DrawCircle(SDL_Renderer * renderer, int32_t centreX, int32_t centreY, int32_t radius);

int main(){

    Game game ={
        .window = NULL,
        .renderer = NULL,
    };

    if(sdl_initialize(&game)) { // in case of an error
        game_cleanup(&game,EXIT_FAILURE);
    }

    Paddle paddle = {
        .rect = {
            .h = PADDLE_HEIGHT,
            .w = PADDLE_WIDTH,
            .x = ((float) SCREEN_WIDTH - PADDLE_WIDTH) / 2.0 , 
            .y = (float) SCREEN_HEIGHT - PADDLE_HEIGHT,
        },
        .directions = {
            .right=false,
            .left=false,
        },
    };

    while (true) { // game or render loop
        SDL_Event event; // see if there are any events 

        while(SDL_PollEvent(&event)){ // handling events in the game 
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    game_cleanup(&game, EXIT_SUCCESS);
                    break;
                case SDL_EVENT_KEY_DOWN:
                    switch(event.key.key){
                        case 'a':
                            paddle.directions.left = true;
                            break;
                        case 'd':
                            paddle.directions.right = true;
                            break;
                        default:
                            break;
                    }
                break;
                case SDL_EVENT_KEY_UP:
                    switch(event.key.key){
                        case 'a':
                            paddle.directions.left = false;
                            break;
                        case 'd':
                            paddle.directions.right = false;
                            break;
                        default:
                            break;
                    }
                break;
                default:
                    break;
            }
        }

        SDL_SetRenderDrawColor(game.renderer, 01, 01, 01, 255); // canvas background  
        SDL_RenderClear(game.renderer); // clearing with that color

        // update paddle 
        paddle_update(&paddle);
        // draw paddle
        paddle_draw(&game,&paddle);
        DrawCircle(game.renderer, 300, 300, 20);

        SDL_RenderPresent(game.renderer); // render the canvas

        SDL_Delay(16); // fps like setup a 16ms delay gives 60fps
    }

    game_cleanup(&game,EXIT_SUCCESS);

    return 0;
}

void paddle_update(Paddle *paddle){
    //  handle movement
    if(paddle->directions.right) 
        paddle->rect.x+=PADDLE_SPEED;
    if(paddle->directions.left) 
        paddle->rect.x-=PADDLE_SPEED;
    //handle wall collision
    if(paddle->rect.x< 0 ) 
        paddle->rect.x = 0;
    if(paddle->rect.x + PADDLE_WIDTH > SCREEN_WIDTH ) 
        paddle->rect.x = SCREEN_WIDTH - PADDLE_WIDTH;
}
void paddle_draw(Game *game,Paddle *paddle){
    // #00ffbb
    SDL_SetRenderDrawColor(game->renderer, 0x00, 0xff, 0xbb, 0xff);  // square color
    SDL_RenderFillRect(game->renderer,&paddle->rect); // draw the square 
} 

bool sdl_initialize(Game *game){
    if(!SDL_Init(SDL_INIT_VIDEO)){ // check if initializing SDL worked or not
        fprintf(stderr,"Error initializing SDL : %s\n", SDL_GetError());
        return 1;
    }

    game->window = SDL_CreateWindow(WINDOW_TITLE,
                                    SCREEN_WIDTH,
                                    SCREEN_HEIGHT,
                                    SDL_WINDOW_OPENGL);
    if(!game->window) { // check if its NULL
        fprintf(stderr,"Error creating window : %s\n", SDL_GetError());
        return 1;
    }
    game->renderer = SDL_CreateRenderer(game->window, NULL); // NULL to let SDL choose his own graphics driver
    if(!game->renderer) { // check if its NULL
        fprintf(stderr,"Error creating renderer : %s\n", SDL_GetError());
        return 1;
    }

    return 0;
}
void game_cleanup(Game *game,int exit_status){
    SDL_DestroyRenderer(game->renderer);
    SDL_DestroyWindow(game->window);
    SDL_Quit();
    exit(exit_status);
}

// optional
void DrawCircle(SDL_Renderer * renderer, int32_t centreX, int32_t centreY, int32_t radius)
{
   const int32_t diameter = (radius * 2);

   int32_t x = (radius - 1);
   int32_t y = 0;
   int32_t tx = 1;
   int32_t ty = 1;
   int32_t error = (tx - diameter);

   while (x >= y)
   {
      //  Each of the following renders an octant of the circle
      SDL_RenderPoint(renderer, centreX + x, centreY - y);
      SDL_RenderPoint(renderer, centreX + x, centreY + y);
      SDL_RenderPoint(renderer, centreX - x, centreY - y);
      SDL_RenderPoint(renderer, centreX - x, centreY + y);
      SDL_RenderPoint(renderer, centreX + y, centreY - x);
      SDL_RenderPoint(renderer, centreX + y, centreY + x);
      SDL_RenderPoint(renderer, centreX - y, centreY - x);
      SDL_RenderPoint(renderer, centreX - y, centreY + x);

      if (error <= 0)
      {
         ++y;
         error += ty;
         ty += 2;
      }

      if (error > 0)
      {
         --x;
         tx += 2;
         error += (tx - diameter);
      }
   }
}

