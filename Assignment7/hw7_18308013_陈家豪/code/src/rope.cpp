#include <iostream>
#include <vector>

#include "CGL/vector2D.h"

#include "mass.h"
#include "rope.h"
#include "spring.h"

namespace CGL {

    Rope::Rope(Vector2D start, Vector2D end, int num_nodes, float node_mass, float k, vector<int> pinned_nodes)
    {
        // TODO (Part 1): Create a rope starting at `start`, ending at `end`, and containing `num_nodes` nodes.
        Mass *lMP=nullptr;  // 上一个质点
        Mass *cMP=nullptr;  // 当前质点
        for (int i=0;i<num_nodes;i++)
        {
            Vector2D CurrentPosition;// 当前质点位置
            if (num_nodes==1)
                CurrentPosition=start;
            else
                CurrentPosition=start+i*(end-start)/(num_nodes-1);
            cMP=new Mass(CurrentPosition,node_mass,false);// 创建质点
            masses.push_back(cMP);
            if (i>0)
            {
                Spring *cSP=new Spring(lMP,cMP,k);// 创建质点间的绳子
                springs.push_back(cSP);
            }
            lMP=cMP;
        }
        for (auto &i : pinned_nodes)
            masses[i]->pinned = true;// 固定相应质点
    }

    void Rope::simulateEuler(float delta_t, Vector2D gravity)
    {
        // 计算每个质点受到的弹簧力
        for (auto &s : springs)
        {
            // TODO (Part 2): Use Hooke's law to calculate the force on a node
            Vector2D a2b=s->m2->position - s->m1->position;
            Vector2D f=s->k * (a2b/a2b.norm()) * (a2b.norm()-s->rest_length);// 弹簧力
            s->m1->forces+=f;
            s->m2->forces-=f;
        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                // TODO (Part 2): Add gravity and global damping, then compute the new velocity and position
                m->forces+=gravity * m->mass;// 重力
                float k_d=0.1;
                m->forces+=-k_d*m->velocity; // 阻力
                Vector2D a=m->forces/m->mass;// 加速度
                if (false)// 显式欧拉法
                {
                    m->position+=m->velocity*delta_t;
                    m->velocity+=a*delta_t;
                }
                else    // 半隐式欧拉法
                {
                    m->velocity+=a*delta_t;
                    m->position+=m->velocity*delta_t;
                }
            }
            // Reset all forces on each mass
            m->forces = Vector2D(0, 0);
        }
    }

    void Rope::simulateVerlet(float delta_t, Vector2D gravity)
    {
        if (true)// 显式Velnet法：固定绳子长度
        {
            for (auto &s : springs)
            {
                // TODO (Part 3): Simulate one timestep of the rope using explicit Verlet （solving constraints)
                // 在此进行质点的位置调整，维持弹簧的原始长度
                Vector2D a2b=s->m2->position - s->m1->position;
                float L=a2b.norm() - s->rest_length;
                if (s->m1->pinned)
                {
                    if (s->m2->pinned)// 两端质点均固定，不调整
                        continue;
                    else              // 只有一端质点固定，移动L
                        s->m2->position-=L*a2b.unit();
                }
                else
                {
                    if (s->m2->pinned)// 只有一端质点固定，移动L
                        s->m1->position+=L*a2b.unit();
                    else              // 两端质点均不固定，各移动L/2
                    {
                        s->m1->position+=L/2*a2b.unit();
                        s->m2->position-=L/2*a2b.unit();
                    }
                }
            }
            for (auto &m : masses)
            {
                if (!m->pinned)
                {
                    Vector2D a=gravity;// 重力加速度
                    Vector2D temp_position = m->position;
                    // TODO (Part 3): Set the new position of the rope mass
                    // 在此计算重力影响下，质点的位置变化
                    float damping_factor=0;// 阻尼系数
                    m->position=m->position+(1-damping_factor)*(m->position-m->last_position)+a*delta_t*delta_t;
                    m->last_position=temp_position;
                }
            }
        }
        else
        {
            for (auto &s : springs)
            {
                // TODO (Part 3): Simulate one timestep of the rope using explicit Verlet （solving constraints)
                // 在此进行质点的位置调整，维持弹簧的原始长度
                Vector2D a2b=s->m2->position - s->m1->position;
                Vector2D f=s->k * (a2b/a2b.norm()) * (a2b.norm()-s->rest_length);// 弹簧力
                s->m1->forces+=f;
                s->m2->forces-=f;
            }
            for (auto &m : masses)
            {
                if (!m->pinned)
                {
                    m->forces+=gravity * m->mass;// 重力
                    float k_d=0.1;// 阻尼系数
                    m->forces+=-k_d*m->velocity; // 阻力
                    Vector2D a=m->forces/m->mass;// 加速度

                    Vector2D temp_position = m->position;
                    // TODO (Part 3): Set the new position of the rope mass
                    m->position=m->position+(m->position-m->last_position)+a*delta_t*delta_t;
                    m->last_position=temp_position;
                }
                m->forces=Vector2D(0,0);
            }
        }
    }
}
