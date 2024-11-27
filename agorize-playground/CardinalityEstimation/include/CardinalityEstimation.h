// Header file (CardinalityEstimation.h)
#ifndef CARDINALITYESTIMATION_ENGINE
#define CARDINALITYESTIMATION_ENGINE

#include <executer/DataExecuter.h>
#include <common/Expression.h>
#include <vector>
#include <unordered_map>
#include <functional>
#include <chrono>

#define NUM_REGISTERS 2048
#define REGISTER_SIZE 32

struct VectorHash {
    template <typename T>
    size_t operator () (const std::vector<T>& vec) const {
        size_t hash = 0;
        for (const auto& elem : vec) {
            hash = std::hash<T>{}(elem) + (hash << 6) + (hash >> 2);
        }
        return hash;
    }
};

class CEEngine {
public:
    CEEngine(int initialSize, DataExecuter *dataExecuter, double prepareSamplingRate, int actionSamplingFrequency);
    void insertTuple(const std::vector<int>& tuple);
    void deleteTuple(const std::vector<int>& tuple, int tupleId);
    int query(const std::vector<CompareExpression>& quals);
    int count();
    void prepare();
    ~CEEngine();
    void printTotalTime();
    
private:
    void updateHLL(const std::vector<int>& tuple, bool isInsert);
    int countLeadingZeros(uint32_t hash);
    DataExecuter *dataExecuter;
    double totalElapsedTime = 0.0;
    int nextTupleId = 0;
    int* registers;
    int initialSize;

    double prepareSamplingRate;
    int actionSamplingFrequency;
    int actionCounter = 0;
};


#endif
