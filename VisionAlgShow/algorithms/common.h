#ifndef COMMON_H
#define COMMON_H

#include <time.h>
#include <vector>
#include <string>


struct BoundingBox
{
    float x;  // x-component of top left coordinate
    float y;  // y-component of top left coordinate
    float w;  // width of the box
    float h;  // height of the box
    float confidence;  // score of the box
    int frame;  // index of off-line video
    time_t time;  // time stamp of online video
    int id;
};

struct Track
{
    std::vector<BoundingBox> boxes;
    float max_score;
    int start_frame;
    int id;
};

time_t getCurrentTimeStamp();
std::string getTimeStrByTimeStamp(time_t timeStamp);



#endif // COMMON_H
