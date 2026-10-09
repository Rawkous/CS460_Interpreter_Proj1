#include <cstdlib>
#include <iostream>
#include <string>


#include "Parser.hpp"


void Parser::die(const std::string &where,
                const std::string &message,
                const Token &token) const {
   std::cerr << where << ": " << message << " at line " << token.lineNumber()
             << ", column " << token.columnNumber() << ". Got: ";
   token.print(std::cerr);
   std::cerr << "\n\nTokens identified up to this point:\n";
   tokenizer.printProcessedTokens(std::cerr);
   std::exit(EXIT_FAILURE);
}


Statements *Parser::program() {
   // <program> -> <statements> EOF
   Statements *parsedStatements = statements();
   Token eof = tokenizer.getToken();
   if (!eof.isEof()) {
       delete parsedStatements;
       die("Parser::program", "expected EOF", eof);
   }
   return parsedStatements;
}


Statements *Parser::statements() {
   // phase 2: <statements> → <statement> { <statement> }
   auto *parsedStatements = new Statements();

   parsedStatements->addStatement(statement());

   Token next = tokenizer.getToken();
   while (next.isIdentifier() || next.isKeyword() || next.isForKeyword() || next.isPrintKeyword()) {
       tokenizer.ungetToken();

       parsedStatements->addStatement(statement());

       next = tokenizer.getToken();
   }

   tokenizer.ungetToken();
   return parsedStatements;
}


Statement *Parser::statement() {
   //phase 2: <statement> → <simple-statement> NEWLINE <compound-statement>
   Token token = tokenizer.getToken();
  
   // compound: for-statement
   if (token.isForKeyword()) {
       tokenizer.ungetToken();
       return forStatement();
   }  
   // Simple: assignment-statement
   if (token.isIdentifier()) {
       tokenizer.ungetToken();
       
       Statement *stmt = assignmentStatement();
       Token newline = tokenizer.getToken();
         if (!newline.isNewline()) {
             delete stmt;
             die("Parser::statement", "expected NEWLINE after assignment statement", newline);
         }
       return stmt;
   }
   // Simple: print-statement
    if (token.isPrintKeyword()) {
       tokenizer.ungetToken();

       Statement *stmt = printStatement();
       Token newline = tokenizer.getToken();
         if (!newline.isNewline()) {
             delete stmt;
             die("Parser::statement", "expected NEWLINE after print statement", newline);
         }
       return stmt;
   }
   
   die("Parser::statement", "expected assignment, print, or for statement", token);
}


AssignmentStatement *Parser::assignmentStatement() {
   // <assignment-statement> -> <id> = <rel-expr>
   // The caller consumes the context-dependent terminator: NEWLINE in a
   // statement list or ';' in a future for-loop header.
   Token variable = tokenizer.getToken();
   if (!variable.isIdentifier())
       die("Parser::assignmentStatement", "expected an identifier", variable);


   Token assignmentOperator = tokenizer.getToken();
   if (!assignmentOperator.isAssignmentOperator())
       die("Parser::assignmentStatement", "expected '='", assignmentOperator);


   return new AssignmentStatement(variable.identifier(), relExpr());
}

// Added: The derived class for print-statement -> "print" <rel-expr>
PrintStatement *Parser::printStatement() {
   // <print-statement> -> "print" <rel-expr>
   Token printKeyword = tokenizer.getToken();
   if (!printKeyword.isPrintKeyword())
       die("Parser::printStatement", "expected 'print' keyword", printKeyword);

   return new PrintStatement(relExpr());
}

// Added: The derived class for for statement
ForStatement *Parser::forStatement() {
   //<for-statement> -> "for" <id> "in" <range> ":" <suite>

   Token forKeyword = tokenizer.getToken();
   if (!forKeyword.isForKeyword())
       die("Parser::forStatement", "expected 'for' keyword", forKeyword);

   Token variable = tokenizer.getToken();
   if (!variable.isIdentifier())
       die("Parser::forStatement", "expected an identifier", variable);

   Token inKeyword = tokenizer.getToken();
   if (!inKeyword.isInKeyword())
       die("Parser::forStatement", "expected 'in' keyword", inKeyword);

   RangeExpression *rangeExpr = range();

   Token colon = tokenizer.getToken();
   if (!colon.isColon())
       die("Parser::forStatement", "expected ':'", colon);

   Statements *bodyStmts = suite();

   return new ForStatement(variable.identifier(), rangeExpr, bodyStmts);
}


Statements *Parser::suite() {
    //<suite> -> NEWLINE INDENT <statements> DEDENT
    Token newline = tokenizer.getToken();
    if (!newline.isNewline()) {
        die("Parser::suite", "expected NEWLINE", newline);
    }
    Token indent = tokenizer.getToken();
    if (!indent.isIndent()) {
        die("Parser::suite", "expected INDENT", indent);
    }
    Statements *statements = this->statements();

    Token dedent = tokenizer.getToken();
    if (!dedent.isDedent()) {
        die("Parser::suite", "expected DEDENT", dedent);
    }
    return statements;
}
/*
Phase 2: 
 <for-statement> -> "for" <id> "in" <range> ":" <suite>

<range>
    -> "range" "(" <range-arguments> ")"

<range-arguments>
    -> <rel-expr>
     | <rel-expr> "," <rel-expr>
     | <rel-expr> "," <rel-expr> "," <rel-expr>
*/
RangeExpression *Parser::range() {
    // <range>  -> "range" "(" <range-arguments> ")"
    Token rangeKeyword = tokenizer.getToken();

    if (!rangeKeyword.isRangeKeyword()) 
    {
        die("Parser::range", "expected 'range' keyword", rangeKeyword);
    }
    Token openParen = tokenizer.getToken();
    if (!openParen.isOpenParen()) {
       die("Parser::range", "expected '('", openParen);
    }
    RangeExpression *rangeExpression = rangeArguments();
    
    Token closeParen = tokenizer.getToken();
    if (!closeParen.isCloseParen())
       die("Parser::range", "expected ')'", closeParen);
    
    return rangeExpression;
}


RangeExpression *Parser::rangeArguments() {
    // <range-arguments>  -> <rel-expr> | <rel-expr> "," <rel-expr>
    //  | <rel-expr> "," <rel-expr> "," <rel-expr>

    ExprNode *first = relExpr();

    // Case 1: <rel-expr>
    Token next = tokenizer.getToken();
    if (!next.isComma()) {
        tokenizer.ungetToken();
        return new RangeExpression(first);
    }
    // Case 2: <rel-expr> "," <rel-expr>
    ExprNode *second = relExpr();
    next = tokenizer.getToken();
    if (!next.isComma()) {
        tokenizer.ungetToken();
        return new RangeExpression(first, second);
    }
    // Case 3: <rel-expr> "," <rel-expr> "," <rel-expr>
    ExprNode *third = relExpr();
    
    return new RangeExpression(first, second, third);
}


ExprNode *Parser::relExpr() {
   // <rel-expr> -> <rel-term> [ <equality-op> <rel-term> ]
   ExprNode *left = relTerm();
   Token token = tokenizer.getToken();


   if (token.isEqualityOperator()) {
       ExprNode *right = relTerm();
       left = new BinaryExprNode(token, left, right);
   } else {
       tokenizer.ungetToken();
   }


   return left;
}


ExprNode *Parser::relTerm() {
   // <rel-term> -> <rel-primary> [ <ordering-op> <rel-primary> ]
   ExprNode *left = relPrimary();
   Token token = tokenizer.getToken();


   if (token.isRelationalOperator()) {
       ExprNode *right = relPrimary();
       left = new BinaryExprNode(token, left, right);
   } else {
       tokenizer.ungetToken();
   }


   return left;
}


ExprNode *Parser::relPrimary() {
   // <rel-primary> -> <arith-expr>
   return arithExpr();
}


ExprNode *Parser::arithExpr() {
   // <arith-expr> -> <arith-term> { <add-op> <arith-term> }
   ExprNode *left = arithTerm();
   Token token = tokenizer.getToken();


   while (token.isAdditionOperator() || token.isSubtractionOperator()) {
       ExprNode *right = arithTerm();
       left = new BinaryExprNode(token, left, right);
       token = tokenizer.getToken();
   }


   tokenizer.ungetToken();
   return left;
}


ExprNode *Parser::arithTerm() {
   // <arith-term> -> <arith-primary> { <mult-op> <arith-primary> }
   ExprNode *left = arithPrimary();
   Token token = tokenizer.getToken();


   while (token.isMultiplicationOperator() ||
          token.isDivisionOperator() ||
          token.isModuloOperator()) {
       ExprNode *right = arithPrimary();
       left = new BinaryExprNode(token, left, right);
       token = tokenizer.getToken();
   }


   tokenizer.ungetToken();
   return left;
}


ExprNode *Parser::arithPrimary() {
   // <arith-primary> -> [ <sign> ] <arith-atom>
   Token token = tokenizer.getToken();
   if (token.isAdditionOperator() || token.isSubtractionOperator())
       return new UnaryExprNode(token, arithAtom());


   tokenizer.ungetToken();
   return arithAtom();
}


ExprNode *Parser::arithAtom() {
   // <arith-atom> -> <id> | <integer> | '(' <rel-expr> ')'
   Token token = tokenizer.getToken();


   if (token.isInteger())
       return new IntegerLiteral(token);
   if (token.isIdentifier())
       return new Variable(token);
   if (token.isOpenParen()) {
       ExprNode *expression = relExpr();
       Token closeParen = tokenizer.getToken();
       if (!closeParen.isCloseParen())
           die("Parser::arithAtom", "expected ')'", closeParen);
       return expression;
   }


   die("Parser::arithAtom", "expected an identifier, integer, or '('", token);
}

