#ifndef EXPRINTER_STATEMENTS_HPP
#define EXPRINTER_STATEMENTS_HPP

#include <string>
#include <vector>

#include "Expr.hpp"
#include "Range.hpp"
#include "SymbolTable.hpp"


class Statement {
public:
    virtual ~Statement() = default;

    virtual void print() const = 0;
    virtual void evaluate(SymbolTable &symbolTable) const = 0;
};

class Statements {
public:
    ~Statements();

    void addStatement(Statement *statement);
    void evaluate(SymbolTable &symbolTable) const;
    void print() const;

private:
    std::vector<Statement *> statements;
};

class AssignmentStatement final : public Statement {
public:
    AssignmentStatement(std::string variableName,
                        ExprNode *expression);
    ~AssignmentStatement() override;

    void evaluate(SymbolTable &symbolTable) const override;
    void print() const override;
    // helper function that prints without a newline
    void printInline() const;

private:
    std::string variableName;
    ExprNode *expression;
};



// The derived class for print-statement -> "print" <rel-expr>
class PrintStatement final : public Statement {
public:
    PrintStatement(ExprNode *printExpr);
    ~PrintStatement() override;


    void evaluate(SymbolTable &symbolTable) const override;
    void print() const override;
    
private:
    ExprNode *printExpr;
};

// Python-style for loops:
class ForStatement final : public Statement {
public:
    ForStatement(std::string variableName, RangeExpression *rangeExpr, Statements *bodyStmts);

    ~ForStatement() override;

    void evaluate(SymbolTable &symbolTable) const override;
    void print() const override;

private:
    std::string variableName;
    RangeExpression *rangeExpr;
    Statements *bodyStmts;
};

// Phase 4: if / elif / else
class IfStatement final : public Statement {
public:
    IfStatement() = default;
    ~IfStatement() override;

    void addBranch(ExprNode *condition, Statements *body);
    void setElseBranch(Statements *body);

    void evaluate(SymbolTable &symbolTable) const override;
    void print() const override;

private:
    struct Branch {
        ExprNode *condition;
        Statements *body;
    };

    std::vector<Branch> branches;
    Statements *elseBody = nullptr;
};

// Phase 4: while loops
class WhileStatement final : public Statement {
public:
    WhileStatement(ExprNode *condition, Statements *body);
    ~WhileStatement() override;

    void evaluate(SymbolTable &symbolTable) const override;
    void print() const override;

private:
    ExprNode *condition;
    Statements *body;
};

// Phase 4: index assignment statement
class IndexAssignmentStatement final : public Statement {
public:
    IndexAssignmentStatement(std::string variableName, ExprNode *index, ExprNode *value);
    ~IndexAssignmentStatement() override;       // delete index; delete value;
    IndexAssignmentStatement(const IndexAssignmentStatement &) = delete;
    IndexAssignmentStatement &operator=(const IndexAssignmentStatement &) = delete;

    void evaluate(SymbolTable &symbolTable) const override;
    void print() const override;                // name "[" index "] = " value "\n"
private:
    std::string variableName;
    ExprNode *indexExpr;
    ExprNode *valueExpr;
};

// Phase 4: append statement
class AppendStatement final : public Statement {
public:
    AppendStatement(ExprNode *target, ExprNode *value);
    ~AppendStatement() override;                // delete both
    AppendStatement(const AppendStatement &) = delete;
    AppendStatement &operator=(const AppendStatement &) = delete;

    void evaluate(SymbolTable &symbolTable) const override;
    void print() const override;                // "append(" target ", " value ")\n"
private:
    ExprNode *targetExpr;
    ExprNode *valueExpr;
};

#endif // EXPRINTER_STATEMENTS_HPP
