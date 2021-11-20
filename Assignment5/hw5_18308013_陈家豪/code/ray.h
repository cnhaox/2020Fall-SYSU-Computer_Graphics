#ifndef RAY_H
#define RAY_H

#include "vec3.h"

class ray {
    public:
        ray() {}
        ray(const vec3& origin, const vec3& direction)
        {
            orig=origin;
            dir=direction;
        }
        vec3 origin() const  { return orig; }   // 返回原点
        vec3 direction() const { return dir; }  // 返回方向
        vec3 point_at_parameter(float t) const {return orig + t*dir;}// 返回目标点

    public:
        vec3 orig;  // 起始位置
        vec3 dir;   // 方向
};

#endif
