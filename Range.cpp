#include "Range.hpp"

#include <iostream>
#include <cstdint>
#include <memory>
#include <stdexcept>

using namespace std;

/*
 constructor reports a runtime error if step is zero. 
 This error occurs before the loop variable is assigned 
 or the suite is executed, even if the range would otherwise 
 be empty.
 */
EvaluatedRange::EvaluatedRange(std::int64_t start, std::int64_t stop, std::int64_t step)
    : start_{start}, stop_{stop}, step_{step} {
        if (step == 0){
            throw std::runtime_error(
                "step cannot be zero");
        }
    }


std::int64_t EvaluatedRange::start() const {
    return start_;
}
std::int64_t EvaluatedRange::stop() const {
    return stop_;
}
std::int64_t EvaluatedRange::step() const {
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
bool EvaluatedRange::shouldContinue(std::int64_t nextValue) const {
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
 // Phase 3: Updated so each arg is evaluated and that its descriptor is an integer, extract the int64 value
 // release temp descriptor
*/
EvaluatedRange RangeExpression::evaluate(const SymbolTable& symbolTable) const {

    auto evaluateInteger = [&](ExprNode *expression) -> std::int64_t {
        std::unique_ptr<ValueDescriptor> value{
                expression->evaluate(symbolTable)
        };

        if (value->type() != ValueDescriptor::Type::integer) {
            throw std::runtime_error(
                    "range argument must be integer, got " + value->typeName()
            );
        }

        return static_cast<IntegerValueDescriptor *>(value.get())->value();
    };

    std::int64_t startVal = startExpression
                            ? evaluateInteger(startExpression) : 0;

    std::int64_t stopVal = evaluateInteger(stopExpression);

    std::int64_t stepVal = stepExpression
                           ? evaluateInteger(stepExpression) : 1;

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
