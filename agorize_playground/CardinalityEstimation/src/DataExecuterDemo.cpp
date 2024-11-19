#include <executer/DataExecuterDemo.h>
#include <unordered_map>
#include <cstdlib>
#include <vector>
#include <cmath>
#include <algorithm>

// Global variable to track visited tuples
std::unordered_map<int, bool> vis;

// Constructor: Initializes the data executer with the specified range and count
DataExecuterDemo::DataExecuterDemo(int end, int count) : DataExecuter() {
    this->end = end;
    this->count = count;

    // Generate initial tuples
    for (int i = 0; i <= end; ++i) {
        std::vector<int> tuple;
        tuple.push_back(rand() % 10000);  // Random value within the range
        tuple.push_back(rand() % 10000);  // Random value within the range
        set.push_back(tuple);
    }
}

// Generate a new tuple for insertion
std::vector<int> DataExecuterDemo::generateInsert() {
    std::vector<int> tuple;
    tuple.push_back(rand() % 10000);  // Random value within the range
    tuple.push_back(rand() % 10000);  // Random value within the range
    set.push_back(tuple);
    end++;
    return tuple;
}

// Generate a random tuple ID for deletion
int DataExecuterDemo::generateDelete() {
    int x = rand() % end;
    while (vis[x]) {
        x = rand() % end;
    }
    vis[x] = true;
    return x;
}

// Retrieve tuples within the specified range for debugging or batch processing
void DataExecuterDemo::readTuples(int start, int offset, std::vector<std::vector<int>>& vec) {
    for (int i = start; i < start + offset; ++i) {
        if (!vis[i]) {
            vec.push_back(set[i]);
        }
    }
}

// Retrieve the next action for the engine
Action DataExecuterDemo::getNextAction() {
    Action action;

    // If no more actions are left, return NONE
    if (count == 0) {
        action.actionType = NONE;
        return action;
    }

    // Determine the type of action to perform
    if (count % 100 == 99) {
        action.actionType = QUERY;

        // Generate a random query condition
        CompareExpression expr = {rand() % 2, CompareOp(rand() % 2), rand() % 10000};
        action.quals.push_back(expr);

    } else if (count % 100 < 90) {
        action.actionType = INSERT;
        action.actionTuple = generateInsert();

    } else {
        action.actionType = DELETE;
        action.tupleId = generateDelete();
        action.actionTuple = set[action.tupleId];
    }

    count--;
    curAction = action;
    return action;
}

double DataExecuterDemo::answer(int ans) {
    int cnt = 0;
    
     for (int i = 0; i <= end; ++i) {
        if (vis[i]) continue;

        bool flag = true;
        for (const auto& expr : curAction.quals) {
            if (expr.compareOp == GREATER && set[i][expr.columnIdx] <= expr.value) {
                flag = false;
                break;
            }
            if (expr.compareOp == EQUAL && set[i][expr.columnIdx] != expr.value) {
                flag = false;
                break;
            }
        }

        if (flag) cnt++;
    }

    double error = fabs(std::log((ans + 1.0) / (cnt + 1.0)));
    return error;
}
