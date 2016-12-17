//
//  polygon.h
//  NYUCodebase
//
//  Created by Kristin Moser on 12/7/16.
//  Copyright © 2016 Ivan Safrin. All rights reserved.
//

#ifndef polygon_h
#define polygon_h


#endif /* polygon_h */
//
//  polygon.hpp
//  NYUCodebase
//
//  Created by Brian Quinn on 12/6/16.
//  Copyright © 2016 Ivan Safrin. All rights reserved.
//

#ifndef polygon_hpp
#define polygon_hpp

#include <stdio.h>
#include <vector>
#include "ShaderProgram.h"

#endif /* polygon_hpp */

using namespace std;

struct Polygon {
    Matrix modelMatrix;
    
    vector<float>* coordinates;
    vector<float>* texcoordinates;
    int vertices;
    float scale;
    float rotationSpeed;
    
    float rotation;
    pair<float,float> position;
    pair<float,float> velocity;
    
    void update(float elapsed);
    void render(ShaderProgram* program);
    
protected:
    Polygon(float x, float y, int vertices);
};

struct Triangle : Polygon {
    Triangle(float x, float y);
};

struct Square : Polygon {
    Square(float x, float y);
};

struct Pentagon : Polygon {
    Pentagon(float x, float y);
};

struct Hexagon : Polygon {
    Hexagon(float x, float y);
};
