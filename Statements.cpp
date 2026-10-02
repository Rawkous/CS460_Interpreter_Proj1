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

// ForStatement class implementation 

ForStatement::ForStatement(AssignmentStatement *initStmt, ExprNode *condition, AssignmentStatement *updateStmt, Statements *bodyStmts)
    : initStmt{initStmt}, condition{condition}, updateStmt{updateStmt}, bodyStmts{bodyStmts} {}

// Destructor for all pointers created
ForStatement::~ForStatement() {
    delete initStmt;
    delete condition;
    delete updateStmt;
    delete bodyStmts;
}

// Evaluate the for statement 
void ForStatement::evaluate(SymbolTable & symbolTable) const {
   

    // Evaluate initialization assignment once.
    initStmt->evaluate(symbolTable);

    // Evaluate the relational condition before every iteration of the loop. 
    int value = condition->evaluate(symbolTable);
    if (value == 0) {
        // If the condition evaluates to zero, exit the loop without executing the body.
        return;
    }

    // Continue while the condition evaluates to a nonzero value.
    while (value != 0) {
        // Evaluate every statement in the loop body.
        bodyStmts->evaluate(symbolTable);

        // Evaluate the update assignment after every iteration of the loop.
        updateStmt->evaluate(symbolTable);

        // Re-evaluate the relational condition for the next iteration.
        value = condition->evaluate(symbolTable);
    }
}


// Print complete for statement
// for-statement -> "for" (<assign-statement>) ; <rel-expr> ; <assign-statement>) {NEWLINE <statements>}
void ForStatement::print() const {
    std::cout << "for (";
    initStmt->printInline();
    std::cout << "; ";
    condition->print();
    std::cout << "; ";
    updateStmt->printInline();
    std::cout << ") {\n";
    bodyStmts->print();
    std::cout << "}\n";

}
