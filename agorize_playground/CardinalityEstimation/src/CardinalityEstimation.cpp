//
// You should modify this file.
//
#include "../include/common/Root.h"
#include "../include/CardinalityEstimation.h"
std::vector<std::vector<int>> tuples;
std::vector<int> tupleIds;
std::unordered_map<int, std::unordered_map<int, std::vector<int>>> columnIndex;
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
}

int CEEngine::query(const std::vector<CompareExpression>& quals)
{
    int matchCount = 0;

    for (const auto& tuple : tuples)
    {
        bool matches = true;

        for (const auto& qual : quals)
        {
            if (qual.columnIdx >= tuple.size())
            {
                matches = false;
                break;
            }

            int tupleValue = tuple[qual.columnIdx];
            switch (qual.compareOp)
            {
                case EQUAL:
                    if (tupleValue != qual.value)
                    {
                        matches = false;
                    }
                    break;
                case GREATER:
                    if (tupleValue <= qual.value)
                    {
                        matches = false;
                    }
                    break;
                default:
                    throw std::invalid_argument("Unsupported CompareOp");
            }

            if (!matches)
            {
                break; 
            }
        }

        if (matches)
        {
            matchCount++;
        }
    }

    return matchCount;
}


void CEEngine::prepare()
{
    // Implement your prepare function here.
}

CEEngine::CEEngine(int num, DataExecuter *dataExecuter)
{
    // Implement your constructor here.
    this->dataExecuter = dataExecuter;
}
