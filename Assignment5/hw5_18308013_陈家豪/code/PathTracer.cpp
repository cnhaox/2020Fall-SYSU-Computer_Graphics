#include "PathTracer.h"

#include <time.h>
#include <iostream>
#include "ray.h"
#include "sphere.h"
#include "hitable_list.h"
#include "camera.h"
#include "random.h"
#include "material.h"
#include <cfloat>

#define MAXFLOAT FLT_MAX
PathTracer::PathTracer()
	: m_channel(4), m_width(800), m_height(600), m_image(nullptr) {}

PathTracer::~PathTracer()
{
	if (m_image != nullptr)
		m_image;
	m_image = nullptr;
}

void PathTracer::initialize(int width, int height)
{
	m_width = width;
	m_height = height;
	if (m_image != nullptr)
		delete m_image;

	// allocate pixel buffer, RGBA format.
	m_image = new unsigned char[width * height * m_channel];
}

unsigned char * PathTracer::render(double & timeConsuming)
{
	if (m_image == nullptr)
	{
		std::cout << "Must call initialize() before rendering.\n";
		return nullptr;
	}

	// record start time.
	double startFrame = clock();

	get_scene();
	hitable* world=get_scene();
	vec3 lookfrom(13,2,3);
	vec3 lookat(0,0,0);
	float dist_to_focus=10.0;
	float aperture=0.1;
	camera cam(lookfrom,lookat,vec3(0,1,0),20,float(m_width)/float(m_height),aperture,dist_to_focus);
	int max_depth=50;	// 递归深度

	// render the image pixel by pixel.
	int num=10;
	for (int row = m_height - 1; row >= 0; --row)
	{
		for (int col = 0; col < m_width; ++col)
		{
			vec3 color(0.0,0.0,0.0);
			for (int s=0;s<num;s++)
			{
				float v=(float(row) + random_double())/float(m_height);
				float u=(float(col) + random_double())/float(m_width);
				ray r=cam.get_ray(u,v);	// 与原ray稍有偏离的随机ray
				color += get_color(r, world, max_depth);
			}
			color/=float(num);			// 取平均
			color=vec3(sqrt(color[0]),sqrt(color[1]),sqrt(color[2]));
			// TODO: implement your ray tracing algorithm by yourself.
			drawPixel(col, row, color);
		}
	}

	// record end time.
	double endFrame = clock();

	// calculate time consuming.
	timeConsuming = static_cast<double>(endFrame - startFrame) / CLOCKS_PER_SEC;

	return m_image;
}

void PathTracer::drawPixel(unsigned int x, unsigned int y, const vec3 & color)
{
	// Check out 
	// 如果坐标在范围外，返回
	if (x < 0 || x >= m_width || y < 0 || y >= m_height)
		return;
	// x is column's index, y is row's index.
	unsigned int index = (y * m_width + x) * m_channel;
	// store the pixel.
	// red component.
	m_image[index + 0] = static_cast<unsigned char>(255 * color.x());
	// green component.
	m_image[index + 1] = static_cast<unsigned char>(255 * color.y());
	// blue component.
	m_image[index + 2] = static_cast<unsigned char>(255 * color.z());
	// alpha component.
	m_image[index + 3] = static_cast<unsigned char>(255);
}

vec3 PathTracer::random_in_unit_sphere()
{
	vec3 p;
	do
	{
		p=2.0*vec3(random_double(), random_double(),random_double())-vec3(1,1,1);
	}while (p.squared_length()>=1.0);
	return p;
}

vec3 PathTracer::get_color(const ray& r, hitable *world, int depth) 
{
	hit_record rec;
	if (depth<=0)
	return vec3(0.0,0.0,0.0);
	if (world->hit(r, 0.001, MAXFLOAT, rec))
	{
		ray scattered;// 下一个ray
		vec3 attenuation;// 衰减系数
		if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))// 如果反射光线有效
		return attenuation*get_color(scattered,world,depth-1);
		else
		return rec.orig_color;
	}
    else// 未击中物体，返回背景色
	{
        vec3 unit_direction = unit_vector(r.direction());
        float t = 0.5*(unit_direction.y() + 1.0);
        return (1.0-t)*vec3(0.5, 0.5, 0.5) + t*vec3(0.5, 0.7, 1.0);
    }
}

hitable* PathTracer::get_scene()
{
	int n = 400;
    hitable **list = new hitable*[n+1];
    list[0] =  new sphere(vec3(0,-1000,0), 1000, new lambertian(vec3(0.5, 0.5, 0.5)),vec3(0,0,0));
    int i = 1;
    for (int a = -8; a < 8; a++) 
	{
        for (int b = -8; b < 8; b++) 
		{
            float choose_mat = random_double();
            vec3 center(a+0.9*random_double(),0.2,b+0.9*random_double());
            if ((center-vec3(4,0.2,0)).length() > 0.9) 
			{
                if (choose_mat < 0.5) 
				{  // diffuse
                    list[i++] = new sphere(
                        center, 0.2,
                        new lambertian(vec3(random_double()*random_double(),
                                            random_double()*random_double(),
                                            random_double()*random_double())),
						vec3(0,0,0)
                    );
                }
                else if (choose_mat < 0.7) 
				{ // metal
                    list[i++] = new sphere(
                        center, 0.2,
                        new metal(vec3(0.5*(1 + random_double()),
                                       0.5*(1 + random_double()),
                                       0.5*(1 + random_double())),
                                  	   0.5*random_double()),
						vec3(0,0,0)
                    );
                }
                else if (choose_mat < 0.95)
				{  // glass
                    list[i++] = new sphere(center, 0.2, new dielectric(1.5),vec3(0,0,0));
                }
				else
				{  // light
					list[i++] = new sphere(center, 0.2, new light(), vec3(0.9, 0.9, 0.9));
				}
            }
        }
	}
    list[i++] = new sphere(vec3(0, 1, 0), 1.0, new dielectric(1.5),vec3(0,0,0));
    list[i++] = new sphere(vec3(-4, 1, 0), 1.0, new lambertian(vec3(0.4, 0.2, 0.1)),vec3(0,0,0));
    list[i++] = new sphere(vec3(4, 1, 0), 1.0, new metal(vec3(0.7, 0.6, 0.5), 0.0),vec3(0,0,0));
	list[i++] = new sphere(vec3(0, 4, 1), 1.0, new light(),vec3(1,1,1));
	return new hitable_list(list, i);
}