#include <algorithm>
#include <iostream>
#include "algorithms/common.h"
#include "iou.h"


IOUTracker::IOUTracker()
{

}


IOUTracker::~IOUTracker()
{

}


// return the iou between two boxes
float IOUTracker::intersectionOverUnion(BoundingBox &box1, BoundingBox &box2)
{
    float minx1 = box1.x;
    float maxx1 = box1.x + box1.w;
    float miny1 = box1.y;
    float maxy1 = box1.y+ box1.h;

    float minx2 = box2.x;
    float maxx2 = box2.x + box2.w;
    float miny2 = box2.y;
    float maxy2 = box2.y + box2.h;

    if (minx1 > maxx2 || maxx1 < minx2 || miny1 > maxy2 || maxy1 < miny2)
        return 0.0f;
    else
    {
        float dx = std::min(maxx2, maxx1) - std::max(minx2, minx1);
        float dy = std::min(maxy2, maxy1) - std::max(miny2, miny1);
        float area1 = (maxx1 - minx1)*(maxy1 - miny1);
        float area2 = (maxx2 - minx2)*(maxy2 - miny2);
        float inter = dx*dy; // Intersection
        float uni = area1 + area2 - inter; // Union
        float IoU = inter / uni;
        return IoU;
    }
//	return 0.0f;
}

// return the index of the bounding box with the highest iou
int IOUTracker::highestIOU(BoundingBox &box, std::vector<BoundingBox> &boxes)
{
    float iou = 0, highest = 0;
    int index = -1;
    for (int i = 0; i < boxes.size(); i++)
    {
        iou = intersectionOverUnion(box, boxes[i]);
        if ( iou >= highest)
        {
            highest = iou;
            index = i;
        }
    }
    return index;
}


// start IOU Tracker
int IOUTracker::track_iou(std::vector<BoundingBox> &detections)
{
    std::vector<BoundingBox> tmp_detections = detections;
    if(active_tracks.size() == 0)
    {
        for(int i=0; i<detections.size(); i++)
        {
            detections[i].id = max_id+1;
            Track track;
            track.boxes.push_back(detections[i]);
            track.id = max_id+1;
            track.max_score = detections[i].confidence;
            track.start_frame = 0;
            active_tracks.push_back(track);
            max_id++;
        }
        return 0;
    }

    for(int i=0; i<active_tracks.size(); i++)
    {
        Track track = active_tracks[i];
        bool updated = false;
        //Get the index of the detection with the highest IOU
        int index = highestIOU(track.boxes.back(), detections);
        if(index!=-1 && intersectionOverUnion(track.boxes.back(), detections[index])>=iou_sigma_iou)
        {
            detections[index].id = track.id;
            track.boxes.push_back(detections[index]);
            if(track.max_score < detections[index].confidence)
                track.max_score = detections[index].confidence;
            if(track.boxes.size()>10)
                track.boxes.erase(track.boxes.begin());
            tmp_detections.erase(tmp_detections.begin()+index);
            active_tracks[i] = track;
            updated = true;
        }

        //if the track was not updated
        if(!updated)
        {
//            if(track.max_score>=iou_sigma_h && track.boxes.size()>=iou_t_min)
//            {
//                finished_tracks.push_back(track);
//            }
            active_tracks.erase(active_tracks.begin()+i);
            // Workaround used because of the previous line "erase" call
            i--;
        }
    }  //end for active tracks
    for(auto box:tmp_detections)
    {
        std::vector<BoundingBox> b;
        b.push_back(box);
        // Track_id is set to 0 because we don't know if the track will "surviv" or not
        Track t = {b, box.confidence, 0, max_id+1};
        for(int i=0;i<detections.size();i++)
        {
            if(box.x = detections[i].x)
            {
                detections[i].id = max_id+1;
            }
        }
        max_id++;
        active_tracks.push_back(t);
    }
    return 0;
}


