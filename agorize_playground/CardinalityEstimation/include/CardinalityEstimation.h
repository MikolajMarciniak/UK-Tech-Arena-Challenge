#ifndef CARDINALITYESTIMATION_ENGINE
#define CARDINALITYESTIMATION_ENGINE

#include <executer/DataExecuter.h>
#include <common/Expression.h>
#include <vector>
#include <unordered_map>
#include <cstdint>

class CEEngine {
private:
    int numBins;                         // Number of bins in the histogram
    int minValue;                        // Minimum value of the histogram range
    int maxValue;                        // Maximum value of the histogram range
    double binWidth;                     // Width of each bin
    std::vector<int> histogram;          // Histogram data
    bool dirty;                          // Flag to indicate if the histogram is dirty

    DataExecuter* dataExecuter;          // Pointer to the data executor

    // Helper method declarations
    int getBinIndex(int value) const;    // Get the bin index for a value
    void updateHistogram(int value, int delta); // Update the histogram

public:
    CEEngine(int numBins, DataExecuter* dataExecuter);
    void insertTuple(const std::vector<int>& tuple);
    void deleteTuple(const std::vector<int>& tuple, int count);
    int query(const std::vector<CompareExpression>& expressions);
    void prepare();
};


#endif

