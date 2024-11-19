#include "../include/common/Root.h"
#include "../include/CardinalityEstimation.h"
#include <iostream>
#include <algorithm>
#include <limits>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <functional>
#include <cmath>

// Global variables for tuples and their IDs
std::vector<std::vector<int>> tuples;
std::vector<int> tupleIds;
std::unordered_map<int, int> tupleIdToIndex;
std::vector<int> freeIndices;
int nextTupleId = 0;

// Simple hash function
unsigned int hash1(int value) {
    return value * 2654435761u;
}

// Constructor
CEEngine::CEEngine(int num, DataExecuter* dataExecuter) {
    this->dataExecuter = dataExecuter;
    this->precision = 14;  // Using 2^14 registers
    this->registers.resize(1 << precision, 0);
    this->dirty = false; // Registers start clean

    // Calculate alpha based on precision
    switch (precision) {
        case 4:  this->alpha = 0.673; break;
        case 5:  this->alpha = 0.697; break;
        case 6:  this->alpha = 0.709; break;
        default: this->alpha = 0.7213 / (1.0 + 1.079 / (1 << precision));
    }
}

// Insert a tuple
void CEEngine::insertTuple(const std::vector<int>& tuple) {
    // Reuse deleted indices if available
    if (!freeIndices.empty()) {
        int index = freeIndices.back();
        freeIndices.pop_back();
        tuples[index] = tuple;
        tupleIdToIndex[nextTupleId] = index;
    } else {
        tuples.push_back(tuple);
        tupleIdToIndex[nextTupleId] = tuples.size() - 1;
    }

    // Update the HLL registers directly for all values in the tuple
    for (int value : tuple) {
        hllInsert(value);
    }

    nextTupleId++;
}

// Delete a tuple
void CEEngine::deleteTuple(const std::vector<int>& tuple, int tupleId) {
    auto it = tupleIdToIndex.find(tupleId);
    if (it == tupleIdToIndex.end()) {
        return; // Tuple ID not found
    }

    int index = it->second;
    tupleIdToIndex.erase(it);
    freeIndices.push_back(index);

    // Mark registers as dirty for a future rebuild
    dirty = true;

    // Optionally rebuild if too many deletions have occurred
    if (tupleIdToIndex.size() < (tuples.size() / 2)) {
        prepare();
        dirty = false;
    }
}

// HyperLogLog Insert
void CEEngine::hllInsert(int value) {
    uint32_t hash = hash1(value);
    int index = hash >> (32 - precision);
    int zeros = __builtin_clz(hash) - (32 - precision) + 1;
    registers[index] = std::max(registers[index], static_cast<uint8_t>(zeros));
}

// HyperLogLog Query
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
    else if (estimate > (1.0 / 30.0) * std::pow(2, 32)) {
        estimate = -std::pow(2, 32) * std::log(1.0 - estimate / std::pow(2, 32));
    }

    return static_cast<int>(estimate);
}

// Cardinality Query with Qualifiers
int CEEngine::query(const std::vector<CompareExpression>& quals) {
    if (dirty) {
        prepare(); // Ensure registers are up-to-date
        dirty = false;
    }

    if (quals.empty()) {
        return hllQuery(0); // Return total cardinality estimate
    }

    int maxEstimate = 0;
    for (const auto& qual : quals) {
        maxEstimate = std::max(maxEstimate, hllQuery(qual.value));
    }
    return maxEstimate;
}

void CEEngine::prepare() {
    if (!dirty) {
        return;  // No need to prepare if not dirty
    }

    // Reset HLL registers
    std::fill(registers.begin(), registers.end(), 0);

    // Create a set of unique values for the current tuples
    std::unordered_set<int> uniqueValues;

    // Iterate over existing tuples and collect unique values
    for (const auto& tuple : tuples) {
        for (int value : tuple) {
            uniqueValues.insert(value);
        }
    }

    // Rebuild registers from the unique values
    for (int value : uniqueValues) {
        hllInsert(value);
    }

    dirty = false; // Mark as clean after rebuilding
}


