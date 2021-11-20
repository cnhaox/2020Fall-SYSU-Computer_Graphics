#pragma once

#include "rtweekend.h"
#include <glm/mat4x4.hpp>

class transformer
{
public:
    virtual ~transformer() {}
    virtual vec3 transform(const vec3 &orig) = 0;
};

class identity_transformer : public transformer
{
public:
    virtual ~identity_transformer();
    virtual vec3 transform(const vec3 &orig);
};

class matrix_transformer : public transformer
{
    glm::dmat4x4 _transform;

public:
    matrix_transformer(glm::dmat4x4 transform);
    virtual ~matrix_transformer();
    virtual vec3 transform(const vec3 &orig);
};
