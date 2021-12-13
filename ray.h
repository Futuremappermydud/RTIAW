#ifndef RAY_H
#define RAY_H

#include "vec3.h"

class ray {
public:
    ray() {}
    ray(const point3& origin, const vec3& direction)
        : orig(origin), dir(direction)
    {}

    point3 origin() const { return orig; }
    vec3 direction() const { return dir; }

    point3 at(double t) const {
        return orig + t * dir;
    }

public:
    point3 orig;
    vec3 dir;
};

struct ray_color_result
{
    color pixel_color;
    color normal_color;
    ray_color_result(const color& p, const color& n) : pixel_color(p), normal_color(n) {}
    ray_color_result(const color& p) : pixel_color(p), normal_color(color()) {}
};

#endif