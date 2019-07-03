//
//  sphere.h
//  RayTracing
//
//  Created by Abhijit Bhelande on 6/25/19.
//  Copyright © 2019 Abhijit Bhelande. All rights reserved.
//

#ifndef sphere_h
#define sphere_h

#include "object.hpp"
#include "material.hpp"
#include <cmath>

// The math in the book seems off (even though output is similar) ??
// Im guessing hes taken advantage of the fact that a in the quadratic
// to solve is 1 for unit ray direction
// so disabling
#define SHIRLEY_ROOTS 0

class sphere : public object
{
public:
    sphere() = delete;
    sphere(vec3 cen,
           float rad,
           material* mat) : center(cen),
                            radius(rad),
                            surfaceMat(mat) {}
    
    bool hit(const ray& r,
             float t_min,
             float t_max,
             intersectParams& rec) const
    {
        // For sphere whose center is located at C (<Cx, Cy, Cz>) with Radius R (scalar),
        // Eqn for point P (<Px, Py, Pz>) on the spheres surface,
        // (Px - Cx)^2 + (Py - Cy)^2 + (Px - Cx)^2 = R^2
        // i.e. (P - C).(P - C) - R^2 = 0
        //
        // Substitute P = Ray eqn = O + t*D
        // you get quadratic form: a*x^2 + b*x + c = 0
        // a = |D|^2
        // b = 2 *(O - C).(D)
        // c = |(O - C)|^2 - R^2
        //
        // solving this,
        // discriminant < 0 => no intersection
        // otherwise use roots to determine point of intersection
        vec3 oc = r.origin() - center;
        float a = dot(r.direction(), r.direction());
        float b = dot(oc, r.direction());
        float c = dot(oc, oc) - radius * radius;
        
        float root = 0.0f;
        bool didHit = false;
#if SHIRLEY_ROOTS
        float discriminant = b * b - a * c;
        if (discriminant > 0) {
            bool didHit = false;
            float root = 0.0f;
            
            // 0 discriminant
            if (discriminant < kEpsilon) {
                root = -b / a * 0.5f;
                didHit = true;
            } else {
                // check root 1
                root = (-b - sqrt(discriminant)) / a;
                if (root < t_max && root > t_min) {
                    didHit = true;
                } else {
                    // root 2
                    root = (-b + sqrt(discriminant)) / a;
                    didHit = (root < t_max && root > t_min);
                }
            }
        }
#else
        b *= 2.0f;
        float t0, t1;
        if (getQuadraticRoots(a, b, c, t0, t1)) {
            if (t0 < t_max && t0 > t_min) {
                root = t0;
                didHit = true;
            } else if (t1 < t_max && t1 > t_min) {
                root = t1;
                didHit = true;
            }
        }
#endif
        
        if (didHit) {
            rec.t = root;
            // root is used to find point of intersection
            rec.p = r.point_at_parameter(root);
            // normal is simply outwards from center to that point
            rec.normal = (rec.p - center) / radius;
            rec.surfaceMat = surfaceMat;
            // uv calc (cylindrical coords)
            // divide by (2 x PI) to convert the returned angle to [-0.5, 0.5] range
            // N.y = v
            // 0.5 add to shift to [0,1] range
            rec.u = atan2(rec.normal.x(), rec.normal.z()) / (2 * M_PI) + 0.5f;
            rec.v = rec.normal.y() * 0.5f + 0.5f;
            return true;
        }
        
        return false;
    }
    
    vec3 center;
    float radius;
    material *surfaceMat;
};


#endif /* sphere_h */
