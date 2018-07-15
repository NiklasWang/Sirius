#ifndef _TIME_EX_H_
#define _TIME_EX_H_

inline int64_t currentUs()
{
    int64_t t = 0;

    struct timeval tv;
    gettimeofday(&tv, NULL);
    t = (int64_t)tv.tv_usec + tv.tv_sec * 1000000ll;

    return t;
}

#endif
