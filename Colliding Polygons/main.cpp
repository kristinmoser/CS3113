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
        float fakeX = x;
        x = y;
        y = -fakeX;
    }
};


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
    Pentagon(float x, float xaccel) : x(x), xaccel(xaccel) {}
    float x;
    float y = 1.0f;
   // float yaccel = -((float) std::rand() / (RAND_MAX)) + 2;
    float xaccel;
   // float friction = 0.5f;
    //float rotation = 1.0f;
    Matrix modelMatrix;
    float vertices[30] = {
        
        0.0f, 0.0f,
        0.0f, -1.0f,
        -0.95f, -0.31f,
        
        0.0f, 0.0f,
        -0.95f, -0.31f,
        -0.59f, 0.81f,
        
        0.0f, 0.0f,
        -0.59f, 0.81f,
        0.59f, 0.81f,
        
        0.0f, 0.0f,
        0.59f, 0.81f,
        0.95f, -0.31f,
        
        0.0f, 0.0f,
        0.95f, -0.31f,
        0.0f, -1.0f
    };
    
    std::vector<Vector> normVertices = {
        Vector(0.0f, 0.0),
        Vector(0.0f, -1.0f),
        Vector(-0.95f, -0.31f),
        
        Vector(0.0f, 0.0f),
        Vector(-0.95f, -0.31f),
        Vector(-0.59f, 0.81f),
        
       Vector( 0.0f, 0.0f),
        Vector(-0.59f, 0.81f),
        Vector(0.59f, 0.81f),
        
        Vector(0.0f, 0.0f),
        Vector(0.59f, 0.81f),
        Vector(0.95f, -0.31f),
        
        Vector(0.0f, 0.0f),
        Vector(0.95f, -0.31f),
        Vector(0.0f, -1.0f)
    };
    
    float texcoords[30] = {
        0.0f, 0.0f,
        0.0f, 1.0f,
        0.95f, 0.31f,
        
        0.0f, 0.0f,
        0.95f, 0.31f,
        0.59f, 0.81f,
        
        0.0f, 0.0f,
        0.59f, 0.81f,
        0.59f, 0.81f,
        
        0.0f, 0.0f,
        0.59f, 0.81f,
        0.95f, 0.31f,
        
        0.0f, 0.0f,
        0.95f, 0.31f,
        0.0f, 1.0f
    };
    

    void draw(ShaderProgram program)
    {

        program.setModelMatrix(modelMatrix);
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texcoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 15);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
    }
    
    
};

void somethingToWorld(const Matrix& modelMatrix, Vector& vec){
    float x = vec.x;
    float y = vec.y;
    vec.x = modelMatrix.m[0][0] * x + modelMatrix.m[0][1] * y + modelMatrix.m[2][0] * 1.0f + modelMatrix.m[3][0] * 1.0f;
    vec.y = modelMatrix.m[1][0] * x + modelMatrix.m[1][1] * y + modelMatrix.m[2][1] * 1.0f + modelMatrix.m[3][1] * 1.0f;
}


bool checkCollision(Pentagon shape1, Pentagon shape2){
    //call checkSATCollision(const std::vector<Vector> &e1Points, const std::vector<Vector> &e2Points) with
    //vertices from shape1 and shape2 put into world space
    for (int i = 0; i < shape1.normVertices.size(); i++){
            somethingToWorld(modelMatrix, shape1.normVertices[i]);
    }
    for (int i = 0; i < shape1.normVertices.size(); i++){
        somethingToWorld(modelMatrix, (shape2.normVertices[i], shape2.normVertices[i+1]));
    }
    return checkSATCollision(shape1.normVertices, shape2.normVertices);
}



int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("colliding pentagons", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 700, 600, SDL_WINDOW_OPENGL);
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

   Pentagon pent1 = Pentagon(3.0, -3.0);
    Pentagon pent2 = Pentagon(-3.0, 3.0);

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
        int maxChecks = 10;
        //for (int i = 0; i < pentagons.size(); ++i){
           // if(-5.0 < pentagons[i].y < 5.0){
               pent1.y += -1.0 * elapsed;
            //}
            //if (-5.0 < pentagons[i].x < 5.0){
                pent1.x += pent1.xaccel * elapsed;
                pent2.x += pent2.xaccel * elapsed;
            //}
            //pentagons[i].rotation += elapsed;
            pent1.modelMatrix.identity();
            //pentagons[i].modelMatrix.Rotate(pentagons[i].rotation);
            pent1.modelMatrix.Translate(pent1.x, pent1.y, 0);
            pent1.draw(program);
            //pentagons[i].rotation += elapsed;
            pent2.modelMatrix.identity();
            //pentagons[i].modelMatrix.Rotate(pentagons[i].rotation);
            pent2.modelMatrix.Translate(pent2.x, pent2.y, 0);
            pent2.draw(program);
        //}
        while(checkCollision(pent1, pent2) && maxChecks > 0) {
            Vector responseVector = Vector(pent1.x - pent2.x, pent1.y - pent2.y);
            responseVector.normalize();
            pent1.x -= responseVector.x * 0.0002;
            pent1.y -= responseVector.y * 0.0002;
            pent2.x -= responseVector.x * 0.0002;
            pent2.y -= responseVector.y * 0.0002;
            maxChecks -= 1;
//            
        }

        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}

