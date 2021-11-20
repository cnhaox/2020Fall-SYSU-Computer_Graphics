#include "bvh_node.h"

#ifdef _WIN64 
#include<random>
extern std::mt19937 e;

#endif // __windows__


int box_x_compare(const void *a, const void *b)
{
    AABB boxLeft, boxRight;
    hittable *ah = *(hittable **)a;
    hittable *bh = *(hittable **)b;
    if (!ah->boundingBox(boxLeft) || !bh->boundingBox(boxRight))
    {
        std::cerr << "no bounding box in bvh_node constructor\n";
    }

    if (boxLeft.min().x() < boxRight.min().x())
        return -1;
    else
        return 1;
}

int box_y_compare(const void *a, const void *b)
{
    AABB boxLeft, boxRight;
    hittable *ah = *(hittable **)a;
    hittable *bh = *(hittable **)b;
    if (!ah->boundingBox(boxLeft) || !bh->boundingBox(boxRight))
    {
        std::cerr << "no bounding box in bvh_node constructor\n";
    }

    if (boxLeft.min().y() < boxRight.min().y())
        return -1;
    else
        return 1;
}

int box_z_compare(const void *a, const void *b)
{
    AABB boxLeft, boxRight;
    hittable *ah = *(hittable **)a;
    hittable *bh = *(hittable **)b;
    if (!ah->boundingBox(boxLeft) || !bh->boundingBox(boxRight))
    {
        std::cerr << "no bounding box in bvh_node constructor\n";
    }

    if (boxLeft.min().z() < boxRight.min().z())
        return -1;
    else
        return 1;
}

bool BVHnode::boundingBox(AABB &box) const
{
    box = this->box;
    return true;
}

bool BVHnode::hit(const ray &r, double tmin, double tmax, hit_record &rec) const
{
    if (box.hit(r, tmin, tmax))
    {
        hit_record leftRec, rightRec;
        bool hitLeft = left->hit(r, tmin, tmax, leftRec);
        bool hitRight = right->hit(r, tmin, tmax, rightRec);

        if (hitLeft && hitRight)
        {
            rec = (leftRec.t < rightRec.t) ? leftRec : rightRec;
            return true;
        }
        else if (hitLeft)
        {
            rec = leftRec;
            return true;
        }
        else if (hitRight)
        {
            rec = rightRec;
            return true;
        }
        else
        {
            return false;
        }
    }
    else
        return false;
}

BVHnode::BVHnode(hittable **l, int n)
{
#ifdef _WIN64 
    int axis = int(3 * e());
#else
    int axis = int(3 * drand48());
#endif // __windows__

    if (axis == 0)
    {
        qsort(l, n, sizeof(hittable *), box_x_compare);
    }
    else if (axis == 1)
    {
        qsort(l, n, sizeof(hittable *), box_y_compare);
    }
    else
    {
        qsort(l, n, sizeof(hittable *), box_z_compare);
    }

    if (n == 1)
    {
        left = right = l[0];
    }
    else if (n == 2)
    {
        left = l[0];
        right = l[1];
    }
    else
    {
        left = new BVHnode(l, n / 2);
        right = new BVHnode(l + n / 2, n - n / 2);
    }

    AABB boxLeft, boxRight;
    if (!left->boundingBox(boxLeft) || !right->boundingBox(boxRight))
    {
        std::cerr << "no bounding box in bvh_node constructor\n";
    }

    box = surroundingBox(boxLeft, boxRight);
}