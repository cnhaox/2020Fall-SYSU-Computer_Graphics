#include "transformer.hpp"
#include <iostream>

identity_transformer::~identity_transformer()
{
}

vec3 identity_transformer::transform(const vec3 &orig)
{
    return orig;
}

matrix_transformer::matrix_transformer(glm::dmat4x4 transform)
{
    _transform = transform;
}

matrix_transformer::~matrix_transformer()
{
}

vec3 matrix_transformer::transform(const vec3 &orig)
{
    glm::vec4 o(orig[0], orig[1], orig[2], 1);
    glm::vec4 r = _transform * o;
    return vec3(r.x / r.w, r.y / r.w, r.z / r.w);
}
