#ifndef EXPRINTER_EXPR_HPP
#define EXPRINTER_EXPR_HPP

#include "SymbolTable.hpp"
#include "Token.hpp"
#include "ValueDescriptor.hpp"
#include "ListObject.hpp"
#include <vector>
#include <string>
#include <cstddef>

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


// Throws unless value is an integer >= 0. Does not take ownership.
std::size_t requireIndex(const ValueDescriptor *value, const std::string &operation);



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

// Phase 4: one-argument calls. Each node owns its operand expression.
// int(x)
class IntConversion final : public ExprNode {
public:
    IntConversion(Token token, ExprNode *operand);
    ~IntConversion() override;
    IntConversion(const IntConversion &) = delete;
    IntConversion &operator=(const IntConversion &) = delete;

    void print() const override;
    [[nodiscard]] ValueDescriptor *evaluate(const SymbolTable &symbolTable) const override;

private:
    ExprNode *expression;
};

//Phase 4: float(x)
class FloatConversion final : public ExprNode {
public:
    FloatConversion(Token token, ExprNode *operand);
    ~FloatConversion() override;
    FloatConversion(const FloatConversion &) = delete;
    FloatConversion &operator=(const FloatConversion &) = delete;

    void print() const override;
    [[nodiscard]] ValueDescriptor *evaluate(const SymbolTable &symbolTable) const override;

private:
    ExprNode *expression;
};

// Phase 4: len(x): x is a string or a list
class LenExpr final : public ExprNode {
public:
    LenExpr(Token token, ExprNode *operand);
    ~LenExpr() override;
    LenExpr(const LenExpr &) = delete;
    LenExpr &operator=(const LenExpr &) = delete;

    void print() const override;
    [[nodiscard]] ValueDescriptor *evaluate(const SymbolTable &symbolTable) const override;

private:
    ExprNode *expression;
};

// Phase 4: split(s): returns a new string-category list
class SplitExpr final : public ExprNode {
public:
    SplitExpr(Token token, ExprNode *operand);
    ~SplitExpr() override;
    SplitExpr(const SplitExpr &) = delete;
    SplitExpr &operator=(const SplitExpr &) = delete;

    void print() const override;
    [[nodiscard]] ValueDescriptor *evaluate(const SymbolTable &symbolTable) const override;

private:
    ExprNode *expression;
};

// Phase 4: pop(list): removes the last element and transfers it to the caller
class PopExpr final : public ExprNode {
public:
    PopExpr(Token token, ExprNode *operand);
    ~PopExpr() override;
    PopExpr(const PopExpr &) = delete;
    PopExpr &operator=(const PopExpr &) = delete;

    void print() const override;
    [[nodiscard]] ValueDescriptor *evaluate(const SymbolTable &symbolTable) const override;

private:
    ExprNode *expression;
};

// Phase 4: subscript expression
class SubscriptExpr final : public ExprNode {
public:
    // token = the '['
    SubscriptExpr(Token token, ExprNode *target, ExprNode *index);  
    ~SubscriptExpr() override;                                    
    SubscriptExpr(const SubscriptExpr &) = delete;
    SubscriptExpr &operator=(const SubscriptExpr &) = delete;

    void print() const override;    
    [[nodiscard]] ValueDescriptor *evaluate(const SymbolTable &symbolTable) const override;

private:
    ExprNode *targetExpr;
    ExprNode *indexExpr;
};
// Phase 4: input() or input(prompt)
class InputExpr final : public ExprNode {
public:
    InputExpr(Token token, ExprNode *prompt);   // prompt may be nullptr
    ~InputExpr() override;                      // delete prompt; (deleting nullptr is safe)
    InputExpr(const InputExpr &) = delete;
    InputExpr &operator=(const InputExpr &) = delete;

    void print() const override;
    [[nodiscard]] ValueDescriptor *evaluate(const SymbolTable &symbolTable) const override;

private:
    ExprNode *promptExpr;
};


#endif // EXPRINTER_EXPR_HPP
