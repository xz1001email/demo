#include "timeStamp.h"
#include <sys/time.h>
#include <string.h>

timeStamp::timeStamp()
{
}
timeStamp::~timeStamp()
{

}

string timeStamp::getTimeStamp()
{
    struct tm* timeinfo;
    struct timeval tv;
    char msglog[128];
    char buffer[64];

    memset(buffer, 0, sizeof buffer);
    gettimeofday(&tv, NULL);
    timeinfo = localtime(&tv.tv_sec);
    strftime(buffer, sizeof buffer, "%Y-%m-%d %H:%M:%S.", timeinfo);
    snprintf(msglog, sizeof(msglog), "%s%ld", buffer, tv.tv_usec/1000);

    return msglog;
}
