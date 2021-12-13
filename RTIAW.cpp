#include "rtweekend.h"

#include "color.h"
#include "hittable_list.h"
#include "sphere.h"
#include "aarect.h"
#include "box.h"
#include "camera.h"
#include "material.h"
#include "metal.h"
#include "lambertian.h"
#include "Median.h"
#include "dielectric.h"
#include "mirror.h"
#include "diffuse_light.h"
#include "checker_texture.h"
#include "constant_medium.h"
#include "threadPool.h"
#include "bvh.h"
#include "bvh.h"

#include "ryml_std.hpp"
#include "ryml.hpp"
#include <c4/format.hpp>
#include <c4/std/string.hpp>

#include "../oidn/include/OpenImageDenoise/oidn.hpp"

#include <iostream>
#include <string>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
ray_color_result ray_color(const ray& r, const color& background, const hittable& world, int depth) {
    hit_record rec;

    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0)
        return ray_color_result(color(0, 0, 0), color(0, 0, 0));

    if (world.hit(r, 0.001, infinity, rec)) {
        ray scattered;
        color attenuation;
        color emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);
        if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
        {
            auto scatter_result = ray_color(scattered, background, world, depth - 1);
            return ray_color_result(emitted + (attenuation * scatter_result.pixel_color), rec.normal);
        }
        else
        {
            float normalize = std::max(std::max(emitted[0], emitted[1]), emitted[2]);
            return ray_color_result(emitted / 1.4, rec.normal);
        }
    }
    else
    {
        return background;
    }
}

hittable_list random_scene() {
    hittable_list world;

    auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));

    for (int a = -3; a < 3; a++) {
        for (int b = -3; b < 3; b++) {
            auto choose_mat = random_double();
            point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());
            center *= 2.0f;

            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = random() * random();
                    sphere_material = make_shared<lambertian>(albedo);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
                else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
                else {
                    // glass
                    sphere_material = make_shared<dielectric>(1.5);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material2 = make_shared<lambertian>(make_shared<checker_texture>(make_shared<solid_color>(color(0, 0, 0)), make_shared<solid_color>(color(0.5, 0.15, 0.5))));
    world.add(make_shared<sphere>(point3(-2, 1, 0), 1.0, material2));

    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.5);
    world.add(make_shared<sphere>(point3(2, 1, 0), 1.0, material3));

    auto material4 = make_shared<diffuse_light>(color(4, 2, 2));
    world.add(make_shared<sphere>(point3(0, 1, 2), 1.0, material4));

    return world;
}

hittable_list final_scene() {
    hittable_list boxes1;
    auto ground = make_shared<lambertian>(color(0.48, 0.83, 0.53));

    const int boxes_per_side = 20;
    for (int i = 0; i < boxes_per_side; i++) {
        for (int j = 0; j < boxes_per_side; j++) {
            auto w = 100.0;
            auto x0 = -1000.0 + i * w;
            auto z0 = -1000.0 + j * w;
            auto y0 = 0.0;
            auto x1 = x0 + w;
            auto y1 = random_double(1, 101);
            auto z1 = z0 + w;

            boxes1.add(make_shared<box>(point3(x0, y0, z0), point3(x1, y1, z1), ground));
        }
    }

    hittable_list objects;

    objects.add(make_shared<bvh_node>(boxes1, 0, 1));

    auto light = make_shared<diffuse_light>(color(7, 7, 7));
    objects.add(make_shared<xz_rect>(123, 423, 147, 412, 554, light));

    auto center1 = point3(400, 400, 200);
    auto center2 = center1 + vec3(30, 0, 0);
    auto moving_sphere_material = make_shared<lambertian>(color(0.7, 0.3, 0.1));
    //objects.add(make_shared<moving_sphere>(center1, center2, 0, 1, 50, moving_sphere_material));

    objects.add(make_shared<sphere>(point3(260, 150, 45), 50, make_shared<dielectric>(1.5)));
    objects.add(make_shared<sphere>(
        point3(0, 150, 145), 50, make_shared<metal>(color(0.8, 0.8, 0.9), 1.0)
        ));

    auto boundary = make_shared<sphere>(point3(360, 150, 145), 70, make_shared<dielectric>(1.5));
    objects.add(boundary);
    objects.add(make_shared<constant_medium>(boundary, 0.2, color(0.2, 0.4, 0.9)));
    boundary = make_shared<sphere>(point3(0, 0, 0), 5000, make_shared<dielectric>(1.5));
    objects.add(make_shared<constant_medium>(boundary, .0001, color(1, 1, 1)));

    //auto emat = make_shared<lambertian>(make_shared<image_texture>("earthmap.jpg"));
    //objects.add(make_shared<sphere>(point3(400, 200, 400), 100, emat));
    //auto pertext = make_shared<noise_texture>(0.1);
    //objects.add(make_shared<sphere>(point3(220, 280, 300), 80, make_shared<lambertian>(pertext)));

    hittable_list boxes2;
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    int ns = 1000;
    for (int j = 0; j < ns; j++) {
        boxes2.add(make_shared<sphere>(random(0, 165), 10, white));
    }

    objects.add(make_shared<translate>(
        make_shared<rotate_y>(
            make_shared<bvh_node>(boxes2, 0.0, 1.0), 15),
        vec3(-100, 270, 395)
        )
    );

    return objects;
}

shared_ptr<material> makeMaterial(c4::yml::NodeRef object)
{
    shared_ptr<material> obj;

    auto materialType = object["type"].val();

    shared_ptr<texture> tex;
    if (object.has_child("color"))
    {
        float r = std::stod(object["color"][0].val().str);
        float g = std::stod(object["color"][1].val().str);
        float b = std::stod(object["color"][2].val().str);
        tex = make_shared<solid_color>(color(r, g, b));
    }

    if (materialType == "lambertian")
    {
        obj = make_shared<lambertian>(tex);
    }
    if (materialType == "metal")
    {
        float fuzz = std::stod(object["fuzz"].val().str);

        obj = make_shared<metal>(tex, fuzz);
    }
    if (materialType == "diffuse_light")
    {
        obj = make_shared<diffuse_light>(tex);
    }
    if (materialType == "mirror")
    {
        float ir = std::stod(object["ir"].val().str);

        obj = make_shared<mirror>(ir);
    }

    return obj;
}

shared_ptr<hittable> makeObject(c4::yml::NodeRef object)
{
    shared_ptr<hittable> obj;

    auto objectType =  object["type"].val();
    std::cout << objectType << std::endl;

    float x = std::stod(object["position"][0].val().str);
    float y = std::stod(object["position"][1].val().str);
    float z = std::stod(object["position"][2].val().str);
    auto position = vec3(x, y, z);

    auto material = makeMaterial(object["material"]);

    if (objectType == "sphere")
    {
        std::cout << x << ", " << y << ", " << z << std::endl;
        float radius = std::stod(object["radius"].val().str);
        obj = make_shared<sphere>(position, radius, material);
    }
    if (objectType == "box")
    {
        float scaleX = std::stod(object["scale"][0].val().str);
        float scaleY = std::stod(object["scale"][1].val().str);
        float scaleZ = std::stod(object["scale"][2].val().str);
        auto position = vec3(scaleX, scaleY, scaleZ);
        auto scale = vec3(x, y, z);
        point3 from = position - scale;
        point3 to = position + scale;
        obj = make_shared<box>(from, to, material);
    }


    return obj;
}

hittable_list sceneTree(ryml::Tree tree) {
    hittable_list objectsList;

    auto root = tree.rootref();
    if (root.has_child("objects"))
    {
        auto objects = root["objects"];
        for (int i = 0; i < objects.num_children(); i++)
        {
            std::cout << i << std::endl;
            auto object = objects[i];
            objectsList.add(makeObject(object));
        }
    }

    return objectsList;
}

enum class RenderState { Ready, Running, Finished, Stopped };

std::string read_string_from_file(const std::string& file_path) {
    const std::ifstream input_stream(file_path, std::ios_base::binary);

    if (input_stream.fail()) {
        throw std::runtime_error("Failed to open file");
    }

    std::stringstream buffer;
    buffer << input_stream.rdbuf();

    return buffer.str();
}

int main() {

    std::cout << "What Scene do you want to open?" << std::endl;
    std::string scene;
    std::getline(std::cin, scene);

    std::string sceneYaml = read_string_from_file("scenes/" + scene + ".scene");
    std::cout << sceneYaml << std::endl;
    ryml::Tree tree = ryml::parse(c4::to_csubstr(sceneYaml));

    // Image
    auto aspect_ratio = 3.0 / 2.0;
    int image_width = 1024;
    int image_height = static_cast<int>(image_width / aspect_ratio);
    int samples_per_pixel = 40;
    int max_depth = 6;

    RenderState m_state = RenderState::Ready;
    std::thread m_renderingThread;
    RTIAW::Utils::Pool m_threadPool{};

    // World

    hittable_list world;
    color background = color(0.0, 0.0, 0.0);

    // Camera

    point3 lookfrom(13, 20, 6);
    point3 lookat(0, 0, 0);
    vec3 vup(0, 1, 0);
    auto dist_to_focus = 10.0;
    auto aperture = 0.1;
    auto vfov = 20.0;

    /*switch (1) {
    case 1:
        world = random_scene();
        lookfrom = point3(13, 20, 6);
        lookat = point3(0, 0, 0);
        background = color(0.70, 0.80, 1.00);
        aperture = 0.1;
        vfov = 20.0;
        break;

    default:
    case 2:
        world = cornell_box();
        aspect_ratio = 1.0;
        image_width = 600;
        samples_per_pixel = 200;
        background = color(0, 0, 0);
        lookfrom = point3(278, 278, -800);
        lookat = point3(278, 278, 0);
        vfov = 40.0;
        break;
    }*/

    //Parse scene tree

    world = sceneTree(tree);
    auto root = tree.rootref();
    if (root.has_child("metadata"))
    {
        std::cout << "Has MetaData" << std::endl;
        if (root["metadata"].has_child("name"))
        {
            std::cout << root["metadata"]["name"].val() << std::endl;
        }
        if (root["metadata"].has_child("fov"))
        {
            vfov = std::stod(root["metadata"]["fov"].val().str);
            std::cout << root["metadata"]["fov"].val() << std::endl;
        }
        if (root["metadata"].has_child("lookfrom"))
        {
            float x = std::stod(root["metadata"]["lookfrom"][0].val().str);
            float y = std::stod(root["metadata"]["lookfrom"][1].val().str);
            float z = std::stod(root["metadata"]["lookfrom"][2].val().str);
            lookfrom = vec3(x, y, z);
        }
        if (root["metadata"].has_child("lookat"))
        {
            float x = std::stod(root["metadata"]["lookat"][0].val().str);
            float y = std::stod(root["metadata"]["lookat"][1].val().str);
            float z = std::stod(root["metadata"]["lookat"][2].val().str);
            lookat = vec3(x, y, z);
        }
        if (root["metadata"].has_child("background"))
        {
            float r = std::stod(root["metadata"]["background"][0].val().str);
            float g = std::stod(root["metadata"]["background"][1].val().str);
            float b = std::stod(root["metadata"]["background"][2].val().str);
            background = color(r, g, b);
        }
    }/*
    world = final_scene();
    aspect_ratio = 1.0;
    image_width = 800;
    image_height = static_cast<int>(image_width / aspect_ratio);
    samples_per_pixel = 75;
    background = color(0, 0, 0);
    lookfrom = point3(478, 278, -600);
    lookat = point3(278, 278, 0);
    vfov = 40.0;*/

    camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus);

    // Camera

    auto viewport_height = 2.0;
    auto viewport_width = aspect_ratio * viewport_height;
    auto focal_length = 1.0;

    auto origin = point3(0, 0, 0);
    auto horizontal = vec3(viewport_width, 0, 0);
    auto vertical = vec3(0, viewport_height, 0);
    auto lower_left_corner = origin - horizontal / 2 - vertical / 2 - vec3(0, 0, focal_length);

    // Render

    std::vector<float> pixelBuffer = std::vector<float>((image_height * image_width) * 3 + 3);
    std::vector<float> dePepperBuffer  = std::vector<float>((image_height * image_width) * 3 + 3);
    std::vector<float> normalBuffer = std::vector<float>((image_height * image_width) * 3 + 3);
    std::vector<float> outputBuffer = std::vector<float>((image_height * image_width) * 3 + 3);
    int pixel = 0;
    int finishedPixel = 0;

    auto startTime = std::chrono::system_clock::now();
    bool isDone = false;

    m_renderingThread = std::thread{ [&]() {

        auto renderPixel = [&](int i, int j, int curPixel)
        {
            color pixel_color(0, 0, 0);
            color normal_color(0, 0, 0);
            for (int s = 0; s < samples_per_pixel; ++s) {
                auto u = (i + random_double()) / (image_width - 1);
                auto v = (j + random_double()) / (image_height - 1);
                ray r = cam.get_ray(u, v);
                auto ray_col = ray_color(r, background, world, max_depth);
                pixel_color += ray_col.pixel_color;
                normal_color += ray_col.normal_color;
            }
            pixelBuffer[curPixel * 3] = pixel_color.x() / 255.0;
            pixelBuffer[curPixel * 3 + 1] = pixel_color.y() / 255.0;
            pixelBuffer[curPixel * 3 + 2] = pixel_color.z() / 255.0;

            normalBuffer[curPixel * 3] = normal_color.x() / 255.0;
            normalBuffer[curPixel * 3 + 1] = normal_color.y() / 255.0;
            normalBuffer[curPixel * 3 + 2] = normal_color.z() / 255.0;
            finishedPixel++;
        };

        m_state = RenderState::Running;

        for (int ind = 0; ind < (image_height * image_width); ind++)
        {
            int i = ind % image_width;
            int j = image_height - (ind / image_width);
            m_threadPool.AddTask(renderPixel, i, j, pixel);
            pixel++;
        }

        while (!m_threadPool.IsEmpty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        for (int row = 1; row <= image_height; ++row) {
            for (int col = 1; col <= image_width; ++col) {

                // check if intensity is black or white
                if (pixelBuffer[(row * image_height) + col] == 0 || pixelBuffer[(row * image_height) + col] == 255)
                {
                    // compute average of neighbours pixels
                    // and store the pixel value in another
                    // array for output image
                    //((row-1) * image_height) + col - 1
                    dePepperBuffer[(row * image_height) + col] =
                        (pixelBuffer[((row - 1) * image_height) + col] +
                            pixelBuffer[((row - 1) * image_height) + col - 1] +
                            pixelBuffer[((row - 1) * image_height) + col + 1] +
                            pixelBuffer[((row)*image_height) + col - 1] +
                            pixelBuffer[((row)*image_height) + col + 1] +
                            pixelBuffer[((row + 1) * image_height) + col + 1] +
                            pixelBuffer[((row + 1) * image_height) + col] +
                            pixelBuffer[((row + 1) * image_height) + col - 1]) / 8;
                }
                else {
                    // store pixel value in another
                    // array for output image
                    dePepperBuffer[(row * image_height) + col] = pixelBuffer[(row * image_height) + col];
                }
            }
        }

        m_state = RenderState::Finished;
    }};

    while (m_state != RenderState::Finished)
    {
        auto currentTime = std::chrono::system_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
        auto percentage = ((finishedPixel / (float)(image_height * image_width)) * 100.0f);
        auto remainingPercentage = 100 - percentage;
        auto timePerPercentage = time / percentage;
        std::cerr << "\rRendering: " << percentage << "%" << " Time Remaining: " << timePerPercentage * remainingPercentage << "s" << ' ' << std::flush;

        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    auto stopTime = std::chrono::system_clock::now();
    std::string time = std::to_string(std::chrono::duration_cast<std::chrono::seconds>(stopTime - startTime).count());
    std::cerr << "\nRendering took " << time << " seconds" << ' ' << std::flush;
    // Create an Intel Open Image Denoise device
    oidn::DeviceRef device = oidn::newDevice();
    device.commit();

    oidn::FilterRef filter = device.newFilter("RT"); // generic ray tracing filter
    filter.setImage("color", pixelBuffer.data(), oidn::Format::Float3, image_width, image_height); // beauty
    filter.setImage("albedo", dePepperBuffer.data(), oidn::Format::Float3, image_width, image_height); // beauty
    filter.setImage("normal", normalBuffer.data(), oidn::Format::Float3, image_width, image_height); // auxiliary
    filter.setImage("output", outputBuffer.data(), oidn::Format::Float3, image_width, image_height); // denoised beauty
    filter.set("srgb", true); // beauty image is srgb
    //filter.set("cleanAux", true); // auxiliary images will be prefiltered
    filter.commit();

    // Filter the beauty image
    filter.execute();

    const char* errorMessage;
    if (device.getError(errorMessage) != oidn::Error::None)
        std::cout << "Error: " << errorMessage << std::endl;

    std::cerr << "\nWriting Image..." << ' ' << std::flush;

    std::string pixelFilename = "pixel.ppm";
    std::fstream pixelStream(pixelFilename, std::fstream::trunc | std::fstream::in | std::fstream::out);
    pixelStream << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    std::string normalFilename = "normal.ppm";
    std::fstream normalStream(normalFilename, std::fstream::trunc | std::fstream::in | std::fstream::out);
    normalStream << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    std::string outputFilename = "output.ppm";
    std::fstream outputStream(outputFilename, std::fstream::trunc | std::fstream::in | std::fstream::out);
    outputStream << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    for (int i = 0; i < (image_height * image_width); i++)
    {   
        //std::cerr << "\r" << i << ' ' << std::flush;
        write_color(pixelStream, color(pixelBuffer[i * 3] * 255.0, pixelBuffer[i * 3 + 1] * 255.0, pixelBuffer[i * 3 + 2] * 255.0), samples_per_pixel);
        write_color(normalStream, color(normalBuffer[i * 3] * 255.0, normalBuffer[i * 3 + 1] * 255.0, normalBuffer[i * 3 + 2] * 255.0), samples_per_pixel);
        write_color(outputStream, color(outputBuffer[i * 3] * 255.0, outputBuffer[i * 3 + 1] * 255.0, outputBuffer[i * 3 + 2] * 255.0), samples_per_pixel);
    }
    std::cerr << "\nDone.";

    return 0;
}   