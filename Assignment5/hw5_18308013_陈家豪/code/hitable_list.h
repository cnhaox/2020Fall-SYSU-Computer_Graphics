#ifndef HITABLELIST_H
#define HITABLELIST_H

#include "hitable.h"

class hitable_list: public hitable  {
    public:
        hitable_list() {}
        hitable_list(hitable **l, int n) { list = l; list_size = n; }
        virtual bool hit(const ray& r, float tmin, float tmax, hit_record& rec) const;
        hitable **list;
        int list_size;  // ��������
};
bool hitable_list::hit(const ray& r, float t_min, float t_max, hit_record& rec) const {
    hit_record temp_rec;    // �ù��ߵĻ��м�¼
    bool hit_anything = false;
    double closest_so_far = t_max;
    for (int i = 0; i < list_size; i++) 
    {   // Ѱ�Ҹù��߻��е�������������¼
        if (list[i]->hit(r, t_min, closest_so_far, temp_rec))// �ڹ涨��Χ�ڻ���list[i]
        {
            hit_anything = true;
            closest_so_far = temp_rec.t;// ���¾���
            rec = temp_rec;             // ���»��м�¼
        }
    }
    return hit_anything;
}


#endif