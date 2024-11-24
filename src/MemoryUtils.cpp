#include "MemoryUtils.h"
#include <iostream>

void MemoryUtils::update(long long memoryUsage) {
    if (memoryUsage > maxMemory) {
        maxMemory = memoryUsage;
    }
}

void MemoryUtils::recordMemoryUsage(ActionType actionType, long long memoryUsage) {
    if (actionType == INSERT) {
        totalMemoryInsert += memoryUsage;
    } else if (actionType == DELETE) {
        totalMemoryDelete += memoryUsage;
    } else if (actionType == QUERY) {
        totalMemoryQuery += memoryUsage;
        queryCount++;
    }
}

void MemoryUtils::printMaxMemory(const std::string& label) const {
    std::cout << label << ": " << maxMemory << " KB" << std::endl;
}

void MemoryUtils::printMemoryUsage() const {
    std::cout << "\nTotal Memory Usage:\n";
    std::cout << "  Insert: " << totalMemoryInsert << " KB\n";
    std::cout << "  Delete: " << totalMemoryDelete << " KB\n";
    std::cout << "  Query: " << totalMemoryQuery << " KB\n";

    double avgMemoryPerQuery = queryCount > 0 ? static_cast<double>(totalMemoryQuery) / queryCount : 0;
    std::cout << "Average Memory Usage Per Query: " << avgMemoryPerQuery << " KB\n";

}
