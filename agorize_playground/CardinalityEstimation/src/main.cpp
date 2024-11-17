#include "../include/CardinalityEstimation.h"
#include <executer/DataExecuterDemo.h>
#include <iostream>

int main(int argc, char* argv[]) {
    int initSize = 100000;
    int opSize = 10000;
    double score = 0;
    int cnt = 0;

    DataExecuterDemo dataExecuter(initSize - 1, opSize);
    CEEngine ceEngine(initSize, &dataExecuter);

    Action action = dataExecuter.getNextAction();

    while (action.actionType != NONE) {
        ceEngine.prepare();
        if (action.actionType == INSERT) {
            ceEngine.insertTuple(action.actionTuple);
        } else if (action.actionType == DELETE) {
            ceEngine.deleteTuple(action.actionTuple, action.tupleId);
        } else if (action.actionType == QUERY) {
            int estimatedCount = ceEngine.query(action.quals);
            // std::cout << "Estimated Count: " << estimatedCount << std::endl;
            score += dataExecuter.answer(estimatedCount);
            cnt++;
        }
        action = dataExecuter.getNextAction();
    }

    std::cout << "Average Error: " << score / cnt << std::endl;
    
    return 0;
}
