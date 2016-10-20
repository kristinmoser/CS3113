#include <stdio.h>
#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "Matrix.h"
#include "ShaderProgram.h"
#include <vector>

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

////-------- SETUP ----------

SDL_Window* displayWindow;

////-------- GAME STATE & BOOLS -------------

bool done = false;
enum GameState { STATE_MAIN_MENU, STATE_GAME_LEVEL };
int state = STATE_MAIN_MENU;


////-------- BACKGROUND VERTICES & TEXTCOORDS -------

GLuint background;
GLuint font;
GLuint sprites;


float fullScreenVertices[] = {
    -3.55f, -2.0f, //BL
    3.55f, -2.0f, //BR
    3.55f, 2.0f, //TR
    -3.55f, -2.0f, //BL
    3.55f, 2.0f, //TR
    -3.55f, 2.0f //TL
};

float wholeTexCoords[] = {
    0.0, 1.0,
    1.0, 1.0,
    1.0, 0.0,
    0.0, 1.0,
    1.0, 0.0,
    0.0, 0.0
};

////-------- MATRICES--------

Matrix projectionMatrix;
Matrix modelMatrix;
Matrix viewMatrix;
Matrix playerMatrix;
Matrix doughnutMatrix;
Matrix vegetableMatrix;
Matrix titleMatrix;
Matrix backgroundMatrix;


////-------- TIME ----------

float lastFrameTicks = 0.0f;
float ticks;
float elapsed;

////------ EVENTS ------

SDL_Event event;
bool moveLeft = false;
bool moveRight = false;
bool spacePress = false;


////------ LOAD TEXTURE ---------

GLuint LoadTexture(const char * image_path) {
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

void draw(GLuint texture, float vertices[], ShaderProgram program, float texCoords[]){
    glBindTexture(GL_TEXTURE_2D, texture);
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program.texCoordAttribute);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
}

///------ ENTITY CLASSES -------------


class SheetSprite {
public:
    SheetSprite(){}
    SheetSprite(GLuint texture, Matrix placeholderMatrix, float u, float v, float width, float height, float size): texture(texture), matrix(placeholderMatrix), u(u), v(v), width(width), height(height), size(size){}
    void Draw(ShaderProgram program){
        
            glBindTexture(GL_TEXTURE_2D, texture);
            float texCoords[] = {
                u, v+height,
                u+width, v,
                u, v,
                u+width, v,
                u, v+height,
                u+width, v+height
            };
        
        float aspect = width / height;
        
        float vertices[] = {
            -0.5f * size * aspect, -2.0f * size, //BL
            0.5f * size * aspect, -1.0f * size, //TR
            -0.5f * size * aspect, -1.0f * size, //TL
            0.5f * size * aspect, -1.0f * size, //TR
            -0.5f * size * aspect, -2.0f * size , //BL
            0.5f * size * aspect, -2.0f * size //BR
        };
        
        
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        matrix.identity();
        matrix.Translate(x, y, 0.0);
        program.setModelMatrix(matrix);
        
        glBindTexture(GL_TEXTURE_2D, texture);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
    
    }
    float size;
    GLuint texture;
    float u;
    float v;
    float width;
    float height;
    float x = 0.0f;
    float y = 0.0f;
    float velocity = 2.0f;
    Matrix matrix;
};



SheetSprite player;
SheetSprite doughnut;
std::vector<SheetSprite> doughnuts;
std::vector<SheetSprite> vegetables;


////------ SETUP ---------


ShaderProgram Setup(){
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("doughnuts > diets", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    glViewport(0, 0, 640, 360);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    background = LoadTexture("background.jpg");
    font = LoadTexture("pixel_font.png");
    sprites = LoadTexture("sprites.png");
    projectionMatrix.setOrthoProjection(-3.50, 3.50, -2.0f, 2.0f, -1.0f, 1.0f);
    ShaderProgram program(RESOURCE_FOLDER "vertex_textured.glsl", RESOURCE_FOLDER "fragment_textured.glsl");
    glUseProgram(program.programID);
    program.setModelMatrix(modelMatrix);
    program.setProjectionMatrix(projectionMatrix);
    program.setViewMatrix(viewMatrix);
    return program;
}

////----------- EVENT PROCESSING ---------------

void ProcessMainMenu() {
    // our SDL event loop
    // check input events
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
            
        }
        else if (event.type == SDL_KEYDOWN){
            state = STATE_GAME_LEVEL;
        }
    }
}

void ProcessGame() {
    // our SDL event loop
    // check input events
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
        }
        else if (event.type == SDL_KEYDOWN){
            if (event.key.keysym.scancode == SDL_SCANCODE_LEFT){
                moveLeft = true;
            }
            if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT){
                moveRight = true;
                
            }
            if (event.key.keysym.scancode == SDL_SCANCODE_SPACE){
                if (doughnuts.size()< 30){
                    spacePress = true;
                    for (SheetSprite d : doughnuts){
                        d.x = player.x;
                    }
                }
            }
        }
        else if (event.type == SDL_KEYUP){
            if (event.key.keysym.scancode == SDL_SCANCODE_LEFT){
                moveLeft = false;
            }
            if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT){
                moveRight = false;
                
            }
            if (event.key.keysym.scancode == SDL_SCANCODE_SPACE){
                spacePress = false;
                
            }
        }
    }
}


void ProcessEvents() {
    // our SDL event loop
    // check input events
    switch (state) {
        case STATE_MAIN_MENU:
            ProcessMainMenu();
            break;
        case STATE_GAME_LEVEL:
            ProcessGame();
            break;
        default:
            break;
    }
}


///---------- RENDERING ---------

void DrawText(ShaderProgram *program, int fontTexture, std::string text, float size, float spacing) {
    float texture_size = 1.0/16.0f;
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    for(int i=0; i < text.size(); i++) {
        float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
        float texture_y = (float)(((int)text[i]) / 16) / 16.0f;
        vertexData.insert(vertexData.end(), {
            (float(size+spacing) * i) + (-0.5f * size), 0.5f * size,
            (float(size+spacing) * i) + (-0.5f * size), -0.5f * size,
            (float(size+spacing) * i) + (0.5f * size), 0.5f * size,
            (float(size+spacing) * i) + (0.5f * size), -0.5f * size,
            (float(size+spacing) * i) + (0.5f * size), 0.5f * size,
            (float(size+spacing) * i) + (-0.5f * size), -0.5f * size
        });

        texCoordData.insert(texCoordData.end(), {
            texture_x, texture_y,
            texture_x, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x + texture_size, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x, texture_y + texture_size,
        });
    }
    glUseProgram(program->programID);
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}
                          
            

void RenderMainMenu(ShaderProgram program){
    // for all main menu elements
    // setup transforms, render sprites
    backgroundMatrix.identity();
    program.setModelMatrix(backgroundMatrix);
    draw(background, fullScreenVertices, program, wholeTexCoords);
    titleMatrix.identity();
    titleMatrix.Translate(-2.5, 0.0, 0.0);
    program.setModelMatrix(titleMatrix);
    //glTranslatef(-1.0f, 2.0f, 0.0f);
    DrawText(&program, font, "DOUGHNUTS > DIETS", 0.3f, 0.0f);
    titleMatrix.identity();
    titleMatrix.Translate(-1.5, -0.5, 0.0);
    program.setModelMatrix(titleMatrix);
    DrawText(&program, font, "Press ANY key to start", 0.15f, 0.0f);
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
}

void RenderGameLevel(ShaderProgram program){
    // for all game elements
    // setup transforms, render sprites
    
    if (moveLeft){
         player.x -= player.velocity * elapsed;
    }
    if (moveRight){
        player.x += player.velocity * elapsed;
    }
    if (spacePress){
        SheetSprite Adoughnut = SheetSprite(sprites, doughnutMatrix, 0.0f/1024.0f, 724.0f/1024.0f, 345.0f/1024.0f, 270.0f/1024.0f,0.3f);
        Adoughnut.x = player.x;
        doughnuts.push_back(Adoughnut);
        spacePress = false;
    }
    for (int i = 0; i < 15; ++i){
        SheetSprite apple = SheetSprite(sprites, vegetableMatrix, 0.0f/1024.0f, 0.0f/1024.0f, 375.0f/1024.0f, 345.0f/1024.0f,0.3f);
        SheetSprite pepper = SheetSprite(sprites, vegetableMatrix, 377.0f/1024.0f, 0.0f/1024.0f, 300.0f/1024.0f, 330.0f/1024.0f,0.3f);
        
        
        
        SheetSprite carrot = SheetSprite(sprites, vegetableMatrix, 0.0f/1024.0f, 724.0f/1024.0f, 345.0f/1024.0f, 270.0f/1024.0f,0.3f);
        SheetSprite lettuce = SheetSprite(sprites, vegetableMatrix, 0.0f/1024.0f, 724.0f/1024.0f, 345.0f/1024.0f, 270.0f/1024.0f,0.3f);
        vegetables[i].Draw(program);
    }
    for (int i = 0; i < doughnuts.size(); ++i){
        doughnuts[i].y += doughnuts[i].velocity * elapsed;
        doughnuts[i].Draw(program);
        //std::cout << Adoughnut.y << " RENDER DONUT Y VALUE" << std::endl;
    }
    player.Draw(program);


}
void Render(ShaderProgram program) {
    //std::cout << "Entered Render" << "\n";
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    switch(state) {
        case STATE_MAIN_MENU:
            //std::cout << "Enter Main Menu Render" << "\n";
            RenderMainMenu(program);
            break;
        case STATE_GAME_LEVEL:
            //std::cout << "Enter Game Render" << "\n";
            RenderGameLevel(program);
            break;
    }
}

////---------------- MOVEMENT & COLLISION -----------------------

//void UpdateMainMenu(){
    // move stuff and check for collisions
//}

void UpdateGameLevel(){
    // move stuff and check for collisions
    //call .Update() on all entities
    playerMatrix.Translate(player.x, player.y, 0);
    for (int i = 0; i < doughnuts.size(); ++i){
        doughnuts[i].matrix.Translate(doughnuts[i].x, doughnuts[i].y, 0.0);
        if (doughnuts[i].y >= 2.0){
            doughnuts.erase(doughnuts.begin() + i);
        }
    }
    for (int i = 0; i < vegetables.size(); ++i){
        
    }

}



void Update() {
    // move stuff and check for collisions
    ticks = (float) SDL_GetTicks()/1000.0f;
    elapsed = ticks - lastFrameTicks;
    lastFrameTicks = ticks;
    
    
    switch(state) {
        case STATE_MAIN_MENU:
            //UpdateMainMenu();
            break;
        case STATE_GAME_LEVEL:
            UpdateGameLevel();
            break;
    }
}



void Cleanup() {
   SDL_Quit();

}

int main() {
    ShaderProgram prog = Setup();
    player = SheetSprite(sprites, playerMatrix, 679.0f/1024.0f, 0.0f, 240.0f/1024.0f, 288.0f/1024.0f, 0.7f);
    doughnut = SheetSprite(sprites, doughnutMatrix, 0.0f/1024.0f, 724.0f/1024.0f, 345.0f/1024.0f, 270.0f/1024.0f,0.3f);
    while(!done) {
        ProcessEvents();
        Update();
        Render(prog);
        SDL_GL_SwapWindow(displayWindow);
    }
    Cleanup();
    return 0;
}




