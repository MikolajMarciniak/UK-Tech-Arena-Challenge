#include "../include/CardinalityEstimation.h"
#include <executer/DataExecuterDemo.h>
#include <iostream>
#include "../include/common/Root.h"
#include "TimingUtils.h"

int main(int argc, char* argv[]) {
    // Initialize parameters
    int initSize = 100000;  // Initial number of tuples
    int opSize = 10000;    // Number of operations
    double score = 0;
    int cnt = 0;

    // Initialize data generator and cardinality estimation engine
    DataExecuterDemo dataExecuter(initSize - 1, opSize);
    CEEngine ceEngine(initSize, &dataExecuter);

    // Create TimingData instances for measuring time for each operation
    TimingData insertTiming;
    TimingData deleteTiming;
    TimingData queryTiming;
    TimingData prepareTiming;

    // Create a MaxMemoryTracker to track maximum memory usage
    MaxMemoryTracker memoryTracker;

    // Get the first action from the data executer
    Action action = dataExecuter.getNextAction();

    // Process actions until none are left
    while (action.actionType != NONE) {
        // Prepare operation (e.g., cleanup or initialization)
        measureExecutionTime(
            [&]() { ceEngine.prepare(); },
            prepareTiming,
            memoryTracker,
            "Prepare"
        );

        // Process action based on its type
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
                    int estimatedCount = ceEngine.query(action.quals); // Perform query
                    score += dataExecuter.answer(estimatedCount);      // Update error score
                },
                queryTiming,
                memoryTracker,
                "Query"
            );
            cnt++; // Increment the number of queries processed
        }

        // Get the next action
        action = dataExecuter.getNextAction();
    }

    // Print the timing results for each operation
    insertTiming.print("Insert");
    deleteTiming.print("Delete");
    queryTiming.print("Query");
    prepareTiming.print("Prepare");

    // Print the maximum memory usage observed
    memoryTracker.printMaxMemory("Max Memory");

    // Print the average error across all queries
    std::cout << "Average Error: " << score / cnt << std::endl;

    return 0;
}
