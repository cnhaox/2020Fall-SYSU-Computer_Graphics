#include <vector>
#include <limits>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "our_gl.h"

// model list.
std::vector<Model*> models;

const int width  = 700;
const int height = 700;

Vec3f light_dir(1,1,1);
Vec3f       eye(1,1,3);
Vec3f    center(0,0,0);
Vec3f        up(0,1,0);

struct Shader : public IShader {
    mat<2,3,float> varying_uv;  // triangle uv coordinates, written by the vertex shader, read by the fragment shader
    mat<4,3,float> varying_tri; // triangle coordinates (clip coordinates), written by VS, read by FS
    mat<3,3,float> varying_nrm; // normal per vertex to be interpolated by FS
    mat<3,3,float> ndc_tri;     // triangle in normalized device coordinates

    // vertex shader (per vertex)
    virtual Vec4f vertex(unsigned int index, int iface, int nthvert) {
        varying_uv.set_col(nthvert, models[index]->uv(iface, nthvert));
        varying_nrm.set_col(nthvert, proj<3>((Projection*ModelView).invert_transpose()*
                embed<4>(models[index]->normal(iface, nthvert), 0.f)));
        Vec4f gl_Vertex = Projection*ModelView*embed<4>(models[index]->vert(iface, nthvert));
        face_vertices[nthvert] = models[index]->vert(iface, nthvert);
        varying_tri.set_col(nthvert, gl_Vertex);
        ndc_tri.set_col(nthvert, proj<3>(gl_Vertex/gl_Vertex[3]));
        return gl_Vertex;
    }

    // fragment shader (per pxiel)
    virtual bool fragment(unsigned int index, Vec3f pos, Vec3f bar, TGAColor &color) {
        Vec3f bn = (varying_nrm*bar).normalize();
        Vec2f uv = varying_uv*bar;

        mat<3,3,float> A;
        A[0] = ndc_tri.col(1) - ndc_tri.col(0);
        A[1] = ndc_tri.col(2) - ndc_tri.col(0);
        A[2] = bn;

        mat<3,3,float> AI = A.invert();

        Vec3f i = AI * Vec3f(varying_uv[0][1] - varying_uv[0][0], varying_uv[0][2] - varying_uv[0][0], 0);
        Vec3f j = AI * Vec3f(varying_uv[1][1] - varying_uv[1][0], varying_uv[1][2] - varying_uv[1][0], 0);

        mat<3,3,float> B;
        B.set_col(0, i.normalize());
        B.set_col(1, j.normalize());
        B.set_col(2, bn);

		// use these vectors to finish your assignment.
        // the normal vector of this fragment.
        Vec3f normal = (B*models[index]->normal(uv)).normalize();
        // the lighting direction.
        Vec3f lightDir = light_dir;
        // the position of this fragment in world space.
        Vec3f fragPos = pos;
        // the viewing direction from eye to this fragment.
        Vec3f viewDir = (eye - fragPos).normalize();
        // the texture color of this fragment (sample from texture using uv coordinate)
        TGAColor fragColor = models[index]->diffuse(uv);

        static float ka = 0.0f;
        static float kd = 0.8f;
        static float ks = 0.4f;

        float diff = 1.0f;
        // TODO 1:
        // calculate the diffuse coefficient using Phong model, and then save it to diff.
        {
            diff=kd*MAX(0,normal*lightDir.normalize());
        }
        TGAColor diffColor = (fragColor*diff);

        float spec = 0.0f;
        // TODO 2:
        // calculate the specular coefficient using Phong model, and then save it to spec.
        // TODO 3:
        // calculate the specular coefficient using Blinn-Phong model, and then save it to spec.
        {
            int p=16;
            // phong
            /*
            Vec3f R=normal*(2*(normal*lightDir.normalize()))-lightDir.normalize();
            float temp=pow(R*viewDir.normalize(),p);
            spec=ks*MAX(0,temp);
            */
            // blinn-phong
            Vec3f h=(viewDir+lightDir).normalize();
            float temp2=pow(normal*h,p);
            spec=ks*MAX(0,temp2);
        }
        TGAColor specColor = TGAColor(255,255,255)*spec;

        // the final color of this fragment taking lighting into account.
        color = diffColor + specColor;

        return false;
    }
};

int main(int argc, char** argv) {
    if (2>argc) {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    // load the models from .obj
    for (int m=1; m<argc; m++) {
        models.push_back(new Model(argv[m]));
    }

    // the shader for rendering.
    Shader shader;

    // z-buffer, framebuffer.
    float *zbuffer = new float[width*height];
    TGAImage frame(width, height, TGAImage::RGB);

    // settings of model, camera and projection matrices.
    viewport(width/8, height/8, width*3/4, height*3/4);
    lookat(eye, center, up);
    projection(-1.f/(eye-center).norm());

    // light direction.
    light_dir = proj<3>((Projection*ModelView*embed<4>(light_dir, 0.f))).normalize();

    int key = 0;
    int frame_count = 0;
    while(key != 27)
    {
        // clear the framebuffer before rendering, and so does z-buffer.
        frame.clear();
        for (int i=width*height; i--; zbuffer[i] = -std::numeric_limits<float>::max());

        // TODO 4: rotate the eye around y-axis.
        {
            // do something to variable "eye" here to achieve rotation.
            eye=rotate_y(10);
        }
        lookat(eye, center, up);
        projection(-1.f/(eye-center).norm());


        // rendering per model.
        for (unsigned int m=0;m<models.size(); ++m) {
            // for each triangle of model,
            // call vertex shader, rasterization and fragment shader to render it.
            for (int i=0; i<models[m]->nfaces(); i++) {
                // vertex shader.
                for (int j=0; j<3; j++) {
                    shader.vertex(m, i, j);
                }
                // rasterization, z-buffering and fragment shader.
                triangle(m, shader.varying_tri, shader, frame, zbuffer);
            }
        }

        // display the rendered image.
        frame.flip_vertically(); // to place the origin in the bottom left corner of the image
        cv::Mat image(width, width, CV_8UC3, frame.buffer());
        cv::imshow("Lighting", image);

        // key event.
		key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';
    }

    frame.write_tga_file("rendered.tga");

    // release the models
    for (unsigned int i = 0; i < models.size();++i) {
        delete models[i];
    }
    std::vector<Model*>().swap(models);
    delete [] zbuffer;

    return 0;
}

Vec3f rotate_y(float angle)
{
    float theta=M_PI*angle/180; // 将角度转换成弧度
    float cos_angle=cos(theta);
    float sin_angle=sin(theta);
    Vec3f new_eye(cos_angle*eye.x+sin_angle*eye.z, eye.y, -sin_angle*eye.x+cos_angle*eye.z);
    return new_eye;
}