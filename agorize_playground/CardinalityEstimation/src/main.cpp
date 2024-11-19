#include "../include/CardinalityEstimation.h"
#include <executer/DataExecuterDemo.h>
#include <iostream>
#include "../include/common/Root.h"
#include "TimingUtils.h"

int main(int argc, char* argv[]) {
    int initSize = 100000;
    int opSize = 100000;
    double score = 0;
    int cnt = 0;

    DataExecuterDemo dataExecuter(initSize - 1, opSize);
    CEEngine ceEngine(initSize, &dataExecuter);

    // Create TimingData instances for each operation
    TimingData insertTiming;
    TimingData deleteTiming;
    TimingData queryTiming;
    TimingData prepareTiming;

    // Create a MaxMemoryTracker to track maximum memory usage
    MaxMemoryTracker memoryTracker;

    Action action = dataExecuter.getNextAction();

    while (action.actionType != NONE) {
        measureExecutionTime(
                [&]() { ceEngine.prepare(); },
                prepareTiming,
                memoryTracker,
                "Prepare"
            );

        if (action.actionType == INSERT) {
            measureExecutionTime(
                [&]() { ceEngine.insertTuple(action.actionTuple); },
                insertTiming,
                memoryTracker,
                "Insert"
            );

        } else if (action.actionType == DELETE) {
            measureExecutionTime(
                [&]() { ceEngine.deleteTuple(action.actionTuple, action.tupleId); },
                deleteTiming,
                memoryTracker,
                "Delete"
            );

        } else if (action.actionType == QUERY) {
            measureExecutionTime(
                [&]() { 
                    int estimatedCount = ceEngine.query(action.quals);
                    score += dataExecuter.answer(estimatedCount); // Update score
                },
                queryTiming,
                memoryTracker,
                "Query"
            );
            cnt++;
        }

        action = dataExecuter.getNextAction();
    }

    // Print the results for each operation type
    insertTiming.print("Insert");
    deleteTiming.print("Delete");
    queryTiming.print("Query");
    prepareTiming.print("Prepare");

    // Print the maximum memory usage observed
    memoryTracker.printMaxMemory("Max Memory");

    std::cout << "Average Error: " << score / cnt << std::endl;

    return 0;
}