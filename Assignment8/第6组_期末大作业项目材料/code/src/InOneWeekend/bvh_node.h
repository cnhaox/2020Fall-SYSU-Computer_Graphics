#ifndef BVH_NODE_H
#define BVH_NODE_H

#include "aabb.h"
#include "hittable.h"

#include <iostream>

int box_x_compare(const void *a, const void *b);
int box_y_compare(const void *a, const void *b);
int box_z_compare(const void *a, const void *b);
AABB surroundingBox(const AABB& box0, const AABB& box1);

class BVHnode : public hittable
{
public:
    hittable *left;
    hittable *right;
    AABB box;

public:
    BVHnode() {}
    BVHnode(hittable **l, int n);
    virtual bool hit(const ray &r, double tmin, double tmax, hit_record &rec) const;
    virtual bool boundingBox(AABB &box) const;
};


#endif