#ifndef __OUR_GL_H__
#define __OUR_GL_H__
#include "tgaimage.h"
#include "geometry.h"

extern Matrix ModelView;
extern Matrix Projection;

void viewport(int x, int y, int w, int h);
void projection(float coeff=0.f); // coeff = -1/c
void lookat(Vec3f eye, Vec3f center, Vec3f up);

struct IShader {
    Vec3f face_vertices[3];// current triangle's vertices in world space.

    virtual ~IShader();
    virtual Vec4f vertex(unsigned int index,int iface, int nthvert) = 0;
    virtual bool fragment(unsigned int index,Vec3f pos,Vec3f bar, TGAColor &color) = 0;
};

//void triangle(Vec4f *pts, IShader &shader, TGAImage &image, float *zbuffer);
void triangle(unsigned int index, mat<4,3,float> &pts, IShader &shader, TGAImage &image, float *zbuffer);
#endif //__OUR_GL_H__

