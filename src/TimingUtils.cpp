#include "TimingUtils.h"
#include <chrono>
#include <iostream>
#include <sys/resource.h>
#include <unistd.h>

long long getMemoryUsage() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_maxrss;
}

void TimingData::update(long long duration) {
    if (duration < minTime) minTime = duration;
    if (duration > maxTime) maxTime = duration;
    totalTime += duration;
    count++;
}



void TimingData::print(const std::string& label) const {
    long long avgTime = totalTime / count;
    std::cout << label << ": " << std::endl;
    std::cout << "  Min Time: " << minTime / 1000.0 << " ms" << std::endl;
    std::cout << "  Max Time: " << maxTime / 1000.0 << " ms" << std::endl;
    std::cout << "  Avg Time: " << avgTime / 1000.0 << " ms" << std::endl;
    std::cout << "  Total Time: " << totalTime / 1000.0 << " ms" << std::endl;
}



void measureExecutionTime(const std::function<void()>& operation,
                          TimingData& timingData,
                          MemoryUtils& memoryTracker,
                          const std::string& label) {
    using namespace std::chrono;

    auto start = high_resolution_clock::now();

    operation(); 

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    long long durationInMicroseconds = duration.count();

    timingData.update(durationInMicroseconds);
    }
