#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#include <vector>
using namespace std;

SDL_Window* displayWindow;

Matrix projectionMatrix;
Matrix viewMatrix;
Matrix modelMatrix;

class Vector{
public:
    Vector(){}
    Vector(float x, float y) : x(x), y(y) {}
    float x;
    float y;
    void normalize(){
        float length = sqrtf(x * x + y * y);
        x /= length;
        y /= length;
    }
};

Vector modelToWorldCoordinates(const Matrix& modelMatrix, const Vector & modelCoordinates)
{
    Vector worldCoordinates;
    worldCoordinates.x = modelMatrix.m[0][0] * modelCoordinates.x + modelMatrix.m[0][1] * modelCoordinates.y + modelMatrix.m[2][0] * 1.0f + modelMatrix.m[3][0] * 1.0f;
    worldCoordinates.y = modelMatrix.m[1][0] * modelCoordinates.x + modelMatrix.m[1][1] * modelCoordinates.y + modelMatrix.m[2][1] * 1.0f + modelMatrix.m[3][1] * 1.0f;
    return worldCoordinates;
}


bool testSATSeparationForEdge(float edgeX, float edgeY, const std::vector<Vector> &points1, const std::vector<Vector> &points2) {
    float normalX = -edgeY;
    float normalY = edgeX;
    float len = sqrtf(normalX*normalX + normalY*normalY);
    normalX /= len;
    normalY /= len;
    
    std::vector<float> e1Projected;
    std::vector<float> e2Projected;
    
    for(int i=0; i < points1.size(); i++) {
        e1Projected.push_back(points1[i].x * normalX + points1[i].y * normalY);
    }
    for(int i=0; i < points2.size(); i++) {
        e2Projected.push_back(points2[i].x * normalX + points2[i].y * normalY);
    }
    
    std::sort(e1Projected.begin(), e1Projected.end());
    std::sort(e2Projected.begin(), e2Projected.end());
    
    float e1Min = e1Projected[0];
    float e1Max = e1Projected[e1Projected.size()-1];
    float e2Min = e2Projected[0];
    float e2Max = e2Projected[e2Projected.size()-1];
    float e1Width = fabs(e1Max-e1Min);
    float e2Width = fabs(e2Max-e2Min);
    float e1Center = e1Min + (e1Width/2.0);
    float e2Center = e2Min + (e2Width/2.0);
    float dist = fabs(e1Center-e2Center);
    float p = dist - ((e1Width+e2Width)/2.0);
    
    if(p < 0) {
        return true;
    }
    return false;
}

bool checkSATCollision(const std::vector<Vector> &e1Points, const std::vector<Vector> &e2Points) {
    for(int i=0; i < e1Points.size(); i++) {
        float edgeX, edgeY;
        
        if(i == e1Points.size()-1) {
            edgeX = e1Points[0].x - e1Points[i].x;
            edgeY = e1Points[0].y - e1Points[i].y;
        } else {
            edgeX = e1Points[i+1].x - e1Points[i].x;
            edgeY = e1Points[i+1].y - e1Points[i].y;
        }
        
        bool result = testSATSeparationForEdge(edgeX, edgeY, e1Points, e2Points);
        if(!result) {
            return false;
        }
    }
    for(int i=0; i < e2Points.size(); i++) {
        float edgeX, edgeY;
        
        if(i == e2Points.size()-1) {
            edgeX = e2Points[0].x - e2Points[i].x;
            edgeY = e2Points[0].y - e2Points[i].y;
        } else {
            edgeX = e2Points[i+1].x - e2Points[i].x;
            edgeY = e2Points[i+1].y - e2Points[i].y;
        }
        bool result = testSATSeparationForEdge(edgeX, edgeY, e1Points, e2Points);
        if(!result) {
            return false;
        }
    }
    return true;
}


class Pentagon{
public:
    Pentagon(float x) : x(x) {}
    float x;
    float y = 4.0f;
    float width = 2.0f;
    float height = 3.0f;
    float yaccel = -((float) std::rand() / (RAND_MAX)) + 2;
    float xaccel = -((float) std::rand() / (RAND_MAX)) + 2;
    float friction = 0.5f;
    float rotation = 1.0f;
    Matrix modelMatrix;
    vector<Vector> edges ={
        Vector(x+width/2.0f, y-height/2.0f),
        Vector(x-width/2.0f, y+height/2.0f),
        Vector(x+width/2.0f, y+height/2.0f),
        Vector(x-width/2.0f, y+height/2.0f)
        
    };
    void update(ShaderProgram program){
        if (y - height/2 <= -5.0f){
            y = -5.0f + height/2;
            yaccel *= -1;
        }
        if (x + width/2 >= 5.0f){
            x = 5.0f - width/2;
            xaccel *= -1;
        }
        if (x - width/2 <= -5.0f){
            x = -5.0f + width/2;
            xaccel *= -1;
        }
        if (y + height/2 >= 5.0f){
            y = 5.0f - height/2;
            yaccel *= -1;
        }

        
    }
    
    void draw(ShaderProgram program)
    {
        float vertices[] = {
            -1.0f, -0.5f,    // A
            1.0f, -0.5f,    // B
            0.0f,  0.5f,    // C
            -1.5f,  0.0f,    // D
            -1.5f, -1.0f,    // E
            0.5f, -1.5f,    // F
            1.0f, -0.5f,
        };
        float texcoords[] = {
            0.0, 1.0,
            1.0, 1.0,
            1.0, 0.0,
            0.0, 1.0,
            1.0, 0.0,
            0.0, 0.0
        };

        program.setModelMatrix(modelMatrix);
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texcoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLE_FAN, 0, 7);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
    }
    
    
};

Vector convertToWorldCoords(Matrix transformations, Vector objCoords){
    Vector worldCoords;
    worldCoords.x = ((transformations.m[0][0] * objCoords.x) + (transformations.m[1][0] * objCoords.y));
    worldCoords.y = ((transformations.m[0][1] * objCoords.x) + (transformations.m[1][1] * objCoords.y));
    return worldCoords;
}

bool checkCollision(Pentagon pent1, Pentagon pent2){
    vector<Vector> e1Points;
    vector<Vector> e2Points;
    for (int i = 0; i < pent1.edges.size(); ++i){
        e1Points.push_back(Vector(convertToWorldCoords(pent1.modelMatrix, pent1.edges[i])));
    }
    for (int i = 0; i < pent1.edges.size(); ++i){
        e2Points.push_back(Vector(convertToWorldCoords(pent2.modelMatrix, pent2.edges[i])));
    }
    return checkSATCollision(e1Points, e2Points);
}




int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 700, 600, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    
    SDL_Event event;
    bool done = false;
    float lastFrameTicks = 0.0f;
    glViewport(0, 0, 700, 600);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    ShaderProgram program(RESOURCE_FOLDER "vertex.glsl", RESOURCE_FOLDER "fragment.glsl");
    projectionMatrix.setOrthoProjection(-5.0f, 5.0f, -5.0f, 5.0f, -5.0f, 5.0f);
    glUseProgram(program.programID);
    program.setProjectionMatrix(projectionMatrix);
    program.setViewMatrix(viewMatrix);

    vector<Pentagon> pentagons;
    for (int i = -2; i < 4; i += 2){
        pentagons.push_back(*new Pentagon(i));
    }
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }
    
        glClear(GL_COLOR_BUFFER_BIT);
        
        float ticks = (float) SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        int maxChecks = 100;
        for (int i = 0; i < pentagons.size()-1; ++i){
            pentagons[i].update(program);
            while(checkCollision(pentagons[i], pentagons[i+1]) && maxChecks > 0) {
                Vector responseVector = Vector(pentagons[i].x - pentagons[i + 1].x, pentagons[i].y - pentagons[i +1].y);
                responseVector.normalize();
                std::cout << "reached it balh" << std::endl;
                pentagons[i].x -= responseVector.x * 0.002;
                pentagons[i].y -= responseVector.y * 0.002;
                pentagons[i+1].x += responseVector.x * 0.002;
                pentagons[i+1].y += responseVector.y * 0.002;
                maxChecks -= 1;
            }
        }
        for (int i = 0; i < pentagons.size(); ++i){
            pentagons[i].y += pentagons[i].yaccel * elapsed;
            pentagons[i].x += pentagons[i].xaccel * elapsed;
            pentagons[i].rotation += elapsed;
            pentagons[i].modelMatrix.identity();
            pentagons[i].modelMatrix.Rotate(pentagons[i].rotation);
            pentagons[i].modelMatrix.Translate(pentagons[i].x, pentagons[i].y, 0);
            pentagons[i].draw(program);
        }


        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}

