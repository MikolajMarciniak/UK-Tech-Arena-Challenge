#include "../include/common/Root.h"
#include "../include/CardinalityEstimation.h"
#include <iostream>
#include <algorithm>
#include <limits>
#include <unordered_map>
#include <functional> 

std::vector<std::vector<int>> tuples;
std::vector<int> tupleIds;
std::unordered_map<int, std::unordered_map<int, std::vector<int>>> columnIndex;
int nextTupleId = 0;

unsigned int hash1(int value) {
    return value * 2654435761u;
}

unsigned int hash2(int value) {
    return (value ^ (value >> 16)) * 0x85ebca6b;
    }

unsigned int hash3(int value) {
    unsigned int hash = 2166136261u;
    hash ^= value & 0xFF;
    hash *= 16777619;
    return hash;
}

unsigned int hash4(int value) {
    return (value * 31) ^ (value >> 16);
}

unsigned int hash5(int value) {
    return (value + 7) * 0x9e3779b9;
}

CEEngine::CEEngine(int num, DataExecuter* dataExecuter) {
    this->dataExecuter = dataExecuter;
    this->cmsWidth = 1000;
    this->cmsDepth = 5;
    cmsTable.resize(cmsDepth, std::vector<int>(cmsWidth, 0));
}

void CEEngine::insertTuple(const std::vector<int>& tuple) {
    tuples.push_back(tuple);
    tupleIds.push_back(nextTupleId);
    nextTupleId++;

    int value = tuple[0];
    cmsInsert(value);
}

void CEEngine::deleteTuple(const std::vector<int>& tuple, int tupleId) {
    auto idIt = std::find(tupleIds.begin(), tupleIds.end(), tupleId);
    if (idIt != tupleIds.end()) {
        int index = std::distance(tupleIds.begin(), idIt);
        tuples.erase(tuples.begin() + index);
        tupleIds.erase(idIt);
    }
}

void CEEngine::cmsInsert(int value) {
    unsigned int hashValues[5] = { hash1(value), hash2(value), hash3(value), hash4(value), hash5(value) };
    
    for (int i = 0; i < cmsDepth; ++i) {
        int hash = hashValues[i] % cmsWidth; 
        cmsTable[i][hash]++;
    }
}

int CEEngine::cmsQuery(int value) {
    int minCount = std::numeric_limits<int>::max();
    
    unsigned int hashValues[5] = { hash1(value), hash2(value), hash3(value), hash4(value), hash5(value) };

    for (int i = 0; i < cmsDepth; ++i) {
        int hash = hashValues[i] % cmsWidth; 
        minCount = std::min(minCount, cmsTable[i][hash]);
    }

    return minCount;
}

int CEEngine::query(const std::vector<CompareExpression>& quals) {
    int matchCount = 0;
    for (const auto& qual : quals) {
        int estimatedCount = cmsQuery(qual.value);
        matchCount += estimatedCount;
    }

    return matchCount;
}

void CEEngine::prepare() {
}
