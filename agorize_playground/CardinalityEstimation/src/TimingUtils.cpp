#include "TimingUtils.h"
#include <iostream>
#include <limits>
#include <chrono>

// Update min and max times
void TimingData::update(long long duration) {
    if (duration < minTime) minTime = duration;
    if (duration > maxTime) maxTime = duration;
}

// Print timing results
void TimingData::print(const std::string& operationName) const {
    std::cout << "Minimum time for " << operationName << ": "
              << (minTime == std::numeric_limits<long long>::max() ? 0 : static_cast<double>(minTime) / 1e6) << " seconds" << std::endl;
    std::cout << "Maximum time for " << operationName << ": "
              << (maxTime == std::numeric_limits<long long>::min() ? 0 : static_cast<double>(maxTime) / 1e6) << " seconds" << std::endl;
}

// Measure execution time
void measureExecutionTime(const std::function<void()>& operation, TimingData& timingData, const std::string& operationName) {
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::microseconds;

    auto start = high_resolution_clock::now();
    operation(); // Execute the passed operation
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start).count();

    // Update the timing data
    timingData.update(duration);

    // Print the execution time; uncessary at this point, but left it in just in case
    //std::cout << "Time taken by " << operationName << ": " << static_cast<double>(duration) / 1e6 << " seconds" << std::endl;
}