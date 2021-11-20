#ifndef SPHERE_H
#define SPHERE_H

#include "rtweekend.h"

#include "hittable.h"
#include "onb.h"

class sphere : public hittable
{
public:
    sphere() {}

    sphere(point3 cen, double r, shared_ptr<material> m, double _phi = 0, double _theta = 0, bool out = true)
        : center(cen), radius(r), mat_ptr(m),phi(_phi),theta(_theta), outer(out ? 1 : -1){};

    virtual bool hit(
        const ray &r, double t_min, double t_max, hit_record &rec) const override;
    virtual bool boundingBox(AABB &box) const;
    //x=-cos(phi)sin(theta),y=-cos(theta),z=sin(phi)sin(theta)
    virtual double pdf_value(const point3& o, const vec3& v) const override;
    virtual vec3 random(const point3& o) const override;
    void getTexCoord(const point3 &p, double &u, double &v) const
    {
        auto _theta = theta + acos(-p.y());
        auto _phi = phi + atan2(-p.z(), p.x()) + pi;

        u = _phi / (2 * pi);
        v = _theta / pi;
    }

public:
    point3 center;
    double radius;
    int outer;
    shared_ptr<material> mat_ptr;
    double phi, theta;
};

bool sphere::hit(const ray &r, double t_min, double t_max, hit_record &rec) const
{
    vec3 oc = r.origin() - center;
    auto a = r.direction().length_squared();
    auto half_b = dot(oc, r.direction());
    auto c = oc.length_squared() - radius * radius;

    auto discriminant = half_b * half_b - a * c;
    if (discriminant < 0)
        return false;
    auto sqrtd = sqrt(discriminant);

    // Find the nearest root that lies in the acceptable range.
    auto root = (-half_b - sqrtd) / a;
    if (root < t_min || t_max < root)
    {
        root = (-half_b + sqrtd) / a;
        if (root < t_min || t_max < root)
            return false;
    }

    rec.t = root;
    rec.p = r.at(rec.t);
    if (outer == -1)
        rec.isSkybox = true;
    vec3 outward_normal = (rec.p - center) / radius;
    rec.set_face_normal(r, outward_normal * outer);
    rec.mat_ptr = mat_ptr;
    getTexCoord(outward_normal, rec.u, rec.v);
    return true;
}

bool sphere::boundingBox(AABB &box) const
{
    box = AABB(center - vec3(radius, radius, radius), center + vec3(radius, radius, radius));
    return true;
}

double sphere::pdf_value(const point3& origin, const vec3& direction) const 
{
    hit_record rec;
    if (!this->hit(ray(origin, direction), 0.001, infinity, rec))
    return 0;
    auto cos_theta_max = sqrt(1 - radius*radius/(center-origin).length_squared());
    auto solid_angle = 2*pi*(1-cos_theta_max);

    return  1 / solid_angle;
}

vec3 sphere::random(const point3& origin) const 
{
    vec3 direction = center - origin;
    auto distance_squared = direction.length_squared();
    onb uvw;
    uvw.build_from_w(direction);
    return uvw.local(random_to_sphere(radius, distance_squared));
}
#endif
