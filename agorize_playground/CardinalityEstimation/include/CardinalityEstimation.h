#ifndef CARDINALITYESTIMATION_ENGINE
#define CARDINALITYESTIMATION_ENGINE

#include <executer/DataExecuter.h>
#include <common/Expression.h>
#include <vector>
#include <unordered_map>
#include <cstdint>

class CEEngine {
public:
    // Constructor and destructor
    CEEngine(int num, DataExecuter* dataExecuter);
    ~CEEngine() = default;

    // Public methods
    void insertTuple(const std::vector<int>& tuple);
    void deleteTuple(const std::vector<int>& tuple, int tupleId);
    int query(const std::vector<CompareExpression>& quals);
    void prepare();

private:
    // Private members
    DataExecuter* dataExecuter;
    int precision;  // Precision for HyperLogLog
    std::vector<uint8_t> registers; // HyperLogLog registers
    double alpha; // Correction factor for HyperLogLog
    bool dirty; // Tracks if the registers need rebuilding

    // Tuple management
    std::vector<int> freeIndices; // Reusable indices for deleted tuples
    std::unordered_map<int, int> tupleIdToIndex; // Maps tuple IDs to indices

    // Private methods
    void hllInsert(int value);
    int hllQuery(int value);
};

#endif

