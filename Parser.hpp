#ifndef EXPRINTER_PARSER_HPP
#define EXPRINTER_PARSER_HPP

#include <string>

#include "Expr.hpp"
#include "Statements.hpp"
#include "Token.hpp"
#include "Tokenizer.hpp"
// Phase 2 added class
#include "Range.hpp"

class Parser {
public:
    explicit Parser(Tokenizer &tokenizer) : tokenizer{tokenizer} {}

    Statements *program();
    Statements *statements();
    Statement *statement();
    Statements *suite(); // Phase 2: added suite() function declaration
    AssignmentStatement *assignmentStatement();
    // Added: derived function declarations for print-statement and for-statement
    PrintStatement *printStatement();
    ForStatement *forStatement();
    /*
    Phase 2: 
    <for-statement> -> "for" <id> "in" <range> ":" <suite>

    <range> -> "range" "(" <range-arguments> ")"

    <range-arguments>
        -> <rel-expr>
        | <rel-expr> "," <rel-expr>
        | <rel-expr> "," <rel-expr> "," <rel-expr>
    */
    RangeExpression *range();
    RangeExpression *rangeArguments();

    ExprNode *relExpr();
    ExprNode *relTerm();
    ExprNode *relPrimary();
    ExprNode *arithExpr();
    ExprNode *arithTerm();
    ExprNode *arithPrimary();
    ExprNode *arithAtom();

private:
    Tokenizer &tokenizer;

    [[noreturn]] void die(const std::string &where,
                          const std::string &message,
                          const Token &token) const;
};

#endif // EXPRINTER_PARSER_HPP
