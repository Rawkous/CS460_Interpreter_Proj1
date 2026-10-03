#ifndef RANGE_HPP
#define RANGE_HPP

#include <iosfwd>

#include "Expr.hpp"
#include "SymbolTable.hpp"


class EvaluatedRange {
public:
    EvaluatedRange(int start, int stop, int step);

    [[nodiscard]] int start() const;
    [[nodiscard]] int stop() const;
    [[nodiscard]] int step() const;

    [[nodiscard]] bool hasIteration() const;
    [[nodiscard]] bool shouldContinue(int nextValue) const;

private:
    int start_;
    int stop_;
    int step_;
};

class RangeExpression final {
public:
    explicit RangeExpression(ExprNode* stop);
    RangeExpression(ExprNode* start, ExprNode* stop);
    RangeExpression(
        ExprNode* start,
        ExprNode* stop,
        ExprNode* step
    );
    ~RangeExpression();

    RangeExpression(const RangeExpression&) = delete;
    RangeExpression& operator=(const RangeExpression&) = delete;

    [[nodiscard]] EvaluatedRange evaluate(const SymbolTable& symbolTable) const;

    void print(std::ostream& output) const;

private:
    ExprNode* startExpression;
    ExprNode* stopExpression;
    ExprNode* stepExpression;
};

#endif
