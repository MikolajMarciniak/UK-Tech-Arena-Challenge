#ifndef CARDINALITYESTIMATION_ENGINE
#define CARDINALITYESTIMATION_ENGINE

#include <executer/DataExecuter.h>
#include <common/Expression.h>

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

    int cmsWidth;
    int cmsDepth;
    std::vector<std::vector<int>> cmsTable;

    void cmsInsert(int value);
    int cmsQuery(int value);
};

#endif
