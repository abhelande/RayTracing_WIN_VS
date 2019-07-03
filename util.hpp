//
//  util.hpp
//  RayTracing
//
//  Created by Abhijit Bhelande on 6/25/19.
//  Copyright © 2019 Abhijit Bhelande. All rights reserved.
//

#ifndef util_h
#define util_h

#include <cmath>
#include "vec3.hpp"

constexpr float kEpsilon = 1e-8;

#ifdef WIN32
inline double drand48()
{
	return double(rand()) / RAND_MAX;
}

#define M_PI 3.14159265358979323846264338327950288
#endif

inline float degToRad(float degrees)
{
    return degrees * (M_PI / 180.0f);
}

// polynomial approximation by Christophe Schlick
// to approximate reflectivity factor depending on transparent surface
// viewing angle (i.e. glass reflects some light depending on viewing angle)
//
// specular reflection coefficient R can be approximated by:
// R(θ) = R0 + (1 - R0) * ((1 - cos(θ))^5)
// R0 = [(n_in - n_out) / (n_in + n_out)]^2
// below we take n_in = 1
// θ: angle between incident light and the normal at point of incidence
//    i.e. cos(θ) = I.N
float schlick(float cosine,
              float outRefIdx) {
    float r0 = (1.0f - outRefIdx) / (1.0f + outRefIdx);
    r0 = r0 * r0;
    return r0 + (1.0f - r0) * pow((1.0f - cosine), 5);
}

// This is the same as glsl reflect
// Reflected = I - 2 * dot (I, N) * N
// I: Incident Ray
// N: Normal at point of incidence
vec3 reflect(const vec3& I, const vec3& N)
{
    return I - 2.0f * dot(I, N) * N;
}

// same as glsl refract
// but needs refracted ray as returnable parameter
// I: incident vector.
// N: Specifies the normal vector.
// eta: ratio of indices of refraction.
// refraction vector, R:
// k = 1.0 - eta * eta * (1.0 - dot(N, I) * dot(N, I));
// if (k < 0.0)
//     R = vec(0.0);
// else
//
//     R = eta * I - (eta * dot(N, I) + sqrt(k)) * N;
//
bool refract(const vec3& I, const vec3& N, float eta, vec3& refracted) {
    vec3 uv = unit_vector(I);
    float dt = dot(uv, N);
    float discriminant = 1.0f - eta * eta * (1.0f - dt * dt);
    if (discriminant > kEpsilon) {
        refracted = eta * (uv - N * dt) - N * sqrt(discriminant);
        return true;
    }
    else
        return false;
}

// generate a random unit vector, direction
// of which is towards a point on unit sphere surface
vec3 unitSphereRandomRadVec()
{
    vec3 rdm;
    do {
        rdm = 2.0f * vec3(drand48(), drand48(), drand48()) - vec3(1.0f);
    } while (rdm.squared_length() >= 1.0f);
    
    return rdm;
}

// solve for ax^2 + bx + c = 0
bool getQuadraticRoots(float a,
                       float b,
                       float c,
                       float &root0,
                       float &root1)
{
    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0.0f) {
        return false;
    }
    
    if (discriminant == 0) {
        root0 = root1 = - 0.5 * b / a;
    } else {
        // when b and the root of the discriminant are nearly equal
        // we may have precision losses.
        // so formulate this as:
        // q = -1/2 * (b + sign(b) * sqrt(discriminant))
        // where sign(b) is '-'ve for b < 0 only
        // mainly anything added to q is now the same sign
        // root0 = q / a
        // root1 = c / q
        //
        float q = (b > 0.0f) ? -0.5f * (b + sqrt(discriminant)) :
                               -0.5f * (b - sqrt(discriminant));
        root0 = q / a;
        root1 = c / q;
        
        if (root0 > root1) {
            std::swap(root0, root1);
        }
    }
    
    return true;
}


#endif /* util_h */
