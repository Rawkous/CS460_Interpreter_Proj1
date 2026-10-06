#include "Range.hpp"

#include <iostream>

using namespace std;

/*
 constructor reports a runtime error if step is zero. 
 This error occurs before the loop variable is assigned 
 or the suite is executed, even if the range would otherwise 
 be empty.
 */
EvaluatedRange::EvaluatedRange(int start, int stop, int step)
    : start_{start}, stop_{stop}, step_{step} {
        if (step == 0){
            throw std::runtime_error(
                "step cannot be zero");
        }
    }


int EvaluatedRange::start() const {
    return start_;
}
int EvaluatedRange::stop() const {
    return stop_;
}
int EvaluatedRange::step() const {
    return step_;
}

/*
determines whether the start value belongs to the range. 
For a positive step, it returns whether start < stop. 
For a negative step, it returns whether start > stop.
*/
bool EvaluatedRange::hasIteration() const {
    if (step_ > 0) {
        return start_ < stop_;
    } else {
        return start_ > stop_;
    }
}
bool EvaluatedRange::shouldContinue(int nextValue) const {
    if (step_ > 0) {
        return nextValue < stop_;
    } else {
        return nextValue > stop_;
    }
}


// For range(stop), startExpression and 
// stepExpression are null, and stopExpression is non-null.
RangeExpression::RangeExpression(ExprNode* stop) : startExpression{nullptr}, stopExpression{stop}, stepExpression{nullptr} {}

// For range(start, stop), stepExpression is null,
// and the other two pointers are non-null.
RangeExpression::RangeExpression(ExprNode* start, ExprNode* stop) : startExpression{start}, stopExpression{stop}, stepExpression{nullptr} {}


// For range(start, stop, step), all three pointers are non-null.
RangeExpression::RangeExpression(ExprNode* start, ExprNode* stop, ExprNode* step) : startExpression{start}, stopExpression{stop}, stepExpression{step} {}
    
RangeExpression::~RangeExpression() {
    delete startExpression;
    delete stepExpression;
    delete stopExpression;

}

/*
 evaluates each supplied expression exactly once and in source order.
 It applies start = 0 and step = 1 when the corresponding expressions are omitted,
 then constructs an EvaluatedRange from the three concrete values.
*/
EvaluatedRange RangeExpression::evaluate(const SymbolTable& symbolTable) const {
    const int startVal = startExpression ? startExpression->evaluate(symbolTable) : 0;
    const int stopVal = stopExpression->evaluate(symbolTable);
    const int stepVal = stepExpression ? stepExpression->evaluate(symbolTable) : 1;

    return EvaluatedRange(startVal, stopVal, stepVal);
}

void RangeExpression::print(std::ostream& output) const {
    output << "range(";
    if (startExpression) {
        startExpression->print();
        output << ", ";
    }
    stopExpression->print();
    if (stepExpression) {
        output << ", ";
        stepExpression->print();
    }
    output << ")";
}
