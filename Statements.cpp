#include <iostream>
#include <string>
#include <utility>

#include "Statements.hpp"

Statements::~Statements() {
    for (auto *statement : statements)
        delete statement;
}

void Statements::addStatement(Statement *statement) {
    statements.push_back(statement);
}

void Statements::print() const {
    for (const auto *statement : statements)
        statement->print();
}

void Statements::evaluate(SymbolTable &symbolTable) const {
    for (const auto *statement : statements)
        statement->evaluate(symbolTable);
}

AssignmentStatement::AssignmentStatement(
    std::string variableName,
    ExprNode *expression)
    : variableName{std::move(variableName)}, expression{expression} {}

AssignmentStatement::~AssignmentStatement() {
    delete expression;
}

void AssignmentStatement::evaluate(SymbolTable &symbolTable) const {
    symbolTable.setValueFor(variableName, expression->evaluate(symbolTable));
}

void AssignmentStatement::print() const {
    std::cout << variableName << " = ";
    expression->print();
    std::cout << '\n';
}

// helper function implementation
void AssignmentStatement::printInline() const {
    std::cout << variableName << " = ";
    expression->print();
}

// PrintStatement implementation
PrintStatement::PrintStatement(ExprNode *printExpr)
    : printExpr{printExpr} {}

// Destructor
PrintStatement::~PrintStatement() {
    delete printExpr;
}

// Evaluate the print statement by evaluating the expression and printing its value
void PrintStatement::evaluate(SymbolTable &symbolTable) const {
    int value = printExpr->evaluate(symbolTable);
    std::cout << value << '\n';
    
}


void PrintStatement::print() const {
    std::cout << "print ";
    printExpr->print();
    // Makes readability better to add a newline after
    std::cout << '\n';
}

// Python-style ForStatement class implementation
ForStatement::ForStatement(std::string variableName, RangeExpression *rangeExpr, Statements *bodyStmts)
        : variableName{std::move(variableName)}, rangeExpr{rangeExpr}, bodyStmts{bodyStmts} {}


// Destructor for ForStatement
ForStatement::~ForStatement() {
    delete rangeExpr;
    delete bodyStmts;
}

// Evaluate the python-style for statement
void ForStatement::evaluate(SymbolTable &symbolTable) const {
    EvaluatedRange range = rangeExpr->evaluate(symbolTable);

    if (!range.hasIteration())
        return;

    int nextValue = range.start();

    while (range.shouldContinue(nextValue)) {
        symbolTable.setValueFor(variableName, nextValue);

        bodyStmts->evaluate(symbolTable);

        nextValue += range.step();
    }
}

// Print complete python-style for statement
// <for-statement> -> "for" <id> "in" <range> ":" <suite>
void ForStatement::print() const {
    std::cout << "for " << variableName << " in ";
    rangeExpr->print(std::cout);
    std::cout << ":\n";

    bodyStmts->print();
}
