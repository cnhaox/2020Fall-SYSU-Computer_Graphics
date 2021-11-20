#pragma once
#include "rtweekend.h"

#include "hittable.h"
#include <array>

class vertex : public point3
{
public:
    vec3 normal = {0, 0, 0};
    double u = 0;
    double v = 0;
};
class triangle : public hittable
{

private:
    vertex v0, v1, v2;
    shared_ptr<material> mat_ptr;
    vec3 face_normal;
    vec3 v0v1, v0v2;

public:
    triangle(const vertex &_v0, const vertex &_v1, const vertex &_v2, shared_ptr<material> m) : v0(_v0), v1(_v1), v2(_v2), mat_ptr(m)
    {
        face_normal = (v1.normal + v2.normal + v0.normal) / 3;
        v0v1=v1-v0;
        v0v2=v2-v0;
    }
    bool hit(const ray &r, double t_min, double t_max, hit_record &rec) const
    {
        vec3 tvec = r.orig - v0;
        vec3 pvec = cross(r.direction(), v0v2);
        auto det = dot(v0v1, pvec);
        if (fabs(det) < 1e-5)
            return false;
        vec3 qvec = cross(tvec, v0v1);
        auto invDet = 1.0 / det;
        double t = dot(qvec, v0v2) *invDet;
        if (t >= t_max || t <= t_min)
            return false;
        auto u = dot(tvec, pvec) * invDet;
        auto v = dot(r.direction(), qvec) * invDet;
        if (v < 0 || u + v > 1 || u < 0 || u > 1)
            return false;
        double w = 1 - u - v;
        rec.p = w * v0 + u * v1 + v * v2;
        rec.t = t;
        rec.mat_ptr = mat_ptr;
        rec.u = w * v0.u + u * v1.u + v * v2.u;
        rec.v = w * v0.v + u * v1.v + v * v2.v;
        rec.set_face_normal(r, w * v0.normal + u * v1.normal + v * v2.normal);
        return true;
    }

    bool boundingBox(AABB &box) const
    {
        double x, y, z;
        x = (v0.x() < v1.x() ? v0.x() : v1.x()) < v2.x() ? (v0.x() < v1.x() ? v0.x() : v1.x()) : v2.x();
        y = (v0.y() < v1.y() ? v0.y() : v1.y()) < v2.y() ? (v0.y() < v1.y() ? v0.y() : v1.y()) : v2.y();
        z = (v0.z() < v1.z() ? v0.z() : v1.z()) < v2.z() ? (v0.z() < v1.z() ? v0.z() : v1.z()) : v2.z();

        vec3 bottom(x, y, z);

        x = (v0.x() > v1.x() ? v0.x() : v1.x()) > v2.x() ? (v0.x() > v1.x() ? v0.x() : v1.x()) : v2.x();
        y = (v0.y() > v1.y() ? v0.y() : v1.y()) > v2.y() ? (v0.y() > v1.y() ? v0.y() : v1.y()) : v2.y();
        z = (v0.z() > v1.z() ? v0.z() : v1.z()) > v2.z() ? (v0.z() > v1.z() ? v0.z() : v1.z()) : v2.z();

        vec3 top(x, y, z);

        box = AABB(bottom, top);

        return true;
    }
    double pdf_value(const point3& origin, const vec3& direction) const
    {
        hit_record rec;
        if (!this->hit(ray(origin, direction), 0.001, infinity, rec))
        return 0;

        vec3 temp = cross(v0v1, v0v2);
        double area = temp.length()/2;
        double distance_squared = rec.t*rec.t*direction.length_squared();
        double cosine = fabs(dot(direction, rec.normal)/direction.length());
        return distance_squared/(cosine*area);
    }

    vec3 random(const point3& origin) const
    {
        double length1 = random_double();
        double length2 = random_double();
        point3 random_point = v0+v0v1*length1+v0v2*length2;
        return random_point-origin;
    }
};
