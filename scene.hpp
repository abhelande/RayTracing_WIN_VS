//
//  scene.hpp
//  RayTracing
//
//  Created by Abhijit Bhelande on 6/25/19.
//  Copyright © 2019 Abhijit Bhelande. All rights reserved.
//

#ifndef scene_h
#define scene_h

#include "object.hpp"
#include <vector>

class scene: public object  {
public:
    scene() {}
    scene(std::vector<object*> &l) {objects = l;}
    virtual bool hit(const ray& r, float tmin, float tmax, intersectParams& rec) const;
    
    std::vector<object*> objects;
};

// Given a ray, for each object in the scene:
// . test if ray intersects its surface (facing the camera)
// . If yes, check if it the closest object to the camera
// . If yes, update the intersection record and ray parameter (t)
//   corresponding to this surface point
bool scene::hit(const ray& r, float t_min, float t_max, intersectParams& rec) const {
    intersectParams temp_rec;
    bool hit_anything = false;
    double closest_so_far = t_max;
    for (uint32_t i = 0; i < objects.size(); i++) {
        if (objects[i]->hit(r, t_min, closest_so_far, temp_rec)) {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            rec = temp_rec;
        }
    }
    return hit_anything;
}

#endif /* scene_h */
