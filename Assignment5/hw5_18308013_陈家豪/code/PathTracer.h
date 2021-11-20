#ifndef PATHTRACERH
#define PATHTRACERH

#include "vec3.h"
#include "hitable.h"
class PathTracer
{
private:
	// RGBA format.
	int m_channel;
	// image's size.
	int m_width, m_height;
	// image's pixel buffer.
	unsigned char *m_image;

public:
	// Ctor/Dtor.
	PathTracer();
	~PathTracer();

	// Getter.
	int getWidth() const { return m_width; }
	int getHeight() const { return m_height; }
	int getChanne() const { return m_channel; }
	unsigned char *getImage() const { return m_image; }

	// Must call it for the first of all.
	void initialize(int width, int height);

	// Render a frame.
	unsigned char *render(double &timeConsuming);

private:
	// Draw one pixel in (y,x).
	void drawPixel(unsigned int x, unsigned int y, const vec3 &color);
	vec3 get_color(const ray& r, hitable* world, int depth);
	vec3 random_in_unit_sphere();
	hitable* get_scene();
};

#endif