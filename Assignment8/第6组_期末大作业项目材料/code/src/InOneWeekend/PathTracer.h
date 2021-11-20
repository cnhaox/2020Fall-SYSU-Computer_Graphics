#ifndef PATHTRACER_H
#define PATHTRACER_H
#include "camera.h"
#include "hittable_list.h"
#include "bvh_node.h"

#include<vector>

class camera;

hittable_list random_scene();
hittable_list make_scene1();
void scene_tree(hittable_list &world, hittable_list &lights);
void light_scene(hittable_list &world, hittable_list &lights);
class PathTracer
{
private:
	// RGBA format.
	int m_channel;
	// image's size.
	int m_width, m_height;
	// image's pixel buffer.
	float *temp_image=nullptr;
	unsigned char *m_image=nullptr;
	
public:
	// Ctor/Dtor.
	PathTracer(camera _cam);
	~PathTracer();

	// Getter.
	int getWidth() const { return m_width; }
	int getHeight() const { return m_height; }
	int getChanne() const { return m_channel; }

	void setCamera(camera cam) { _cam = cam; }
	void setWorldScene(hittable_list&& scene) { _world= scene; }
	void setWorldBVHroot(BVHnode root) {_WorldRoot = root;}
	void setLightScene(hittable_list&& scene) { _light = scene; }
	void setLightBVHroot(BVHnode root) {_LightRoot = root;}
	unsigned char *getImage() const { return m_image; }

	// Must call it for the first of all.
	void initialize(int width, int height);
	
	// Render a frame.
	unsigned char *render(double &timeConsuming);
	color ray_color(const ray &r, const hittable &world, shared_ptr<hittable> lights);
	camera _cam;
	hittable_list _world;
	hittable_list _light;
	BVHnode _WorldRoot;
	BVHnode _LightRoot;

private:
	// Draw one pixel in (y,x).
	void drawPixel(unsigned int x, unsigned int y, const vec3 &color);
	void turnImage();
};

#endif // PATHTRACER
