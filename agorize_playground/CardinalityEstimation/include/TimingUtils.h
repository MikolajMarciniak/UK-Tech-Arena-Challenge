#ifndef TIMING_UTILS_H
#define TIMING_UTILS_H

#include <functional>
#include <string>
#include <limits>

struct TimingData {
    long long minTime = std::numeric_limits<long long>::max();
    long long maxTime = std::numeric_limits<long long>::min();

    void update(long long duration);

    void print(const std::string& operationName) const;
};

// Timing utility function to measure execution time
void measureExecutionTime(const std::function<void()>& operation, TimingData& timingData, const std::string& operationName);

#endif // TIMING_UTILS_H