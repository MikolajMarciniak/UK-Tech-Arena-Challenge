//
// You should modify this file.
//
#include "../include/common/Root.h"
#include "../include/CardinalityEstimation.h"

std::vector<std::vector<int>> tuples;
std::vector<int> tupleIds;
int nextTupleId = 0;

void CEEngine::insertTuple(const std::vector<int>& tuple)
{
    // Implement your insert tuple logic here.
    // push_back -> vectors are useful for our task given 
    //we are dealing with millions of rows
    
 tuples.push_back(tuple);
    tupleIds.push_back(nextTupleId);
    nextTupleId++;
}
void CEEngine::deleteTuple(const std::vector<int>& tuple, int tupleId)
{
    // Implement your delete tuple logic here.
auto idIt = std::find(tupleIds.begin(), tupleIds.end(), tupleId);
    if (idIt != tupleIds.end())
    {
        int index = std::distance(tupleIds.begin(), idIt);
        tuples.erase(tuples.begin() + index);
        tupleIds.erase(idIt);
    }
}
int CEEngine::query(const std::vector<CompareExpression>& quals)
{
    // Implement your query logic here.
    
    return 0;
}

void CEEngine::prepare()
{
    // Implement your prepare logic here.
}

CEEngine::CEEngine(int num, DataExecuter *dataExecuter)
{
    // Implement your constructor here.
    this->dataExecuter = dataExecuter;
}

