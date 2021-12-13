#pragma once
#include "hittable.h"
#include "material.h"
#include "texture.h"


class metal : public material {
public:
    metal(const color& c, double f) : albedo(make_shared<solid_color>(c)), fuzz(f < 1 ? f : 1) {}
    metal(shared_ptr<texture> a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}

    virtual bool scatter(
        const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
    ) const override {
        vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
        scattered = ray(rec.p, reflected + fuzz * random_in_unit_sphere());
        attenuation = albedo->value(rec.u, rec.v, rec.p);
        return (dot(scattered.direction(), rec.normal) > 0);
    }

public:
    shared_ptr<texture> albedo;
    double fuzz;
};