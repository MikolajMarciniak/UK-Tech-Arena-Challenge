//
// You should modify this file.
//
#include "../include/common/Root.h"
#include "../include/CardinalityEstimation.h"
#include "CardinalityEstimation.h"


std::vector<std::vector<int>> tuples;
std::vector<int> tupleIds;
std::unordered_map<int, std::unordered_map<int, std::vector<int>>> columnIndex;
int nextTupleId = 0;

std::vector<int> hllRegisters;
int hllNumBuckets;
double hllAlphaM;

double computeAlphaM(int m) {
        if (m == 16) return 0.673;
        if (m == 32) return 0.697;
        if (m == 64) return 0.709;
        return 0.7213 / (1 + 1.079 / m);
    }

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
    std::cout << "Estimated Cardinality: " << count() << std::endl;
}

double CEEngine::count() {
    double sum = 0.0;

    for (int r : hllRegisters) {
        sum += std::pow(2.0, -r);
    }
    double Z = 1.0 / sum;
    double E = hllAlphaM * hllNumBuckets * hllNumBuckets * Z;


    if (E <= 2.5 * hllNumBuckets) {
            int zeroCount = std::count(hllRegisters.begin(), hllRegisters.end(), 0);
            if (zeroCount > 0) {
                E = hllNumBuckets * std::log(static_cast<double>(hllNumBuckets) / zeroCount);
            }
        }
    
    double threshold = (1ULL << 32) / 30.0; 
    if (E > threshold) {
        E = -((1ULL << 32) * std::log(1.0 - (E / (1ULL << 32))));
    }

    return E;
}


CEEngine::CEEngine(int num, DataExecuter *dataExecuter){
    // Implement your constructor here.
    this->dataExecuter = dataExecuter;
}
