#include<cmath>
#include<eigen3/Eigen/Core>
#include<eigen3/Eigen/Dense>
#include<iostream>
using namespace std;
int main(){

    // Basic Example of cpp
    std::cout << "Example of cpp \n";
    float a = 1.0, b = 2.0;
    std::cout << a << std::endl;
    std::cout << a/b << std::endl;
    std::cout << std::sqrt(b) << std::endl;
    std::cout << std::acos(-1) << std::endl;
    std::cout << std::sin(30.0/180.0*acos(-1)) << std::endl;

    // Example of vector
    std::cout << "Example of vector \n";
    // vector definition
    Eigen::Vector3f v(1.0f,2.0f,3.0f);
    Eigen::Vector3f w(1.0f,0.0f,0.0f);
    // vector output
    std::cout << "Example of output \n";
    std::cout << v << std::endl;
    // vector add
    std::cout << "Example of add \n";
    std::cout << v + w << std::endl;
    // vector scalar multiply
    std::cout << "Example of scalar multiply \n";
    std::cout << v * 3.0f << std::endl;
    std::cout << 2.0f * v << std::endl;
	
	// job1: 实现v和w的向量点积并输出结果
	{
        std::cout << "Example of dot product between v and w\n";
        std::cout << "1#using transpose(): ";
		std::cout << v.transpose()*w << std::endl;// 方式1
        std::cout << "2#using adjoint(): ";
        std::cout << v.adjoint()*w << std::endl;// 方式2
        std::cout << "3#using dot(): ";
	    std::cout << v.dot(w) << std::endl;// 方式3
	}

    // Example of matrix
    std::cout << "Example of matrix \n";
    // matrix definition
    Eigen::Matrix3f i,j;
    i << 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0;
    j << 2.0, 3.0, 1.0, 4.0, 6.0, 5.0, 9.0, 7.0, 8.0;
    // matrix output
    std::cout << "Example of output \n";
    std::cout << i << std::endl;
    // matrix add i + j
    // matrix scalar multiply i * 2.0
    // matrix multiply i * j
    // matrix multiply vector i * v
	
	// job2：实现i与j的矩阵相加、i与2.0的数乘、i与j的矩阵相乘、i与v的矩阵乘向量，并输出相应的结果
	{
		cout << "i+j=\n" << i+j << endl;// 矩阵相加
        cout << "i*2.0=\n" << i*2.0 << endl;// 数乘
        cout << "i*j=\n" << i*j << endl;// 矩阵相乘
        cout << "i*v=\n" << i*v << endl;// 矩阵乘向量
	}

    return 0;
}
