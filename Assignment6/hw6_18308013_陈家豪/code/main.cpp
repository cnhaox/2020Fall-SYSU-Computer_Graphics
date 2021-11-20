#include <chrono>
#include <iostream>
#include <opencv2/opencv.hpp>
#define point_num 10
std::vector<cv::Point2f> control_points;

void mouse_handler(int event, int x, int y, int flags, void *userdata) 
{
    if (event == cv::EVENT_LBUTTONDOWN && control_points.size() < point_num) 
    {
        std::cout << "Left button of the mouse is clicked - position (" << x << ", "
        << y << ")" << '\n';
        control_points.emplace_back(x, y);
    }     
}

void naive_bezier(const std::vector<cv::Point2f> &points, cv::Mat &window) 
{
    auto &p_0 = points[0];
    auto &p_1 = points[1];
    auto &p_2 = points[2];
    auto &p_3 = points[3];

    for (double t = 0.0; t <= 1.0; t += 0.001) 
    {
        auto point = std::pow(1 - t, 3) * p_0 + 3 * t * std::pow(1 - t, 2) * p_1 +
                 3 * std::pow(t, 2) * (1 - t) * p_2 + std::pow(t, 3) * p_3;

        window.at<cv::Vec3b>(point.y, point.x)[2] = 255;
    }
}

cv::Point2f de_Casteljau(const std::vector<cv::Point2f> &control_points, float t) 
{
    // TODO: Implement de Casteljau's algorithm
    // return cv::Point2f();
    if (control_points.size()==0)     // 如果没有点，返回初始值
    return cv::Point2f();
    else if (control_points.size()==1)// 如果只有一个点，返回该点
    return control_points[0];
    else if (control_points.size()==2)// 如果只有两个点，进行插值返回
    return control_points[0]+t*(control_points[1]-control_points[0]);

    std::vector<cv::Point2f> new_control_points;// 新的点vector
    for (int i=0;i<control_points.size()-1;i++) // 依次对相邻的点插值，新点存入vector
    new_control_points.push_back(control_points[i]+t*(control_points[i+1]-control_points[i]));
    return de_Casteljau(new_control_points,t);  // 递归
}

void bezier(const std::vector<cv::Point2f> &control_points, cv::Mat &window) 
{
    // TODO: Iterate through all t = 0 to t = 1 with small steps, and call de Casteljau's 
    // recursive Bezier algorithm.
    float interval=0.0001;
    for (float t=0;t<=1;t+=interval)
    {
        cv::Point2f newPoint=de_Casteljau(control_points,t);// 该t下对应的点
        window.at<cv::Vec3b>(newPoint.y,newPoint.x)[1]=255; // 对该点所在坐标的对应像素填充绿色
    }
}

int main() 
{
    cv::Mat window = cv::Mat(600, 800, CV_8UC3, cv::Scalar(0));
    cv::cvtColor(window, window, cv::COLOR_BGR2RGB);
    cv::namedWindow("Bezier Curve", cv::WINDOW_AUTOSIZE);

    cv::setMouseCallback("Bezier Curve", mouse_handler, nullptr);

    int key = -1;
    while (key != 27) 
    {
        for(size_t i = 0;i < control_points.size();++i)
        {
            auto &point = control_points[i];
            cv::circle(window, point, 3, {255, 255, 255}, 3);
            if(i + 1 < control_points.size())
            {
                auto &next = control_points[i+1];
                cv::line(window, point, next, {255, 255, 255}, 1);
            }
        }

        if (control_points.size() == point_num) 
        {
            //naive_bezier(control_points, window);
            bezier(control_points, window);

            cv::imshow("Bezier Curve", window);
            cv::imwrite("my_bezier_curve.png", window);
            key = cv::waitKey(0);

            return 0;
        }

        cv::imshow("Bezier Curve", window);
        key = cv::waitKey(20);
    }

return 0;
}
