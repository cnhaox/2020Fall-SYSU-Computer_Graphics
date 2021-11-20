#ifndef HITABLELIST_H
#define HITABLELIST_H

#include "hitable.h"

class hitable_list: public hitable  {
    public:
        hitable_list() {}
        hitable_list(hitable **l, int n) { list = l; list_size = n; }
        virtual bool hit(const ray& r, float tmin, float tmax, hit_record& rec) const;
        hitable **list;
        int list_size;  // 物体数量
};
bool hitable_list::hit(const ray& r, float t_min, float t_max, hit_record& rec) const {
    hit_record temp_rec;    // 该光线的击中记录
    bool hit_anything = false;
    double closest_so_far = t_max;
    for (int i = 0; i < list_size; i++) 
    {   // 寻找该光线击中的最近的物体与记录
        if (list[i]->hit(r, t_min, closest_so_far, temp_rec))// 在规定范围内击中list[i]
        {
            hit_anything = true;
            closest_so_far = temp_rec.t;// 更新距离
            rec = temp_rec;             // 更新击中记录
        }
    }
    return hit_anything;
}


#endif