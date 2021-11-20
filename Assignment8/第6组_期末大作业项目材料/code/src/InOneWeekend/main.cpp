#include "PathTracer.h"
#include <random>
#include <string>
#include <iostream>
#include "stb_image_write.h"

std::mt19937 e;
std::uniform_real_distribution<double> u(0, 1);

int main(int argc, char *argv[])
{
    // initialization.
    // Image

    const auto aspect_ratio = 16.0 / 9.0;
#ifdef NDEBUG
    const int image_width = 200;
#else
    const int image_width = 800;
#endif
    const int image_height = static_cast<int>(image_width / aspect_ratio);

    // Camera
    point3 lookfrom(-4, 3, 7);
    point3 lookat(2, 2, 0);
    vec3 vup(0, 1, 0);
    auto dist_to_focus = 10.0;
    auto aperture = 0;

    camera cam(lookfrom, lookat, vup, 45, aspect_ratio, aperture, dist_to_focus);

    // Render
    PathTracer tracer(cam);
    tracer.initialize(image_width, image_height);
    hittable_list world;
    hittable_list light;
    scene_tree(world, light);

    hittable **hittableList = new hittable *[world.objects.size()];
    if (!hittableList)
        std::cerr << "can not allocate memory" << std::endl;
    for (int i = 0; i < world.objects.size(); ++i)
        hittableList[i] = world.objects[i].get();

    BVHnode root(hittableList, world.objects.size());

    tracer.setWorldScene(std::move(world));
    tracer.setWorldBVHroot(root);
    tracer.setLightScene(std::move(light));
    //tracer.setLightBVHroot(root2);

    // rendering.
    double timeConsuming = 0.0f;
    unsigned char *pixels = tracer.render(timeConsuming);

    std::string filename = "../img/" + std::string(argv[1]) + ".png";
    // save pixels to .png file using stb_image.
    stbi_flip_vertically_on_write(1);
    if (!stbi_write_png(filename.c_str(),
                        tracer.getWidth(),
                        tracer.getHeight(),
                        4,
                        static_cast<void *>(tracer.getImage()),
                        tracer.getWidth() * 4))
    {
        std::cerr << "Failed to save image!\n"
                  << std::endl;
    }

    std::cout << "Rendering Finished.\n";
    std::cout << "Time consuming: " << timeConsuming << " secs.\n";

    return 0;
}
