#include <iostream>
#include <string>
#include <utility>
#include <stdexcept>

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
// Phase 3: prints all types correctly and deletes temp descriptor
void PrintStatement::evaluate(SymbolTable &symbolTable) const {
    ValueDescriptor *value = printExpr->evaluate(symbolTable);
    std::cout << value->toString() << '\n';
    delete value;
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
    std::int64_t nextValue = range.start();

    while (range.shouldContinue(nextValue)) {
        // Set the loop variable in the symbol table
        symbolTable.setValueFor(variableName, new IntegerValueDescriptor(nextValue));

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

    ++printIndentLevel;

    bodyStmts->print();

    --printIndentLevel;
}

// Phase 4: IfStatement
IfStatement::~IfStatement() {
    for (const auto &branch : branches) {
        delete branch.condition;
        delete branch.body;
    }

    delete elseBody;
}

void IfStatement::addBranch(ExprNode *condition, Statements *body) {
    branches.push_back({condition, body});
}

void IfStatement::setElseBranch(Statements *body) {
    elseBody = body;
}

// Phase 4: Evaluate if / elif / else
void IfStatement::evaluate(SymbolTable &symbolTable) const {
    for (const auto &branch : branches) {
        ValueDescriptor *value =
                branch.condition->evaluate(symbolTable);

        if (value->type() != ValueDescriptor::Type::boolean) {
            std::string typeName = value->typeName();
            delete value;

            throw std::runtime_error(
                    "if/elif condition must be boolean, got " + typeName
            );
        }

        bool conditionIsTrue =
                static_cast<BooleanValueDescriptor *>(value)->value();

        delete value;

        if (conditionIsTrue) {
            branch.body->evaluate(symbolTable);
            return;
        }
    }

    if (elseBody != nullptr) {
        elseBody->evaluate(symbolTable);
    }
}

// Phase 4: Print if / elif / else statement
void IfStatement::print() const {
    for (std::size_t i = 0; i < branches.size(); ++i) {
        // The first branch is if; the rest are elif
        if (i == 0) {
            std::cout << "if ";
        } else {
            for (int level = 0; level < printIndentLevel; ++level) {
                std::cout << "   ";
            }
            std::cout << "elif ";
        }

        branches[i].condition->print();
        std::cout << ":\n";

        ++printIndentLevel;
        branches[i].body->print();
        --printIndentLevel;
    }

    // Print the optional else suite
    if (elseBody != nullptr) {
        for (int level = 0; level < printIndentLevel; ++level) {
            std::cout << "   ";
        }

        std::cout << "else:\n";

        ++printIndentLevel;
        elseBody->print();
        --printIndentLevel;
    }
}

// Phase 4: WhileStatement
WhileStatement::WhileStatement(ExprNode *condition, Statements *body)
        : condition{condition}, body{body} {}

WhileStatement::~WhileStatement() {
    delete condition;
    delete body;
}

// Phase 4: Evaluate while loop
void WhileStatement::evaluate(SymbolTable &symbolTable) const {
    while (true) {
        ValueDescriptor *value = condition->evaluate(symbolTable);

        if (value->type() != ValueDescriptor::Type::boolean) {
            std::string typeName = value->typeName();
            delete value;

            throw std::runtime_error("while condition must be boolean, got " + typeName);
        }

        bool conditionIsTrue =
                static_cast<BooleanValueDescriptor *>(value)->value();

        delete value;

        if (!conditionIsTrue) {
            break;
        }

        body->evaluate(symbolTable);
    }
}

// Phase 4: Print while statement
void WhileStatement::print() const {
    std::cout << "while ";
    condition->print();
    std::cout << ":\n";

    ++printIndentLevel;
    body->print();
    --printIndentLevel;
}