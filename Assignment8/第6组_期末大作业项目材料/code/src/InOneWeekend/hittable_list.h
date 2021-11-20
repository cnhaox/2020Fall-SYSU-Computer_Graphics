#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include "rtweekend.h"
#include "aabb.h"
#include "hittable.h"

#include <memory>
#include <vector>

class hittable_list : public hittable
{
public:
    hittable_list() {}
    hittable_list(shared_ptr<hittable> object) { add(object); }

    void clear() { objects.clear(); }
    void add(shared_ptr<hittable> object) { objects.push_back(object); }

    virtual bool hit(
        const ray &r, double t_min, double t_max, hit_record &rec) const override;
    virtual bool boundingBox(AABB &box) const override;
    virtual double pdf_value(const vec3 &o, const vec3 &v) const override;
    virtual vec3 random(const vec3 &o) const override;

public:
    std::vector<shared_ptr<hittable>> objects;
};

#endif
