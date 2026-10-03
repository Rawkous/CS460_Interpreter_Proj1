#include "Range.hpp"

#include <iostream>

using namespace std;

// EvaluatedRange(int start, int stop, int step);

//     [[nodiscard]] int start() const;
//     [[nodiscard]] int stop() const;
//     [[nodiscard]] int step() const;

//     [[nodiscard]] bool hasIteration() const;
//     [[nodiscard]] bool shouldContinue(int nextValue) const;


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
        startExpression->print(output);
        output << ", ";
    }
    stopExpression->print(output);
    if (stepExpression) {
        output << ", ";
        stepExpression->print(output);
    }
    output << ")";
}
