#ifndef TIMING_UTILS_H
#define TIMING_UTILS_H

#include <limits>
#include <string>
#include <functional>
#include "./common/ActionType.h"
#include "./MemoryUtils.h"

long long getMemoryUsage();

class TimingData {
public:
    long long minTime = std::numeric_limits<long long>::max();
    long long maxTime = std::numeric_limits<long long>::min();
    long long totalTime = 0;
    long long count = 0;

    void update(long long duration);

    void print(const std::string& operationName) const;
};

void measureExecutionTime(const std::function<void()>& operation,
                          TimingData& timingData,
                          MemoryUtils& memoryTracker, 
                          const std::string& label);

#endif
