#include "hittable_list.h"
#include <cassert>

extern AABB surroundingBox(const AABB& box0, const AABB& box1) ;

bool hittable_list::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    hit_record temp_rec;
    auto hit_anything = false;
    auto closest_so_far = t_max;

    for (const auto& object : objects) {
        if (object->hit(r, t_min, closest_so_far, temp_rec)) {
            hit_anything = true;
            assert(closest_so_far >= temp_rec.t);
            closest_so_far = temp_rec.t;
            rec = temp_rec;
        }
    }

    return hit_anything;
}

bool hittable_list::boundingBox(AABB &box) const {
    if (objects.size() < 1) return false;
    AABB temp;

    bool firstTrue = objects[0]->boundingBox(temp);
    if (!firstTrue) return false;

    box = temp;
    for ( int i = 1; i < objects.size(); ++i ) {
        if(objects[i]->boundingBox(temp)) {
            box = surroundingBox(box, temp);
        } else {
            return false;
        }
    }

    return true;
}

AABB surroundingBox(const AABB& box0, const AABB& box1) 
{
    vec3 small(fmin(box0.min().x(), box1.min().x()),
               fmin(box0.min().y(), box1.min().y()),
               fmin(box0.min().z(), box1.min().z()));
    vec3 big(fmax(box0.max().x(), box1.max().x()),
               fmax(box0.max().y(), box1.max().y()),
               fmax(box0.max().z(), box1.max().z()));  
    return AABB(small, big);          
}

double hittable_list::pdf_value(const point3& o, const vec3& v) const 
{
    auto weight = 1.0/objects.size();
    auto sum = 0.0;

    for (const auto& object : objects)
    sum += weight * object->pdf_value(o, v);
    return sum;
}


vec3 hittable_list::random(const vec3 &o) const 
{
    auto int_size = static_cast<int>(objects.size());
    return objects[random_int(0, int_size-1)]->random(o);
}
