#ifndef MATERIAL_H
#define MATERIAL_H

#include "rtweekend.h"
#include "pdf.h"
#include "texture.h"

//struct hit_record;
struct scatter_record
{
    ray specular_ray;
    bool is_specular;
    color attenuation;
    shared_ptr<pdf> pdf_ptr;
};

class material
{
public:
    virtual bool scatter(
        const ray &r_in, const hit_record &rec, scatter_record &srec
    ) const {
        return false;
    }
    
    virtual double scattering_pdf(
        const ray& r_in, const hit_record& rec, const ray& scattered
    ) const {
        return 0;
    }

    virtual color emit(const ray& r_in, const hit_record& rec) const 
    {
        return color(0, 0, 0);
    }
};

class lambertian : public material
{
public:
    lambertian(const color &a) : albedo(make_shared<solid_color>(a)) {}
    lambertian(shared_ptr<texture> a) : albedo(a) {}

    virtual bool scatter(
        const ray &r_in, const hit_record &rec, scatter_record &srec
    ) const override
    {
        srec.is_specular = false;
        srec.attenuation = albedo->value(rec.u,rec.v,rec.p);
        srec.pdf_ptr = make_shared<cosine_pdf>(rec.normal);
        return true;
    }

    double scattering_pdf(
        const ray& r_in, const hit_record& rec, const ray& scattered
    ) const 
    {
        auto cosine = dot(rec.normal, unit_vector(scattered.direction()));
        return cosine < 0 ? 0 : cosine/pi;
    }
public:
    shared_ptr<texture> albedo;
};

class metal : public material
{
public:
    metal(const color &a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}

    virtual bool scatter(
        const ray &r_in, const hit_record &rec, scatter_record &srec
    ) const override
    {
        vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
        srec.specular_ray = ray(rec.p, reflected + fuzz * random_in_unit_sphere());
        srec.attenuation = albedo;
        srec.is_specular = true;
        srec.pdf_ptr = nullptr;
        // return (dot(srec.specular_ray.direction(), rec.normal) > 0);
        return true;
    }

public:
    color albedo;
    double fuzz;
};

class dielectric : public material
{
public:
    dielectric(double index_of_refraction) : ir(index_of_refraction) {}

    virtual bool scatter(
        const ray &r_in, const hit_record &rec, scatter_record &srec
    ) const override 
    {
        srec.is_specular = true;
        srec.pdf_ptr = nullptr;
        srec.attenuation = color(1.0, 1.0, 1.0);
        double refraction_ratio = rec.front_face ? (1.0 / ir) : ir;

        vec3 unit_direction = unit_vector(r_in.direction());
        double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

        bool cannot_refract = refraction_ratio * sin_theta > 1.0;
        vec3 direction;

        if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double())
            direction = reflect(unit_direction, rec.normal);
        else
            direction = refract(unit_direction, rec.normal, refraction_ratio);

        srec.specular_ray = ray(rec.p, direction);
        return true;
    }

public:
    double ir; // Index of Refraction

private:
    static double reflectance(double cosine, double ref_idx)
    {
        // Use Schlick's approximation for reflectance.
        auto r0 = (1 - ref_idx) / (1 + ref_idx);
        r0 = r0 * r0;
        return r0 + (1 - r0) * pow((1 - cosine), 5);
    }
};

class dielectric_light : public material
{
public:
    dielectric_light(double index_of_refraction, shared_ptr<texture> t) : ir(index_of_refraction), emitted_texture(t) {}
    dielectric_light(double index_of_refraction, color c) : ir(index_of_refraction), emitted_texture(make_shared<solid_color>(c)) {}
    virtual bool scatter(
        const ray &r_in, const hit_record &rec, scatter_record &srec
    ) const override 
    {
        srec.is_specular = true;
        srec.pdf_ptr = nullptr;
        srec.attenuation = color(1.0, 1.0, 1.0);
        double refraction_ratio = rec.front_face ? (1.0 / ir) : ir;

        vec3 unit_direction = unit_vector(r_in.direction());
        double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

        bool cannot_refract = refraction_ratio * sin_theta > 1.0;
        vec3 direction;

        if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double())
            direction = reflect(unit_direction, rec.normal);
        else
            direction = refract(unit_direction, rec.normal, refraction_ratio);

        srec.specular_ray = ray(rec.p, direction);
        return true;
    }

    virtual color emit(const ray& r_in, const hit_record& rec) const override
    {
        if (rec.front_face)
        return emitted_texture->value(rec.u, rec.v, rec.p);
        else
        return color(0,0,0);
    }
public:
    double ir; // Index of Refraction
    shared_ptr<texture> emitted_texture;
private:
    static double reflectance(double cosine, double ref_idx)
    {
        // Use Schlick's approximation for reflectance.
        auto r0 = (1 - ref_idx) / (1 + ref_idx);
        r0 = r0 * r0;
        return r0 + (1 - r0) * pow((1 - cosine), 5);
    }
};

class light_source : public material
{
public:
    light_source(shared_ptr<texture> t) : emitted_texture(t) {}
    light_source(color c) : emitted_texture(make_shared<solid_color>(c)) {}
    virtual color emit(const ray& r_in, const hit_record& rec) const override
    {
        if (rec.front_face)
        return emitted_texture->value(rec.u, rec.v, rec.p);
        else
        return color(0,0,0);
    }

public:
    shared_ptr<texture> emitted_texture;
};
/*
class mix_material : public material
{
public:
    mix_material(shared_ptr<texture> t, double loss, double op, double gl, double Ir) : albedo(t), lossp(loss), opacity(op), gloss(gl), ir(Ir){}
    mix_material(color c, double loss, double op, double gl, double Ir) : albedo(make_shared<solid_color>(c)), lossp(loss), opacity(op), gloss(gl), ir(Ir){}
    virtual bool scatter(const ray &r_in, const hit_record &rec, color &attenuation, ray &scattered) const override
    {
        if (random_double()<lossp)
        return false;
        else
        {
            if (random_double()<opacity)
            {
                if (random_double()<gloss)
                {// reflect
                    vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
                    scattered = ray(rec.p, reflected);
                    attenuation = albedo->value(rec.u,rec.v,rec.p)/lossp;
                    return (dot(scattered.direction(), rec.normal) > 0);
                }
                else
                {// reflact
                    auto scatter_direction = rec.normal + random_unit_vector();
                    if (scatter_direction.near_zero())
                        scatter_direction = rec.normal;
                    scattered = ray(rec.p, scatter_direction);
                    attenuation = albedo->value(rec.u,rec.v,rec.p)/lossp;
                    return true;
                }
            }
            else
            {
                double refraction_ratio = rec.front_face ? (1.0 / ir) : ir;

                vec3 unit_direction = unit_vector(r_in.direction());
                double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
                double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

                bool cannot_refract = refraction_ratio * sin_theta > 1.0;
                vec3 direction;

                if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double())
                {
                    direction = reflect(unit_direction, rec.normal);
                    attenuation = albedo->value(rec.u,rec.v,rec.p)/lossp;
                }
                else
                {
                    direction = refract(unit_direction, rec.normal, refraction_ratio);
                    attenuation = color(1.0, 1.0, 1.0)/lossp;
                }
                scattered = ray(rec.p, direction);
                return true;
            }
        }
    }
public:
    shared_ptr<texture> albedo;
    double lossp;   // 不发射光线的比例
    double opacity; // 不透明度(lambertian+metal : dielectric)
    double gloss;   // 光泽度(metal:lambertian)
    double ir;      // Index of Refraction

private:
    static double reflectance(double cosine, double ref_idx)
    {
        // Use Schlick's approximation for reflectance.
        auto r0 = (1 - ref_idx) / (1 + ref_idx);
        r0 = r0 * r0;
        return r0 + (1 - r0) * pow((1 - cosine), 5);
    }
};*/
#endif
