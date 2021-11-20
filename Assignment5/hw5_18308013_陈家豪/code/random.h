#ifndef RANDOMH
#define RANDOMH

#include <cstdlib>

inline float random_double() {
    return float(rand()) / float(RAND_MAX + 1.0);
}

#endif