//
//  texture.hpp
//  RayTracing
//
//  Created by Abhijit Bhelande on 6/25/19.
//  Copyright © 2019 Abhijit Bhelande. All rights reserved.
//

#ifndef texture_h
#define texture_h

#include "vec3.hpp"

class texture
{
public:
    virtual vec3 texelAt(float u,
                         float v,
                         const vec3& p) const = 0;
};

class flatShade : public texture
{
public:
    flatShade() = delete;
    flatShade(vec3 col) : color(col) {}
    
    virtual vec3 texelAt(float u, float v, const vec3& p) const
    {
        return color;
    }
    
    vec3 color;
};

class checkerBoard : public texture
{
public:
    checkerBoard() = delete;
    checkerBoard(flatShade* t0,
                 flatShade* t1) : shade0(t0),
                                  shade1(t1) {}
    
    virtual vec3 texelAt(float u, float v, const vec3& p) const
    {
        constexpr float tileFactor = 25.0f;
        // scale uv to tile factor
        // the tile factor determines how many pixels
        // each check will cover.
        // # of checkers is proportional to tileFactor
        // i.e. checkerSize is inversely proportional
        const float a = floor(u * tileFactor);
        const float b = floor(v * tileFactor);
        // use (a + b) to decide if we're in even or odd square
        if (fmod(a + b, 2.0) > 0.5) {
            // odd shade
            return shade0->texelAt(u, v, p);
        }
        else {
            // even shade
            return shade1->texelAt(u, v, p);
        }
    }
    
    flatShade *shade0;
    flatShade *shade1;
};

#endif /* texture_h */
