#ifndef EXPRINTER_STATEMENTS_HPP
#define EXPRINTER_STATEMENTS_HPP

#include <string>
#include <vector>

#include "Expr.hpp"
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


// The derived class for for-statement -> "for" (<assign-statement>) ; <rel-expr> ; <assign-statement>) {NEWLINE <statements>}
class ForStatement final : public Statement {
public:
    ForStatement(AssignmentStatement *initStmt, ExprNode *condition, AssignmentStatement *updateStmt, Statements *bodyStmts);
    ~ForStatement() override;

    void evaluate(SymbolTable &symbolTable) const override;
    void print() const override;

private:
    AssignmentStatement *initStmt;
    ExprNode *condition;
    AssignmentStatement *updateStmt;
    Statements *bodyStmts;
};

#endif // EXPRINTER_STATEMENTS_HPP