#ifndef MAX_MEMORY_TRACKER_H
#define MAX_MEMORY_TRACKER_H

#include "./common/ActionType.h"
#include <string> 


class MemoryUtils {
private:
    long long maxMemory = 0;
    long long totalMemoryInsert = 0;
    long long totalMemoryDelete = 0;
    long long totalMemoryQuery = 0;
    long long queryCount = 0;

public:
    void update(long long memoryUsage);
    void recordMemoryUsage(ActionType actionType, long long memoryUsage);
    void printMaxMemory(const std::string& label) const;
    void printMemoryUsage() const;
};

#endif
