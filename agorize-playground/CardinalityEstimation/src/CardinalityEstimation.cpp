#include "../include/common/Root.h"
#include "../include/CardinalityEstimation.h"
#include <chrono>
#include <functional>
#include <bitset>
#include <iostream>

double insertElapsedTime = 0.0;
double deleteElapsedTime = 0.0;
double queryElapsedTime = 0.0;

void CEEngine::insertTuple(const std::vector<int>& tuple)
{
    auto start = std::chrono::high_resolution_clock::now();

    updateHLL(tuple, true);

    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    
    insertElapsedTime += elapsed;
}

void CEEngine::deleteTuple(const std::vector<int>& tuple, int tupleId)
{
    auto start = std::chrono::high_resolution_clock::now();

    updateHLL(tuple, false);

    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    
    deleteElapsedTime += elapsed;
}

int CEEngine::query(const std::vector<CompareExpression>& quals)
{
    auto start = std::chrono::high_resolution_clock::now();

    int result = 0;

    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    
    queryElapsedTime += elapsed;
    return result;
}

void CEEngine::updateHLL(const std::vector<int>& tuple, bool isInsert)
{
    VectorHash hash_fn;
    size_t hash = hash_fn(tuple);

    uint32_t h = static_cast<uint32_t>(hash);

    int registerIndex = h & 0xF;

    if (isInsert) {
        registers[registerIndex * REGISTER_SIZE]++;
    } else {
        registers[registerIndex * REGISTER_SIZE]--;
    }
}

void CEEngine::printTotalTime()
{
    std::cout << "Total time for insert operations: " << insertElapsedTime << " seconds" << std::endl;
    std::cout << "Total time for delete operations: " << deleteElapsedTime << " seconds" << std::endl;
    std::cout << "Total time for query operations: " << queryElapsedTime << " seconds" << std::endl;
    double totalTime = insertElapsedTime + deleteElapsedTime + queryElapsedTime;
    std::cout << "Overall total time: " << totalTime << " seconds" << std::endl;
}



int CEEngine::countLeadingZeros(uint32_t hash)
{
    return __builtin_clz(hash);
}

void CEEngine::prepare()
{
    // You can initialize any data or structures here before any operations.
}

CEEngine::CEEngine(int num, DataExecuter *dataExecuter)
{
    this->dataExecuter = dataExecuter;
    
    registers = new int[NUM_REGISTERS * REGISTER_SIZE]();
}

CEEngine::~CEEngine() {
    delete[] registers;
}