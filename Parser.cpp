#include <cstdlib>
#include <iostream>
#include <string>
#include <utility> 
#include <vector>    


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
   // These are the tokens a statement can start with. elif and else are
   // left out on purpose: they end the current suite and belong to ifStatement.
   while (next.isIdentifier() || next.isForKeyword() || next.isPrintKeyword() || next.isIfKeyword()
   || next.isWhileKeyword() || next.isAppendKeyword()) {
       tokenizer.ungetToken();

       parsedStatements->addStatement(statement());

       next = tokenizer.getToken();
   }

   tokenizer.ungetToken();
   return parsedStatements;
}


Statement *Parser::statement() {
   //phase 2: <statement> → <simple-statement> NEWLINE
   //                     | <compound-statement>
   Token token = tokenizer.getToken();
  
   // compound: for-statement
   if (token.isForKeyword()) {
       tokenizer.ungetToken();
       return forStatement();
   }
    // Phase 4: if / elif / else
    if (token.isIfKeyword()) {
        tokenizer.ungetToken();
        return ifStatement();
    }

    // Phase 4: while
    if (token.isWhileKeyword()) {
        tokenizer.ungetToken();
        return whileStatement();
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
   // Phase 4 simple statement: append-statement
   if (token.isAppendKeyword()) {
       tokenizer.ungetToken();

       Statement *stmt = appendStatement();
       Token newline = tokenizer.getToken();
         if (!newline.isNewline()) {
             delete stmt;
             die("Parser::statement", "expected NEWLINE after append statement", newline);
         }
       return stmt;
   }

   die("Parser::statement", "expected assignment, print, append, for, if, or while statement", token);
}


// Phase 4: <assignment-statement> -> <assignment-target> "=" <rel-expr>
//          <assignment-target>    -> <id> [ "[" <rel-expr> "]" ]
// Returns Statement* because it builds one of two node types.
Statement *Parser::assignmentStatement() {
   // The caller (statement()) consumes the NEWLINE after it.
   Token variable = tokenizer.getToken();
   if (!variable.isIdentifier())
       die("Parser::assignmentStatement", "expected an identifier", variable);

   // One token of lookahead decides which kind of assignment this is.
   Token next = tokenizer.getToken();

   // x[i] = value
   if (next.isOpenBracket()) {
       // Parse order matches the spec's evaluation order: target, index, RHS.
       ExprNode *index = relExpr();

       Token closeBracket = tokenizer.getToken();
       if (!closeBracket.isCloseBracket()) {
           delete index;
           die("Parser::assignmentStatement", "expected ']' after index", closeBracket);
       }

       Token assignmentOperator = tokenizer.getToken();
       if (!assignmentOperator.isAssignmentOperator()) {
           delete index;
           die("Parser::assignmentStatement", "expected '='", assignmentOperator);
       }

       ExprNode *value = relExpr();
       return new IndexAssignmentStatement(variable.identifier(), index, value);
   }

   // x = value   (same as Phase 3)
   if (!next.isAssignmentOperator())
       die("Parser::assignmentStatement", "expected '=' or '['", next);

   return new AssignmentStatement(variable.identifier(), relExpr());
}

// Phase 4: <append-statement> -> "append" "(" <rel-expr> "," <rel-expr> ")"
// append is a statement (it returns nothing); pop is an expression (it returns a value).
AppendStatement *Parser::appendStatement() {
   Token appendKeyword = tokenizer.getToken();
   if (!appendKeyword.isAppendKeyword())
       die("Parser::appendStatement", "expected 'append' keyword", appendKeyword);

   Token openParen = tokenizer.getToken();
   if (!openParen.isOpenParen())
       die("Parser::appendStatement", "expected '(' after append", openParen);

   ExprNode *target = relExpr();

   Token comma = tokenizer.getToken();
   if (!comma.isComma()) {
       delete target;
       die("Parser::appendStatement", "expected ',' between append arguments", comma);
   }

   ExprNode *value = relExpr();

   Token closeParen = tokenizer.getToken();
   if (!closeParen.isCloseParen()) {
       delete target;
       delete value;
       die("Parser::appendStatement", "expected ')' after append arguments", closeParen);
   }

   return new AppendStatement(target, value);
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
   if (!colon.isColon()) {
       delete rangeExpr;
       die("Parser::forStatement", "expected ':'", colon);
   }

   Statements *bodyStmts = suite();

   return new ForStatement(variable.identifier(), rangeExpr, bodyStmts);
}

// Phase 4: while statement
WhileStatement *Parser::whileStatement() {
    // <while-statement> -> "while" <rel-expr> ":" <suite>

    Token whileKeyword = tokenizer.getToken();

    if (!whileKeyword.isWhileKeyword()) {
        die("Parser::whileStatement",
            "expected 'while' keyword", whileKeyword);
    }

    ExprNode *condition = relExpr();

    Token colon = tokenizer.getToken();

    if (!colon.isColon()) {
        delete condition;
        die("Parser::whileStatement",
            "expected ':'", colon);
    }

    Statements *body = suite();

    return new WhileStatement(condition, body);
}

// Phase 4: if / elif / else statement
IfStatement *Parser::ifStatement() {
    // <if-statement> ->
    // "if" <rel-expr> ":" <suite>
    // { "elif" <rel-expr> ":" <suite> }
    // [ "else" ":" <suite> ]

    Token ifKeyword = tokenizer.getToken();

    if (!ifKeyword.isIfKeyword()) {
        die("Parser::ifStatement",
            "expected 'if' keyword", ifKeyword);
    }

    auto *ifStmt = new IfStatement();

    ExprNode *condition = relExpr();

    Token colon = tokenizer.getToken();

    if (!colon.isColon()) {
        delete condition;
        delete ifStmt;
        die("Parser::ifStatement", "expected ':'", colon);
    }

    Statements *body = suite();
    ifStmt->addBranch(condition, body);

    Token next = tokenizer.getToken();

    while (next.isElifKeyword()) {
        ExprNode *elifCondition = relExpr();

        Token elifColon = tokenizer.getToken();

        if (!elifColon.isColon()) {
            delete elifCondition;
            delete ifStmt;
            die("Parser::ifStatement",
                "expected ':' after elif condition", elifColon);
        }

        Statements *elifBody = suite();
        ifStmt->addBranch(elifCondition, elifBody);

        next = tokenizer.getToken();
    }

    if (next.isElseKeyword()) {
        Token elseColon = tokenizer.getToken();

        if (!elseColon.isColon()) {
            delete ifStmt;
            die("Parser::ifStatement",
                "expected ':' after else", elseColon);
        }

        Statements *elseBody = suite();
        ifStmt->setElseBranch(elseBody);
    } else {
        tokenizer.ungetToken();
    }

    return ifStmt;
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
    Statements *body = statements();

    Token dedent = tokenizer.getToken();
    if (!dedent.isDedent()) {
        delete body;
        die("Parser::suite", "expected DEDENT", dedent);
    }
    return body;
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
    if (!closeParen.isCloseParen()) {
        delete rangeExpression;
        die("Parser::range", "expected ')'", closeParen);
    }
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

// Phase 3: added isFloorDivision condition to while loop
ExprNode *Parser::arithTerm() {
   // <arith-term> -> <arith-primary> { <mult-op> <arith-primary> }
   ExprNode *left = arithPrimary();
   Token token = tokenizer.getToken();


   while (token.isMultiplicationOperator() ||
          token.isDivisionOperator() ||
          token.isModuloOperator() ||
          token.isFloorDivisionOperator()) {

       ExprNode *right = arithPrimary();
       left = new BinaryExprNode(token, left, right);
       token = tokenizer.getToken();
   }


   tokenizer.ungetToken();
   return left;
}


ExprNode *Parser::arithPrimary() {
   // Phase 4: <arith-primary> -> [ <sign> ] <postfix-expression>
   // The sign wraps the whole postfix expression, so -x[0] means -(x[0]).
   Token token = tokenizer.getToken();
   if (token.isAdditionOperator() || token.isSubtractionOperator())
       return new UnaryExprNode(token, postfixExpression());


   tokenizer.ungetToken();
   return postfixExpression();
}

// Phase 4: <postfix-expression> -> <atom> [ "[" <rel-expr> "]" ]
// "[ ... ]" in the grammar means optional, so this is an if, not a loop:
// x[0] is allowed, x[0][1] is not part of Phase 4.
ExprNode *Parser::postfixExpression() {
    ExprNode *target = arithAtom();

    Token openBracket = tokenizer.getToken();
    if (!openBracket.isOpenBracket()) {
        // No subscript, so give the token back for the caller to read.
        tokenizer.ungetToken();
        return target;
    }

    ExprNode *index = relExpr();

    Token closeBracket = tokenizer.getToken();
    if (!closeBracket.isCloseBracket()) {
        // die() exits, so free both nodes we own first.
        delete target;
        delete index;
        die("Parser::postfixExpression", "expected ']' after subscript", closeBracket);
    }

    // The new node takes ownership of target and index.
    return new SubscriptExpr(openBracket, target, index);
}

// Phase 4: <one-argument-call> -> <one-argument-name> "(" <rel-expr> ")"
// The caller has already consumed the name token (int, float, str, len, split, pop).
// Parsing is the same for all six; only the node we build differs.
ExprNode *Parser::oneArgumentCall(const Token &name) {
    Token openParen = tokenizer.getToken();
    if (!openParen.isOpenParen())
        die("Parser::oneArgumentCall", "expected '(' after function name", openParen);

    ExprNode *argument = relExpr();

    Token closeParen = tokenizer.getToken();
    if (!closeParen.isCloseParen()) {
        delete argument;
        die("Parser::oneArgumentCall", "expected ')' after argument", closeParen);
    }

    if (name.isStrKeyword())   return new StrConversion(name, argument);
    if (name.isIntKeyword())   return new IntConversion(name, argument);
    if (name.isFloatKeyword()) return new FloatConversion(name, argument);
    if (name.isLenKeyword())   return new LenExpr(name, argument);
    if (name.isSplitKeyword()) return new SplitExpr(name, argument);
    if (name.isPopKeyword())   return new PopExpr(name, argument);

    // Only reachable if a caller passes a name that isn't one of the six.
    delete argument;
    die("Parser::oneArgumentCall", "unknown one-argument function", name);
}

// Phase 3: added support for double, bool and string literal types and str conversion
// Phase 4: <atom> -> <id> | <literal> | <list-literal> | <input-expression>
//                  | <one-argument-call> | "(" <rel-expr> ")"
ExprNode *Parser::arithAtom() {
   Token token = tokenizer.getToken();


   if (token.isInteger())
       return new IntegerLiteral(token);
   if (token.isDouble())
       return new DoubleLiteral(token);
   if (token.isTrueKeyword() || token.isFalseKeyword())
       return new BooleanLiteral(token);
   if (token.isString())
       return new StringLiteral(token);
   if (token.isIdentifier())
       return new Variable(token);
   // Phase 4: str was the only call in Phase 3. All six calls now share one helper.
   if (token.isStrKeyword() || token.isIntKeyword() || token.isFloatKeyword() ||
       token.isLenKeyword() || token.isSplitKeyword() || token.isPopKeyword())
       return oneArgumentCall(token);

   // Phase 4: <input-expression> -> "input" "(" ")"   (no arguments)
   if (token.isInputKeyword()) {
       Token openParen = tokenizer.getToken();
       if (!openParen.isOpenParen())
           die("Parser::arithAtom", "expected '(' after input", openParen);
       Token closeParen = tokenizer.getToken();
       if (!closeParen.isCloseParen())
           die("Parser::arithAtom", "expected ')' after input(", closeParen);
       return new InputExpr(token, nullptr);
   }
   // Phase 4: <list-literal> -> "[" [ <rel-expr> { "," <rel-expr> } ] "]"
   if (token.isOpenBracket()) {
         std::vector<ExprNode*> elements;
    
       Token next = tokenizer.getToken();
       if (!next.isCloseBracket()) {
           tokenizer.ungetToken();
           while (true) {
            elements.push_back(relExpr());
            next = tokenizer.getToken();
            if (next.isCloseBracket()) {
                break;
            } else if (!next.isComma()) {
                for (ExprNode *element : elements) {
                    delete element;
                }
                die("Parser::arithAtom", "expected ',' or ']' in list literal", next);
            }
        }
       }
       // ListLiteral takes ownership of every element node.
       return new ListLiteral(token, std::move(elements));
   }

   // "(" <rel-expr> ")": the parentheses only group, so return the inner node.
   if (token.isOpenParen()) {
       ExprNode *expression = relExpr();
       Token closeParen = tokenizer.getToken();
       if (!closeParen.isCloseParen()) {
           delete expression;
           die("Parser::arithAtom", "expected ')'", closeParen);
       }
       return expression;
   }

   die("Parser::arithAtom",
       "expected an identifier, literal, list, input(), function call, or '('", token);
}

