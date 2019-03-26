#ifndef IOU_H
#define IOU_H

#include "algorithms/common.h"
#include <vector>
#include <time.h>




class IOUTracker
{
public:
    IOUTracker();
    ~IOUTracker();
    float iou_sigma_l = 0;  // low detection threshold
    float iou_sigma_h = 0.2;  // high detection threshold
    float iou_sigma_iou = 0.6;  // IOU threshold
    float iou_t_min = 2;  // minimum track length in frames
    int max_id = 0;

    std::vector<Track> active_tracks;
    std::vector<Track> finished_tracks;

    // start IOU Tracker
    std::vector<BoundingBox> track_iou(std::vector<BoundingBox> &detections);

private:
    // return the iou between two boxes
    float intersectionOverUnion(BoundingBox &box1, BoundingBox &box2);

    // return the index of the bounding box with the highest iou
    int highestIOU(BoundingBox &box, std::vector<BoundingBox> &boxes);


};






#endif
