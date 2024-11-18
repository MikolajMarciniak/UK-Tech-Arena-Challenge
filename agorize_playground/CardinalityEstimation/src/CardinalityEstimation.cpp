#include "../include/common/Root.h"
#include "../include/CardinalityEstimation.h"
#include <iostream>
#include <algorithm>
#include <limits>
#include <unordered_map>
#include <functional>
#include <cmath>

std::vector<std::vector<int>> tuples;
std::vector<int> tupleIds;
std::unordered_map<int, std::unordered_map<int, std::vector<int>>> columnIndex;
int nextTupleId = 0;

unsigned int hash1(int value) {
    return value * 2654435761u;
}

CEEngine::CEEngine(int num, DataExecuter* dataExecuter) {
    this->dataExecuter = dataExecuter;
    this->precision = 14;  // Using 2^14 registers
    this->registers.resize(1 << precision, 0);
    
    // Calculate alpha based on precision
    switch (precision) {
        case 4:  this->alpha = 0.673; break;
        case 5:  this->alpha = 0.697; break;
        case 6:  this->alpha = 0.709; break;
        default: this->alpha = 0.7213 / (1.0 + 1.079 / (1 << precision));
    }
}

void CEEngine::insertTuple(const std::vector<int>& tuple) {
    tuples.push_back(tuple);
    tupleIds.push_back(nextTupleId);
    nextTupleId++;
    
    for (int value : tuple) {
        hllInsert(value);
    }
}

void CEEngine::deleteTuple(const std::vector<int>& tuple, int tupleId) {
    auto idIt = std::find(tupleIds.begin(), tupleIds.end(), tupleId);
    if (idIt != tupleIds.end()) {
        int index = std::distance(tupleIds.begin(), idIt);
        tuples.erase(tuples.begin() + index);
        tupleIds.erase(idIt);
        prepare();  // Rebuild estimates after deletion
    }
}

void CEEngine::hllInsert(int value) {
    uint32_t hash = hash1(value);
    int index = hash >> (32 - precision);
    int zeros = __builtin_clz((hash << precision) | (1 << (precision - 1))) + 1;
    registers[index] = std::max(registers[index], static_cast<uint8_t>(zeros));
}

int CEEngine::hllQuery(int value) {
    double harmonicMean = 0;
    int numRegisters = 1 << precision;
    
    for (uint8_t rank : registers) {
        harmonicMean += std::pow(2.0, -rank);
    }
    
    double estimate = alpha * numRegisters * numRegisters / harmonicMean;
    
    // Small range correction
    if (estimate <= 2.5 * numRegisters) {
        int zeros = std::count(registers.begin(), registers.end(), 0);
        if (zeros > 0) {
            estimate = numRegisters * std::log(static_cast<double>(numRegisters) / zeros);
        }
    }
    // Large range correction
    else if (estimate > (1.0/30.0) * std::pow(2, 32)) {
        estimate = -std::pow(2, 32) * std::log(1.0 - estimate / std::pow(2, 32));
    }
    
    return static_cast<int>(estimate);
}

int CEEngine::query(const std::vector<CompareExpression>& quals) {
    if (quals.empty()) {
        return hllQuery(0);  // Return total cardinality estimate
    }
    
    // For queries with conditions, estimate based on the values in quals
    int maxEstimate = 0;
    for (const auto& qual : quals) {
        maxEstimate = std::max(maxEstimate, hllQuery(qual.value));
    }
    
    return maxEstimate;
}

void CEEngine::prepare() {
    // Reset HLL registers
    std::fill(registers.begin(), registers.end(), 0);
    
    // Rebuild estimates from existing tuples
    for (const auto& tuple : tuples) {
        for (int value : tuple) {
            hllInsert(value);
        }
    }
}