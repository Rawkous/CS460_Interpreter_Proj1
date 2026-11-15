#include <iostream>
#include <string>
#include <utility>
#include <stdexcept>
#include <cstdint>

#include "Statements.hpp"
#include "ListObject.hpp"


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

// Phase 4: Indexed assignment
IndexAssignmentStatement::IndexAssignmentStatement(
        std::string variableName,
        ExprNode *index,
        ExprNode *value)
        : variableName{std::move(variableName)},
          indexExpr{index},
          valueExpr{value} {}

IndexAssignmentStatement::~IndexAssignmentStatement() {
    delete indexExpr;
    delete valueExpr;
}

void IndexAssignmentStatement::evaluate(SymbolTable &symbolTable) const {

    const ValueDescriptor *target = symbolTable.getValueFor(variableName);

    if (target->type() != ValueDescriptor::Type::list) {
        throw std::runtime_error("list assignment: target must be a list, got " + target->typeName());
    }

    const auto *listDescriptor = static_cast<const ListValueDescriptor *>(target);

    ListObject *list = const_cast<ListObject *>(listDescriptor->object()
    );

    ValueDescriptor *indexValue = indexExpr->evaluate(symbolTable);

    if (indexValue->type() != ValueDescriptor::Type::integer) {
        std::string typeName = indexValue->typeName();
        delete indexValue;

        throw std::runtime_error(
                "list assignment: index must be integer, got " +
                typeName
        );
    }

    std::int64_t index = static_cast<IntegerValueDescriptor *>(indexValue)->value();

    delete indexValue;

    if (index < 0 || static_cast<std::uint64_t>(index) >= list->size()) {

        throw std::out_of_range("list assignment: index " + std::to_string(index) + " out of range");
    }

    ValueDescriptor *newValue = valueExpr->evaluate(symbolTable);

    try {
        list->setElement(static_cast<std::size_t>(index),newValue);
    } catch (...) {
        delete newValue;
        throw;
    }

}

void IndexAssignmentStatement::print() const {
    std::cout << variableName << '[';
    indexExpr->print();
    std::cout << "] = ";
    valueExpr->print();
    std::cout << '\n';
}

// Phase 4: Append statement
AppendStatement::AppendStatement(
        ExprNode *target,
        ExprNode *value)
        : targetExpr{target},
          valueExpr{value} {}

AppendStatement::~AppendStatement() {
    delete targetExpr;
    delete valueExpr;
}

void AppendStatement::evaluate(SymbolTable &symbolTable) const {

    ValueDescriptor *target = targetExpr->evaluate(symbolTable);

    if (target->type() != ValueDescriptor::Type::list) {
        std::string typeName = target->typeName();
        delete target;

        throw std::runtime_error(
                "append: first argument must be list, got " +
                typeName
        );
    }

    auto *listDescriptor = static_cast<ListValueDescriptor *>(target);

    ValueDescriptor *newValue = nullptr;

    try {
        newValue = valueExpr->evaluate(symbolTable);

        listDescriptor->object()->append(newValue);

    } catch (...) {
        delete newValue;
        delete target;
        throw;
    }

    delete target;
}

void AppendStatement::print() const {
    std::cout << "append(";
    targetExpr->print();
    std::cout << ", ";
    valueExpr->print();
    std::cout << ")\n";
}