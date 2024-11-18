#include "../include/CardinalityEstimation.h"
#include <executer/DataExecuterDemo.h>
#include <iostream>
#include "../include/common/Root.h"
#include "TimingUtils.h"

int main(int argc, char* argv[]) {
    int initSize = 30000000;
    int opSize = 20000000;
    double score = 0;
    int cnt = 0;

    DataExecuterDemo dataExecuter(initSize - 1, opSize);
    CEEngine ceEngine(initSize, &dataExecuter);

    // Create TimingData instances for each operation
    TimingData insertTiming;
    TimingData deleteTiming;
    TimingData queryTiming;

    Action action = dataExecuter.getNextAction();

    while (action.actionType != NONE) {
        ceEngine.prepare();
        if (action.actionType == INSERT) {
            measureExecutionTime(
                [&]() { ceEngine.insertTuple(action.actionTuple); },
                insertTiming,
                "insert"
            );

        } else if (action.actionType == DELETE) {
            measureExecutionTime(
                [&]() { ceEngine.deleteTuple(action.actionTuple, action.tupleId); },
                deleteTiming,
                "delete"
            );

        } else if (action.actionType == QUERY) {
            measureExecutionTime(
                [&]() { 
                    int estimatedCount = ceEngine.query(action.quals);
                    score += dataExecuter.answer(estimatedCount); // Update score
                },
                queryTiming,
                "query"
            );
            cnt++;
        }
        action = dataExecuter.getNextAction();
    }

    // Print results for each operation
    insertTiming.print("insert");
    deleteTiming.print("delete");
    queryTiming.print("query");

    std::cout << "Average Error: " << score / cnt << std::endl;

    return 0;
}
