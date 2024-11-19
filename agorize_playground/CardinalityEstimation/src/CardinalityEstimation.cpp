#include "../include/common/Root.h"
#include "../include/CardinalityEstimation.h"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <numeric>
#include <cmath>

// Global variables for tuples and their IDs
std::vector<std::vector<int>> tuples;
std::unordered_map<int, int> tupleIdToIndex;
std::vector<int> freeIndices;
int nextTupleId = 0;

CEEngine::CEEngine(int numBins, DataExecuter* dataExecuter)
    : dataExecuter(dataExecuter), numBins(numBins), minValue(0), maxValue(10000), dirty(false) {
    histogram.resize(numBins, 0);
    binWidth = static_cast<double>(maxValue - minValue) / numBins;
}

// Helper: Get bin index for a value
int CEEngine::getBinIndex(int value) const {
    return static_cast<int>((value - minValue) / binWidth);
}

// Helper: Update histogram for a value
void CEEngine::updateHistogram(int value, int delta) {
    int binIndex = getBinIndex(value);
    if (binIndex >= 0 && binIndex < numBins) {
        histogram[binIndex] += delta;
        if (histogram[binIndex] < 0) histogram[binIndex] = 0; // Prevent negative counts
    }
}

// Insert a tuple
void CEEngine::insertTuple(const std::vector<int>& tuple) {
    for (int value : tuple) {
        updateHistogram(value, 1);  // Increment histogram count
    }

    if (!freeIndices.empty()) {
        int index = freeIndices.back();
        freeIndices.pop_back();
        tuples[index] = tuple;
        tupleIdToIndex[nextTupleId] = index;
    } else {
        tuples.push_back(tuple);
        tupleIdToIndex[nextTupleId] = tuples.size() - 1;
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

    for (int value : tuple) {
        updateHistogram(value, -1);  // Decrement histogram count
    }

    if (!dirty && tupleIdToIndex.size() < (tuples.size() / 2)) {
        dirty = true;
    }
}

// Cardinality Query
int CEEngine::query(const std::vector<CompareExpression>& quals) {
    if (dirty) {
        prepare();
        dirty = false;
    }

    if (quals.empty()) {
        return std::accumulate(histogram.begin(), histogram.end(), 0);
    }

    int totalCount = 0;
    for (const auto& qual : quals) {
        for (int bin = 0; bin < numBins; ++bin) {
            double binStart = minValue + bin * binWidth;
            double binEnd = binStart + binWidth;

            bool matches = false;
            if (qual.compareOp == GREATER) {
                matches = binStart > qual.value;
            } else if (qual.compareOp == EQUAL) {
                matches = binStart <= qual.value && binEnd > qual.value;
            }

            if (matches) {
                totalCount += histogram[bin];
            }
        }
    }

    return totalCount;
}

// Prepare for Rebuild
void CEEngine::prepare() {
    if (!dirty) return;

    std::fill(histogram.begin(), histogram.end(), 0);

    for (const auto& tuple : tuples) {
        for (int value : tuple) {
            updateHistogram(value, 1);
        }
    }

    dirty = false;
}
