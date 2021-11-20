#ifndef SPHEREH
#define SPHEREH
#include "hitable.h"


class sphere: public hitable  
{
    public:
        sphere() {}
        sphere(vec3 cen, float r, material *m, vec3 mcolor) : center(cen), radius(r), mat_ptr(m), color(mcolor)  {};
        //sphere(vec3 cen, float r, material *m) : center(cen), radius(r), mat_ptr(m)  {};
        virtual bool hit(const ray& r, float tmin, float tmax, hit_record& rec) const;
        vec3 center;// 球心坐标
        float radius;// 球半径
        material *mat_ptr;// 材质
        vec3 color;//原始颜色
};
bool sphere::hit(const ray& r, float t_min, float t_max, hit_record& rec) const 
{
    vec3 oc = r.origin() - center;
    // a*t^2+b*t+c=0
    float a = dot(r.direction(), r.direction());
    float half_b = dot(oc, r.direction());
    float c = dot(oc, oc) - radius*radius;
    float discriminant = half_b*half_b - a*c;
    if (discriminant<0)
    return false;
    float temp = (-half_b - sqrt(discriminant))/a; // 第一个相交点处的t值
    if (temp > t_max || temp < t_min)
    {
        temp = (-half_b + sqrt(discriminant)) / a; // 第二个相交点处的t值
        if (temp > t_max || temp < t_min)
        return false;
    }
    rec.t = temp;
    rec.p = r.point_at_parameter(rec.t);
    rec.normal = (rec.p - center) / radius;
    rec.mat_ptr = mat_ptr;
    rec.orig_color=color;
    return true;
}


#endif