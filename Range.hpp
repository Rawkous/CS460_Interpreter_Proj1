#ifndef RANGE_HPP
#define RANGE_HPP

#include <iosfwd>
#include <cstdint>

#include "Expr.hpp"
#include "SymbolTable.hpp"


class EvaluatedRange {
public:
    EvaluatedRange(std::int64_t start, std::int64_t stop, std::int64_t step);

    [[nodiscard]] std::int64_t start() const;
    [[nodiscard]] std::int64_t stop() const;
    [[nodiscard]] std::int64_t step() const;

    [[nodiscard]] bool hasIteration() const;
    [[nodiscard]] bool shouldContinue(std::int64_t nextValue) const;

private:
    std::int64_t start_;
    std::int64_t stop_;
    std::int64_t step_;
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
