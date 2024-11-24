#include "../include/CardinalityEstimation.h"
#include <executer/DataExecuterDemo.h>
#include <iostream>
#include "../include/common/Root.h"
#include "TimingUtils.h"
#include "MemoryUtils.h"

int main(int argc, char* argv[]) {
    int initSize = 100000;
    int opSize = 100;
    double score = 0;
    int cnt = 0;

    DataExecuterDemo dataExecuter(initSize - 1, opSize);
    CEEngine ceEngine(initSize, &dataExecuter);

    TimingData insertTiming;
    TimingData deleteTiming;
    TimingData queryTiming;
    TimingData prepareTiming;

    MemoryUtils memoryTracker;

    Action action = dataExecuter.getNextAction();

    while (action.actionType != NONE) {
        long long beforeMemory = getMemoryUsage();

     measureExecutionTime(
    [&]() { 
        if (action.actionType == INSERT) {
            ceEngine.insertTuple(action.actionTuple);
        } else if (action.actionType == DELETE) {
            ceEngine.deleteTuple(action.actionTuple, action.tupleId);
        } else if (action.actionType == QUERY) {
            int estimatedCount = ceEngine.query(action.quals);
            score += dataExecuter.answer(estimatedCount); 
            cnt++;
        }
    },
    (action.actionType == INSERT ? insertTiming :
     action.actionType == DELETE ? deleteTiming :
     queryTiming),
    memoryTracker,
    (action.actionType == INSERT ? "Insert" :
     action.actionType == DELETE ? "Delete" :
     "Query")
);

        long long afterMemory = getMemoryUsage();
        long long memoryDelta = afterMemory - beforeMemory;

        memoryTracker.recordMemoryUsage(action.actionType, memoryDelta);

        action = dataExecuter.getNextAction();
    }

    insertTiming.print("Insert");
    deleteTiming.print("Delete");
    queryTiming.print("Query");

    memoryTracker.printMaxMemory("Max Memory");
    memoryTracker.printMemoryUsage();

    std::cout << "Average Error: " << (cnt > 0 ? score / cnt : 0) << std::endl;

    return 0;
}
