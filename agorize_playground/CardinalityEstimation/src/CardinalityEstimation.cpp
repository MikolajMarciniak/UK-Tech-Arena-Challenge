#include "../include/common/Root.h"
#include "../include/CardinalityEstimation.h"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <numeric>
#include <cmath>

std::vector<std::vector<int>> tuples;
std::unordered_map<int, int> tupleIdToIndex;
std::vector<int> freeIndices;
int nextTupleId = 0;

CEEngine::CEEngine(int numBins, DataExecuter* dataExecuter)
    : dataExecuter(dataExecuter), numBins(numBins), minValue(0), maxValue(10000), dirty(false) {
    histogram.resize(numBins, 0);
    binWidth = static_cast<double>(maxValue - minValue) / numBins;
}

int CEEngine::getBinIndex(int value) const {
    return static_cast<int>((value - minValue) / binWidth);
}

void CEEngine::updateHistogram(int value, int delta) {
    int binIndex = getBinIndex(value);
    if (binIndex >= 0 && binIndex < numBins) {
        histogram[binIndex] += delta;
        if (histogram[binIndex] < 0) histogram[binIndex] = 0;
    }
}

void CEEngine::insertTuple(const std::vector<int>& tuple) {
    for (int value : tuple) {
        updateHistogram(value, 1); 
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

void CEEngine::deleteTuple(const std::vector<int>& tuple, int tupleId) {
    auto it = tupleIdToIndex.find(tupleId);
    if (it == tupleIdToIndex.end()) return;

    int index = it->second;
    tupleIdToIndex.erase(it);
    freeIndices.push_back(index);

    for (int value : tuple) {
        updateHistogram(value, -1);
    }

    if (!dirty && tupleIdToIndex.size() < (tuples.size() / 2)) {
        dirty = true;
    }
}
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
