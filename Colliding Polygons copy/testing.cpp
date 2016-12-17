

ShaderProgram setup(){
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("space in(love)ers", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
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
    
    void render(){
        
    }
};




int main(int argc, char *argv[]){
    ShaderProgram program = setup();
    GLuint leftText = LoadTexture("/images/yellow.png");
    SDL_Event event;
    bool done = false;
    Paddle player;
    
    bool akeydown = false;
    bool skeydown = false;
    bool startMenu = true;
    while (!done) {
        if (startMenu){
            
        }
        else{
            
            
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


        }
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
        
}
