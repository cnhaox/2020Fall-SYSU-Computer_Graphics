#ifndef CAMERA_H
#define CAMERA_H

#include "ray.h"
#include "random.h"
#define M_PI 3.14159

vec3 random_in_unit_disk() {
    vec3 p;
    do {
        p = 2.0*vec3(random_double(),random_double(),0) - vec3(1,1,0);
    } while (dot(p,p) >= 1.0);
    return p;
}

class camera {
    public:
        camera(vec3 lookfrom, vec3 lookat, vec3 vup, float vfov, float aspect, float aperture, float focus_dist) 
        {
            // vfov is top to bottom in degrees
            lens_radius = aperture / 2;
            float theta = vfov*M_PI/180;// 视域范围角度
            float half_height = tan(theta/2);// 半高
            float half_width = aspect * half_height;// 半宽
            origin = lookfrom;// camera坐标
            w = unit_vector(lookfrom - lookat);// camera朝向的反向单位向量
            u = unit_vector(cross(vup, w));// camera平行单位向量
            v = cross(w, u);// camera垂直单位向量
            lower_left_corner = origin  - half_width*focus_dist*u -half_height*focus_dist*v - focus_dist*w;// 屏幕左下角坐标
            horizontal = 2*half_width*focus_dist*u;// 宽
            vertical = 2*half_height*focus_dist*v;// 高
        }
        ray get_ray(float s, float t) 
        {
            vec3 rd = lens_radius*random_in_unit_disk();
            vec3 offset = u * rd.x() + v * rd.y();// 以origin为圆心的光圈内的随机坐标
            return ray(origin + offset, lower_left_corner + s*horizontal + t*vertical - origin - offset);
        }
        vec3 origin;            // camera坐标
        vec3 lower_left_corner; // 屏幕左下角坐标
        vec3 horizontal;        // 屏幕水平距离
        vec3 vertical;          // 屏幕垂直距离
        vec3 u, v, w;           // camera的基坐标
        float lens_radius;
};


#endif