#pragma once
#include "hittable.h"
#include "material.h"
#include "PerlinNoise.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

vec3 refract_water(vec3 i, vec3 n, float eta)
{
    eta = 2.0f - eta;
    float cosi = dot(n, i);
    vec3 o = (i * eta - n * (-cosi + eta * cosi));
    return o;
}

class mirror : public material {
public:
    const static int bytes_per_pixel = 3;
    mirror(double index_of_refraction) : ir(index_of_refraction) {
        auto components_per_pixel = bytes_per_pixel;

        std::string file = "nmapWater.png";
        data = stbi_load(
            file.c_str(), &width, &height, &components_per_pixel, components_per_pixel);

        if (!data) {
            std::cerr << "ERROR: Could not load texture image file '" << file << "'.\n";
            width = height = 0;
        }

        bytes_per_scanline = bytes_per_pixel * width;
    }
    
    vec3 sampleNMap(float u, float v) const
    {
        if (data == nullptr)
            return color(0, 1, 1);

        int i = (u)*width;
        int j = (1 - v) * height - 0.001;

        if (i < 0) i = 0;
        if (j < 0) j = 0;

        if (i > width - 1) i = width - 1;
        if (j > height - 1) j = height - 1;

        float r = int(data[3 * i + 3 * width * j]) / 255.0;
        float g = int(data[3 * i + 3 * width * j + 1]) / 255.0;
        float b = int(data[3 * i + 3 * width * j + 2]) / 255.0;

        return vec3(r, g, b);
    }

    virtual bool scatter(
        const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
    ) const override {
        //attenuation = color(0.4, 0.45, 0.9);

        vec3 unit_direction = unit_vector(r_in.direction());
        vec3 direction;

        hit_record rec2 = rec;
        rec2.normal[1] += (sampleNMap(rec2.u, rec2.v) * 2.0f)[1];
        attenuation = color(0.4, 0.45, 0.9) * (sampleNMap(rec2.u, rec2.v));

        direction = reflect(unit_direction, rec2.normal);

        scattered = ray(rec2.p, direction);
        return true;
    }   

public:
    double ir; // Index of Refraction
    unsigned char* data;
    int width, height;
    int bytes_per_scanline;
private:

    static double reflectance(double cosine, double ref_idx) {
        // Use Schlick's approximation for reflectance.
        auto r0 = (1 - ref_idx) / (1 + ref_idx);
        r0 = r0 * r0;
        return r0 + (1 - r0) * pow((1 - cosine), 5);
    }
};