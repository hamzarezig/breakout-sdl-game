#include <SDL3/SDL.h>

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define WINDOW_TITLE "Breakout Game"
#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 800

#define PADDLE_HEIGHT 50.0
#define PADDLE_WIDTH 300.0
#define PADDLE_SPEED 10.0

#define BALL_SPEED 20.0f

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

typedef struct {
    float x;
    float y;
} Vector2D;

typedef struct {
    SDL_Texture *texture;
    SDL_FRect rect; 
    Vector2D velocity;
} Ball;



bool sdl_initialize(Game *game);
void game_cleanup(Game *game,int exit_status);
// TODO make paddle, ball init funcs
// void paddle_init();
void paddle_update(Paddle *paddle);
void paddle_draw(Game *game,Paddle *paddle);
// TODO func init ball
void ball_draw(Game *game,Ball *ball);
void ball_update(Ball *ball,Paddle *paddle);

int main(){

    Game game ={
        .window = NULL,
        .renderer = NULL,
    };

    if(sdl_initialize(&game)) { // in case of an error
        game_cleanup(&game,EXIT_FAILURE);
    }

    // init paddle 
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

    // init ball
    Ball ball = {
        .texture = NULL,
    };

    char *ballpngpath = NULL;
    SDL_Surface *ballsurface = NULL; // surface is a pixel data the cpu can use the texture is for the gpu 

    asprintf(&ballpngpath,"%sassets/ball.png",SDL_GetBasePath());
    ballsurface = SDL_LoadPNG(ballpngpath);

    if(!ballsurface){
        fprintf(stderr,"Error loading ball surface : %s\n",SDL_GetError());
        game_cleanup(&game, EXIT_FAILURE);
    }
    free(ballpngpath);

    ball.texture = SDL_CreateTextureFromSurface(game.renderer, ballsurface);

    if(!ball.texture){
        fprintf(stderr,"Error loading ball texture : %s\n",SDL_GetError());
        game_cleanup(&game, EXIT_FAILURE);
    }

    ball.rect.h = 30.0f;
    ball.rect.w = 30.0f;
    ball.rect.x = 10.0f;
    ball.rect.y = 40.0f;
    ball.velocity.x = BALL_SPEED;
    ball.velocity.y = BALL_SPEED;

    SDL_DestroySurface(ballsurface);
    // we have the ball texture to render now 
    

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

        SDL_SetRenderDrawColor(game.renderer, 11, 11 , 11, 255); // canvas background  
        SDL_RenderClear(game.renderer); // clearing with that color

        paddle_update(&paddle);
        paddle_draw(&game,&paddle);

        ball_update(&ball,&paddle);
        ball_draw(&game,&ball);

        SDL_RenderPresent(game.renderer); // render the canvas

        SDL_Delay(16); // fps like setup a 16ms delay gives 60fps
    }

    SDL_DestroyTexture(ball.texture);
    game_cleanup(&game,EXIT_SUCCESS);

    return 0;
}

void ball_update(Ball *ball,Paddle *paddle){
    //check for collision
    // wall collision
    if(ball->rect.x <= 0 || (ball->rect.x + ball->rect.w) >= SCREEN_WIDTH) {
        ball->velocity.x *=-1.0f;
    }
    if(ball->rect.y <= 0 || (ball->rect.y + ball->rect.h) >= SCREEN_HEIGHT) {
        ball->velocity.y *=-1.0f;
    }
    // paddle collision
    if(ball->rect.y + ball->rect.h >= paddle->rect.y &&
       ball->rect.x <= paddle->rect.x + paddle->rect.w && 
       ball->rect.x + ball->rect.w >= paddle->rect.x &&
       ball->rect.y <= paddle->rect.y + paddle->rect.h 
    ) { 
        // collision detected 
        ball->velocity.y *=-1.0f;
        // collision fix 
        float top,down,right,left;
        float minX,minY;

        top   = ball->rect.y + ball->rect.h   - paddle->rect.y;
        down  = ball->rect.y - paddle->rect.y + paddle->rect.h;
        right = ball->rect.x - paddle->rect.x + paddle->rect.w; 
        left  = ball->rect.x + ball->rect.w   - paddle->rect.x;

        minX = right < left ? right : left;
        minY = top   < down ? top   : down;

        if(minX<minY){
            if(right<left){
                ball->rect.x = paddle->rect.x + paddle->rect.w ; 
            } else {
                ball->rect.x = paddle->rect.x - ball->rect.w ; 
            }
        } else {
            if(down<top){
                ball->rect.y = paddle->rect.y + paddle->rect.h ; 
            } else {
                ball->rect.y = paddle->rect.y - ball->rect.h ; 
            }

        }

    }

    //update ball position
    ball->rect.x += ball->velocity.x;
    ball->rect.y += ball->velocity.y;

}

void ball_draw(Game *game,Ball *ball){
        SDL_RenderTexture(game->renderer,
                          ball->texture, 
                          NULL,
                          &ball->rect);
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

