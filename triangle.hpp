//
//  triangle.hpp
//  RayTracing
//
//  Created by Abhijit Bhelande on 6/25/19.
//  Copyright © 2019 Abhijit Bhelande. All rights reserved.
//

#ifndef triangle_h
#define triangle_h

#include "object.hpp"
#include "material.hpp"

// TODO: Add these to compiler flags
#define OPTIMIZE_INTERSECT 1
#define MOLLER_TRUMBORE OPTIMIZE_INTERSECT
#define CULLING OPTIMIZE_INTERSECT
#define INTERPOLATE_PARAMETRIC_NORM 1

class triangle : public object
{
public:
    triangle() = delete;
    triangle(vec3 a,
             vec3 b,
             vec3 c,
             material *mat) : vtx0(a),
                              vtx1(b),
                              vtx2(c),
                              norm(cross(vtx1 - vtx0, vtx2 - vtx0)),
                              D(dot(norm, vtx0)),
                              surfaceMat(mat)
    {}
    
    bool hit(const ray& r, float t_min, float t_max, intersectParams& rec) const
    {
        const vec3 e1 = vtx1 - vtx0;
        const vec3 e2 = vtx2 - vtx0;

#if MOLLER_TRUMBORE
        // We can use simple in / out tri edge test or
        // use optimized Barycentric coordinates (MOLLER TRUMBORE)
        // we solve:
        // [t u v] = 1 / (E1 x E2).R * [ (T x E1).E2 ]
        //                             [ (R x E2).T  ]
        //                             [ (T x E1).R  ]
        // E1, E2: triangle egde vectors (v1 - v0), (v2 - v0)
        // T: ray_origin - v0
        // R: dirn vector of ray
        //
        const vec3 pvec = cross(r.direction(), e2);
        const float det = dot(e1, pvec);

#if CULLING
        // enables check on if tri is front facing or back facing the ray
        //
        // NOTE: when culling is off and the determinant negative, you have
        // to u by multiplying it by the inverse of the determinant.
        // Because we later test if u is greater than 0 and lower than 1, when u
        // u is negative but the determinant also negative, then the sign of u
        // u when normalized (multiplied by invDet) is inverted (it becomes positive).
        //
        // when the ray and the normal of the triangle are facing each other
        // (they go in opposite directions), the determinant is positive.
        //
        // when the ray hits the triangle from "behind" (the ray and the normal
        // points in the same direction), the determinant is negative
        //
        // if the determinant is close to 0, the ray misses the triangle
        if (det < kEpsilon) {
            return false;
        }
#else
        // ray and triangle are parallel if det is close to 0
        if (fabs(det) < kEpsilon) {
            return false;
        }
#endif
        // inverse determinant for cramers rule application
        // to calculate u,v and hence t
        const float invDet = 1 / det;
        
        // calculate u
        const vec3 tvec = r.origin() - vtx0;
        const float u = dot(tvec, pvec) * invDet;
        // u belongs to [0,1] for intersect
        if (u < 0 || u > 1) {
            return false;
        }
        
        // calculate v
        const vec3 qvec = cross(tvec, e1);
        const float v =  dot(qvec, r.direction()) * invDet;
        // v should be > 0 and u + v should be <= 1 for intersect
        if (v < 0 || u + v > 1) {
            return false;
        }
        
        // calculate t
        const float t = dot(e2, qvec) * invDet;
        
        // compute the intersection point
        const vec3 P = r.point_at_parameter(t);
#else
        // n.n
        float denom = norm.squared_length();
        
        // Step 1: finding P
        
        // check if ray and plane are parallel
        const float NdotRayDirection = dot(norm, r.direction());
        if (fabs(NdotRayDirection) < kEpsilon) // almost 0
        {
            return false; // they are parallel so they don't intersect.
        }
        
        // compute t
        const float t = (dot(norm, r.origin()) + D) / NdotRayDirection;
        // check if the triangle is in behind the ray
        if (t < 0) {
            return false; // the triangle is behind
        }
        
        // compute the intersection point
        const vec3 P = r.point_at_parameter(t);
        
        // Step 2: inside-outside test
        vec3 tmp; // vector perpendicular to triangle's plane
        
        // edge 0
        const vec3 edge0 = vtx1 - vtx0;
        const vec3 vp0 = P - vtx0;
        tmp = cross(edge0, vp0);
        if (dot(norm, tmp) < 0) {
            return false; // P is on the right side
        }
        
        // edge 1
        const vec3 edge1 = vtx2 - vtx1;
        const vec3 vp1 = P - vtx1;
        tmp = cross(edge1, vp1);
        float u = dot(norm, tmp);
        if (u < 0) {
            return false; // P is on the right side
        }
        
        // edge 2
        const vec3 edge2 = vtx0 - vtx2;
        const vec3 vp2 = P - vtx2;
        tmp = cross(edge2, vp2);
        float v = dot(norm, tmp);
        if (v < 0) {
            return false; // P is on the right side;
        }
        
        u /= denom;
        v /= denom;
        
#endif
        rec.t = t;
        rec.u = u;
        rec.v = v;
        rec.p = P;
        // Note: You can return the common surface plane normal (norm)
        // Or better yet - the normal interpolated along the edges
        // For the latterm simply use the u,v,w parametric offsets
#if INTERPOLATE_PARAMETRIC_NORM
        rec.normal = vec3(norm.x() + u, norm.y() + v, norm.z() + (1 - u - v));
#else
        rec.normal = norm;
#endif
        rec.surfaceMat = surfaceMat;
        return true;
    }
    
    // vertices
    const vec3 vtx0;
    const vec3 vtx1;
    const vec3 vtx2;
    // Normal to triangle plane
    const vec3 norm;
    // d
    const float D;
    // material
    material* surfaceMat;
};



#endif /* triangle_h */
