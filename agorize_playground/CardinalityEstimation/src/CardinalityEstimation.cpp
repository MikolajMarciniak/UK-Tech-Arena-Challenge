#include "CardinalityEstimation.h"
#include <algorithm>
#include <cmath>
#include <random>
#include <climits> 
uint32_t fastHash(int value) {
    uint32_t hash = 2166136261u;  // FNV offset basis
    for (size_t i = 0; i < sizeof(value); ++i) {
        hash ^= (value >> (i * 8)) & 0xff;  // Extract each byte
        hash *= 16777619u;  // FNV prime
    }
    return hash;
}
void CEEngine::hllInsert(int value) {
    uint32_t hash = fastHash(value);
    int index = hash >> (32 - precision);
    int zeros = __builtin_clz(hash) - (32 - precision) + 1;
    registers[index] = std::max(registers[index], static_cast<uint8_t>(zeros));
}

CEEngine::CEEngine(int num, DataExecuter* dataExecuter) {
    this->dataExecuter = dataExecuter;
    this->precision = 13;  // (2^13 registers)
    this->registers.resize(1 << precision, 0);
    this->dirty = false;
    
    // Initialize CMS
    cms_width = 2048;  // Width of each row
    cms_depth = 4;     // Number of hash functions
    cms_matrix.resize(cms_depth, std::vector<int32_t>(cms_width, 0));
    
    // Initialize hash seeds for CMS
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dis;
    cms_seeds.resize(cms_depth);
    for (size_t i = 0; i < cms_depth; ++i) {
        cms_seeds[i] = dis(gen);
    }

    // Precompute alpha for HLL precision
    this->alpha = (precision <= 6) 
        ? (precision == 4 ? 0.673 : (precision == 5 ? 0.697 : 0.709)) 
        : 0.7213 / (1.0 + 1.079 / (1 << precision));
}

void CEEngine::insertTuple(const std::vector<int>& tuple) {
    // Update free indices and mapping
    if (!freeIndices.empty()) {
        int index = freeIndices.back();
        freeIndices.pop_back();
        tupleIdToIndex[nextTupleId] = index;
    } else {
        tupleIdToIndex[nextTupleId] = nextTupleId;
    }
    
    // Update both HLL and CMS for each value
    for (int value : tuple) {
        // Update HLL
        hllInsert(value);
        
        // Update CMS
        for (size_t i = 0; i < cms_depth; ++i) {
            uint32_t hash = fastHash(value ^ cms_seeds[i]);
            uint32_t index = hash % cms_width;
            cms_matrix[i][index]++;
        }
    }
    
    nextTupleId++;
}

void CEEngine::deleteTuple(const std::vector<int>& tuple, int tupleId) {
    auto it = tupleIdToIndex.find(tupleId);
    if (it != tupleIdToIndex.end()) {
        freeIndices.push_back(it->second);
        tupleIdToIndex.erase(it);
        
        // Update CMS counts
        for (int value : tuple) {
            for (size_t i = 0; i < cms_depth; ++i) {
                uint32_t hash = fastHash(value ^ cms_seeds[i]);
                uint32_t index = hash % cms_width;
                cms_matrix[i][index] = std::max(0, cms_matrix[i][index] - 1);
            }
        }

        if (!dirty && freeIndices.size() > tupleIdToIndex.size() / 4) {
            dirty = true;
        }
    }
}

int CEEngine::estimateFrequency(int value) {
    int minCount = INT_MAX;
    for (size_t i = 0; i < cms_depth; ++i) {
        uint32_t hash = fastHash(value ^ cms_seeds[i]);
        uint32_t index = hash % cms_width;
        minCount = std::min(minCount, cms_matrix[i][index]);
    }
    return minCount;
}

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

    // Apply HLL corrections
    if (estimate <= 2.5 * (1 << precision)) {
        int zeros = std::count(registers.begin(), registers.end(), 0);
        if (zeros > 0) {
            estimate = (1 << precision) * std::log(static_cast<double>(1 << precision) / zeros);
        }
    }
    else if (estimate > (1.0 / 30.0) * std::pow(2, 32)) {
        estimate = -std::pow(2, 32) * std::log(1.0 - estimate / std::pow(2, 32));
    }

    return static_cast<int>(estimate);
}

int CEEngine::query(const std::vector<CompareExpression>& quals) {
    if (dirty) {
        prepare();
        dirty = false;
    }

    if (quals.empty()) {
        return hllQuery(0);
    }

    // Use both HLL and CMS for better accuracy
    int maxEstimate = 0;
    for (const auto& qual : quals) {
        int hllEst = hllQuery(qual.value);
        int cmsEst = estimateFrequency(qual.value);
        // Take minimum of both estimates as upper bound
        maxEstimate = std::max(maxEstimate, std::min(hllEst, cmsEst));
    }
    return maxEstimate;
}

void CEEngine::prepare() {
    if (!dirty) return;

    // Process only modified indices
    for (int idx : freeIndices) {
        if (idx < registers.size()) {
            registers[idx] = 0;  // Reset only the modified HLL registers
        }
    }
    
    // Reset CMS entries affected by deleted tuples
    for (const auto& tupleId : freeIndices) {
        for (size_t i = 0; i < cms_depth; ++i) {
            uint32_t hash = fastHash(tupleId ^ cms_seeds[i]);
            uint32_t index = hash % cms_width;
            cms_matrix[i][index] = 0;  // Clear the CMS entry
        }
    }
    
    // Clear the free index list
    freeIndices.clear();
    dirty = false;
}
