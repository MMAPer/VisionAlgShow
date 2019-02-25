#include <algorithm>
#include <iostream>
#include "algorithms/common.h"
#include "iou.h"

inline float intersectionOverUnion(BoundingBox box1, BoundingBox box2)
{
    float minx1 = box1.x;
    float maxx1 = box1.x + box1.w;
    float miny1 = box1.y;
    float maxy1 = box1.y + box1.h;

    float minx2 = box2.x;
    float maxx2 = box2.x + box2.w;
    float miny2 = box2.y;
    float maxy2 = box2.y + box2.h;

    if(minx1>maxx2 || maxx1<minx2 || miny1>maxy2 || maxy1<miny2)
        return 0.0f;
    else
    {
        float dx = std::min(maxx1, maxx1) - std::max(minx2, minx1);
        float dy = std::min(maxy2, maxy1) - std::max(miny2, miny1);
        float area1 = (maxx1-minx1)*(maxy1-miny1);
        float area2 = (maxx2-minx2)*(maxy2-miny2);
        float inter = dx*dy;
        float uni = area1 + area2 - inter;
        float iou = inter / uni;
        return iou;
    }
}


