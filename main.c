#include <SDL3/SDL.h>

#include <SDL3/SDL_audio.h>
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
#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 800

#define PADDLE_HEIGHT 20.0
#define PADDLE_WIDTH 200.0
#define PADDLE_SPEED 10.0

#define BALL_SPEED 10.0f

// audio define 
SDL_AudioStream *stream = NULL;
Uint8 *wav_data = NULL;
Uint32 wav_data_len = 0;

typedef struct {
    SDL_Texture *texture;
    SDL_FRect rect;

} Bg ;

typedef struct{
    SDL_Window *window;
    SDL_Renderer *renderer;
    Bg bg;
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
typedef struct {
    SDL_FRect rect;
    SDL_Texture *texture;
}Brick;



bool sdl_initialize(Game *game);
void game_cleanup(Game *game,int exit_status);
// TODO make paddle, ball init funcs
// void paddle_init();
void paddle_update(Paddle *paddle);
void paddle_draw(Game *game,Paddle *paddle);
// TODO func init ball
void ball_update(Ball *ball,Paddle *paddle,Brick *bricks[5][5]);
void ball_draw(Game *game,Ball *ball);

int main(){

    // game init 
    Game game ={
        .window = NULL,
        .renderer = NULL,
        .bg = {
            .texture = NULL,
            .rect ={
                .x = 0.0f,
                .y = 0.0f,
                .w = SCREEN_WIDTH,
                .h = SCREEN_HEIGHT,
            } ,

        },
    };

    if(sdl_initialize(&game)) { // in case of an error
        game_cleanup(&game,EXIT_FAILURE);
    }

    // init background 
    char *backgroundpath = NULL;
    SDL_Surface *backgroundsurface = NULL; // surface is a pixel data the cpu can use the texture is for the gpu 

    asprintf(&backgroundpath,"%sassets/background.png",SDL_GetBasePath());
    backgroundsurface = SDL_LoadPNG(backgroundpath);

    if(!backgroundsurface){
        fprintf(stderr,"Error loading ball surface : %s\n",SDL_GetError());
        game_cleanup(&game, EXIT_FAILURE);
    }
    free(backgroundpath);

    game.bg.texture = SDL_CreateTextureFromSurface(game.renderer, backgroundsurface);

    if(!game.bg.texture){
        fprintf(stderr,"Error loading ball texture : %s\n",SDL_GetError());
        game_cleanup(&game, EXIT_FAILURE);
    }

    float scale = (float)SCREEN_WIDTH / backgroundsurface->w;
    game.bg.rect.w = backgroundsurface->w * scale;
    game.bg.rect.h = backgroundsurface->h *scale ;
    SDL_DestroySurface(backgroundsurface);


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

    ball.rect.h = 20.0f;
    ball.rect.w = 20.0f;
    ball.rect.x = 50.0f;
    ball.rect.y = 300.0f;
    ball.velocity.x = BALL_SPEED;
    ball.velocity.y = BALL_SPEED;

    SDL_DestroySurface(ballsurface); // we have the ball texture to render now 

    // audio init 
    SDL_AudioSpec spec;
    char *wav_path = NULL;

    asprintf(&wav_path,"%sassets/skulls_adventure.wav",SDL_GetBasePath());
    if(!SDL_LoadWAV(wav_path,&spec, &wav_data,&wav_data_len)) {
        fprintf(stderr,"Error loading wav file : %s\n",SDL_GetError());
        game_cleanup(&game, EXIT_FAILURE);
    }

    free(wav_path);

    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
                                       &spec,
                                       NULL,
                                       NULL);
    if(!stream){
        fprintf(stderr,"Error create audio stream : %s\n",SDL_GetError());
        game_cleanup(&game, EXIT_FAILURE);
    }
    SDL_ResumeAudioStreamDevice(stream);// start the adudio
    
    // bricks init

    Brick *bricks[5][5];

    for(int i=0;i<5;i++){
        for(int j=0;j<5;j++){
            bricks[i][j] = malloc(sizeof(Brick));
            bricks[i][j]->rect = (SDL_FRect ){
                    //        width   padding   start_padding
                    .x = j * (50.0f + 20.0f)  +   100,
                    .y = i * (20.0f + 20.0f)  +   100,
                    .w = 50.0f,
                    .h = 20.0f,
            };
        }
    }

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
        // audio
        if (SDL_GetAudioStreamQueued(stream) < (int)wav_data_len) {
            SDL_PutAudioStreamData(stream, wav_data, wav_data_len);
        }

        // #222034
        SDL_SetRenderDrawColor(game.renderer, 34, 32, 52, 255); // canvas background  
        SDL_RenderClear(game.renderer); // clearing with that color
        SDL_RenderTexture(game.renderer,game.bg.texture,NULL,&game.bg.rect);

        paddle_update(&paddle);
        ball_update(&ball,&paddle,bricks);

        paddle_draw(&game,&paddle);
        ball_draw(&game,&ball);
        // TODO brick draw func
        if(bricks[3][3] != NULL)free(bricks[3][3]); // free memo when it breaks
        bricks[3][3] = NULL; 
        for(int i=0;i<5;i++){
            for(int j=0;j<5;j++) {
                if(bricks[i][j] != NULL){
                    SDL_SetRenderDrawColor(game.renderer, 0xe8,0x7f,0x24,0xff); // color #E87F24
                    SDL_RenderFillRect(game.renderer,&bricks[i][j]->rect);
                }
            }
        }

        SDL_RenderPresent(game.renderer); // render the canvas

        SDL_Delay(16); // fps like setup a 16ms delay gives 60fps
    }

    // game free 
    SDL_DestroyTexture(ball.texture);
    free(wav_data);
    for(int i=0;i<5;i++){
        for(int j=0;j<5;j++){
            free(bricks[i][j]);
        }
    }
    game_cleanup(&game,EXIT_SUCCESS);

    return 0;
}

void ball_update(Ball *ball,Paddle *paddle,Brick *bricks[5][5]){
    //update ball position
    ball->rect.x += ball->velocity.x;
    ball->rect.y += ball->velocity.y;
    //check for collision
    // brick collision
    for(int i=0;i<5;i++){
        for(int j=0;j<5;j++){
            if(bricks[i][j] == NULL ) continue;
            if(ball->rect.y + ball->rect.h > bricks[i][j]->rect.y &&
               ball->rect.x < bricks[i][j]->rect.x + bricks[i][j]->rect.w && 
               ball->rect.x + ball->rect.w > bricks[i][j]->rect.x &&
               ball->rect.y < bricks[i][j]->rect.y + bricks[i][j]->rect.h 
            ) { // collision with brick
                
                // resolve collision
                float top,down,right,left;
                float minX,minY;

                top   = ball->rect.y + ball->rect.h         - bricks[i][j]->rect.y;
                down  = ball->rect.y - bricks[i][j]->rect.y + bricks[i][j]->rect.h;
                right = ball->rect.x - bricks[i][j]->rect.x + bricks[i][j]->rect.w; 
                left  = ball->rect.x + ball->rect.w         - bricks[i][j]->rect.x;

                minX = right < left ? right : left;
                minY = top   < down ? top   : down;

                //flip ball
                if(minX<minY){
                    if(right<left){
                        ball->rect.x = bricks[i][j]->rect.x + bricks[i][j]->rect.w ; 
                    } else {
                        ball->rect.x = bricks[i][j]->rect.x - ball->rect.w ; 
                    }
                    ball->velocity.x *=-1.0f;
                } else {
                    if(down<top){
                        ball->rect.y = bricks[i][j]->rect.y + bricks[i][j]->rect.h ; 
                    } else {
                        ball->rect.y = bricks[i][j]->rect.y - ball->rect.h ; 
                    }
                    ball->velocity.y *=-1.0f;

                }
                // delete brick 
                free(bricks[i][j]);
                bricks[i][j] = NULL;

            }
        }
    }

    // paddle collision
    if(ball->rect.y + ball->rect.h > paddle->rect.y &&
       ball->rect.x < paddle->rect.x + paddle->rect.w && 
       ball->rect.x + ball->rect.w > paddle->rect.x &&
       ball->rect.y < paddle->rect.y + paddle->rect.h 
    ) { // collision detected 
        
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
            ball->velocity.x *=-1.0f;
        } else {
            if(down<top){
                ball->rect.y = paddle->rect.y + paddle->rect.h ; 
            } else {
                ball->rect.y = paddle->rect.y - ball->rect.h ; 
            }
            ball->velocity.y *=-1.0f;

        }

    }
    // wall collision
    if(ball->rect.x <= 0 || (ball->rect.x + ball->rect.w) >= SCREEN_WIDTH) {
        ball->velocity.x *=-1.0f;
    }
    if(ball->rect.y <= 0 || (ball->rect.y + ball->rect.h) >= SCREEN_HEIGHT) {
        ball->velocity.y *=-1.0f;
    }


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
    // rgb(99, 155, 255)
    SDL_SetRenderDrawColor(game->renderer, 99, 155, 255, 255);  // square color
    SDL_RenderFillRect(game->renderer,&paddle->rect); // draw the square 
} 

bool sdl_initialize(Game *game){
    if(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)){ // check if initializing SDL worked or not
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

