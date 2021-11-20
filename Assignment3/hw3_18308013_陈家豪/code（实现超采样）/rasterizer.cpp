// clang-format off
//
// Created by goksu on 4/6/19.
//

#include <algorithm>
#include <vector>
#include "rasterizer.hpp"
#include <opencv2/opencv.hpp>
#include <math.h>


rst::pos_buf_id rst::rasterizer::load_positions(const std::vector<Eigen::Vector3f> &positions)
{
    auto id = get_next_id();
    pos_buf.emplace(id, positions);

    return {id};
}

rst::ind_buf_id rst::rasterizer::load_indices(const std::vector<Eigen::Vector3i> &indices)
{
    auto id = get_next_id();
    ind_buf.emplace(id, indices);

    return {id};
}

rst::col_buf_id rst::rasterizer::load_colors(const std::vector<Eigen::Vector3f> &cols)
{
    auto id = get_next_id();
    col_buf.emplace(id, cols);

    return {id};
}

auto to_vec4(const Eigen::Vector3f& v3, float w = 1.0f)
{
    return Vector4f(v3.x(), v3.y(), v3.z(), w);
}

static float cross2D(float x1, float y1, float x2, float y2)
{
	return x1 * y2 - x2 * y1;
}

static bool insideTriangle(float x, float y, const Vector3f* _v)
{   
    // TODO 2: Implement this function to check if the point (x, y) is inside the triangle represented by _v[0], _v[1], _v[2]
    Eigen::Vector3f p(x,y,0.0f);
    Eigen::Vector3f v01=_v[1]-_v[0];
    Eigen::Vector3f c01=v01.cross(p-_v[0]);// 叉乘结果1
    Eigen::Vector3f v12=_v[2]-_v[1];
    Eigen::Vector3f c12=v12.cross(p-_v[1]);// 叉乘结果2
    Eigen::Vector3f v20=_v[0]-_v[2];
    Eigen::Vector3f c20=v20.cross(p-_v[2]);// 叉乘结果3
    // 如果方向相同，返回真
    if ((c01[2]>0 && c12[2]>0 && c20[2]>0)||(c01[2]<0 && c12[2]<0 && c20[2]<0))
    return true;
    else
    return false;
}

static std::tuple<float, float, float> computeBarycentric2D(float x, float y, const Vector3f* v)
{
    float c1 = (x*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*y + v[1].x()*v[2].y() - v[2].x()*v[1].y()) / (v[0].x()*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*v[0].y() + v[1].x()*v[2].y() - v[2].x()*v[1].y());
    float c2 = (x*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*y + v[2].x()*v[0].y() - v[0].x()*v[2].y()) / (v[1].x()*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*v[1].y() + v[2].x()*v[0].y() - v[0].x()*v[2].y());
    float c3 = (x*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*y + v[0].x()*v[1].y() - v[1].x()*v[0].y()) / (v[2].x()*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*v[2].y() + v[0].x()*v[1].y() - v[1].x()*v[0].y());
    return {c1,c2,c3};
}

void rst::rasterizer::draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer, col_buf_id col_buffer, Primitive type)
{
    auto& buf = pos_buf[pos_buffer.pos_id];
    auto& ind = ind_buf[ind_buffer.ind_id];
    auto& col = col_buf[col_buffer.col_id];

    float f1 = (50 - 0.1) / 2.0;
    float f2 = (50 + 0.1) / 2.0;

    Eigen::Matrix4f mvp = projection * view * model;
    for (auto& i : ind)
    {
        Triangle t;
        Eigen::Vector4f v[] = {
                mvp * to_vec4(buf[i[0]], 1.0f),
                mvp * to_vec4(buf[i[1]], 1.0f),
                mvp * to_vec4(buf[i[2]], 1.0f)
        };
        //Homogeneous division
        for (auto& vec : v) {
            vec /= vec.w();
        }
        //Viewport transformation
        for (auto & vert : v)
        {
            vert.x() = 0.5*width*(vert.x()+1.0);
            vert.y() = 0.5*height*(vert.y()+1.0);
            vert.z() = vert.z() * f1 + f2;
        }

        for (int i = 0; i < 3; ++i)
        {
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
        }

        auto col_x = col[i[0]];
        auto col_y = col[i[1]];
        auto col_z = col[i[2]];

        t.setColor(0, col_x[0], col_x[1], col_x[2]);
        t.setColor(1, col_y[0], col_y[1], col_y[2]);
        t.setColor(2, col_z[0], col_z[1], col_z[2]);

        rasterize_triangle(t);
    }
}

//Screen space rasterization
void rst::rasterizer::rasterize_triangle(const Triangle& t) {
    auto v = t.toVector4();
        
    float xmin = FLT_MAX, xmax = FLT_MIN;
    float ymin = FLT_MAX, ymax = FLT_MIN;
    // TODO 1: Find out the bounding box of current triangle.
    {
        xmin=MIN(v[0][0],MIN(v[1][0],v[2][0]));// x方向上的最小值
        xmax=MAX(v[0][0],MAX(v[1][0],v[2][0]));// x方向上的最大值
        ymin=MIN(v[0][1],MIN(v[1][1],v[2][1]));// y方向上的最小值
        ymax=MAX(v[0][1],MAX(v[1][1],v[2][1]));// y方向上的最大值
    }
    // After you have calculated the bounding box, please comment the following code.
    // return;
        
    // iterate through the pixel and find if the current pixel is inside the triangle
    for(int x = static_cast<int>(xmin);x <= xmax;++x)
    {
        for(int y = static_cast<int>(ymin);y <= ymax;++y)
        {
            // 对每个像素的子采样点进行遍历
            for (int subx=0; subx<2; subx++)
            {
                for (int suby=0; suby<2; suby++)
                {
                    float x1=x+0.5*subx;// 子采样点的x坐标
                    float y1=y+0.5*suby;// 子采样点的x坐标
                    // if it's not in the area of current triangle, just do nothing.
                	if(!insideTriangle(x1, y1, t.v))// 判断子采样点是否在三角形内
                	continue;
                	// otherwise we need to do z-buffer testing.
                    // use the following code to get the depth value of pixel (x,y), it's stored in z_interpolated
                    // 计算子采样的深度
                    auto[alpha, beta, gamma] = computeBarycentric2D(x1, y1, t.v);
                    float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                    float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                    z_interpolated *= w_reciprocal;

                    // TODO 3: perform Z-buffer algorithm here.
                    // 如果子采样点的深度大于当前深度，不更新
                    if (z_interpolated>=depth_buf[get_subindex(x,y,subx,suby)])
                    continue;
                    // 子采样点的深度小于当前深度，更新depth_buf和preframe_buf
                    depth_buf[get_subindex(x,y,subx,suby)]=z_interpolated;
                    preframe_buf[get_subindex(x,y,subx,suby)]=t.getColor();
                }
            }
            // 计算像素点的平均颜色值
            Eigen::Vector3f newcolor(0.0f,0.0f,0.0f);
            for (int subx=0; subx<2; subx++)
            {
                for (int suby=0; suby<2; suby++)
                newcolor+=preframe_buf[get_subindex(x,y,subx,suby)];
            }
            // 对该像素点的颜色进行赋值
            set_pixel(Vector3f(x,y,1.0f),newcolor/4);
        }
    }
}

void rst::rasterizer::set_model(const Eigen::Matrix4f& m)
{
    model = m;
}

void rst::rasterizer::set_view(const Eigen::Matrix4f& v)
{
    view = v;
}

void rst::rasterizer::set_projection(const Eigen::Matrix4f& p)
{
    projection = p;
}

void rst::rasterizer::clear(rst::Buffers buff)
{
    if ((buff & rst::Buffers::Color) == rst::Buffers::Color)
    {
        std::fill(preframe_buf.begin(), preframe_buf.end(), Eigen::Vector3f{0, 0, 0});
        std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f{0, 0, 0});
    }
    if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth)
    {
        std::fill(depth_buf.begin(), depth_buf.end(), std::numeric_limits<float>::infinity());
    }
}

rst::rasterizer::rasterizer(int w, int h) : width(w), height(h)
{
    preframe_buf.resize(4*w*h);
    frame_buf.resize(w*h);
    depth_buf.resize(4*w*h);
}

int rst::rasterizer::get_index(int x, int y)
{
    return (height-1-y)*width + x;
}

int rst::rasterizer::get_subindex(int x, int y, int subx, int suby)
{
    int X=2*x+subx;
    int Y=2*y+suby;
    return (2*height-1-Y)*2*width + X;
}

void rst::rasterizer::set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color)
{
    //old index: auto ind = point.y() + point.x() * width;
    auto ind = (height-1-point.y())*width + point.x();
    frame_buf[ind] = color;

}

// clang-format on