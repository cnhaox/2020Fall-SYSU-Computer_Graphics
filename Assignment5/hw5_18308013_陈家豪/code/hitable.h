#ifndef HITABLEH
#define HITABLEH

#include "ray.h"

class material;
struct hit_record   // 击中记录
{
    float t;        // 距离
    vec3 p;         // 击中坐标
    vec3 normal;    // 法向量
    material *mat_ptr;// 材质
    vec3 orig_color;//原始颜色
};

class hitable  {
    public:
        virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const = 0;
};


#endif