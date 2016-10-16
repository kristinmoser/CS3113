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




int main(int argc, char *argv[])
{
    ShaderProgram program = setup();
    
    
    Matrix projectionMatrix;
    Matrix modelMatrix;
    Matrix viewMatrix;
    Matrix playerMatrix;
    
    float lastFrameTicks = 0.0f;
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
    glUseProgram(program.programID);
    
    
    GLuint leftText = LoadTexture("yellow.png");

    
    

    
    SDL_Event event;
    bool done = false;
    Paddle player;
    
    bool akeydown = false;
    bool skeydown = false;
    bool gameover = false;
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
        float playerV[] = {
            -3.0f, -.75f, //BL
            -2.75f, -.75f, //BR
            -2.75f, .75f, //TR
            -3.0f, -.75f, //BL
            -2.75f, .75f, //TR
            -3.0f, .75f//TL
        };
        
        float texCoords[] = {
            0.0, 1.0,
            1.0, 1.0,
            1.0, 0.0,
            0.0, 1.0,
            1.0, 0.0,
            0.0, 0.0
        };
        //draw(background, backgroundv, program, texCoords);
        
        program.setModelMatrix(playerMatrix);
        playerMatrix.identity();
        draw(leftText, playerV, program, texCoords);
        playerMatrix.Translate(player.x_pos, player.y_pos, 0);

        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        

        
        
        
        //moving the paddles
        if (akeydown){
            if (player.y_pos + player.height > 2.0){
                //don't left player go past the top of the screen
                player.y_pos = 2.0 - player.height;
            }
            player.y_pos += player.velocity * elapsed;
            
        }
        if (skeydown ){
            if (player.y_pos - player.height < -2.0){
                //don't left player go past the top of the screen
                player.y_pos = -2.0 + player.height;
            }
            player.y_pos -= player.velocity * elapsed;
            
        }

        
        
        while (SDL_PollEvent(&event)) {
            
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            
            }
            
            else if(event.type == SDL_KEYDOWN) {
                if(event.key.keysym.scancode == SDL_SCANCODE_A) { //and < 2.0
                    //player.velocity = fabsf(player.velocity);
                    akeydown = true;
                    
                }
                else if(event.key.keysym.scancode == SDL_SCANCODE_S){ // and > -2.0
                    //player.velocity = -player.velocity;
                    skeydown = true;
                    
                    
                }
                
            }
            else if(event.type == SDL_KEYUP){
                if(event.key.keysym.scancode == SDL_SCANCODE_W) {
                    akeydown = false;
                    
                }
                else if(event.key.keysym.scancode == SDL_SCANCODE_S){
                    skeydown = false;
                }
                
            }
        
        }
        
        SDL_GL_SwapWindow(displayWindow);
        }

    SDL_Quit();
    return 0;
}
