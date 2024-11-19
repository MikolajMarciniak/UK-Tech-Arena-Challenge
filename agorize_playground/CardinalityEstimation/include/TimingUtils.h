#ifndef TIMING_UTILS_H
#define TIMING_UTILS_H

#include <functional>
#include <string>
#include <limits>

struct TimingData {
    long long minTime = std::numeric_limits<long long>::max();
    long long maxTime = std::numeric_limits<long long>::min();
    long long totalTime = 0;

    void update(long long duration);

    void print(const std::string& operationName) const;
};

// Struct to track memory usage
struct MaxMemoryTracker {
    long long maxMemory = 0;

    // Update the maximum memory usage
    void update(long long memoryUsage);

    // Print the maximum memory usage
    void printMaxMemory(const std::string& label) const;
};

// Function to measure execution time and memory usage
void measureExecutionTime(const std::function<void()>& operation, 
                          TimingData& timingData, 
                          MaxMemoryTracker& memoryTracker, 
                          const std::string& operationName);

// Function to get the current memory usage (in KB)
long long getMemoryUsage();

#endif // TIMING_UTILS_H