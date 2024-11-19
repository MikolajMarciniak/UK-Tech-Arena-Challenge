#include "../include/common/Root.h"
#include "../include/CardinalityEstimation.h"
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cmath>

// Global variables for tuples and their IDs
std::vector<std::vector<int>> tuples;
std::vector<int> tupleIds;
std::unordered_map<int, int> tupleIdToIndex;
std::vector<int> freeIndices;
int nextTupleId = 0;

// Faster hash function (optimized multiplicative hash)
inline uint32_t fastHash(int value) {
    return value * 0x9E3779B1;
}

// Constructor
CEEngine::CEEngine(int num, DataExecuter* dataExecuter) {
    this->dataExecuter = dataExecuter;
    this->precision = 14;  // Using 2^14 registers
    this->registers.resize(1 << precision, 0);
    this->dirty = false;

    // Precompute alpha for precision
    this->alpha = (precision <= 6) 
        ? (precision == 4 ? 0.673 : (precision == 5 ? 0.697 : 0.709)) 
        : 0.7213 / (1.0 + 1.079 / (1 << precision));
}

// Insert a tuple
void CEEngine::insertTuple(const std::vector<int>& tuple) {
    if (!freeIndices.empty()) {
        int index = freeIndices.back();
        freeIndices.pop_back();
        tuples[index] = tuple;
        tupleIdToIndex[nextTupleId] = index;
    } else {
        tuples.push_back(tuple);
        tupleIdToIndex[nextTupleId] = tuples.size() - 1;
    }

    // Batch update HLL registers for the tuple
    for (int value : tuple) {
        hllInsert(value);
    }

    nextTupleId++;
}

// Delete a tuple
void CEEngine::deleteTuple(const std::vector<int>& tuple, int tupleId) {
    auto it = tupleIdToIndex.find(tupleId);
    if (it == tupleIdToIndex.end()) return;

    int index = it->second;
    tupleIdToIndex.erase(it);
    freeIndices.push_back(index);

    // Minimal rebuild logic
    if (!dirty && tupleIdToIndex.size() < (tuples.size() / 2)) {
        dirty = true;
    }
}

// HyperLogLog Insert
inline void CEEngine::hllInsert(int value) {
    uint32_t hash = fastHash(value);
    int index = hash >> (32 - precision);
    int zeros = __builtin_clz(hash) - (32 - precision) + 1;
    registers[index] = std::max(registers[index], static_cast<uint8_t>(zeros));
}

// HyperLogLog Query
int CEEngine::hllQuery(int value) {
    static std::vector<double> precomputedPowers = [] {
        std::vector<double> powers(256, 0.0);
        for (int i = 0; i < 256; ++i) {
            powers[i] = std::pow(2.0, -i);
        }
        return powers;
    }();

    double harmonicMean = 0;
    for (uint8_t rank : registers) {
        harmonicMean += precomputedPowers[rank];
    }

    double estimate = alpha * (1 << precision) * (1 << precision) / harmonicMean;

    // Small range correction
    if (estimate <= 2.5 * (1 << precision)) {
        int zeros = std::count(registers.begin(), registers.end(), 0);
        if (zeros > 0) {
            estimate = (1 << precision) * std::log(static_cast<double>(1 << precision) / zeros);
        }
    }
    // Large range correction
    else if (estimate > (1.0 / 30.0) * std::pow(2, 32)) {
        estimate = -std::pow(2, 32) * std::log(1.0 - estimate / std::pow(2, 32));
    }

    return static_cast<int>(estimate);
}

// Cardinality Query with Qualifiers
int CEEngine::query(const std::vector<CompareExpression>& quals) {
    if (dirty) {
        prepare();
        dirty = false;
    }

    if (quals.empty()) return hllQuery(0);

    int maxEstimate = 0;
    for (const auto& qual : quals) {
        maxEstimate = std::max(maxEstimate, hllQuery(qual.value));
    }
    return maxEstimate;
}

// Prepare for Rebuild
void CEEngine::prepare() {
    if (!dirty) return;

    std::fill(registers.begin(), registers.end(), 0);

    std::unordered_set<int> uniqueValues;
    for (const auto& tuple : tuples) {
        for (int value : tuple) {
            uniqueValues.insert(value);
        }
    }

    for (int value : uniqueValues) {
        hllInsert(value);
    }

    dirty = false;
}
