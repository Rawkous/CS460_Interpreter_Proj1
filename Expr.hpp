#ifndef EXPRINTER_EXPR_HPP
#define EXPRINTER_EXPR_HPP

#include "SymbolTable.hpp"
#include "Token.hpp"
#include "ValueDescriptor.hpp"
#include "ListObject.hpp"
#include <vector>

class ExprNode {
public:
    explicit ExprNode(Token token);
    virtual ~ExprNode() = default;

    [[nodiscard]] const Token &token() const;
    virtual void print() const = 0;
    [[nodiscard]] virtual ValueDescriptor *evaluate(const SymbolTable &symbolTable) const = 0;

private:
    Token expressionToken;
};

class BinaryExprNode final : public ExprNode {
public:
    BinaryExprNode(Token token, ExprNode *left, ExprNode *right);
    ~BinaryExprNode() override;

    void print() const override;
    [[nodiscard]] ValueDescriptor *evaluate(const SymbolTable &symbolTable) const override;

private:
    ExprNode *leftOperand;
    ExprNode *rightOperand;
};

class UnaryExprNode final : public ExprNode {
public:
    UnaryExprNode(Token token, ExprNode *operand);
    ~UnaryExprNode() override;

    void print() const override;
    [[nodiscard]] ValueDescriptor *evaluate(const SymbolTable &symbolTable) const override;

private:
    ExprNode *expression;
};

class IntegerLiteral final : public ExprNode {
public:
    explicit IntegerLiteral(Token token);

    void print() const override;
    [[nodiscard]] ValueDescriptor *evaluate(const SymbolTable &symbolTable) const override;
};

// Phase 3: new Literal classes
class DoubleLiteral final : public ExprNode {
public:
    explicit DoubleLiteral(Token token);

    void print() const override;
    [[nodiscard]] ValueDescriptor *evaluate(const SymbolTable &symbolTable) const override;
};

class BooleanLiteral final : public ExprNode {
public:
    explicit BooleanLiteral(Token token);   // token is the True or False keyword

    void print() const override;
    [[nodiscard]] ValueDescriptor *evaluate(const SymbolTable &symbolTable) const override;
};

class StringLiteral final : public ExprNode {
public:
    explicit StringLiteral(Token token);

    void print() const override;
    [[nodiscard]] ValueDescriptor *evaluate(const SymbolTable &symbolTable) const override;
};

class StrConversion final : public ExprNode {
public:
    StrConversion(Token token, ExprNode *operand);
    ~StrConversion() override;

    void print() const override;
    [[nodiscard]] ValueDescriptor *evaluate(const SymbolTable &symbolTable) const override;

private:
    ExprNode *expression;
};

class Variable final : public ExprNode {
public:
    explicit Variable(Token token);

    void print() const override;
    [[nodiscard]] ValueDescriptor *evaluate(const SymbolTable &symbolTable) const override;
};


//Phase 4: <list-literal> -> "[" [ <rel-expr> { "," <rel-expr> } ] "]"
class ListLiteral final : public ExprNode {
public:
    // token is the '[' token (used for error line/column)
    // the node takes ownership of every pointer in elements
    ListLiteral(Token token, std::vector<ExprNode *> elements);
    ~ListLiteral() override;

    ListLiteral(const ListLiteral &) = delete;
    ListLiteral &operator=(const ListLiteral &) = delete;

    void print() const override;
    [[nodiscard]] ValueDescriptor *evaluate(const SymbolTable &symbolTable) const override;

private:
    std::vector<ExprNode *> elementExprs;
};

#endif // EXPRINTER_EXPR_HPP
