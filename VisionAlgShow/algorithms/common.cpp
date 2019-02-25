#include "common.h"

time_t getCurrentTimeStamp()
{
    time_t cTime = time(NULL);
    return cTime;
}

std::string getTimeStrByTimeStamp(time_t timeStamp)
{
    timeStamp += 28800;
    struct tm *pt;
    pt = gmtime(&timeStamp);
    char str[100];
    strftime(str, sizeof(str), "%Y-%m-%d %H:%M:%S", pt);
    std::string timeStr(str);
    return timeStr;
}
