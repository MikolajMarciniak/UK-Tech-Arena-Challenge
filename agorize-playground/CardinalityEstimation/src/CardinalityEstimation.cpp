#include "../include/common/Root.h"
#include "../include/CardinalityEstimation.h"
#include <chrono>
#include <functional>
#include <bitset>
#include <iostream>

double insertElapsedTime = 0.0;
double prepareInsertElapsedTime = 0.0;
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

    double estimate = count();


    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    
    queryElapsedTime += elapsed;
    return estimate;
}

void CEEngine::updateHLL(const std::vector<int>& tuple, bool isInsert)
{
    VectorHash hash_fn;
    size_t hash = hash_fn(tuple);

    uint32_t h = static_cast<uint32_t>(hash);

    int registerIndex = h & 0x800;

    if (isInsert) {
        registers[registerIndex * REGISTER_SIZE]++;
    } else {
        registers[registerIndex * REGISTER_SIZE]--;
    }
}




// int CEEngine::count() {
//     const int SCALE = 1000000;  // Scaling factor for fixed-point arithmetic
//     int harmonicMeanScaled = 0;

//     // Calculate the scaled harmonic mean
//     for (int i = 0; i < NUM_REGISTERS; ++i) {
//         // Simulate pow(2, -registers[i]) as SCALE / (1 << registers[i])
//         harmonicMeanScaled += SCALE / (1 << registers[i]);
//     }

//     // Harmonic mean: NUM_REGISTERS * SCALE / harmonicMeanScaled
//     int harmonicMean = (NUM_REGISTERS * SCALE) / harmonicMeanScaled;

//     // Alpha scaling: Use integer approximation for alpha
//     int alphaScaled = static_cast<int>(0.7213 * SCALE);  // Alpha scaled by SCALE
//     int adjustedAlpha = alphaScaled * SCALE / (SCALE + (1079 * SCALE / NUM_REGISTERS));

//     // Estimate count: adjustedAlpha * NUM_REGISTERS^2 * harmonicMean / SCALE^2
//     int count = (adjustedAlpha * NUM_REGISTERS * NUM_REGISTERS * harmonicMean) / (SCALE * SCALE);

//     return count;
// }


int CEEngine::count() {
    double harmonicMean = 0.0;
    for (int i = 0; i < NUM_REGISTERS; ++i) {
        harmonicMean += std::pow(2.0, -registers[i]);
    }
    harmonicMean = 1.0 / harmonicMean;
    double alpha = 0.7213 / (1 + 1.079 / NUM_REGISTERS);
    double estimatedCount = alpha * NUM_REGISTERS * NUM_REGISTERS * harmonicMean;
    return static_cast<int>(estimatedCount);
}



void CEEngine::printTotalTime()
{
    std::cout << "Total time for insert operations: " << insertElapsedTime << " seconds" << std::endl;
     std::cout << "Total time for prepare operations: " << prepareInsertElapsedTime << " seconds" << std::endl;
    std::cout << "Total time for delete operations: " << deleteElapsedTime << " seconds" << std::endl;
    std::cout << "Total time for query operations: " << queryElapsedTime << " seconds" << std::endl;
    double totalTime = insertElapsedTime + prepareInsertElapsedTime + deleteElapsedTime + queryElapsedTime;
    std::cout << "Overall total time: " << totalTime << " seconds" << std::endl;
}



int CEEngine::countLeadingZeros(uint32_t hash)
{
    return __builtin_clz(hash);
}


void CEEngine::prepare() {
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::vector<int>> initialData;
    dataExecuter->readTuples(0, initialSize, initialData);  // Read initialSize tuples
    
    for (const auto& tuple : initialData) {
        updateHLL(tuple, true);  // Insert tuples into HLL
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    
    prepareInsertElapsedTime += elapsed;    
}



CEEngine::CEEngine(int initialSize, DataExecuter *dataExecuter)
    : dataExecuter(dataExecuter), initialSize(initialSize) {
    registers = new int[NUM_REGISTERS * REGISTER_SIZE]();  // Zero-initialize the registers
}



CEEngine::~CEEngine() {
    delete[] registers;
}