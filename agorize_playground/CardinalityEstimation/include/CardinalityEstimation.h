#ifndef CARDINALITYESTIMATION_ENGINE
#define CARDINALITYESTIMATION_ENGINE
#include <executer/DataExecuter.h>
#include <common/Expression.h>
#include <vector>
#include <cstdint>

class CEEngine {
public:
    CEEngine(int num, DataExecuter* dataExecuter);
    ~CEEngine() = default;

    void insertTuple(const std::vector<int>& tuple);

    void deleteTuple(const std::vector<int>& tuple, int tupleId);

    int query(const std::vector<CompareExpression>& quals);

    void prepare();

private:
    DataExecuter* dataExecuter;
    int precision;  // for HyperLogLog
    std::vector<uint8_t> registers;
    double alpha;

    void hllInsert(int value);

    int hllQuery(int value);
};

#endif
