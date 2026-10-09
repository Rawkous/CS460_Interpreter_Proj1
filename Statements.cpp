#include <iostream>
#include <string>
#include <utility>

#include "Statements.hpp"

namespace {
    int printIndentLevel = 0;
}

Statements::~Statements() {
    for (auto *statement : statements)
        delete statement;
}

void Statements::addStatement(Statement *statement) {
    statements.push_back(statement);
}

void Statements::print() const {
    for (const auto *statement : statements) {

        // Print indentation based on the current level
        for (int i = 0; i < printIndentLevel; ++i) {
            std::cout << "   "; // 3 spaces for each indentation level
        }
    statement->print();
    }
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


// Destructor for all pointers created in ForStatement
ForStatement::~ForStatement() {
    delete rangeExpr;
    delete bodyStmts;
}

// Evaluate the for statement 
void ForStatement::evaluate(SymbolTable & symbolTable) const {
   
    // Range expression
    EvaluatedRange range = rangeExpr->evaluate(symbolTable);

    if (!range.hasIteration()) {
        return; // No iterations to perform
    }

    // Initialize nextValue to the start of the range
    int nextValue = range.start();

    while (range.shouldContinue(nextValue)) {
        // Set the loop variable in the symbol table
        symbolTable.setValueFor(variableName, nextValue);

        // Evaluate the body statements
        bodyStmts->evaluate(symbolTable);

        // Update nextValue for the next iteration
        nextValue += range.step();
    }
}


/// Print complete python-style for statement
// <for-statement> -> "for" <id> "in" <range> ":" <suite>
void ForStatement::print() const {

    std::cout << "for " << variableName << " in ";
    rangeExpr->print(std::cout);
    std::cout << ":\n";

    // for deeper indentation, increment level
    ++printIndentLevel;

    // Print loop body one level deeper
    bodyStmts->print();

    // Decrement indentation level after printing body
    --printIndentLevel;
}
