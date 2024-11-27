#include "../include/CardinalityEstimation.h"
#include <executer/DataExecuterDemo.h>
#include <iostream>
#include "../include/common/Root.h"
#include "TimingUtils.h"
#include "MemoryUtils.h"

#include <iostream>
#include <vector>

void runTest(int initSize, int opSize, double prepareSamplingRate, int actionSamplingFrequency) {
    double score = 0;
    int cnt = 0;

    DataExecuterDemo dataExecuter(initSize - 1, opSize);
    CEEngine ceEngine(initSize, &dataExecuter, prepareSamplingRate, actionSamplingFrequency);

    Action action = dataExecuter.getNextAction();

    while (action.actionType != NONE) {
        ceEngine.prepare();
        if (action.actionType == INSERT) {
            ceEngine.insertTuple(action.actionTuple);
        } else if (action.actionType == DELETE) {
            ceEngine.deleteTuple(action.actionTuple, action.tupleId);
        } else if (action.actionType == QUERY) {
            int ans = ceEngine.query(action.quals);
            score += dataExecuter.answer(ans);
            cnt++;
        }
        action = dataExecuter.getNextAction();
    }

    ceEngine.printTotalTime();
    std::cout << "Average Error: " << score / cnt << std::endl << "\n";
}

int main(int argc, char* argv[]) {
    double prepareSamplingRate = 0.05;
    int actionSamplingFrequency = 10;
runTest(1000, 200, prepareSamplingRate, actionSamplingFrequency);
    runTest(10000, 2000, prepareSamplingRate, actionSamplingFrequency);
    runTest(500000, 20000, prepareSamplingRate, actionSamplingFrequency);

    return 0;
}



//     TimingData insertTiming;
//     TimingData deleteTiming;
//     TimingData queryTiming;
//     TimingData prepareTiming;
//     MemoryUtils memoryTracker;

//     Action action = dataExecuter.getNextAction();

//     while (action.actionType != NONE) {
//         long long beforeMemory = getMemoryUsage();

//      measureExecutionTime(
//     [&]() { 
//         if (action.actionType == INSERT) {
//             ceEngine.insertTuple(action.actionTuple);
//         } else if (action.actionType == DELETE) {
//             ceEngine.deleteTuple(action.actionTuple, action.tupleId);
//         } else if (action.actionType == QUERY) {
//             int estimatedCount = ceEngine.query(action.quals);
//             score += dataExecuter.answer(estimatedCount); 
//             cnt++;
//         }
//     },
//     (action.actionType == INSERT ? insertTiming :
//      action.actionType == DELETE ? deleteTiming :
//      queryTiming),
//     memoryTracker,
//     (action.actionType == INSERT ? "Insert" :
//      action.actionType == DELETE ? "Delete" :
//      "Query")
// );

//         long long memoryDelta = getMemoryUsage() - beforeMemory;

//         memoryTracker.recordMemoryUsage(action.actionType, memoryDelta);

//         action = dataExecuter.getNextAction();
//     }
//     ceEngine.printTotalTime();

//     insertTiming.print("Insert");
//     deleteTiming.print("Delete");
//     queryTiming.print("Query");

//     memoryTracker.printMaxMemory("Max Memory");
//     memoryTracker.printMemoryUsage();

//     std::cout << "Average Error: " << (cnt > 0 ? score / cnt : 0) << std::endl;

//     return 0;
// }
