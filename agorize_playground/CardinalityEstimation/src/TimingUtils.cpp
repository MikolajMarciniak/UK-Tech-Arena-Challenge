#include "TimingUtils.h"
#include <iostream>
#include <limits>
#include <chrono>
#include <sys/resource.h>

// Function to get current memory usage
long long getMemoryUsage() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage); // Get resource usage for the current process

    // Memory usage in kilobytes (RSS)
    return usage.ru_maxrss;
}

// Update min and max times
void TimingData::update(long long duration) {
    if (duration < minTime) minTime = duration;
    if (duration > maxTime) maxTime = duration;
    totalTime += duration;
}

// Update the memory usage tracker
void MaxMemoryTracker::update(long long memoryUsage) {
    if (memoryUsage > maxMemory) {
        maxMemory = memoryUsage;
    }
}

// Print timing results
void TimingData::print(const std::string& operationName) const {
    std::cout << operationName << " - Min: " << static_cast<double>(minTime) / 1e6 << " seconds, "
              << "Max: " << static_cast<double>(maxTime) / 1e6 << " seconds, "
              << "Total: " << static_cast<double>(totalTime) / 1e6 << " seconds" << std::endl;
}

// Print the maximum memory usage
void MaxMemoryTracker::printMaxMemory(const std::string& label) const {
    double maxMemoryInMB = maxMemory / 1024.0; // Convert KB to MB
    std::cout << label << " - " << maxMemoryInMB << " MB" << std::endl;
}

// Measure the execution time and track memory usage
void measureExecutionTime(const std::function<void()>& operation, 
                          TimingData& timingData, 
                          MaxMemoryTracker& memoryTracker, 
                          const std::string& operationName) {
    using namespace std::chrono;

    // Record the start time
    auto start = high_resolution_clock::now();

    // Execute the operation
    operation();

    // Record the stop time
    auto stop = high_resolution_clock::now();

    // Calculate the duration in microseconds
    auto duration = duration_cast<microseconds>(stop - start);
    long long durationInMicroseconds = duration.count();

    // Update timing data
    timingData.update(durationInMicroseconds);

    // Track memory usage during the function execution
    long long memoryUsage = getMemoryUsage();
    memoryTracker.update(memoryUsage); // Update max memory usage

    // Print time and memory usage for the operation
    //std::cout << operationName << " - Time taken: " << durationInMicroseconds / 1000000.0 << " seconds" << std::endl;
    //std::cout << operationName << " - Current memory usage: " << memoryUsage << " KB" << std::endl;
}