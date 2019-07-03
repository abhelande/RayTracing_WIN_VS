//
//  material.hpp
//  RayTracing
//
//  Created by Abhijit Bhelande on 6/25/19.
//  Copyright © 2019 Abhijit Bhelande. All rights reserved.
//

#ifndef material_h
#define material_h

struct intersectParams;

#include "ray.hpp"
#include "object.hpp"
#include "texture.hpp"
#include "util.hpp"

// abstract class representing how surface of intersected object will behave
// scatter is the ray generated from interaction at that point
//         will be directly refelcted for pure metal like surfaces vs
//         randomly scattered for diffuse like behavior
// attenuation represents material absorbtion factor
//             thus reducing strength of the incoming light intensity per
//             bounce (interaction with this surface). Here the radiosity
//             (strenght) is represented by the rgb tupule values that we
//             multiplicatively reduce.
//
// Effectively the interaction at point of intersection may:
// . generate a scattered ray that is attenuated by the attnuation factor
// . absorb part of incident ray by a factor if (1 - (attenuation factor))
// Thus conserving energy and encapsulating a microfacet interaction
//
// Currently we only generate a single scattered ray (This too can be subject to
// multiple spawn and gathers otherwise
class material
{
public:
    virtual bool scatter(const ray& ray_in,
                         const intersectParams& rec,
                         vec3& attenuation,
                         ray& scattered) const = 0;
};

// Lambertian is basic diffuse scattering
// scatter incoming ray in a random direction
// each bounce adds an attenuation
class lambertian : public material
{
public:
    lambertian() = delete;
    lambertian(const vec3& a) : albedo(a) {}
    
    virtual bool scatter(const ray& ray_in,
                         const intersectParams& rec,
                         vec3& attenuation,
                         ray& scattered) const
    {
        // effectively scatter with some probability
        // that probability depends on the pdf of the random fucntion
        // used in unitSphereRandomRadVec
        vec3 bounce = rec.p + rec.normal + unitSphereRandomRadVec();
        scattered = ray(rec.p, bounce - rec.p);
        attenuation = albedo;
        return true;
    }
    
    vec3 albedo;
};

// Same as Lambertian
// excpet diffuse albedo is via texture look up
class lambertianTexture : public material
{
public:
    lambertianTexture() = delete;
    lambertianTexture(texture* a) : albedo(a) {}
    
    virtual bool scatter(const ray& ray_in, const intersectParams& rec, vec3& attenuation, ray& scattered) const
    {
        vec3 bounce = rec.p + rec.normal + unitSphereRandomRadVec();
        scattered = ray(rec.p, bounce - rec.p);
        attenuation = albedo->texelAt(rec.u, rec.v, rec.p);
        return true;
    }
    
    texture* albedo;
};

// Smooth Metal Mirror material
// ray won’t be randomly scattered but is reflected using
// Reflected = I - 2 * dot (I, N) * N
// I: Incident Ray
// N: Normal at point of incidence
// This is easy to derive (See Eric Lengyels book)
//
class metal : public material
{
public:
    metal() = delete;
    metal(const vec3& a, float fuzz) : albedo(a),
                                       fuzziness(std::max(fuzz, 1.0f)) {}
    metal(const vec3& a): albedo(a),
                          fuzziness(0.0f) {}
    
    virtual bool scatter(const ray& ray_in,
                         const intersectParams& rec,
                         vec3& attenuation,
                         ray& scattered) const
    {
        const vec3 reflectedRay = reflect(unit_vector(ray_in.direction()), rec.normal);
        scattered = ray(rec.p, reflectedRay + fuzziness * unitSphereRandomRadVec());
        attenuation = albedo;
        return (dot(scattered.direction(), rec.normal) > 0);
    }
    
    vec3 albedo;
    float fuzziness;
};

// Dielectrics partly reflect + refract (transmit) the incident
// light. Here we select one with certain probablity and generate a single
// scattered ray only (not both)
class dielectric : public material {
public:
    dielectric(float ri) : refractiveIdx(ri) {}
    virtual bool scatter(const ray& ray_in,
                         const intersectParams& rec,
                         vec3& attenuation,
                         ray& scattered) const  {
        vec3 outward_normal;
        // no absorbtion
        attenuation = vec3(1.0, 1.0, 1.0);
        // refracted obeys snell slaw:
        // n_i * sin(θ_i) = n_t * sin(θ_t)
        /// when light passes from 'incident' (i) medium to 'transmitted' (t) medium
        // here we assume in medium is always air, so n_in = 1
        float ni_over_nt; // = sin(θ_t) / sin(θ_i)
        float cosine;
        
        if (dot(ray_in.direction(), rec.normal) > 0) {
            // normal case
            outward_normal = -rec.normal;
            ni_over_nt = refractiveIdx;
            cosine = dot(ray_in.direction(), rec.normal) / ray_in.direction().length();
            cosine = sqrt(1 - refractiveIdx * refractiveIdx * (1.0f - cosine * cosine));
        }
        else {
            // ray is opposite side of surface normal
            // due to internal reflection
            outward_normal = rec.normal;
            ni_over_nt = 1.0 / refractiveIdx;
            cosine = -dot(ray_in.direction(), rec.normal) / ray_in.direction().length();
        }

        // Calculate refracted vector
        float reflectProb = 1.0f;
        vec3 refracted;
        if (refract(ray_in.direction(),
                    outward_normal,
                    ni_over_nt,
                    refracted)) {
            // Use schlick approx to approximate fresnel contribution to specular reflection
            // if refracted
            reflectProb = schlick(cosine, refractiveIdx);
        }
        
        // refract or reflect randomly, depending on specular factor
        if (drand48() < reflectProb) {
            // reflecteds as in metal
            vec3 reflected = reflect(ray_in.direction(), rec.normal);
            scattered = ray(rec.p, reflected);
        } else {
            scattered = ray(rec.p, refracted);
        }
        
        return true;
    }

    float refractiveIdx;
};



#endif /* material_h */
