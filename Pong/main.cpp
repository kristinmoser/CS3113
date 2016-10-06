#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "Matrix.h"
#include "ShaderProgram.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

GLuint LoadTexture(const char* image_path) {
    SDL_Surface* surface = IMG_Load(image_path);
    
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, surface->pixels);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    SDL_FreeSurface(surface);
    
    return textureID;
}

ShaderProgram setup(){
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Baby Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    glViewport(0, 0, 640, 360);
    ShaderProgram program(RESOURCE_FOLDER "vertex_textured.glsl", RESOURCE_FOLDER "fragment_textured.glsl");
    return program;
}


void draw(GLuint texture, float vertices[], ShaderProgram program, float texCoords[]){
    glBindTexture(GL_TEXTURE_2D, texture);
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program.texCoordAttribute);
    glDrawArrays(GL_TRIANGLES, 0, 6);

}



struct Paddle{
    
    GLuint texture;
    float x_pos = 0.0f;
    float y_pos = 0.0f;
    float velocity = 4.0f;
    float height = 1.0f;
    float topy;
    float bottomy;
    float leftx;
    float rightx;
    Paddle(){}
};

struct Ball{
    
    float x = 0.0f;
    float y = 0.0f;
    float dir_x = ((float) std::rand() / (RAND_MAX)) + 1;
    float dir_y = -((float) std::rand() / (RAND_MAX)) + 1;
    float height = 0.4f;
    float width = 0.4f;
    Ball(){}
    
};


int main(int argc, char *argv[])
{
    ShaderProgram program = setup();
    
    
    Matrix projectionMatrix;
    Matrix modelMatrix;
    Matrix viewMatrix;
    Matrix leftPlayerMatrix;
    Matrix rightPlayerMatrix;
    Matrix ballMatrix;
    
    float lastFrameTicks = 0.0f;
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
    glUseProgram(program.programID);
    
    
    GLuint leftText = LoadTexture("yellow.png");
    GLuint rightText = LoadTexture("purple.png");
    GLuint ballText = LoadTexture("blue.png");
    GLuint background = LoadTexture("background.jpg");
    
    

    
    SDL_Event event;
    bool done = false;
    Paddle right_player;
    Paddle left_player;
    Ball ball;
    while (!done) {
        
        
        //color
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        //taking time
        float ticks = (float) SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        //setting matrices
        program.setModelMatrix(modelMatrix);
        program.setProjectionMatrix(projectionMatrix);
        program.setViewMatrix(viewMatrix);
        

        
        //triangle vertex arrays
        float leftv[] = {
            -3.0f, -.75f + left_player.y_pos, //BL
            -2.75f, -.75f + left_player.y_pos, //BR
            -2.75f, .75f + left_player.y_pos, //TR
            -3.0f, -.75f + left_player.y_pos, //BL
            -2.75f, .75f + left_player.y_pos, //TR
            -3.0f, .75f + left_player.y_pos//TL
        };
        
        left_player.bottomy = -.75f + left_player.y_pos;
        left_player.topy = .75f + left_player.y_pos;
        left_player.leftx = -3.0f;
        left_player.rightx = -2.75f;
        
        float backgroundv[] = {
            -3.55f, -2.0f, //BL
            3.55f, -2.0f, //BR
            3.55f, 2.0f, //TR
            -3.55f, -2.0f, //BL
            3.55f, 2.0f, //TR
            -3.55f, 2.0f //TL
        };
        
        
        float rightv[] = {
            3.0f, -.75f + right_player.y_pos, //BL
            2.75f, -.75f + right_player.y_pos, //BR
            2.75f, .75f + right_player.y_pos, //TR
            3.0f, -.75f + right_player.y_pos, //BL
            2.75f, .75f + right_player.y_pos, //TR
            3.0f, .75f + right_player.y_pos//TL
        };
        
        right_player.bottomy = -.75f + right_player.y_pos;
        right_player.topy = .75f + right_player.y_pos;
        right_player.leftx = 2.75f;
        right_player.rightx = 3.0f;
        
        
        float ballv[] = {
            -0.2f + ball.x, -0.2f + ball.y, //BL
            0.2f + ball.x, -0.2f + ball.y, //BR
            0.2f + ball.x, 0.2f + ball.y, //TR
            -0.2f + ball.x, -0.2f + ball.y, //BL
            0.2f + ball.x, 0.2f + ball.y, //TR
            -0.2f + ball.x, 0.2f + ball.y//TL
        };
        
        float texCoords[] = {
            0.0, 1.0,
            1.0, 1.0,
            1.0, 0.0,
            0.0, 1.0,
            1.0, 0.0,
            0.0, 0.0
        };
        draw(background, backgroundv, program, texCoords);
        
        program.setModelMatrix(leftPlayerMatrix);
        leftPlayerMatrix.identity();
        //leftPlayerMatrix.Translate(-3.5, left_player.y_pos, 0);
        draw(leftText, leftv, program, texCoords);
        //std::cout << "DRAW: " << left_player.y_pos << std::endl;
        
        program.setModelMatrix(rightPlayerMatrix);
        draw(rightText, rightv, program, texCoords);
        
        program.setModelMatrix(ballMatrix);
        draw(ballText, ballv, program, texCoords);
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        bool wkeydown = false;
        bool skeydown = false;
        bool ikeydown = false;
        bool kkeydown = false;
        
        bool gameover = false;
        
        while (SDL_PollEvent(&event)) {
            
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            
            }
            
            else if(event.type == SDL_KEYDOWN) {
                if(event.key.keysym.scancode == SDL_SCANCODE_W ) { //and < 2.0
                    //left_player.velocity = fabsf(left_player.velocity);
                    wkeydown = true;
                    
                }
                else if(event.key.keysym.scancode == SDL_SCANCODE_S){ // and > -2.0
                    //left_player.velocity = -left_player.velocity;
                    skeydown = true;
                    
                    
                }
                else if(event.key.keysym.scancode == SDL_SCANCODE_I){ // and < 2.0
                    //right velocity is positive
                    //right_player.velocity = fabsf(right_player.velocity);
                    ikeydown = true;
                    
                }
                else if(event.key.keysym.scancode == SDL_SCANCODE_K){ // and > -2.0
                    //right velocity is negative
                    //right_player.velocity = -right_player.velocity;
                    kkeydown = true;
                    
                }
                
            }
            else if(event.type == SDL_KEYUP){
                if(event.key.keysym.scancode == SDL_SCANCODE_W) {
                    wkeydown = false;
                    
                }
                else if(event.key.keysym.scancode == SDL_SCANCODE_S){
                    skeydown = false;
                    
                }
                else if(event.key.keysym.scancode == SDL_SCANCODE_I){
                    ikeydown = false;
                    
                }
                else if(event.key.keysym.scancode == SDL_SCANCODE_K){
                    kkeydown = false;
                    
                }
                
            }
        
        }
            //moving the paddles
            if (wkeydown){
                if (left_player.y_pos + left_player.height > 2.0){
                    //don't left player go past the top of the screen
                    left_player.y_pos = 2.0 - left_player.height;
                }
                left_player.y_pos += left_player.velocity * elapsed;
            
            }
            if (skeydown ){
                if (left_player.y_pos - left_player.height < -2.0){
                    //don't left player go past the top of the screen
                    left_player.y_pos = -2.0 + left_player.height;
                }
                left_player.y_pos -= left_player.velocity * elapsed;
            
            }
            if (ikeydown){
                if (right_player.y_pos + right_player.height > 2.0){
                    //don't right player go past the top of the screen
                    right_player.y_pos = 2.0 - right_player.height;
                }
                right_player.y_pos += right_player.velocity * elapsed;
            }
            if (kkeydown){
                if (right_player.y_pos - right_player.height < -2.0){
                    //don't right player go past the bottom of the screen
                    right_player.y_pos = -2.0 + right_player.height;
                }
                right_player.y_pos -= right_player.velocity * elapsed;
            }
        
            //ball movement
            //1. boundaries aka ball can't go past orthographic projection coordinates
            //2. ^^ ball has to change direction based on what boundary it hits
            //3. ball colliding into players
            //4. ball hitting left and right side == winner
        
                ball.x += ball.dir_x * elapsed;
                ball.y += ball.dir_y * elapsed;
        if (ball.y + (ball.height/2) > 2.0 || ball.y - (ball.height/2) < -2.0){
            //ball hits the top, change y direction to be neg
            ball.dir_y = -ball.dir_y;
        }
        if (ball.x + (ball.width/2) > 3.55){
            ball.dir_x = - ball.dir_x;
            std::cout << "YELLOW WINS" << std::endl;
            gameover = true;
        }
        if(ball.x - (ball.width/2) < -3.55){
            ball.dir_x = - ball.dir_x;
            std::cout << "PURPLE WINS" << std::endl;
            gameover = true;
        }
        
        if (ball.x - (ball.width/2) < left_player.rightx){
            if (ball.y < left_player.topy && ball.y > left_player.bottomy){
                ball.dir_x = -ball.dir_x;
            }
        
            
        }
        if (ball.x + (ball.width/2) > right_player.leftx){
            if (ball.y < right_player.topy && ball.y > right_player.bottomy){
                ball.dir_x = -ball.dir_x;
            }
        }
        
        
        SDL_GL_SwapWindow(displayWindow);
        }

    SDL_Quit();
    return 0;
}
