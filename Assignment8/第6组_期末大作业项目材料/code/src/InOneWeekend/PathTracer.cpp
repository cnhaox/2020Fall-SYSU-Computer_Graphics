#include "PathTracer.h"
#include "rtweekend.h"

#include "color.h"
#include "material.h"
#include "triangle.h"
#include "sphere.h"
#include "model.hpp"
#include <omp.h>
#include <time.h>
#include <iostream>
#include <atomic>
#include <glm/gtc/matrix_transform.hpp>

#if HAVE_UNISTD
#include <tqdm/tqdm.h>
#endif

color ray_color(const ray &r, const hittable &world, shared_ptr<hittable> lights);
hittable_list random_scene();

PathTracer::PathTracer(camera cam)
	: m_channel(4), m_width(800), m_height(600), m_image(nullptr), _cam(cam) {}

PathTracer::~PathTracer()
{
	if (m_image != nullptr)
		m_image;
	m_image = nullptr;
	if (temp_image != nullptr)
		temp_image;
	temp_image = nullptr;
}

void PathTracer::initialize(int width, int height)
{
	m_width = width;
	m_height = height;
	if (m_image != nullptr)
		delete m_image;
	if (temp_image != nullptr)
		delete temp_image;
	// allocate pixel buffer, RGBA format.
	m_image = new unsigned char[width * height * m_channel];
	temp_image = new float[width * height * 3];
}

unsigned char *PathTracer::render(double &timeConsuming)
{
	if (m_image == nullptr)
	{
		std::cout << "Must call initialize() before rendering.\n";
		return nullptr;
	}

	// record start time.
	double startFrame = clock();
	// shared_ptr<hittable_list> lp(&_light);
	auto lights = make_shared<hittable_list>();
	lights->add(make_shared<sphere>(point3(-4, 53, 7), 28, make_shared<light_source>(color(1, 1.5, 2.3) * 0.8)));
#if HAVE_UNISTD
	tqdm bar;
	std::atomic_size_t progress;
	progress = 0;
#pragma omp parallel for shared(progress, bar)
	// render the image pixel by pixel.
#else
#ifdef NDEBUG
#pragma omp parallel for
#endif
#endif
	for (int row = m_height - 1; row >= 0; --row)
	{
		for (int col = 0; col < m_width; ++col)
		{
			// TODO: implement your ray tracing algorithm by yourself.
			vec3 color(0.0, 0.0, 0.0);
			for (int s = 0; s < MAX_SAMPLE; s++)
			{
				auto u = (col + random_double()) / (m_width - 1);
				auto v = (row + random_double()) / (m_height - 1);
				ray r = _cam.get_ray(u, v);
				color += ray_color(r, _WorldRoot, lights);
			}
			// 防止NaN
			for (int i = 0; i < 3; i++)
				if (color[i] != color[i])
					color[i] = 0.0;
			color /= MAX_SAMPLE;
			color = vec3(sqrt(color[0]), sqrt(color[1]), sqrt(color[2]));
			// 防止值溢出
			double max = 0;
			for (int i = 0; i < 3; i++)
			{
				if (color[i] >= max)
					max = color[i];
			}
			if (max >= 1)
				color /= max;
			//color[0] = static_cast<float>(col) / static_cast<float>(m_width);
			//color[1] = static_cast<float>(row) / static_cast<float>(m_height);
			//color[2] = 0.2;
			drawPixel(col, row, color);
#if HAVE_UNISTD
#pragma omp critical
			bar.progress(progress.fetch_add(1), m_height * m_width);
#endif
		}
	}
#if HAVE_UNISTD
	bar.finish();
#endif
	// record end time.
	double endFrame = clock();

	// calculate time consuming.
	timeConsuming = static_cast<double>(endFrame - startFrame) / CLOCKS_PER_SEC;
	turnImage();

	return m_image;
}

void PathTracer::drawPixel(unsigned int x, unsigned int y, const vec3 &color)
{
	// Check out
	if (x < 0 || x >= m_width || y < 0 || y >= m_height)
		return;
	// x is column's index, y is row's index.
	unsigned int index = (y * m_width + x) * 3;
	// store the pixel.
	// red component.
	temp_image[index + 0] = float(color.x());
	// green component.
	temp_image[index + 1] = float(color.y());
	// blue component.
	temp_image[index + 2] = float(color.z());
}

void PathTracer::turnImage()
{// 将float类型的temp_image转为m_image
	// 归一化，防止溢出
	int image_size = m_width*m_height*3;
	float max_num=0;
	for (int i=0; i<image_size;i++)
	{
		if (max_num<temp_image[i])
		max_num=temp_image[i];
	}
	if (max_num < 1.00)
	max_num=1.00;
	for (int i=0; i<m_width*m_height;i++)
	{
		m_image[i*m_channel] = static_cast<unsigned char>(255*temp_image[i*3]/max_num);
		m_image[i*m_channel+1] = static_cast<unsigned char>(255*temp_image[i*3+1]/max_num);
		m_image[i*m_channel+2] = static_cast<unsigned char>(255*temp_image[i*3+2]/max_num);
		m_image[i*m_channel+3] = static_cast<unsigned char>(255);
	}
}

color PathTracer::ray_color(const ray &r, const hittable &world, shared_ptr<hittable> lights)
{
	// 轮盘赌
	double choice = random_double();
	if (choice > CONTINUE_PROBABILITY || choice > 0.98)
		return color(0, 0, 0);
	hit_record rec;
	if (world.hit(r, 0.001, infinity, rec))
	{
		scatter_record srec;
		color emitted_color = rec.mat_ptr->emit(r, rec);
		color return_color;
		
		vec3 light_path = rec.p - r.orig;
		double distance_s = light_path.length_squared(); // 发射点与击中点欧式距离平方
		
		if (distance_s < 0.8)
			distance_s = 0.8; // 避免distance_s过小导致除法溢出
		if (distance_s > 1.6)
			distance_s = 1.6;
		distance_s=1;
		
		if (rec.mat_ptr->scatter(r, rec, srec) && !rec.isSkybox)
		{
			if (srec.is_specular)
			return_color = srec.attenuation*ray_color(srec.specular_ray, world, lights);
			else
			{
				auto light_ptr = make_shared<hittable_pdf>(lights, rec.p);
				mixture_pdf p(light_ptr, srec.pdf_ptr);
				ray scattered = ray(rec.p, p.generate(), r.time());
				double pdf_val = p.value(scattered.direction());
				return_color = emitted_color + srec.attenuation * rec.mat_ptr->scattering_pdf(r, rec, scattered) * ray_color(scattered, world, lights)/pdf_val;
			}
		}
		else if (rec.isSkybox)
			return_color = emitted_color+0.6 * srec.attenuation;
		else
			return_color = emitted_color; ///(distance*distance);// 返回光源色
		return (return_color/distance_s)/CONTINUE_PROBABILITY;
	}

	vec3 unit_direction = unit_vector(r.direction());
	auto t = 0.5 * (unit_direction.y() + 1.0);
	return 0.1*((1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0));
	// return color(0,0,0);
}

hittable_list random_scene()
{

	hittable_list world;
	world.add(make_shared<sphere>(point3(0.0f, -1000, 0.0f), 1010.0f,
								  make_shared<lambertian>(make_shared<image_texture>("assets/sky.jpg")),
								  0, 0,
								  false));

	auto ground_material = make_shared<lambertian>(make_shared<image_texture>("assets/2k_earth_daymap.jpg"));
	world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));

	for (int a = -9; a < 9; a++)
	{
		for (int b = -9; b < 9; b++)
		{
			auto choose_mat = random_double();
			point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

			if ((center - point3(4, 0.2, 0)).length() > 0.9 && (center - point3(-4, 0.2, 0)).length() > 0.9 && (center - point3(0, 0.2, 0)).length() > 0.9)
			{
				shared_ptr<material> sphere_material;

				if (choose_mat < 0.8)
				{
					// diffuse
					auto albedo = color::random() * color::random();
					sphere_material = make_shared<lambertian>(albedo);
					world.add(make_shared<sphere>(center, 0.2, sphere_material));
				}
				else if (choose_mat < 0.95)
				{
					// metal
					auto albedo = color::random(0.5, 1);
					auto fuzz = random_double(0, 0.5);
					sphere_material = make_shared<metal>(albedo, fuzz);
					world.add(make_shared<sphere>(center, 0.2, sphere_material));
				}
				else
				{
					// glass
					sphere_material = make_shared<dielectric>(1.5);
					world.add(make_shared<sphere>(center, 0.2, sphere_material));
				}
			}
		}
	}

	auto material1 = make_shared<dielectric>(1.5);
	world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

	//auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
	auto material2 = make_shared<lambertian>(make_shared<image_texture>("assets/2k_earth_daymap.jpg"));
	world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

	auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
	world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

	return world;
}

void scene_tree(hittable_list &world, hittable_list &lights)
{

	world.add(make_shared<sphere>(
		point3(0, -1000, 0), 1000,
		make_shared<lambertian>(make_shared<image_texture>("assets/2k_earth_daymap.jpg"))));

	world.add(make_shared<sphere>(point3(0.0f, 0.0f, 0.0f), 1000.0f,
								  make_shared<lambertian>(make_shared<image_texture>("assets/christmas4t.png")),
								  -pi * 67 / 180, -pi * 7 / 180,
								  false));
	
	model(
		"assets/old_house.obj",
		std::unordered_map<std::string, std::shared_ptr<material>>{
			{"default", make_shared<lambertian>(color(0.5, 0.5, 0.5))},
			{"Standard_1", make_shared<lambertian>(make_shared<image_texture>("assets/house/house_body.jpg"))},
			{"Standard_2", make_shared<lambertian>(make_shared<image_texture>("assets/house/plants2.jpg"))},
			{"Standard_3", make_shared<lambertian>(make_shared<image_texture>("assets/house/plants1.jpg"))},
		},
		make_shared<matrix_transformer>(glm::scale(glm::dmat4(1.0), glm::dvec3(0.01, 0.01, 0.01))))
		.add_to_scene(world);
	auto tree_mat = make_shared<lambertian>(make_shared<image_texture>("assets/tree.png"));
	auto glass_mat = make_shared<dielectric>(1.33);
	model(
		"assets/tree.obj",
		std::unordered_map<std::string, std::shared_ptr<material>>{{"default", tree_mat}},
		make_shared<matrix_transformer>(glm::translate(
			glm::scale(glm::dmat4(1.0), glm::dvec3(1.5, 1.5, 1.5)),
			glm::dvec3(3.5, 0.0, -2))))
		.add_to_scene(world);
	model(
		"assets/tree.obj",
		std::unordered_map<std::string, std::shared_ptr<material>>{{"default", glass_mat}},
		make_shared<matrix_transformer>(glm::translate(
			glm::dmat4(1.0), glm::dvec3(-2, 0.0, 1.5))))
		.add_to_scene(world);
	model(
		"assets/tree.obj",
		std::unordered_map<std::string, std::shared_ptr<material>>{{"default", tree_mat}},
		make_shared<matrix_transformer>(glm::translate(glm::dmat4(1.0), glm::dvec3(2, 0, 5))))
		.add_to_scene(world);

	auto snowman_mat = make_shared<lambertian>(make_shared<solid_color>(color(1.0)));

	model(
		"assets/snowman.obj",
		std::unordered_map<std::string, std::shared_ptr<material>>{
			{"default", make_shared<lambertian>(color(1, 1, 1))},
			{"aiStandardSurface11SG", make_shared<lambertian>(color(133, 0, 0) / 255)},	 //帽子
			{"aiStandardSurface18SG", make_shared<lambertian>(color(203, 62, 0) / 255)}, //尖鼻子
			{"aiStandardSurface15SG", make_shared<lambertian>(color(145, 0, 0) / 255)},	 //围巾
			{"aiStandardSurface17SG", make_shared<lambertian>(color(0, 0, 0) / 255)},	 //眼睛
			{"aiStandardSurface16SG", make_shared<lambertian>(color(24, 0, 0) / 255)},	 //纽扣
			{"aiStandardSurface19SG", make_shared<lambertian>(color(85, 38, 18) / 255)}	 //手
		},
		make_shared<matrix_transformer>(
			glm::rotate(
				glm::translate(
					glm::scale(
						glm::dmat4(1.0),
						glm::dvec3(0.3f, 0.3f, 0.3f)),
					glm::dvec3(-2.0f, 0.0f, 3.0f)),
				glm::radians(double(-244.0)), glm::dvec3(0.0f, 1.0f, 0.0f))))
		.add_to_scene(world);
	auto material2 = make_shared<lambertian>(make_shared<image_texture>("assets/2k_earth_daymap.jpg"));
	world.add(make_shared<sphere>(point3(2.8, 0.3, 2), 0.3, material2));

	// light
	lights.add(make_shared<sphere>(point3(5, 40, 7.5), 28, make_shared<light_source>(color(2, 3, 4.6) * 0.5)));
	world.add(make_shared<sphere>(point3(5, 40, 7.5), 28, make_shared<light_source>(color(2, 3, 4.6) * 0.5)));
	auto light_material = make_shared<light_source>(color(2, 2, 2));
	for (int i = 0; i < 40; i++)
		world.add(make_shared<sphere>(point3(-3.4 + random_double() * 7, 1 + random_double() * 5, -2 + random_double() * 7), random_double() / 32, light_material));
	
	auto random_material1 = make_shared<lambertian>(color(0.9,0.9,0.9));
	auto random_material2 = make_shared<dielectric>(1.3);
	for (int i = 0; i < 100; i++)
	{
		if (random_double()<0.6)
			world.add(make_shared<sphere>(point3(-3 + random_double() * 10, -0.5 + random_double() * 5, -3 + random_double() * 10), random_double() / 36, random_material1));
		else
			world.add(make_shared<sphere>(point3(-3 + random_double() * 10, -0.5 + random_double() * 5, -3 + random_double() * 10), random_double() / 36, random_material2));
	}
	auto bmaterial_l1 = make_shared<dielectric_light>(1.5, color(0.51, 0.42, 2) / 8);
	auto bmaterial_l2 = make_shared<dielectric_light>(1.5, color(0.392, 1.607, 0.392) / 8);

	auto bmaterial_m1 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
	auto bmaterial_d1 = make_shared<dielectric>(1.5);
	//orld.add(make_shared<sphere>(point3(-1.7, 0.1, 2), 0.1, bmaterial_l1));
	//world.add(make_shared<sphere>(point3(-2.6, 0.13, -2.3), 0.13, bmaterial_l2));
	//world.add(make_shared<sphere>(point3(-2, 2.1, 1.5), 0.1, bmaterial_m1));
	world.add(make_shared<sphere>(point3(4, 0.4, 1.8), 0.4, bmaterial_m1));
	world.add(make_shared<sphere>(point3(3.2, 0.2, 1.4), 0.2, bmaterial_d1));

}

hittable_list make_scene1()
{
	hittable_list world;
	world.add(make_shared<model>("assets/lowpolytree.obj"));
	auto ground_material = make_shared<lambertian>(make_shared<image_texture>("assets/sky.jpg"));
	auto material1 = make_shared<lambertian>(color(0.1, 0.2, 0.5));
	auto material2 = make_shared<lambertian>(color(0.8, 0.8, 0));
	auto material3 = make_shared<metal>(color(0.8, 0.6, 0.2), 0);
	auto material4 = make_shared<dielectric>(1.5);
	auto material5 = make_shared<lambertian>(color(1, 1, 1));
	auto light_material1 = make_shared<light_source>(color(1, 1, 1));
	auto light_material2 = make_shared<light_source>(color(0.8, 0.4, 0.7));

	world.add(make_shared<sphere>(point3(0.0f, 0.0f, -1.0f), 0.5f, material1));
	world.add(make_shared<sphere>(point3(0.0f, 0.0f, 0.0f), 1000.0f, ground_material, false));
	world.add(make_shared<sphere>(point3(1.0f, 0.0f, -1.0f), 0.5f, material3));
	// world.add(make_shared<sphere>(point3(-1.0f, 0.0f, -1.0f), -0.45f, material4));
	// world.add(make_shared<sphere>(point3(-1.0f, 0.0f, -1.0f), 0.5f, material4));
	// // light
	// world.add(make_shared<sphere>(point3(0.0f, 5.0f, 22.0f), 0.2f, light_material2));
	// // triangle
	// world.add(make_shared<triangle>(vertex{point3{-1.0f, 0.0f, 0.0f},vec3(0,0,1),0,0},vertex{point3{0.0f, 1.0f, 0.0f},vec3(0,0,1),0,1},vertex{point3{1.0f, 0.0f, 0.0f},vec3(0,0,1),1,0}, ground_material));
	// world.add(make_shared<triangle>(vertex{point3{-1.0f, 0.0f, 0.0f},vec3(0,0,1),0,0},vertex{point3{0.0f, 1.0f, 0.0f},vec3(0,0,1),0,1},vertex{point3{1.0f, 0.0f, 0.0f},vec3(0,0,1),1,0}, ground_material));
	return world;
}

void light_scene(hittable_list &world, hittable_list &lights)
{
	//hittable_list world;
	world.add(make_shared<sphere>(point3(0.0f, -1000, 0.0f), 1010.0f,
								  make_shared<lambertian>(make_shared<image_texture>("assets/sky.jpg")),
								  0, 0,
								  false));

	auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
	world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));

	for (int a = -14; a < 14; a++)
	{
		for (int b = -14; b < 14; b++)
		{
			auto choose_mat = random_double();
			point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

			if ((center - point3(4, 0.2, 0)).length() > 0.9 && (center - point3(-4, 0.2, 0)).length() > 0.9 && (center - point3(0, 0.2, 0)).length() > 0.9)
			{
				shared_ptr<material> sphere_material;

				if (choose_mat < 0.5)
				{
					// diffuse
					auto albedo = color::random() * color::random();
					sphere_material = make_shared<lambertian>(albedo);
					world.add(make_shared<sphere>(center, 0.2, sphere_material));
				}
				else if (choose_mat < 0.7)
				{
					// metal
					auto albedo = color::random(0.5, 1);
					auto fuzz = random_double(0, 0.5);
					sphere_material = make_shared<metal>(albedo, fuzz);
					world.add(make_shared<sphere>(center, 0.2, sphere_material));
				}
				else if (choose_mat < 0.85)
				{
					// glass
					sphere_material = make_shared<dielectric>(1.5);
					world.add(make_shared<sphere>(center, 0.2, sphere_material));
				}
				else if (choose_mat < 0.92)
				{
					sphere_material = make_shared<light_source>(color::random() * color::random()*2.0f);
					world.add(make_shared<sphere>(center, 0.2, sphere_material));
					lights.add(make_shared<sphere>(center, 0.2, sphere_material));
				}
				else
				{
					// light
					sphere_material = make_shared<light_source>(color(2.0f, 2.0f, 2.0f));
					world.add(make_shared<sphere>(center, 0.2, sphere_material));
					lights.add(make_shared<sphere>(center, 0.2, sphere_material));
				}
			}
		}
	}

	auto material1 = make_shared<dielectric>(1.5);
	world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

	auto material2 = make_shared<lambertian>(make_shared<image_texture>("assets/2k_earth_daymap.jpg"));
	world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

	auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
	world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

}
