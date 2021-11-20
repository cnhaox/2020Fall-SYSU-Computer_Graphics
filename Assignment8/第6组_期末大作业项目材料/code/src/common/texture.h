#pragma once
#include "stb_image.h"
#include "rtweekend.h"

class texture
{
public:
    virtual color value(double u, double v, const vec3 &p) const = 0;
};

class solid_color : public texture
{
public:
    solid_color();
    solid_color(color c) : texture_color(c) {}
    virtual color value(double u, double v, const vec3 &p) const override
    {
        return texture_color;
    }

private:
    color texture_color;
};

class checker_texture :public texture
{
private:
    shared_ptr<texture> odd;
    shared_ptr<texture> even;
    const double scale = 4;

public:
    checker_texture(): even(make_shared<solid_color>(color(1,1,1))), odd(make_shared<solid_color>(color(0,0,0))) {}
    checker_texture(shared_ptr<texture> _even, shared_ptr<texture> _odd) : even(_even), odd(_odd) {}
    checker_texture(color c1, color c2) : even(make_shared<solid_color>(c1)), odd(make_shared<solid_color>(c2)) {}
    virtual color value(double u, double v, const vec3 &p) const
    {
        // auto sines = sin(10 * p.x()) * sin(10 * p.y()) * sin(10 * p.z());
        auto pattern = (fmodf(fabs(p.x()) * scale, 1) < 0.5) ^ (fmodf(fabs(p.y()) * scale, 1) < 0.5);
        pattern = p.x() > 0 ? pattern : 1-pattern;
        if (pattern == 0)
            return odd->value(u, v, p);
        else
            return even->value(u, v, p);
    }
};

class image_texture:public texture
{
private:
    unsigned char *data;
    int width, height;
    int bytes_per_scanline;

public:
    const static int bytes_per_pixel = 3;
    image_texture(const char *filename)
    {
        auto components_per_pixel = bytes_per_pixel;
        data = stbi_load(filename, &width, &height, &components_per_pixel, components_per_pixel);
        if (!data)
        {
            std::cerr << "ERROR: Could not load texture image file '" << filename << "'.\n";
            width = height = 0;
        }
        bytes_per_scanline = bytes_per_pixel * width;
    }
    ~image_texture() { delete data; }
    virtual color value(double u, double v, const vec3 &p) const{
        if(!data)
            return color(0, 0, 0);
        int i = width * clamp(u, 0.0, 1.0);
        int j = height* (1.0-clamp(v, 0.0, 1.0));
        i = (i >= width) ? width - 1 : i;
        j = (j >= height) ? height - 1 : j;
        auto pixel = data + j * bytes_per_scanline + i * bytes_per_pixel;
        const double invMaxColorVal = 1.0 / 255;
        return color(invMaxColorVal * pixel[0], invMaxColorVal * pixel[1], invMaxColorVal * pixel[2]);
    }

};
