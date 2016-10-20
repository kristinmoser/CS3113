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
enum GameState { STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_GAME_OVER };
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
Matrix carrotMatrix;
Matrix appleMatrix;
Matrix lettuceMatrix;
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
SheetSprite apple;
SheetSprite pepper;
SheetSprite carrot;
SheetSprite lettuce;
std::vector<SheetSprite> doughnuts;
std::vector<SheetSprite> carrots;
std::vector<SheetSprite> apples;
std::vector<SheetSprite> lettuces;
std::vector<SheetSprite> peppers;

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
    player = SheetSprite(sprites, playerMatrix, 679.0f/1024.0f, 0.0f, 240.0f/1024.0f, 288.0f/1024.0f, 0.5f);
    apple = SheetSprite(sprites, appleMatrix, 0.0f/1024.0f, 0.0f/1024.0f, 375.0f/1024.0f, 345.0f/1024.0f, 0.4f);
    pepper = SheetSprite(sprites, appleMatrix, 377.0f/1024.0f, 0.0f/1024.0f, 300.0f/1024.0f, 330.0f/1024.0f, 0.5f);
    carrot = SheetSprite(sprites, carrotMatrix, 362.0f/1024.0f, 347.0f/1024.0f, 345.0f/1024.0f, 405.0f/1024.0f, 0.5f);
    lettuce = SheetSprite(sprites, lettuceMatrix, 0.0f/1024.0f, 347.0f/1024.0f, 360.0f/1024.0f, 375.0f/1024.0f, 0.45f);
    carrot.y = 2.4f;
    apple.y = 1.8f;
    lettuce.y = 1.4f;
    pepper.y = .95f;
    for (int i = 0; i < 17; ++i){
        carrots.push_back(carrot);
    }
    for (int j = 0; j < 11; ++j){
        apples.push_back(apple);
    }
    for (int k = 0; k < 14; ++k){
        lettuces.push_back(lettuce);
    }
    for (int l = 0; l < 11; ++l){
        peppers.push_back(pepper);
    }
    player.y = -0.9f;
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

void ProcessGameOver(){
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
            
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
        case STATE_GAME_OVER:
            ProcessGameOver();
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
        SheetSprite Adoughnut = SheetSprite(sprites, doughnutMatrix, 0.0f/1024.0f, 724.0f/1024.0f, 345.0f/1024.0f, 270.0f/1024.0f,0.2f);
        Adoughnut.x = player.x;
        doughnuts.push_back(Adoughnut);
        spacePress = false;
    }
    std::cout << ticks << std::endl;
    for (int k = 0; k < carrots.size(); ++k){
        carrots[k].x = -3.2f + (k * .40f);
        if (fmod(ticks, 4) > 0 && fmod(ticks, 4) < .02){
            carrots[k].y -= 0.2f;
        }
        carrots[k].Draw(program);
    }
    for (int j = 0; j < apples.size(); ++j){
        apples[j].x = -3.2f + (j * .70f);
        if (fmod(ticks, 4) > 0 && fmod(ticks, 4) < .02){
            apples[j].y -= 0.2f;
        }
        apples[j].Draw(program);
    }
    for (int l = 0; l < lettuces.size(); ++l){
        lettuces[l].x = -3.2f + (l * .50f);
        if (fmod(ticks, 4) > 0 && fmod(ticks, 4) < .02){
            lettuces[l].y -= 0.2f;
        }
        lettuces[l].Draw(program);
    }
    for (int m = 0; m < peppers.size(); ++m){
        peppers[m].x = -3.2f + (m * .650f);
        if (fmod(ticks, 4) > 0 && fmod(ticks, 4) < .02){
            peppers[m].y -= 0.2f;
        }
        peppers[m].Draw(program);
    }
    for (int i = 0; i < doughnuts.size(); ++i){
        doughnuts[i].y += doughnuts[i].velocity * elapsed;
        doughnuts[i].Draw(program);
        //std::cout << Adoughnut.y << " RENDER DONUT Y VALUE" << std::endl;
    }

    player.Draw(program);


}

void RenderGameOver(ShaderProgram program){
    backgroundMatrix.identity();
    program.setModelMatrix(backgroundMatrix);
    draw(background, fullScreenVertices, program, wholeTexCoords);
    titleMatrix.identity();
    titleMatrix.Translate(-2.5, 0.0, 0.0);
    program.setModelMatrix(titleMatrix);
    DrawText(&program, font, "GAME OVER", 0.3f, 0.0f);
    titleMatrix.identity();
    titleMatrix.Translate(-1.5, -0.5, 0.0);
    program.setModelMatrix(titleMatrix);
    //DrawText(&program, font, "SCORE: " + (string)score, 0.15f, 0.0f);
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
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
        case STATE_GAME_OVER:
            RenderGameOver(program);
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
        for (int j = 0; j < carrots.size(); ++j){
            for (int k = 0; k < apples.size(); ++k){
                for (int l = 0; l < lettuces.size(); ++l){
                    for (int m = 0; m < peppers.size(); ++m){
                        if (doughnuts[i].y < carrots[j].y + carrots[j].height/2 && doughnuts[i].y > carrots[j].y - carrots[j].height/2){
                            if (doughnuts[i].x < carrots[j].x + carrots[j].width/2 && doughnuts[i].x > carrots[j].x - carrots[j].width/2){
                                doughnuts.erase(doughnuts.begin() + i);
                                carrots.erase(carrots.begin() + j);
                            }
                        }
                        if (doughnuts[i].y < apples[k].y + apples[k].height/2 && doughnuts[i].y > apples[k].y - apples[k].height/2){
                            if (doughnuts[i].x < apples[k].x + apples[k].width/2 && doughnuts[i].x > apples[k].x - apples[k].width/2){
                                doughnuts.erase(doughnuts.begin() + i);
                                apples.erase(apples.begin() + k);
                            }
                        }
                        if (doughnuts[i].y < lettuces[l].y + lettuces[l].height/2 && doughnuts[i].y > lettuces[l].y - lettuces[l].height/2){
                            if (doughnuts[i].x < lettuces[l].x + lettuces[l].width/2 && doughnuts[i].x > lettuces[l].x - lettuces[l].width/2){
                                doughnuts.erase(doughnuts.begin() + i);
                                lettuces.erase(lettuces.begin() + l);
                            }
                        }
                        if (doughnuts[i].y < peppers[m].y + peppers[m].height/2 && doughnuts[i].y > peppers[m].y - peppers[m].height/2){
                            if (doughnuts[i].x < peppers[m].x + peppers[m].width/2 && doughnuts[i].x > peppers[m].x - peppers[m].width/2){
                                doughnuts.erase(doughnuts.begin() + i);
                                peppers.erase(peppers.begin() + m);
                            }
                         }
                }
            }
        }
    }
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
    while(!done) {
        ProcessEvents();
        Update();
        Render(prog);
        SDL_GL_SwapWindow(displayWindow);
    }
    Cleanup();
    return 0;
}




