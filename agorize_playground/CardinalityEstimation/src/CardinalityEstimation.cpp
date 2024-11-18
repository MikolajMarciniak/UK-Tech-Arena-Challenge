#include "../include/common/Root.h"
#include "../include/CardinalityEstimation.h"
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <vector>
#include <cmath>

std::vector<std::vector<int>> tuples;
std::unordered_map<int, int> tupleIdToIndex;  // Maps tuple ID to index in `tuples`
std::vector<int> freeIndices;  // To reuse freed indices in `tuples`
int nextTupleId = 0;

// Optimized hash function
inline unsigned int hash1(int value) {
    return std::hash<int>{}(value);  // Standard hash function
}

CEEngine::CEEngine(int num, DataExecuter* dataExecuter) {
    this->dataExecuter = dataExecuter;
    this->precision = 14;  // Using 2^14 registers
    int numRegisters = 1 << precision;
    this->registers.resize(numRegisters, 0);

    // Precompute alpha
    switch (precision) {
        case 4:  this->alpha = 0.673; break;
        case 5:  this->alpha = 0.697; break;
        case 6:  this->alpha = 0.709; break;
        default: this->alpha = 0.7213 / (1.0 + 1.079 / numRegisters);
    }
}

void CEEngine::insertTuple(const std::vector<int>& tuple) {
    int index;
    if (!freeIndices.empty()) {
        index = freeIndices.back();
        freeIndices.pop_back();
        tuples[index] = tuple;
    } else {
        index = tuples.size();
        tuples.push_back(tuple);
    }

    tupleIdToIndex[nextTupleId] = index;

    for (int value : tuple) {
        hllInsert(value);
    }
    nextTupleId++;
}

void CEEngine::deleteTuple(const std::vector<int>& tuple, int tupleId) {
    auto it = tupleIdToIndex.find(tupleId);
    if (it != tupleIdToIndex.end()) {
        int index = it->second;
        freeIndices.push_back(index);  // Reuse index for future insertions
        tupleIdToIndex.erase(it);
        prepare();  // Rebuild HLL after deletion
    }
}

void CEEngine::hllInsert(int value) {
    uint32_t hash = hash1(value);
    int index = hash >> (32 - precision);
    int zeros = __builtin_clz(hash << precision) + 1;  // Calculate leading zeros efficiently
    registers[index] = std::max(registers[index], static_cast<uint8_t>(zeros));
}

int CEEngine::hllQuery(int value) {
    double harmonicSum = 0;
    int numRegisters = 1 << precision;
    int zeroCount = 0;

    for (uint8_t rank : registers) {
        if (rank == 0) ++zeroCount;
        harmonicSum += std::pow(2.0, -rank);
    }

    double estimate = alpha * numRegisters * numRegisters / harmonicSum;

    // Small range correction
    if (estimate <= 2.5 * numRegisters && zeroCount > 0) {
        estimate = numRegisters * std::log(static_cast<double>(numRegisters) / zeroCount);
    }
    // Large range correction
    else if (estimate > (1.0 / 30.0) * std::pow(2, 32)) {
        estimate = -std::pow(2, 32) * std::log(1.0 - estimate / std::pow(2, 32));
    }

    return static_cast<int>(estimate);
}

int CEEngine::query(const std::vector<CompareExpression>& quals) {
    if (quals.empty()) {
        return hllQuery(0);  // Return total cardinality estimate
    }

    int maxEstimate = 0;
    for (const auto& qual : quals) {
        maxEstimate = std::max(maxEstimate, hllQuery(qual.value));
    }
    return maxEstimate;
}

void CEEngine::prepare() {
    // Reset HLL registers
    std::fill(registers.begin(), registers.end(), 0);

    // Rebuild HLL from remaining tuples
    for (const auto& tuple : tuples) {
        for (int value : tuple) {
            hllInsert(value);
        }
    }
}
