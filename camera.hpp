//
//  camera.hpp
//  RayTracing
//
//  Created by Abhijit Bhelande on 6/25/19.
//  Copyright © 2019 Abhijit Bhelande. All rights reserved.
//

#ifndef camera_h
#define camera_h

#include "ray.hpp"
#include "util.hpp"

class camera
{
public:
    // TODO: Commonize init code

    // camera is based on gluPerspective
    // . vertical field of view (in degrees)
    // . aspect ratio
    // Left handed coords, so Z faces in '-'ve direction
    // proj plane is at -1 and has lower left origin
    //
    camera(float fovy,
           float aspect) : origin(0.0f)
    {
        const float thetaRad = degToRad(fovy);
        const float height_by_two = tan(thetaRad);
        const float width_by_two = height_by_two * aspect;
        constexpr float znear = -1.0f;
        lensRadius = 0.0f;
        
        fwd = vec3(0.0f, 0.0f, -1.0f);
        right = vec3(1.0f, 0.0f, 0.0f);
        up = vec3(0.0f, 1.0f, 0.0f);
        
        lowerLeft = vec3(-width_by_two, -height_by_two, znear);
        horizontal = vec3(width_by_two * 2.0f, 0.0f, 0.0f);
        vertical = vec3(0.0f, height_by_two * 2.0f, 0.0f);
    }
    
    // lookAt variant
    // additionally takes origin, target as parameters
    // up is fixed at <0, 1, 0> (Keeping things simple for now)
    camera(float fovy,
           float aspect,
           vec3 lookFrom,
           vec3 lookAt) : origin(lookFrom)
    {
        const float thetaRad = degToRad(fovy);
        const float height_by_two = tan(thetaRad);
        const float width_by_two = height_by_two * aspect;
        lensRadius = 0.0f;

        fwd = unit_vector(lookFrom - lookAt);
        right = unit_vector(cross(vec3(0.0f, 1.0f, 0.0f), fwd));
        up = cross(fwd, right);
        
        lowerLeft = origin - width_by_two * right - height_by_two * up - fwd;
        horizontal = 2.0f * width_by_two * right;
        vertical = 2.0f * height_by_two * up;
    }
    
    // depth of field variant
    // dof is simulated by simply having ray origins be generated on unit
    // disk around cam center
    camera(float fovy,
           float aspect,
           vec3 lookFrom,
           vec3 lookAt,
           float aperture,
           float focalDistance) : origin(lookFrom)
    {
        const float thetaRad = degToRad(fovy);
        const float height_by_two = tan(thetaRad);
        const float width_by_two = height_by_two * aspect;
        lensRadius = aperture / 2;
        
        fwd = unit_vector(lookFrom - lookAt);
        right = unit_vector(cross(vec3(0.0f, 1.0f, 0.0f), fwd));
        up = cross(fwd, right);
        
        lowerLeft = origin - focalDistance * (width_by_two * right + height_by_two * up + fwd);
        horizontal = 2.0f * width_by_two * focalDistance * right;
        vertical = 2.0f * height_by_two * focalDistance * up;
    }
    
    // return ray object given a u,v scan coord across the projection plane
    // u,v are normalized to [0, 1] and must be scaled by plane dimensions
    // for offset from origin
    ray getRayAt(float u,
                 float v)
    {
        vec3 radVec = lensRadius * unitSphereRandomRadVec();
        vec3 offset = right * radVec.x() + up * radVec.y();
        return ray(origin + offset,
                   lowerLeft + u * horizontal + v * vertical - origin - offset);
    }
    
    vec3 lowerLeft;
    vec3 horizontal;
    vec3 vertical;
    vec3 origin;
    vec3 fwd;
    vec3 right;
    vec3 up;
    float lensRadius;
};


#endif /* camera_h */
