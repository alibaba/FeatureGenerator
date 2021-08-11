#pragma once

#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <string>

namespace autil {

template<typename T>
float US_TO_MS(T us) {
    return us / 1000.0;
}
template<typename T>
float US_TO_SEC(T us) {
    return us / 1000.0 / 1000.0;
}

class TimeUtility
{
public:
    static int64_t currentTime();
    static int64_t currentTimeInSeconds();
    static int64_t currentTimeInMicroSeconds();
    static int64_t currentTimeInNanoSeconds();
    static int64_t getTime(int64_t usecOffset = 0);
    static timeval getTimeval(int64_t usecOffset = 0);
    static timespec getTimespec(int64_t usecOffset = 0);
    static int64_t us2ms(int64_t us) {
        return us / 1000;
    }
    static int64_t ms2us(int64_t ms) {
        return ms * 1000;
    }
    static int64_t ms2sec(int64_t ms) {
        return ms / 1000;
    }
    static int64_t sec2ms(int64_t sec) {
        return sec * 1000;
    }
    static int64_t us2sec(int64_t us) {
        return us / 1000000;
    }
    static int64_t sec2us(int64_t sec) {
        return sec * 1000000;
    }
    static std::string currentTimeString();
    static std::string currentTimeString(const std::string &format);
};

class ScopedTime {
public:
    ScopedTime(int64_t &t) : _t(t) {
        _b = TimeUtility::currentTime();
    }
    ~ScopedTime() {
        _t += TimeUtility::currentTime() - _b;
    }
private:
    int64_t &_t;
    int64_t _b;
};

class ScopedTime2 {
public:
    ScopedTime2() {
        _beginTime = TimeUtility::currentTime();
    }
    ~ScopedTime2() {
    }
    double done_sec() const {
        int64_t endTime = TimeUtility::currentTime();
        return US_TO_SEC(endTime - _beginTime);
    }
    double done_ms() const {
        int64_t endTime = TimeUtility::currentTime();
        return US_TO_MS(endTime - _beginTime);
    }
    int64_t done_us() const {
        int64_t endTime = TimeUtility::currentTime();
        return endTime - _beginTime;
    }
    int64_t begin_us() const {
        return _beginTime;
    }
private:
    int64_t _beginTime;
};

}

#define AUTIL_TIMEUTILITY_RECORD_CODE_TIME(code) ({                       \
    const auto startTime = autil::TimeUtility::currentTime();             \
    code;                                                                 \
    const auto latency = autil::TimeUtility::currentTime() - startTime;   \
    latency;                                                              \
})

