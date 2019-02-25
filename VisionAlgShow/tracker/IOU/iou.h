#ifndef IOU_H
#define IOU_H

#include "algorithms/common.h"
#include <vector>
#include <time.h>




class IOUTracker
{
public:
    float iou_sigma_l = 0;  // low detection threshold
    float iou_sigma_h = 0.2;  // high detection threshold
    float iou_sigma_iou = 0.6;  // IOU threshold
    float iou_t_min = 2;  // minimum track length in frames
};



// return the iou between two boxes
inline float intersectionOverUnion(BoundingBox box1, BoundingBox box2);

// return the index of the bounding box with the highest iou
inline int highestIOU(BoundingBox box, std::vector<BoundingBox> boxes);

// start IOU Tracker
std::vector<Track> track_iou(std::vector<Track> &active_tracks, std::vector<Track> &finished_tracks);


#endif
