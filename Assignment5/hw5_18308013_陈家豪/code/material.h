#ifndef MATERIALH
#define MATERIALH

#include "ray.h"
#include "hitable.h"
#include "random.h"

struct hit_record;


float schlick(float cosine, float ref_idx) {
    float r0 = (1-ref_idx) / (1+ref_idx);
    r0 = r0*r0;
    return r0 + (1-r0)*pow((1 - cosine),5);
}


bool refract(const vec3& v, const vec3& n, float ni_over_nt, vec3& refracted) 
{
    vec3 uv = unit_vector(v);   // 单位入射向量
    vec3 un = unit_vector(n);   // 单位对外法向量
    float uv_un = dot(uv, un);
    float discriminant = 1.0 - ni_over_nt*ni_over_nt*(1-uv_un*uv_un);
    if (discriminant > 0) 
    {
        refracted = ni_over_nt*(uv - un*uv_un) - un*sqrt(discriminant);
        return true;
    }
    else
    return false;
}


vec3 reflect(const vec3& v, const vec3& n) // 计算镜面反射出射光线
{
    return v - 2*dot(v,n)*n;
}


vec3 random_in_unit_sphere() {
    vec3 p;
    do {
        p = 2.0*vec3(random_double(),random_double(),random_double()) - vec3(1,1,1);
    } while (p.squared_length() >= 1.0);
    return p;
}


class material  // 材质类
{
    public:
        virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const = 0;
};


class lambertian : public material  // lambertian散射
{
    public:
        lambertian(const vec3& a) : albedo(a) {}
        virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const  {
             vec3 target = rec.p + rec.normal + random_in_unit_sphere();// 随机出射光线向量
             scattered = ray(rec.p, target-rec.p); // 出射光线
             attenuation = albedo;                 // 衰减系数
             return true;
        }
        vec3 albedo;
};


class metal : public material // 金属
{
    public:
        metal(const vec3& a, float f) : albedo(a) { if (f < 1) fuzz = f; else fuzz = 1; }
        virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const  
        {
            vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);// 镜面反射光线向量
            scattered = ray(rec.p, reflected+fuzz*random_in_unit_sphere());// 有偏差的镜面反射光线
            attenuation = albedo;// 衰减系数
            return (dot(scattered.direction(), rec.normal) > 0);// 如果反射光线有效，返回true
        }
        vec3 albedo;
        float fuzz;// 偏差值
};


class dielectric : public material 
{
    public:
        dielectric(float ri) : ref_idx(ri) {}
        virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const  
        {
            vec3 outward_normal;// 对外法向量
            vec3 reflected = reflect(r_in.direction(), rec.normal);// 镜面反射向量
            float ni_over_nt;// \frac{\eta}{\eta'}
            attenuation = vec3(1.0, 1.0, 1.0);  // 对介质而言，不存在衰减
            vec3 refracted;// 折射向量
            float reflect_prob;
            float cosine;
            if (dot(r_in.direction(), rec.normal) > 0)// 从介质射入外部
            {
                outward_normal = -rec.normal;  // 入射向量与法向量同向，故取反
                ni_over_nt = ref_idx;          // \frac{\eta'}{\eta}
                cosine = dot(r_in.direction(), rec.normal) / r_in.direction().length();// \cos\theta'
                cosine = sqrt(1 - ref_idx*ref_idx*(1-cosine*cosine));// \cos\theta
            }
            else                                     // 从外部射入介质
            {
                outward_normal = rec.normal;         // 入射向量与法向量反向，不取反
                ni_over_nt = 1.0 / ref_idx;          // \frac{\eta}{\eta'}
                cosine = -dot(r_in.direction(), rec.normal) / r_in.direction().length();// \cos\theta
            }
            if (refract(r_in.direction(), outward_normal, ni_over_nt, refracted))
            reflect_prob = schlick(cosine, ref_idx);// 会有折射，得到反射概率
            else
            reflect_prob = 1.0;                 // 绝对是反射
            if (random_double() < reflect_prob)
            scattered = ray(rec.p, reflected);  // 取反射
            else
            scattered = ray(rec.p, refracted);  // 取折射
            return true;
        }
        float ref_idx;// \frac{\eta'}{\eta}
};

class light : public material 
{
    public:
        light(){}
        virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const  
        {
            return false;
        }
};


#endif
